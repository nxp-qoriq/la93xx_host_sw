#SPDX-License-Identifier: GPL-2.0
#Copyright 2024 NXP

ifndef KERNEL_DIR
$(error KERNEL_DIR is not set)
endif
ifndef LA9310_COMMON_HEADERS
$(error LA9310_COMMON_HEADERS is not set)
endif

LA9310_RFNM_HEADER_DIR ?= ${LA9310_DRV_HEADER_DIR}/../la9310rfnm/

ccflags-y := -DLA9310_HOST_SW_VERSION="${VERSION_STRING}" -I${COMMON_DIR} -I${UAPI_DIR}
ccflags-y += -Wall -Wno-unused-function -Wno-unused-variable
ccflags-y += -std=gnu99 -Wno-declaration-after-statement
#ccflags-y += -Werror

ifeq (${DEBUG}, 1)
ccflags-y += -g -DDEBUG
endif

ccflags-y += -I${LA9310_COMMON_HEADERS}
ccflags-y += -I${LA9310_DRV_HEADER_DIR}/
ccflags-y += -I${LA9310_RFNM_HEADER_DIR}/

ifeq (${CONFIG_ENABLE_FLOAT_BYPASS}, y)
rfnm_mt3812-objs := rfnm_mt3812_m.o
ifeq (${RFNM_BITBAKE}, 1)
LINK_FE_GENERIC = 0
rfnm_mt3812-objs += ../la9310rfnm/rfnm_fe_generic.o
else
LINK_FE_GENERIC = 1
rfnm_mt3812-objs += ../la9310rfnm/rfnm_fe_generic.o
endif
rfnm_mt3812-objs += rfnm_fe_mt3812.o

obj-m += rfnm_mt3812.o
else
$(warning RFNM_RF MT3812 Modules not compiled)
endif

include $(KERNEL_DIR)/.config

all:
ifeq ($(LINK_FE_GENERIC), 1)
	rm -f rfnm_fe_generic.c
	ln -sr ${LA9310_RFNM_HEADER_DIR}/rfnm_fe_generic.c .
endif
	$(MAKE) ARCH=$(ARCH) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	rm -f rfnm_fe_generic.c
	$(MAKE) ARCH=$(ARCH) -C $(KERNEL_DIR) M=$(PWD) clean
	rm -rf *.ko
