#Copyright 2017 NXP

PWD=$(shell pwd)

ifndef KERNEL_DIR
$(error KERNEL_DIR is not set)
endif

ifdef KERNEL_CFG_PATH
include $(KERNEL_CFG_PATH)/.config
else
include $(KERNEL_DIR)/.config
endif

#Add any CFLAGS that needs to be passed to driver
EXTRA_CFLAGS += -g -DLA9310_HOST_SW_VERSION="${VERSION_STRING}" -I${COMMON_DIR} -I${UAPI_DIR}

ifeq (${BOOTROM_USE_EDMA}, 1)
EXTRA_CFLAGS += -DBOOTROM_USE_EDMA=1
endif
ifdef DEBUG
EXTRA_CFLAGS += -g -DDEBUG
endif

#DRV_SRCS := $(wildcard *.c)
#DRV_OBJS := $(patsubst %.c, %.o, ${DRV_SRCS})

obj-y += la9310shiva/
obj-y += la9310demo/

#la9310shiva-objs := ${DRV_OBJS}

all:
	make ARCH=$(ARCH) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make ARCH=$(ARCH) -C $(KERNEL_DIR) M=$(PWD) clean
	rm -rf ${MODULE_INSTALL_DIR}/*.ko

install:
	install  -D  la9310shiva/la9310shiva.ko ${MODULE_INSTALL_DIR}/la9310shiva.ko
	install  -D  la9310demo/la9310demo.ko ${MODULE_INSTALL_DIR}/la9310demo.ko
