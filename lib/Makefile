#Copyright 2021, 2024 NXP

#Add lib directory here

LIB_DIRS := wdog tti ecspi phc_clock
CLEAN_LIB_DIRS = $(patsubst %, %_clean, ${LIB_DIRS})
INSTALL_LIB_DIRS = $(patsubst %, %_install, ${LIB_DIRS})

.PHONY: ${LIB_DIRS} ${CLEAN_LIB_DIRS} ${INSTALL_LIB_DIRS}


${LIB_DIRS}:
	@echo ${LIB_DIRS}
	${MAKE} -C $@ all

${CLEAN_LIB_DIRS}:
	${MAKE} -C $(patsubst %_clean, %, $@) clean

${INSTALL_LIB_DIRS}:
	${MAKE} -C $(patsubst %_install, %, $@) install

all: ${LIB_DIRS}
	@echo ${LIB_DIRS}

clean: ${CLEAN_LIB_DIRS}

install: ${INSTALL_LIB_DIRS}
