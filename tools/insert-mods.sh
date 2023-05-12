#!/bin/bash

# A_BUTTON_PIN=26

# ACCEL_CALIBRATION="-850,920,900"
# GYRO_CALIBRATION="567,1,183"

SENSOR_MOD=sensor_module
DISPLAY_MOD=display_module
LOGIC_MOD=inclinometer


# Inserting modules
# Keeping the right order

# Removing Logic module if loaded
if lsmod | grep -wq "$LOGIC_MOD"; then
	sudo rmmod $LOGIC_MOD
fi

# Sensor Module
if lsmod | grep -wq "$SENSOR_MOD"; then
	sudo rmmod $SENSOR_MOD
fi
sudo insmod ${SENSOR_MOD}.ko

# Display Module
if lsmod | grep -wq "$DISPLAY_MOD"; then
	sudo rmmod $DISPLAY_MOD
fi
sudo insmod ${DISPLAY_MOD}.ko

# Business Logic Module
(
[ -n "${A_BUTTON_PIN}" ] && A_BUTTON_PIN_PARAM="a_button_pin=${A_BUTTON_PIN}"
[ -n "${ACCEL_CALIBRATION}" ] && ACCEL_CALIB_PARAM="accel_calib=${ACCEL_CALIBRATION}"
[ -n "${GYRO_CALIBRATION}" ] && GYRO_CALIB_PARAM="gyro_calib=${GYRO_CALIBRATION}"
set -x
sudo insmod ${LOGIC_MOD}.ko \
	${A_BUTTON_PIN_PARAM} \
	${ACCEL_CALIB_PARAM} \
	${GYRO_CALIB_PARAM}
)

lsmod | head -n 4
echo
dmesg --color=always | tail -n 24
