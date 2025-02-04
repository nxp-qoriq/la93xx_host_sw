#SPDX-License-Identifier: GPL-2.0
#Copyright 2024-2025 NXP

ifndef KERNEL_DIR
$(error KERNEL_DIR is not set)
endif
ifndef LA9310_COMMON_HEADERS
$(error LA9310_COMMON_HEADERS is not set)
endif

ccflags-y := -DLA9310_HOST_SW_VERSION="${VERSION_STRING}" -I${COMMON_DIR} -I${UAPI_DIR}
ccflags-y += -Wall
ccflags-y += -std=gnu99 -Wno-declaration-after-statement
ccflags-y += -Werror -Wno-implicit-fallthrough
LA9310_SDR_HEADER_DIR ?= ${LA9310_DRV_HEADER_DIR}/../la9310sdr/

ifeq (${DEBUG}, 1)
ccflags-y += -g -DDEBUG
endif

ccflags-y += -I${LA9310_COMMON_HEADERS}
ccflags-y += -I${LA9310_DRV_HEADER_DIR}/
ccflags-y += -I${LA9310_SDR_HEADER_DIR}/

#LMS7002M_MODULE_DIR ?= ../../../lms7002m/
LMS7002M_MODULE_DIR ?= ../../../LimeSuiteNG/
LMS7002M_KERNEL_DIR ?= ${LA9310_DRV_HEADER_DIR}/${LMS7002M_MODULE_DIR}
#ccflags-y += -I${LMS7002M_KERNEL_DIR}/include -I${LMS7002M_KERNEL_DIR}/interfaces -I${LMS7002M_KERNEL_DIR}/src
ccflags-y += -DIMX_RFLIME -I${LMS7002M_KERNEL_DIR}/embedded/include

sdr_lime-objs := rfnm_lime_m.o
ifeq (${RFNM_BITBAKE}, 1)
LINK_FE_GENERIC = 0
sdr_lime-objs += ../la9310sdr/rfnm_fe_generic.o
else
LINK_FE_GENERIC = 1
sdr_lime-objs += ../la9310sdr/rfnm_fe_generic.o
endif

sdr_lime-objs += rfnm_fe_lime0.o
sdr_lime-objs += ${LMS7002M_MODULE_DIR}/embedded/lms7002m/lms7002m.o
sdr_lime-objs += ${LMS7002M_MODULE_DIR}/embedded/lms7002m/privates.o
obj-m += sdr_lime.o

include $(KERNEL_DIR)/.config

all:
ifeq ($(LINK_FE_GENERIC), 1)
	rm -f rfnm_fe_generic.c
	ln -sr ${LA9310_SDR_HEADER_DIR}/rfnm_fe_generic.c .
endif
	$(MAKE) ARCH=$(ARCH) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	rm -f rfnm_fe_generic.c
	$(MAKE) ARCH=$(ARCH) -C $(KERNEL_DIR) M=$(PWD) clean
	rm -rf *.ko
