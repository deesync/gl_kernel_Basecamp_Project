obj-m += sensor/sensor_module.o
obj-m += display/display_module.o
obj-m += logic_module.o

KDIR ?= /home/user/pi/linux

export ARCH = arm
export CROSS_COMPILE = arm-linux-gnueabihf-
 
all:
	$(MAKE) -C $(KDIR) M=$(shell pwd) modules

clean:
	$(MAKE) -C $(KDIR) M=$(shell pwd) clean
