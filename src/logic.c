// SPDX-License-Identifier: GPL

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>

#include "sensor/sensor_module.h"
#include "display/display_module.h"
#include "logic.h"
#include "fxpt_math.h"

#define MP KBUILD_MODNAME ": "		/* Log Message Prefix */
#define INIT_DELAY 1000

static struct delayed_work work_update;
static struct logic_state state;


/* Modes definitions */
static int display_raw_prepare(struct logic_mode *mode);
static int display_raw(struct logic_mode *mode);
static int display_inclinometer_prepare(struct logic_mode *mode);
static int display_inclinometer(struct logic_mode *mode);

static struct logic_mode display_inclinometer_mode = {
	.cycle_delay = 20,
	.prepare = display_inclinometer_prepare,
	.cycle = display_inclinometer,
};

static struct logic_mode display_raw_mode = {
	.cycle_delay = 20,
	.prepare = display_raw_prepare,
	.cycle = display_raw,
};
/* End of Modes definitions */


/* Display Raw Data calls */
static int display_raw_prepare(struct logic_mode *mode)
{
	bc_display_clear();

	bc_display_print(11, 0, &fixed_font16, "Sensor Raw Data");

	bc_display_print(SM_TXT_OFFSET, 2, &fixed_font8, "Accel X :");
	bc_display_print(SM_TXT_OFFSET, 3, &fixed_font8, "Accel Y :");
	bc_display_print(SM_TXT_OFFSET, 4, &fixed_font8, "Accel Z :");
	bc_display_print(SM_TXT_OFFSET, 5, &fixed_font8, "Gyro X  :");
	bc_display_print(SM_TXT_OFFSET, 6, &fixed_font8, "Gyro Y  :");
	bc_display_print(SM_TXT_OFFSET, 7, &fixed_font8, "Gyro Z  :");

	return 0;
}

static int display_raw(struct logic_mode *mode)
{
	struct sensor_data raw_data;
	static char s[8];

	bc_poll_sensor_raw_data(&raw_data);

	// pr_info(", %6d, %6d, %6d, %6d, %6d, %6d\n",
	// 	raw_data.accel_x, raw_data.accel_y, raw_data.accel_z,
	// 	raw_data.gyro_x, raw_data.gyro_y, raw_data.gyro_z);

	sprintf(s, "%6d", raw_data.accel_x);
	bc_display_print(SM_TXT_OFFSET + 60, 2, &fixed_font8, s);

	sprintf(s, "%6d", raw_data.accel_y);
	bc_display_print(SM_TXT_OFFSET + 60, 3, &fixed_font8, s);

	sprintf(s, "%6d", raw_data.accel_z);
	bc_display_print(SM_TXT_OFFSET + 60, 4, &fixed_font8, s);

	sprintf(s, "%6d", raw_data.gyro_x);
	bc_display_print(SM_TXT_OFFSET + 60, 5, &fixed_font8, s);

	sprintf(s, "%6d", raw_data.gyro_y);
	bc_display_print(SM_TXT_OFFSET + 60, 6, &fixed_font8, s);

	sprintf(s, "%6d", raw_data.gyro_z);
	bc_display_print(SM_TXT_OFFSET + 60, 7, &fixed_font8, s);

	return 0;
}
/* End of Display Raw Data calls */


/* Display Inclinometer calls */
static int display_inclinometer_prepare(struct logic_mode *mode)
{
	bc_display_clear();

	bc_display_print(0, 0, &bolder_font16, "INCLINOMETER");
	bc_display_print(5, 2, &fixed_font16, "Tilt Y:");
	bc_display_print(5, 5, &fixed_font16, "Tilt X:");
	
	return 0;
}

static int display_inclinometer(struct logic_mode *mode)
{
	struct sensor_data raw_data;
	static char s[5];

	bc_poll_sensor_raw_data(&raw_data);

	// angle += s_data.gyro_y/131*jiffies_to_msecs(jiffies-pin);
	// pin = jiffies;

	// stm32_atan2(s_data.accel_y, s_data.accel_x) / (CORDIC_PI / 180);

	sprintf(s, "%4d",
		fxpt_atan2(raw_data.accel_y, raw_data.accel_x)*180 / FXPT_PI);
	bc_display_print(55, 2, &lcd_font24, s);

	sprintf(s, "%4d",
		fxpt_atan2(raw_data.accel_x, raw_data.accel_z)*180 / FXPT_PI);
	bc_display_print(55, 5, &lcd_font24, s);

	return 0;
}
/* End of Display Inclinometer calls */


/* Magic cycle */
static void refresh(struct work_struct *work)
{
	int res;

	res = process_state(&state);
	if (res < 0) {
		pr_err(MP "uh oh! something wrong with logic state!\n");
		return;
	}

	schedule_delayed_work(&work_update,
			      msecs_to_jiffies(state.mode->cycle_delay));
}


static int __init logic_mod_init(void)
{
	pr_info(MP "initialization...\n");

	switch_mode(&state, &display_inclinometer_mode);
	// switch_mode(&state, &display_raw_mode);

	INIT_DELAYED_WORK(&work_update, refresh);
	schedule_delayed_work(&work_update, msecs_to_jiffies(INIT_DELAY));

	return 0;
}

static void __exit logic_mod_exit(void)
{
	cancel_delayed_work_sync(&work_update);

	pr_info(MP "module removed\n");
}

module_init(logic_mod_init);
module_exit(logic_mod_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Linux Kernel Bootcamp Project: Business Logic Module");
MODULE_AUTHOR("Vlad Degtyarov <deesyync@gmail.com>");
MODULE_VERSION("0.2");
