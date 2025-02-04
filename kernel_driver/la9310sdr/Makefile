#Copyright 2017, 2021-2025 NXP

ccflags-y := -DLA9310_HOST_SW_VERSION="${VERSION_STRING}" -I${COMMON_DIR} -I${UAPI_DIR}
ccflags-y += -Wall -Wno-unused-function -Wno-unused-variable
ccflags-y += -std=gnu99 -Wno-declaration-after-statement
ccflags-y += -Werror
LA9310_SDR_HEADER_DIR ?= ${LA9310_DRV_HEADER_DIR}/../la9310sdr/

ifeq (${DEBUG}, 1)
ccflags-y += -g -DDEBUG
endif

ccflags-y += -I${LA9310_DRV_HEADER_DIR} -I${LA9310_COMMON_HEADERS} 
ccflags-y += -I${KERNEL_DIR}/drivers/usb/gadget/
ccflags-y += -I${KERNEL_DIR}/../kernel-source/drivers/usb/gadget/
ccflags-y += -I${LA9310_SDR_HEADER_DIR}/

la9310sdr-objs := la9310_rfnm.o cache.o rfnm_neon.o

ifeq ($(IMX_SEEVE),1)
	ccflags-y += -DSEEVE="1"
endif

CFLAGS_REMOVE_rfnm_neon.o += -mgeneral-regs-only
la9310sdr-objs += pack16to12.o unpack12to16.o
obj-m := la9310sdr.o
sdr_usb_function-objs := rfnm_usb_function.o
obj-m += sdr_usb_function.o
sdr_usb-objs := rfnm_usb.o
obj-m += sdr_usb.o
sdr_usb_boost-objs := rfnm_usb_boost.o
obj-m += sdr_usb_boost.o
sdr_daughterboard-objs := rfnm_daughterboard.o
obj-m += sdr_daughterboard.o
sdr_gpio-objs := rfnm_gpio.o
obj-m += sdr_gpio.o
sdr_tti-objs := rfnm_tti.o
obj-m += sdr_tti.o
sdr_lalib-objs := rfnm_lalib.o
obj-m += sdr_lalib.o
sdr_breakout-objs := rfnm_breakout_m.o 
obj-m += sdr_breakout.o

ifeq ($(IMX_RFMT3812),1)
LA9310_MT3812_HEADER_DIR ?= ${LA9310_DRV_HEADER_DIR}/../la9310mt3812/
ccflags-y += -DIMX_RFMT3812 -I${LA9310_MT3812_HEADER_DIR}/
endif
