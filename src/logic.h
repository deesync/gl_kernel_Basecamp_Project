/* SPDX-License-Identifier: GPL */

#ifndef __LOGIC_H__
#define __LOGIC_H__

#include <linux/types.h>

#define INIT_DELAY			500

#define LOGIC_CLASS			"bc_project"
#define LOGIC_DEVICE			"inclinometer"
#define SYSFS_ENTRY			"attr"

#define ACCEL_X_SYSFS_ATTR		accel_x
#define ACCEL_Y_SYSFS_ATTR		accel_y
#define ACCEL_Z_SYSFS_ATTR		accel_z
#define GYRO_X_SYSFS_ATTR		gyro_x
#define GYRO_Y_SYSFS_ATTR		gyro_y
#define GYRO_Z_SYSFS_ATTR		gyro_z
#define TEMPERATURE_SYSFS_ATTR		temp
#define MODE_SYSFS_ATTR			mode

#define DEFAULT_A_BUTTON_GPIO_PIN	26
#define A_BUTTON_IRQ_LABEL		LOGIC_DEVICE ": action button"
#define BUTTON_DEBOUNCE_COOLDOWN	(HZ/2)

#define SM_TXT_OFFSET			16

struct logic_mode {
	int cycle_delay;
	int (*prepare)(struct logic_mode *mode);
	int (*cycle)(struct logic_mode *mode);
};

struct logic_state {
	const int mode_count;
	int current_mode;
	int hidden_modes;
	bool switching;
	struct logic_mode *mode;
	struct kobject *kobj;
};

int switch_mode(struct logic_state *state, struct logic_mode *mode);
int next_mode(struct logic_state *state);
int process_state(struct logic_state *state);

#endif /*__LOGIC_H__ */
