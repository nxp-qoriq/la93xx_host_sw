#Copyright 2017, 2021-2024 NXP

ccflags-y := -DLA9310_HOST_SW_VERSION="${VERSION_STRING}" -I${COMMON_DIR} -I${UAPI_DIR}
ccflags-y += -Wall -Wno-unused-function -Wno-unused-variable

ifeq (${DEBUG}, 1)
ccflags-y += -g -DDEBUG
endif

ccflags-y += -I${LA9310_DRV_HEADER_DIR} -I${LA9310_COMMON_HEADERS} 
ccflags-y += -I${KERNEL_DIR}/drivers/usb/gadget/
ccflags-y += -I${KERNEL_DIR}/../kernel-source/drivers/usb/gadget/

obj-m := la9310rfnm.o rfnm_usb_function.o rfnm_usb.o 
la9310rfnm-objs := la9310_rfnm.o  cache.o #pack12bit.o unpack12bit.o
