#/bin/bash

# Config
poll_count=100

sysfs_dir="/sys/class/bc_project/inclinometer/attr"
scan_mode=5


# Error codes
E_SYSFS_NO_DIR=1
E_SYSFS_NO_ATTR=2
E_SYSFS_NO_PERM=3

# Init checks
if [ ! -d "${sysfs_dir}" ]; then
	echo "Cannot find sysfs directory: ${sysfs_dir}"
	exit ${E_SYSFS_NO_DIR}
fi

if [ ! -f "${sysfs_dir}/mode" ]; then
	echo "Cannot find sysfs attribute: ${sysfs_dir}/mode"
	exit ${E_SYSFS_NO_ATTR}
fi

if [ ! -w "${sysfs_dir}/mode" ]; then
	echo "You must have permission to write in sysfs"
	exit ${E_SYSFS_NO_PERM}
fi


scan() {
	mode=$(cat ${sysfs_dir}/mode)
	echo $scan_mode > "${sysfs_dir}/mode"

	echo
	echo "Scanning..."

	for i in $(seq 1 $poll_count); do

		ax=$((ax + $(cat ${sysfs_dir}/accel_x)))
		ay=$((ay + $(cat ${sysfs_dir}/accel_y)))
		az=$((az + $(cat ${sysfs_dir}/accel_z)))

		gx=$((gx + $(cat ${sysfs_dir}/gyro_x)))
		gy=$((gy + $(cat ${sysfs_dir}/gyro_y)))
		gz=$((gz + $(cat ${sysfs_dir}/gyro_z)))

	done

	echo ${mode} > "${sysfs_dir}/mode"

	echo
	echo "The resulting average values are:"

	printf "Accel X: %6d   Gyro X: %6d\n" \
		$((ax / poll_count)) $((gx / poll_count))
	printf "Accel Y: %6d   Gyro Y: %6d\n" \
		$((ay / poll_count)) $((gy / poll_count))
	printf "Accel Z: %6d   Gyro Z: %6d\n" \
		$((az / poll_count)) $((gz / poll_count))

	unset -v ax ay az gx gy gz
}

echo
echo "Accelerometer/Gyroscope data gathering tool"

while true; do

	echo
	echo "Set your device still in proper orientation..."
	read -p "Type (S) to scan or (X) to exit: " ans
	
	case $ans in
	[Xx])
		break
		;;
	*)
		scan
		;;
	esac

done
