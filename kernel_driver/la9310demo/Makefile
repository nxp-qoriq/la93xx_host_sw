#SPDX-License-Identifier: GPL-2.0
#Copyright 2017, 2021-2024 NXP

ccflags-y := -DLA9310_HOST_SW_VERSION="${VERSION_STRING}" -I${COMMON_DIR} -I${UAPI_DIR}
ccflags-y += -Wall -Werror

ifeq (${DEBUG}, 1)
ccflags-y += -g -DDEBUG
endif

ifeq ($(IMX_RFNM),1)
ccflags-y += -DRFNM="1"
endif


ccflags-y += -I${LA9310_DRV_HEADER_DIR} -I${LA9310_COMMON_HEADERS}

obj-m := la9310demo.o
la9310demo-objs := la9310_test.o la9310_v2h_callback_test.o
