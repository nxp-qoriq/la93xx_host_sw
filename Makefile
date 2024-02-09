#SPDX-License-Identifier: GPL-2.0
#Copyright 2017, 2021-2024 NXP

CC              = $(CROSS_COMPILE)gcc
AR              = $(CROSS_COMPILE)ar
LD              = $(CROSS_COMPILE)ld

KERNEL_DIR ?=

ifdef SYSROOT_PATH
LDFLAGS := --sysroot=${SYSROOT_PATH}
CFLAGS := --sysroot=${SYSROOT_PATH}
endif

VERSION_STRING :=\"v3.x\"

INSTALL_DIR ?= ${PWD}/install
LIB_INSTALL_DIR := ${INSTALL_DIR}/usr/lib
BIN_INSTALL_DIR := ${INSTALL_DIR}/usr/bin
MODULE_INSTALL_DIR := ${INSTALL_DIR}/kernel_module
CONFIG_INSTALL_DIR := ${INSTALL_DIR}/etc/config/la9310_config/
SCRIPTS_INSTALL_DIR := ${INSTALL_DIR}/usr/bin
FIRMWARE_INSTALL_DIR := ${INSTALL_DIR}/lib/firmware

COMMON_DIR := ${PWD}/common/
API_DIR := ${PWD}/api/
UAPI_DIR := ${PWD}/uapi/
LA9310_DRV_HEADER_DIR := ${PWD}/kernel_driver/la9310shiva/

INCLUDES += -I${API_DIR}/ 
COMMON_INCLUDES += -I${COMMON_DIR}

CFLAGS += ${COMMON_INCLUDES}
HOST_CFLAGS += ${COMMON_INCLUDES}
GIT_VERSION :=\"$(shell git describe --abbrev=0 --tags)\"

# Config Tweak handles
DEBUG ?= 1
BOOTROM_USE_EDMA ?= 1
IMX_RFNM ?= 1
NLM ?= 0

export CC LIB_INSTALL_DIR BIN_INSTALL_DIR SCRIPTS_INSTALL_DIR MODULE_INSTALL_DIR \
	FIRMWARE_INSTALL_DIR
export CONFIG_INSTALL_DIR API_DIR KERNEL_DIR COMMON_DIR UAPI_DIR LA9310_DRV_HEADER_DIR
export INCLUDES CFLAGS VERSION_STRING LDFLAGS
export DEBUG BOOTROM_USE_EDMA IMX_RFNM

ifeq ($(IMX_RFNM),1)
	export BSP_VERSION := $(shell git describe --tags --abbrev=11 --dirty --match "la12*")
	CFLAGS += -DIMX_RFNM
endif

ifneq ($(BSP_VERSION),)
	GIT_VERSION:=\"${BSP_VERSION}\"
endif

ifneq ($(GIT_VERSION),)
	VERSION_STRING :=${GIT_VERSION}
endif

CFLAGS += -DVERSION=${GIT_VERSION}
CFLAGS += -Wall -DLA9310_HOST_SW_VERSION="${VERSION_STRING}" -Wno-unused-function -Wno-unused-variable

ifeq (${DEBUG}, 1)
CFLAGS += -DDEBUG
endif

DIRS ?= lib app kernel_driver firmware scripts

CLEAN_DIRS = $(patsubst %, %_clean, ${DIRS})
INSTALL_DIRS = $(patsubst %, %_install, ${DIRS})

.PHONY: ${DIRS} ${CLEAN_DIRS} ${INSTALL_DIRS}

default: all

${DIRS}:
	${MAKE} -C $@ all

all: ${DIRS}

${CLEAN_DIRS}:
	${MAKE} -C $(patsubst %_clean, %, $@) clean
	rm -rf ${LIB_INSTALL_DIR}/*;
	rm -rf ${BIN_INSTALL_DIR}/*;
	rm -rf ${SCRIPTS_INSTALL_DIR}/*;
	rm -rf ${CONFIG_INSTALL_DIR}/*;
	rm -rf ${INSTALL_DIR};

${INSTALL_DIRS}:
	${MAKE} -j1 -C $(patsubst %_install, %, $@) install

install: ${INSTALL_DIRS}

clean: ${CLEAN_DIRS}
