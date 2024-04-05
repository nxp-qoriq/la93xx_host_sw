#SPDX-License-Identifier: GPL-2.0
#Copyright 2017-2024 NXP

ccflags-y := -DLA9310_HOST_SW_VERSION="${VERSION_STRING}" -I${COMMON_DIR} -I${UAPI_DIR} -I${LA9310_COMMON_HEADERS}
ccflags-y += -Wall
#ccflags-y += -Wall -Werror

ifeq ($(IMX_RFNM),1)
ccflags-y += -DRFNM=1
ccflags-y += -DVSPA_MUX_IRQ=1
endif

ifeq (${BOOTROM_USE_EDMA}, 1)
ccflags-y += -DBOOTROM_USE_EDMA=1
endif

ifeq (${DEBUG}, 1)
ccflags-y += -g -DDEBUG
endif

obj-m := la9310shiva.o
la9310shiva-objs := la9310_base.o  la9310_boot.o  la9310_pci.o la9310_sysfs.o la9310_irq_mux.o \
		la9310_ipc.o la9310_main_vspa.o la9310_v2h.o la9310_wdog.o la9310_sync_timing_device.o \
		la9310_modinfo.o la9310_tvd.o la9310_tti.o
