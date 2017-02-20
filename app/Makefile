#Copyright 2017 NXP

#Add applicatin/exectuable directories here

APP_DIRS := ipc rfic bincreate


CLEAN_APP_DIRS = $(patsubst %, %_clean, ${APP_DIRS})
INSTALL_APP_DIRS = $(patsubst %, %_install, ${APP_DIRS})

.PHONY: ${APP_DIRS} ${CLEAN_APP_DIRS} ${INSTALL_APP_DIRS}


${APP_DIRS}:
	${MAKE} -C $@ all

${CLEAN_APP_DIRS}:
	${MAKE} -C $(patsubst %_clean, %, $@) clean

${INSTALL_APP_DIRS}:
	${MAKE} -C $(patsubst %_install, %, $@) install

all: ${APP_DIRS}

clean: ${CLEAN_APP_DIRS}

install: ${INSTALL_APP_DIRS}
