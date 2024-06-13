#SPDX-License-Identifier: BSD-3-Clause
#Copyright 2017, 2021, 2024 NXP

#Add applicatin/exectuable directories here

APP_DIRS := bincreate wdog devmem bin2mem modem_info memrw tvd tti memtool freq_correction host_utils


CLEAN_APP_DIRS = $(patsubst %, %_clean, ${APP_DIRS})
INSTALL_APP_DIRS = $(patsubst %, %_install, ${APP_DIRS})

.PHONY: ${APP_DIRS} ${CLEAN_APP_DIRS} ${INSTALL_APP_DIRS}

#Add any CFLAGS that needs to be passed to driver
#EXTRA_CFLAGS += -g -DHOST_SW_VERSION="${VERSION_STRING}" -I${COMMON_DIR}

${APP_DIRS}:
	${MAKE} -C $@ all

${CLEAN_APP_DIRS}:
	${MAKE} -C $(patsubst %_clean, %, $@) clean

${INSTALL_APP_DIRS}:
	${MAKE} -C $(patsubst %_install, %, $@) install

all: ${APP_DIRS}

clean: ${CLEAN_APP_DIRS}

install: ${INSTALL_APP_DIRS}
