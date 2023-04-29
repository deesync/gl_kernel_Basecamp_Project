/* SPDX-License-Identifier: GPL */

#ifndef __SENSOR_MODULE_H__
#define __SENSOR_MODULE_H__

#include "mpu6050.h"

struct sensor_data {
	s16 accel_x;
	s16 accel_y;
	s16 accel_z;
	s16 gyro_x;
	s16 gyro_y;
	s16 gyro_z;
};

enum sensor_value {
	accel_x	= REG_ACCEL_XOUT_H,
	accel_y	= REG_ACCEL_YOUT_H,
	accel_z	= REG_ACCEL_ZOUT_H,
	gyro_x	= REG_GYRO_XOUT_H,
	gyro_y	= REG_GYRO_YOUT_H,
	gyro_z	= REG_GYRO_ZOUT_H,
};

extern int bc_poll_sensor_raw_data(struct sensor_data *data);
extern int bc_poll_sensor_raw_value(s16 *value, enum sensor_value type);
extern int bc_poll_sensor_temperature(s16 *temperature);

#endif /* __SENSOR_MODULE_H__ */
