// SPDX-License-Identifier: GPL

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/math.h>

#include "sensor_module.h"

#define MP KBUILD_MODNAME ": "		/* Log Message Prefix */

#define I2C_BUS 1			/* I2C bus number */
#define I2C_DEVICE_NAME "bc-mpu6050"	/* Our device driver name */


struct i2c_client *mpu6050_client;

/**
 * bc_poll_sensor_raw_data() - poll sensor registers
 * @data: data structure pointer
 *
 * Polling sensor registers and filling data structure
 * with raw data of accelerometer and gyroscope.
 * Implementation for MPU-6050 I2C Device
 *
 * Return: 0 on successful poll, error code if otherwise.
 */
int bc_poll_sensor_raw_data(struct sensor_data *data)
{
	int ret;
	u8 buf[MPU6050_DATA_SIZE];

	if (mpu6050_client == NULL) {
		pr_err(MP "mpu6050 device not found!");
		return -ENODEV;
	}

	ret = i2c_smbus_read_i2c_block_data(mpu6050_client, MPU6050_DATA_ADDR,
					    MPU6050_DATA_SIZE, buf);
	if (ret < 0) {
		dev_err(&mpu6050_client->dev,
			"read i2c block data error: %d\n", ret);
		return ret;
	}

	data->accel_x = (s16)((buf[0] << 8) | buf[1]);
	data->accel_y = (s16)((buf[2] << 8) | buf[3]);
	data->accel_z = (s16)((buf[4] << 8) | buf[5]);
	data->gyro_x  = (s16)((buf[8] << 8) | buf[9]);
	data->gyro_y = (s16)((buf[10] << 8) | buf[11]);
	data->gyro_z = (s16)((buf[12] << 8) | buf[13]);

	return 0;
}
EXPORT_SYMBOL(bc_poll_sensor_raw_data);

/**
 * bc_poll_sensor_raw_value() - get sensor's register value
 * @value: pointer to 16 bit value (being written as result of poll)
 * @type: type of data (see enum sensor_value in header file)
 *
 * Getting sensor's register value.
 * Implementation for MPU-6050 I2C Device
 *
 * Return: 0 on successful poll, error code if otherwise.
 */
int bc_poll_sensor_raw_value(s16 *value, enum sensor_value type)
{
	if (mpu6050_client == NULL) {
		pr_err(MP "mpu6050 device not found!");
		return -ENODEV;
	}

	*value = (s16)i2c_smbus_read_word_swapped(mpu6050_client, type);

	return 0;
}
EXPORT_SYMBOL(bc_poll_sensor_raw_value);

/**
 * bc_poll_sensor_temperature() - get sensor's temperature
 * @temperature: pointer to 16 bit value of temperature
 *
 * Getting sensor's temperature in celsius degrees.
 * Implementation for MPU-6050 I2C Device
 *
 * Return: 0 on successful poll, error code if otherwise.
 */
int bc_poll_sensor_temperature(s16 *temperature)
{
	s16 temp;

	if (mpu6050_client == NULL) {
		pr_err(MP "mpu6050 device not found!");
		return -ENODEV;
	}

	temp = (s16)i2c_smbus_read_word_swapped(mpu6050_client, REG_TEMP_OUT_H);
	*temperature = DIV_ROUND_CLOSEST(temp + 12420, 340);

	return 0;
}
EXPORT_SYMBOL(bc_poll_sensor_temperature);


static int mpu6050_probe(struct i2c_client *drv_client,
			 const struct i2c_device_id *id)
{
	int ret;

	pr_info(MP "probing...\n");

	dev_info(&drv_client->dev,
		"i2c client address is 0x%X\n", drv_client->addr);

	/* Read who_am_i register */
	ret = i2c_smbus_read_byte_data(drv_client, REG_WHO_AM_I);
	if (IS_ERR_VALUE(ret)) {
		dev_err(&drv_client->dev,
			"i2c_smbus_read_byte_data() failed with error: %d\n",
			ret);
		return ret;
	}
	if (ret != MPU6050_I2C_ADDR) {
		dev_err(&drv_client->dev,
			"wrong i2c device found: expected 0x%X, found 0x%X\n",
			MPU6050_I2C_ADDR, ret);
		return -ENODEV;
	}
	dev_info(&drv_client->dev,
		"i2c mpu6050 device found, WHO_AM_I register value = 0x%X\n",
		ret);

	/* Setup the device */
	// i2c_smbus_write_byte_data(drv_client, REG_CONFIG, 0x05);
	i2c_smbus_write_byte_data(drv_client, REG_GYRO_CONFIG, 0);
	i2c_smbus_write_byte_data(drv_client, REG_ACCEL_CONFIG, 0);
	i2c_smbus_write_byte_data(drv_client, REG_PWR_MGMT_1, 0);

	mpu6050_client = drv_client;

	dev_info(&drv_client->dev, "i2c driver probed\n");

	return 0;
}

static int mpu6050_remove(struct i2c_client *drv_client)
{
	mpu6050_client = NULL;

	dev_info(&drv_client->dev, "i2c driver removed\n");
	return 0;
}

static const struct i2c_device_id mpu6050_id[] = {
	{ I2C_DEVICE_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, mpu6050_id);

static struct i2c_driver mpu6050_i2c_driver = {
	.driver = {
		.name = I2C_DEVICE_NAME,
		.owner = THIS_MODULE,
	},
	.probe = mpu6050_probe,
	.remove = mpu6050_remove,
	.id_table = mpu6050_id,
};

static struct i2c_board_info mpu6050_i2c_device_info = {
	I2C_BOARD_INFO(I2C_DEVICE_NAME, MPU6050_I2C_ADDR)
};

static int __init sensor_mod_init(void)
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
	mpu6050_client = i2c_new_client_device(adapter, &mpu6050_i2c_device_info);

	pr_info(MP "client = 0x%p\n", mpu6050_client);

	if (!mpu6050_client) {
		pr_err(MP "failed to create I2C client\n");
		i2c_put_adapter(adapter);
		return -ENODEV;
	}

	/* Create i2c driver */
	ret = i2c_add_driver(&mpu6050_i2c_driver);
	if (ret != 0) {
		pr_err(MP "failed to add new i2c driver: %d\n", ret);
		return ret;
	}

	i2c_put_adapter(adapter);

	pr_info(MP "i2c driver created\n");

	return 0;
}

static void __exit sensor_mod_exit(void)
{
	i2c_unregister_device(mpu6050_client);
	i2c_del_driver(&mpu6050_i2c_driver);
	pr_info(MP "module removed\n");
}

module_init(sensor_mod_init);
module_exit(sensor_mod_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Linux Kernel Bootcamp Project: Sensor Module");
MODULE_AUTHOR("Vlad Degtyarov <deesyync@gmail.com>");
MODULE_VERSION("0.1");
