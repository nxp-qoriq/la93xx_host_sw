# SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
#Copyright 2024 NXP

#Add host_utils directories here

UTIL_DIRS := iq_streamer vspa_mbox iq_app iq_monitor

CLEAN_UTIL_DIRS = $(patsubst %, %_clean, ${UTIL_DIRS})
INSTALL_UTIL_DIRS = $(patsubst %, %_install, ${UTIL_DIRS})

.PHONY: subdirs ${UTIL_DIRS} ${CLEAN_UTIL_DIRS} ${INSTALL_UTIL_DIRS}

subdirs: ${UTIL_DIRS}
${UTIL_DIRS}:
	${MAKE} -C $@

${CLEAN_UTIL_DIRS}:
	${MAKE} -C $(patsubst %_clean, %, $@) clean

${INSTALL_UTIL_DIRS}:
	${MAKE} -C $(patsubst %_install, %, $@) install

all: ${UTIL_DIRS}

clean: ${CLEAN_UTIL_DIRS}

install: ${INSTALL_UTIL_DIRS}
