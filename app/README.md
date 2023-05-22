## An example of a user space app

This script is an example of how we can poll mpu6050 sensor data using the sysfs interface.
It reads the raw accelerometer and gyroscope data for a while and prints the average values that can be used for calibration.
Also, this script shows an example of mode switching using the sysfs attribute **mode**
