// SPDX-License-Identifier: GPL

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

#include "sensor/sensor_module.h"
#include "display/display_module.h"
#include "logic.h"
#include "fxpt_math.h"

#define MP KBUILD_MODNAME ": "		/* Log Message Prefix */

/* Module parameters */
static int a_button_pin = DEFAULT_A_BUTTON_GPIO_PIN;
static int accel_calib[3] = { 0, 0, 0 };
static int gyro_calib[3] = { 0, 0, 0 };

module_param(a_button_pin, int, 0);
MODULE_PARM_DESC(a_button_pin, "Action button GPIO pin");

module_param_array(accel_calib, int, NULL, 0);
MODULE_PARM_DESC(accel_calib, "Accelerometer calibration offsets");

module_param_array(gyro_calib, int, NULL, 0);
MODULE_PARM_DESC(gyro_calib, "Gyroscope calibration offsets");

/* Work loop */
static struct delayed_work work_loop;


#pragma region /* State & Modes */
static int display_raw_prepare(struct logic_mode *mode);
static int display_raw(struct logic_mode *mode);
static int display_calib_prepare(struct logic_mode *mode);
static int display_calib(struct logic_mode *mode);
static int display_inclinometer_prepare(struct logic_mode *mode);
static int display_inclinometer(struct logic_mode *mode);

/* Modes init */
static struct logic_mode modes[] = {
	/* [0] -  Display Clinometer */
	{
		.cycle_delay = 20,
		.prepare = display_inclinometer_prepare,
		.cycle = display_inclinometer,
	},
	/* [1] -  Display Raw Sensor Data */
	{
		.cycle_delay = 50,
		.prepare = display_raw_prepare,
		.cycle = display_raw,
	},
	/* [2] -  Display Calibrated Sensor Data */
	{
		.cycle_delay = 50,
		.prepare = display_calib_prepare,
		.cycle = display_calib,
	},
};

/* State init */
static struct logic_state state = {
	.mode_count = (sizeof(modes) / sizeof(modes[0])),
	.current_mode = 0,
	.hidden_modes = 0,
};
#pragma endregion


#pragma region /* Display Raw Data Mode calls */

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
#pragma endregion

#pragma region /* Display Calibrated Data Mode calls */

static int display_calib_prepare(struct logic_mode *mode)
{
	bc_display_clear();

	bc_display_print(11, 0, &fixed_font16, "Calibrated Data");

	bc_display_print(SM_TXT_OFFSET, 2, &fixed_font8, "Accel X :");
	bc_display_print(SM_TXT_OFFSET, 3, &fixed_font8, "Accel Y :");
	bc_display_print(SM_TXT_OFFSET, 4, &fixed_font8, "Accel Z :");
	bc_display_print(SM_TXT_OFFSET, 5, &fixed_font8, "Gyro X  :");
	bc_display_print(SM_TXT_OFFSET, 6, &fixed_font8, "Gyro Y  :");
	bc_display_print(SM_TXT_OFFSET, 7, &fixed_font8, "Gyro Z  :");

	return 0;
}

static int display_calib(struct logic_mode *mode)
{
	struct sensor_data raw_data;
	static char s[8];

	bc_poll_sensor_raw_data(&raw_data);

	sprintf(s, "%6d", raw_data.accel_x + accel_calib[0]);
	bc_display_print(SM_TXT_OFFSET + 60, 2, &fixed_font8, s);

	sprintf(s, "%6d", raw_data.accel_y + accel_calib[1]);
	bc_display_print(SM_TXT_OFFSET + 60, 3, &fixed_font8, s);

	sprintf(s, "%6d", raw_data.accel_z + accel_calib[2]);
	bc_display_print(SM_TXT_OFFSET + 60, 4, &fixed_font8, s);

	sprintf(s, "%6d", raw_data.gyro_x + gyro_calib[0]);
	bc_display_print(SM_TXT_OFFSET + 60, 5, &fixed_font8, s);

	sprintf(s, "%6d", raw_data.gyro_y + gyro_calib[1]);
	bc_display_print(SM_TXT_OFFSET + 60, 6, &fixed_font8, s);

	sprintf(s, "%6d", raw_data.gyro_z + gyro_calib[2]);
	bc_display_print(SM_TXT_OFFSET + 60, 7, &fixed_font8, s);

	return 0;
}
#pragma endregion

