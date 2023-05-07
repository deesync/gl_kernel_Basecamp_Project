// SPDX-License-Identifier: GPL

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "display_module.h"

#define MP KBUILD_MODNAME ": "		/* Log Message Prefix */

#define I2C_BUS 1			/* I2C bus number */
#define I2C_DEVICE_NAME "bc-ssd1306"	/* Our device driver name */


struct i2c_client *ssd1306_client;

static inline int ssd1306_i2c_cmd(unsigned char cmd)
{
	return i2c_master_send(ssd1306_client, (u8[]){0, cmd}, 2);
}

static inline int ssd1306_i2c_data(unsigned char data)
{
	return i2c_master_send(ssd1306_client, (u8[]){0x40, data}, 2);
}

static int display_clear(void)
{
	int ret;
	u8 *buf;

	buf = kzalloc(SSD1306_SEGMENTS * SSD1306_PAGES + 1, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	buf[0] = 0x40;

	ssd1306_i2c_cmd(SSD1306_COLUMNADDR);
	ssd1306_i2c_cmd(0);
	ssd1306_i2c_cmd(SSD1306_SEGMENTS - 1);

	ssd1306_i2c_cmd(SSD1306_PAGEADDR);
	ssd1306_i2c_cmd(0);
	ssd1306_i2c_cmd(SSD1306_PAGES - 1);

	ret = i2c_master_send(ssd1306_client, buf,
			      SSD1306_SEGMENTS * SSD1306_PAGES + 1);

	kfree(buf);

	return ret;
}

/**
 * display_clear - clears display (filling all GDDRAM with zeroes)
 *
 * Returning negative errno
 * else zero on success.
 */
int bc_display_clear(void)
{
	return display_clear();
}
EXPORT_SYMBOL(bc_display_clear);

/**
 * bc_display_print - prints the text with selected font
 * @offset: Left indent in sectors. One sector is 1 px
 * @line: Top indent in pages. One page height is 8 px
 * @font: Pointer to font data
 * @str: String to print
 *
 * Returning negative errno
 * else zero on success.d.
 */
int bc_display_print(u8 offset, u8 line,
		     const struct display_font_t *font, char *str)
{
	int i, x, sym, ffsym, flsym, maplen;

	static u8 buf[SYM_BUF_SIZE] = {[0] = 0x40};

	if (!font || !str)
		return -EFAULT;

	ffsym = font->first_symbol;
	flsym = font->first_symbol + font->symbols_count;

	if (font->cheight == 1) {
		// Set page start and end address
		ssd1306_i2c_cmd(SSD1306_PAGEADDR);
		ssd1306_i2c_cmd(line);
		ssd1306_i2c_cmd(line);

		// Set column start and end address
		ssd1306_i2c_cmd(SSD1306_COLUMNADDR);
		ssd1306_i2c_cmd(offset);
		ssd1306_i2c_cmd(offset + strlen(str) * font->space);

		maplen = font->width;

		for (i = 0; i < str[i] && i < MAX_STR_LEN; i++) {
			if ((str[i] > ffsym) && (str[i] < flsym))
				sym = str[i] - ffsym;
			else
				sym = 0;

			memcpy(&buf[1], &font->map[sym*maplen], maplen);
			buf[maplen+1] = 0x00; /* space */
			i2c_master_send(ssd1306_client, buf, maplen+2);
		}

	} else {
		// Set page start and end address
		ssd1306_i2c_cmd(SSD1306_PAGEADDR);
		ssd1306_i2c_cmd(line);
		ssd1306_i2c_cmd(line + font->cheight - 1);

		maplen = font->cheight * font->width;

		for (i = 0; str[i] && i < MAX_STR_LEN; i++) {
			x = offset + (i * font->space);

			// Set column start and end address
			ssd1306_i2c_cmd(SSD1306_COLUMNADDR);
			ssd1306_i2c_cmd(x);
			ssd1306_i2c_cmd(x + font->width - 1);

			if ((str[i] > ffsym) && (str[i] < flsym))
				sym = str[i] - ffsym;
			else
				sym = 0;

			memcpy(&buf[1], &font->map[sym*maplen], maplen);
			i2c_master_send(ssd1306_client, buf, maplen+1);
		}
	}

	return 0;
}
EXPORT_SYMBOL(bc_display_print);


static int ssd1306_probe(struct i2c_client *drv_client,
			 const struct i2c_device_id *id)
{
	msleep(100);

	pr_info(MP "probing...\n");

	dev_info(&drv_client->dev,
		"i2c client address is 0x%X\n", drv_client->addr);

	/* Setup the device */

	/* Display OFF */
	ssd1306_i2c_cmd(SSD1306_DISPLAYOFF);

	/* Set Display Clock Divide Ratio and Oscillator Frequency */
	ssd1306_i2c_cmd(SSD1306_SETDISPLAYCLOCKDIV);
	/* Default Setting for Display Clock Divide Ratio and Oscillator Frequency that is recommended */
	ssd1306_i2c_cmd(0x80);

	/* Set Multiplex Ratio */
	ssd1306_i2c_cmd(SSD1306_SETMULTIPLEX);
	/* 64 COM lines */
	ssd1306_i2c_cmd(0x3F);

	/* Set display offset */
	ssd1306_i2c_cmd(SSD1306_SETDISPLAYOFFSET);
	/* 0 offset */
	ssd1306_i2c_cmd(0x00);

	/* Set first line as the start line of the display */
	ssd1306_i2c_cmd(SSD1306_SETSTARTLINE);

	/* Charge pump */
	ssd1306_i2c_cmd(SSD1306_CHARGEPUMP);
	/* Enable charge dump during display on */
	ssd1306_i2c_cmd(0x14);

	/* Set memory addressing mode */
	ssd1306_i2c_cmd(SSD1306_MEMORYMODE);
	/* Horizontal addressing mode */
	ssd1306_i2c_cmd(0x00);

	/* Set segment remap with column address 127 mapped to segment 0 */
	ssd1306_i2c_cmd(SSD1306_SEGREMAP);

	/* Set com output scan direction, scan from com63 to com 0 */
	ssd1306_i2c_cmd(SSD1306_COMSCANDEC);

	/* Set com pins hardware configuration */
	ssd1306_i2c_cmd(SSD1306_SETCOMPINS);
	/* Alternative com pin configuration, disable com left/right remap */
	ssd1306_i2c_cmd(0x12);

	/* Set contrast control */
	ssd1306_i2c_cmd(SSD1306_SETCONTRAST);
	/* Set Contrast to 128 */
	ssd1306_i2c_cmd(0x80);

	/* Set pre-charge period */
	ssd1306_i2c_cmd(SSD1306_SETPRECHARGE);
	/* Phase 1 period of 15 DCLK, Phase 2 period of 1 DCLK */
	ssd1306_i2c_cmd(0xF1);

	/* Set Vcomh deselect level */
	ssd1306_i2c_cmd(SSD1306_SETVCOMDETECT);
	/* Vcomh deselect level ~ 0.77 Vcc */
	ssd1306_i2c_cmd(0x20);

	/* Entire display ON, resume to RAM content display */
	ssd1306_i2c_cmd(SSD1306_DISPLAYALLON_RESUME);

	/* Set Display in Normal Mode */
	ssd1306_i2c_cmd(SSD1306_NORMALDISPLAY);

	/* Deactivate scroll */
	ssd1306_i2c_cmd(SSD1306_DEACTIVATE_SCROLL);

	/* Clear display */
	//ssd1306_fill(0x00);
	display_clear();

	/* Display ON in normal mode */
	ssd1306_i2c_cmd(SSD1306_DISPLAYON);

	ssd1306_client = drv_client;
	dev_info(&drv_client->dev, "i2c driver probed\n");

	return 0;
}

static int ssd1306_remove(struct i2c_client *drv_client)
{
	display_clear();
	ssd1306_i2c_cmd(SSD1306_DISPLAYOFF);

	ssd1306_client = NULL;

	dev_info(&drv_client->dev, "i2c driver removed\n");
	return 0;
}

static const struct i2c_device_id ssd1306_id[] = {
	{ I2C_DEVICE_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ssd1306_id);

static struct i2c_driver ssd1306_i2c_driver = {
	.driver = {
		.name = I2C_DEVICE_NAME,
		.owner = THIS_MODULE,
	},
	.probe = ssd1306_probe,
	.remove = ssd1306_remove,
	.id_table = ssd1306_id,
};

static struct i2c_board_info ssd1306_i2c_device_info = {
	I2C_BOARD_INFO(I2C_DEVICE_NAME, SSD1306_I2C_ADDR)
};

static int __init display_mod_init(void)
{
	int ret;
	struct i2c_adapter *adapter = i2c_get_adapter(I2C_BUS);

	pr_info(MP "initialization...\n");

	pr_info(MP "adapter = 0x%p\n", adapter);

	if (!adapter) {
		pr_err(MP "failed to get I2C adapter\n");
		return -ENODEV;
	}

	/* Create i2c client */
	ssd1306_client = i2c_new_client_device(adapter,
					       &ssd1306_i2c_device_info);

	pr_info(MP "client = 0x%p\n", ssd1306_client);

	if (!ssd1306_client) {
		pr_err(MP "failed to create I2C client\n");
		i2c_put_adapter(adapter);
		return -ENODEV;
	}

	/* Create i2c driver */
	ret = i2c_add_driver(&ssd1306_i2c_driver);
	if (ret != 0) {
		pr_err(MP "failed to add new i2c driver: %d\n", ret);
		return ret;
	}

	i2c_put_adapter(adapter);

	pr_info(MP "i2c driver created\n");

	return 0;
}

static void __exit display_mod_exit(void)
{
	i2c_unregister_device(ssd1306_client);
	i2c_del_driver(&ssd1306_i2c_driver);
	pr_info(MP "module removed\n");
}

module_init(display_mod_init);
module_exit(display_mod_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Linux Kernel Bootcamp Project: Display Module");
MODULE_AUTHOR("Vlad Degtyarov <deesyync@gmail.com>");
MODULE_VERSION("0.1");
