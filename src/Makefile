obj-m += sensor/sensor_module.o
obj-m += display/display_module.o
obj-m += inclinometer.o

inclinometer-objs := logic.o logic_tools.o fxpt_atan2.o

KDIR ?= /home/user/pi/linux
INST_MOD_PATH = /home/user/pi/lib_modules

export ARCH = arm
export CROSS_COMPILE = arm-linux-gnueabihf-

all:
	$(MAKE) -C $(KDIR) M=$(shell pwd) modules

install:
	$(MAKE) -C $(KDIR) M=$(shell pwd) INSTALL_MOD_PATH=$(INST_MOD_PATH) modules_install

clean:
	$(MAKE) -C $(KDIR) M=$(shell pwd) clean