#pragma region /* Display Inclinometer Mode calls */
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
	struct sensor_data clb;
	static char s[5];

	bc_poll_sensor_raw_data(&raw_data);

	clb.accel_x = raw_data.accel_x + accel_calib[0];
	clb.accel_y = raw_data.accel_y + accel_calib[1];
	clb.accel_z = raw_data.accel_z + accel_calib[2];

	// angle += s_data.gyro_y/131*jiffies_to_msecs(jiffies-pin);
	// pin = jiffies;

	// stm32_atan2(s_data.accel_y, s_data.accel_x) / (CORDIC_PI / 180);

	sprintf(s, "%4d", fxpt_atan2(clb.accel_y, clb.accel_x)*180 / FXPT_PI);
	bc_display_print(55, 2, &lcd_font24, s);

	sprintf(s, "%4d", fxpt_atan2(clb.accel_x, clb.accel_z)*180 / FXPT_PI);
	bc_display_print(55, 5, &lcd_font24, s);

	return 0;
}
#pragma endregion


#pragma region /* Sysfs interface */
static ssize_t
accel_x_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	s16 val;

	bc_poll_sensor_raw_value(&val, accel_x);
	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static ssize_t
accel_y_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	s16 val;

	bc_poll_sensor_raw_value(&val, accel_y);
	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static ssize_t
accel_z_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	s16 val;

	bc_poll_sensor_raw_value(&val, accel_z);
	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static ssize_t
gyro_x_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	s16 val;

	bc_poll_sensor_raw_value(&val, gyro_x);
	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static ssize_t
gyro_y_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	s16 val;

	bc_poll_sensor_raw_value(&val, gyro_y);
	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static ssize_t
gyro_z_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	s16 val;

	bc_poll_sensor_raw_value(&val, gyro_z);
	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static ssize_t
temp_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	s16 val;

	bc_poll_sensor_temperature(&val);
	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static ssize_t
mode_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", state.current_mode);
}

static ssize_t 
mode_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf,
	   size_t count)
{
	u8 mode;

	if (kstrtou8(buf, 0, &mode) < 0 || mode > state.mode_count-1)
		return -EPERM;

	state.current_mode = mode;
	switch_mode(&state, &modes[state.current_mode]);

	return count;
}

static struct kobj_attribute accel_x_attr =
	__ATTR(ACCEL_X_SYSFS_ATTR, 0444, accel_x_show, NULL);
static struct kobj_attribute accel_y_attr =
	__ATTR(ACCEL_Y_SYSFS_ATTR, 0444, accel_y_show, NULL);
static struct kobj_attribute accel_z_attr =
	__ATTR(ACCEL_Z_SYSFS_ATTR, 0444, accel_z_show, NULL);

static struct kobj_attribute gyro_x_attr =
	__ATTR(GYRO_X_SYSFS_ATTR, 0444, gyro_x_show, NULL);
static struct kobj_attribute gyro_y_attr =
	__ATTR(GYRO_Y_SYSFS_ATTR, 0444, gyro_y_show, NULL);
static struct kobj_attribute gyro_z_attr =
	__ATTR(GYRO_Z_SYSFS_ATTR, 0444, gyro_z_show, NULL);

static struct kobj_attribute temp_attr =
	__ATTR(TEMPERATURE_SYSFS_ATTR, 0444, temp_show, NULL);

static struct kobj_attribute mode_attr =
	__ATTR(MODE_SYSFS_ATTR, 0664, mode_show, mode_store);

static struct attribute *attrs[] = {
	&accel_x_attr.attr, &accel_y_attr.attr, &accel_z_attr.attr,
	&gyro_x_attr.attr, &gyro_y_attr.attr, &gyro_z_attr.attr,
	&temp_attr.attr,
	&mode_attr.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};
#pragma endregion


