#!/bin/bash

SENSOR_MOD=sensor_module
DISPLAY_MOD=display_module
LOGIC_MOD=inclinometer

# Removing modules
# Keeping right order

sudo rmmod ${LOGIC_MOD}

sudo rmmod ${DISPLAY_MOD}
sudo rmmod ${SENSOR_MOD}
