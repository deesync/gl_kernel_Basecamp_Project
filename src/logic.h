/* SPDX-License-Identifier: GPL */

#ifndef __LOGIC_H__
#define __LOGIC_H__

#include <linux/types.h>

#define SYSFS_NAME		"inclinometer"
#define ACCEL_X_SYSFS_ATTR	accel_x
#define ACCEL_Y_SYSFS_ATTR	accel_y
#define ACCEL_Z_SYSFS_ATTR	accel_z
#define GYRO_X_SYSFS_ATTR	gyro_x
#define GYRO_Y_SYSFS_ATTR	gyro_y
#define GYRO_Z_SYSFS_ATTR	gyro_z
#define TEMPERATURE_SYSFS_ATTR	temp
#define MODE_SYSFS_ATTR		mode

#define SM_TXT_OFFSET 16

struct logic_mode {
	int cycle_delay;
	int (*prepare)(struct logic_mode *mode);
	int (*cycle)(struct logic_mode *mode);
};

struct logic_state {
	const size_t mode_count;
	int current_mode;
	bool switching;
	struct logic_mode *mode;
	struct kobject *kobj;
};

int switch_mode(struct logic_state *state, struct logic_mode *mode);
int process_state(struct logic_state *state);

#endif /*__LOGIC_H__ */