/* Action button interrupt handler */
static irqreturn_t a_button_isr(int irq, void *dev_id)
{
	static unsigned long timestamp;

	if (jiffies - timestamp > BUTTON_DEBOUNCE_COOLDOWN) {
		/* It's a light call. There's no need to schedule bottom half */
		/* All heavy work will be done on the next work loop */
		switch_mode(&state, &modes[next_mode(&state)]);
		timestamp = jiffies;
	}

	return IRQ_HANDLED;
}

/* Magic loop */
static void refresh(struct work_struct *work)
{
	int res;

	res = process_state(&state);
	if (res < 0) {
		pr_err(MP "uh oh! something is wrong with processing state!\n");
		return;
	}

	schedule_delayed_work(&work_loop,
			      msecs_to_jiffies(state.mode->cycle_delay));
}

static int __init logic_mod_init(void)
{
	int ret;

	pr_info(MP "initialization...\n");

	/* Creating sysfs kobj */
	state.kobj = kobject_create_and_add(SYSFS_NAME, NULL);
	if (!state.kobj) {
		pr_err(MP "cannot create kobject\n");
		return -ENOMEM;
	}

	/* Creating sysfs group */
	ret = sysfs_create_group(state.kobj, &attr_group);
	if (ret) {
		pr_err(MP "cannot create sysfs group\n");
		goto r_kobj;
	}
	pr_info(MP "sysfs interface created at /sys/%s\n", SYSFS_NAME);

	/* Checking validity of GPIO pin */
	if (!gpio_is_valid(a_button_pin)) {
		pr_err(MP "GPIO %d is not valid\n", a_button_pin);
		goto r_sysfs;
	}

	/* Request access to the GPIO pin */
	ret = gpio_request(a_button_pin, A_BUTTON_IRQ_LABEL);
	if (ret < 0) {
		pr_err(MP "failed to request GPIO pin %d: %d\n",
		       a_button_pin, ret);
		goto r_sysfs;
	}

	/* Set the GPIO pin as an input with a pull-up resistor */
	ret = gpio_direction_input(a_button_pin);
	if (ret < 0) {
		pr_err(MP "failed to set GPIO direction for pin %d: %d\n",
		       a_button_pin, ret);
		goto r_gpio;
	}

	/* Register the interrupt handler function */
	ret = request_irq(gpio_to_irq(a_button_pin), a_button_isr,
			  IRQF_TRIGGER_FALLING, A_BUTTON_IRQ_LABEL, NULL);
	if (ret < 0) {
		pr_err(MP "failed to register interrupt handler for pin %d: %d\n",
		       a_button_pin, ret);
		goto r_gpio;
	}
	pr_info(MP "action button interrupt handler registered on GPIO pin: %d\n",
		a_button_pin);

	/* Switching Mode */
	pr_info(MP "number of modes: %d\n", state.mode_count);
	ret = switch_mode(&state, &modes[state.current_mode]);
	if (ret < 0) {
		pr_err(MP "cannot switch mode\n");
		goto r_irq;		
	}

	/* Scheduling refresh loop */
	INIT_DELAYED_WORK(&work_loop, refresh);
	schedule_delayed_work(&work_loop, msecs_to_jiffies(INIT_DELAY));

	pr_info(MP "initialization successful\n");

	return 0;

r_irq:
	free_irq(gpio_to_irq(a_button_pin), NULL);
r_gpio:
	gpio_free(a_button_pin);
r_sysfs:
	sysfs_remove_group(state.kobj, &attr_group);
r_kobj:
	kobject_put(state.kobj);

	return ret;
}

static void __exit logic_mod_exit(void)
{
	cancel_delayed_work_sync(&work_loop);
	flush_scheduled_work();

	free_irq(gpio_to_irq(a_button_pin), NULL);
	gpio_free(a_button_pin);
	sysfs_remove_group(state.kobj, &attr_group);
	kobject_put(state.kobj);

	pr_info(MP "module removed\n");
}

module_init(logic_mod_init);
module_exit(logic_mod_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Linux Kernel Bootcamp Project: Business Logic Module");
MODULE_AUTHOR("Vlad Degtyarov <deesyync@gmail.com>");
MODULE_VERSION("0.2");

#pragma GCC diagnostic pop
