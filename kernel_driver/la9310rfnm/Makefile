#Copyright 2017, 2021-2024 NXP

ccflags-y := -DLA9310_HOST_SW_VERSION="${VERSION_STRING}" -I${COMMON_DIR} -I${UAPI_DIR}
ccflags-y += -Wall -Wno-unused-function -Wno-unused-variable

ccflags-y := -std=gnu99 -Wno-declaration-after-statement

LA9310_RFNM_HEADER_DIR ?= ${LA9310_DRV_HEADER_DIR}/../la9310rfnm/
LMS7002M_MODULE_DIR ?= ../../../lms7002m/
LMS7002M_KERNEL_DIR ?= ${LA9310_DRV_HEADER_DIR}/${LMS7002M_MODULE_DIR}

ifeq (${DEBUG}, 1)
ccflags-y += -g -DDEBUG
endif

ccflags-y += -I${LA9310_DRV_HEADER_DIR} -I${LA9310_COMMON_HEADERS} 
ccflags-y += -I${KERNEL_DIR}/drivers/usb/gadget/
ccflags-y += -I${KERNEL_DIR}/../kernel-source/drivers/usb/gadget/
ccflags-y += -I${LA9310_RFNM_HEADER_DIR}/
ccflags-y += -I${LMS7002M_KERNEL_DIR}/include -I${LMS7002M_KERNEL_DIR}/interfaces -I${LMS7002M_KERNEL_DIR}/src

la9310rfnm-objs := la9310_rfnm.o  cache.o
obj-m := la9310rfnm.o
obj-m += rfnm_usb_function.o
obj-m += rfnm_usb.o
obj-m += rfnm_usb_boost.o
obj-m += rfnm_daughterboard.o
obj-m += rfnm_gpio.o
obj-m += rfnm_tti.o
#obj-m += rfnm_kasan.o
obj-m += rfnm_lalib.o
rfnm_breakout-objs := rfnm_breakout_m.o 
obj-m += rfnm_breakout.o

ifeq (${CONFIG_ENABLE_FLOAT_BYPASS}, y)
#la9310rfnm-objs += pack12bit.o unpack12bit.o
rfnm_lime-objs := rfnm_lime_m.o
rfnm_lime-objs += rfnm_fe_generic.o
rfnm_lime-objs += rfnm_fe_lime0.o
rfnm_lime-objs += dtoa.o
rfnm_lime-objs += ${LMS7002M_MODULE_DIR}/src/LMS7002M_afe.o
rfnm_lime-objs += ${LMS7002M_MODULE_DIR}/src/LMS7002M_logger.o
rfnm_lime-objs += ${LMS7002M_MODULE_DIR}/src/LMS7002M_tbb.o
rfnm_lime-objs += ${LMS7002M_MODULE_DIR}/src/LMS7002M_cgen.o
rfnm_lime-objs += ${LMS7002M_MODULE_DIR}/src/LMS7002M_nco.o
rfnm_lime-objs += ${LMS7002M_MODULE_DIR}/src/LMS7002M_time.o
rfnm_lime-objs += ${LMS7002M_MODULE_DIR}/src/LMS7002M_filter_cal.o
rfnm_lime-objs += ${LMS7002M_MODULE_DIR}/src/LMS7002M_rbb.o
rfnm_lime-objs += ${LMS7002M_MODULE_DIR}/src/LMS7002M_trf.o
rfnm_lime-objs += ${LMS7002M_MODULE_DIR}/src/LMS7002M_gfir.o
rfnm_lime-objs += ${LMS7002M_MODULE_DIR}/src/LMS7002M_rfe.o
rfnm_lime-objs += ${LMS7002M_MODULE_DIR}/src/LMS7002M_tx_filter_cal.o
rfnm_lime-objs += ${LMS7002M_MODULE_DIR}/src/LMS7002M_impl.o
rfnm_lime-objs += ${LMS7002M_MODULE_DIR}/src/LMS7002M_rx_filter_cal.o
rfnm_lime-objs += ${LMS7002M_MODULE_DIR}/src/LMS7002M_txtsp.o
rfnm_lime-objs += ${LMS7002M_MODULE_DIR}/src/LMS7002M_ldo.o
rfnm_lime-objs += ${LMS7002M_MODULE_DIR}/src/LMS7002M_rxtsp.o
rfnm_lime-objs += ${LMS7002M_MODULE_DIR}/src/LMS7002M_vco.o
rfnm_lime-objs += ${LMS7002M_MODULE_DIR}/src/LMS7002M_lml.o
rfnm_lime-objs += ${LMS7002M_MODULE_DIR}/src/LMS7002M_sxx.o 
rfnm_lime-objs += ${LMS7002M_MODULE_DIR}/src/LMS7002M_xbuf.o
obj-m += rfnm_lime.o
else
$(warning RFNM_LIME Module not compiled)
endif
