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
endif
