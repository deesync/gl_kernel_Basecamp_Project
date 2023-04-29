#!/bin/bash

SENSOR_MOD=sensor_module
DISPLAY_MOD=display_module
LOGIC_MOD=logic_module


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
sudo insmod ${LOGIC_MOD}.ko


lsmod | head -n 5
echo
dmesg --color=always | tail -n 20

