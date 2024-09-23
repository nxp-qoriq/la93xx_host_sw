#SPDX-License-Identifier: GPL-2.0
#Copyright 2017, 2021-2024 NXP

PWD=$(shell pwd)

ifndef KERNEL_DIR
$(error KERNEL_DIR is not set)
endif

ifndef LA9310_COMMON_HEADERS
$(error LA9310_COMMON_HEADERS is not set)
endif

include $(KERNEL_DIR)/.config

#Add any CFLAGS that needs to be passed to driver
EXTRA_CFLAGS += -g -DLA9310_HOST_SW_VERSION="${VERSION_STRING}" -DVERSION=${GIT_VERSION} -I${COMMON_DIR} -I${UAPI_DIR}
obj-y += la9310shiva/
obj-y += la9310demo/
ifeq ($(IMX_RFNM),1)
obj-y += la9310rfnm/
endif
ifeq ($(IMX_RFGR),1)
obj-y += la9310rfgr/
endif
ifeq ($(IMX_RFMT3812),1)
obj-y += la9310mt3812/
endif

all:
	$(MAKE) ARCH=$(ARCH) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKE) ARCH=$(ARCH) -C $(KERNEL_DIR) M=$(PWD) clean
	rm -rf ${MODULE_INSTALL_DIR}/*.ko

install:
	install  -D  la9310shiva/la9310shiva.ko ${MODULE_INSTALL_DIR}/la9310shiva.ko
	install  -D  la9310demo/la9310demo.ko ${MODULE_INSTALL_DIR}/la9310demo.ko
ifeq ($(IMX_RFNM),1)
	install  -D  la9310rfnm/la9310rfnm.ko ${MODULE_INSTALL_DIR}/la9310rfnm.ko
	install  -D  la9310rfnm/rfnm_usb_boost.ko ${MODULE_INSTALL_DIR}/rfnm_usb_boost.ko
	install  -D  la9310rfnm/rfnm_gpio.ko ${MODULE_INSTALL_DIR}/rfnm_gpio.ko
	install  -D  la9310rfnm/la9310rfnm.ko ${MODULE_INSTALL_DIR}/la9310rfnm.ko
	install  -D  la9310rfnm/rfnm_usb_function.ko ${MODULE_INSTALL_DIR}/rfnm_usb_function.ko
	install  -D  la9310rfnm/rfnm_daughterboard.ko ${MODULE_INSTALL_DIR}/rfnm_daughterboard.ko
	install  -D  la9310rfnm/rfnm_breakout.ko ${MODULE_INSTALL_DIR}/rfnm_breakout.ko
	install  -D  la9310rfnm/rfnm_usb.ko ${MODULE_INSTALL_DIR}/rfnm_usb.ko
	install  -D  la9310rfnm/rfnm_lime.ko ${MODULE_INSTALL_DIR}/rfnm_lime.ko
	install  -D  la9310rfnm/rfnm_tti.ko ${MODULE_INSTALL_DIR}/rfnm_tti.ko
	install  -D  la9310rfnm/rfnm_lalib.ko ${MODULE_INSTALL_DIR}/rfnm_lalib.ko
endif

ifeq ($(IMX_RFGR),1)
	install  -D la9310rfgr/rfnm_granita.ko ${MODULE_INSTALL_DIR}/rfnm_granita.ko
endif
ifeq ($(IMX_RFMT3812),1)
	install  -D la9310mt3812/rfnm_mt3812.ko ${MODULE_INSTALL_DIR}/rfnm_mt3812.ko
endif
