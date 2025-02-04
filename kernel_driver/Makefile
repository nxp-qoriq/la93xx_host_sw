#SPDX-License-Identifier: GPL-2.0
#Copyright 2017, 2021-2025 NXP

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
ifeq ($(IMX_SDR),1)
obj-y += la9310sdr/
endif
ifeq ($(IMX_SEEVE),1)
obj-y += la9310sdr/
endif
ifeq ($(IMX_RFLIME),1)
obj-y += la9310rflime/
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
ifeq ($(IMX_SDR),1)
	install  -D  la9310sdr/la9310sdr.ko ${MODULE_INSTALL_DIR}/la9310sdr.ko
	install  -D  la9310sdr/sdr_usb_boost.ko ${MODULE_INSTALL_DIR}/sdr_usb_boost.ko
	install  -D  la9310sdr/sdr_gpio.ko ${MODULE_INSTALL_DIR}/sdr_gpio.ko
	install  -D  la9310sdr/la9310sdr.ko ${MODULE_INSTALL_DIR}/la9310sdr.ko
	install  -D  la9310sdr/sdr_usb_function.ko ${MODULE_INSTALL_DIR}/sdr_usb_function.ko
	install  -D  la9310sdr/sdr_daughterboard.ko ${MODULE_INSTALL_DIR}/sdr_daughterboard.ko
	install  -D  la9310sdr/sdr_breakout.ko ${MODULE_INSTALL_DIR}/sdr_breakout.ko
	install  -D  la9310sdr/sdr_usb.ko ${MODULE_INSTALL_DIR}/sdr_usb.ko
	install  -D  la9310sdr/sdr_tti.ko ${MODULE_INSTALL_DIR}/sdr_tti.ko
	install  -D  la9310sdr/sdr_lalib.ko ${MODULE_INSTALL_DIR}/sdr_lalib.ko
else ifeq ($(IMX_SEEVE),1)
	install  -D  la9310sdr/la9310sdr.ko ${MODULE_INSTALL_DIR}/la9310sdr.ko
	install  -D  la9310sdr/sdr_gpio.ko ${MODULE_INSTALL_DIR}/sdr_gpio.ko
	install  -D  la9310sdr/sdr_daughterboard.ko ${MODULE_INSTALL_DIR}/sdr_daughterboard.ko
	install  -D  la9310sdr/sdr_tti.ko ${MODULE_INSTALL_DIR}/sdr_tti.ko
	install  -D  la9310sdr/sdr_lalib.ko ${MODULE_INSTALL_DIR}/sdr_lalib.ko
endif
ifeq ($(IMX_RFLIME),1)
	install  -D  la9310rflime/sdr_lime.ko ${MODULE_INSTALL_DIR}/sdr_lime.ko
endif
ifeq ($(IMX_RFGR),1)
	install  -D la9310rfgr/sdr_granita.ko ${MODULE_INSTALL_DIR}/sdr_granita.ko
endif
ifeq ($(IMX_RFMT3812),1)
	install  -D la9310mt3812/sdr_mt3812.ko ${MODULE_INSTALL_DIR}/sdr_mt3812.ko
endif
