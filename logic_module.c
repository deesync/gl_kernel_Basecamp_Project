// SPDX-License-Identifier: GPL

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>

#include "sensor/sensor_module.h"	/* Sensor Module */
#include "display/display_module.h"	/* Display Module*/

#define MODULE_NAME "Logic"
#define MP MODULE_NAME ": "		/* Log Message Prefix */

#define INIT_DELAY 1000

static struct delayed_work work_update;

static unsigned int delay = 20;

static void refresh(struct work_struct *work)
{
	char s[200];
	struct sensor_data s_data;
	s16 temp;

	bc_poll_sensor_raw_data(&s_data);
	bc_poll_sensor_temperature(&temp);

	sprintf(s, "BUF aX=%6d, aY=%6d, aZ=%6d, gX=%6d, gY=%6d, gZ=%6d, t=%d\n",
		s_data.accel_x, s_data.accel_y, s_data.accel_z,
		s_data.gyro_x, s_data.gyro_y, s_data.gyro_z, temp);

	bc_display_data(s);

	schedule_delayed_work(&work_update, msecs_to_jiffies(delay));
}

static int __init logic_mod_init(void)
{
	pr_info(MP "module initialization...\n");
	pr_info(MP "receiving data and sending it to display...\n");

	INIT_DELAYED_WORK(&work_update, refresh);
	schedule_delayed_work(&work_update, msecs_to_jiffies(INIT_DELAY));

	return 0;
}

static void __exit logic_mod_exit(void)
{
	cancel_delayed_work_sync(&work_update);

	pr_info(MP "module exited\n");
}

module_init(logic_mod_init);
module_exit(logic_mod_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Linux Bootcamp Project Business Logic Module");
MODULE_AUTHOR("Vlad Dee <deesyync@gmail.com>");
