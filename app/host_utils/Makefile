# SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
#Copyright 2024 NXP
CFLAGS  +=  -g -O0 -Wall -Werror

SRCS_TEST := vspa_mbox.c
OBJS_TEST := $(SRCS_TEST:.c =.o)
BIN_TEST := vspa_mbox
TONE_TEST := *

.PHONY: all

all: $(BIN_TEST)

$(BIN_TEST): ${OBJS_TEST}
	${CC} ${CFLAGS} ${LDFLAGS} -o $(BIN_TEST) ${OBJS_TEST} $(INCLUDES)

%.o: %.c
	${CC} -c ${CFLAGS} ${INCLUDES}  $< -o $@

clean:
	rm -rf *.o $(BIN_TEST)

install:
	install -D $(BIN_TEST) ${BIN_INSTALL_DIR}/$(BIN_TEST)
	install -m 0755 -d ${SCRIPTS_INSTALL_DIR}
	install -D $(TONE_TEST) ${SCRIPTS_INSTALL_DIR}
	rm -rf "${SCRIPTS_INSTALL_DIR}/Makefile"
	rm -rf "${SCRIPTS_INSTALL_DIR}/vspa_mbox.c"
	rm -rf "${SCRIPTS_INSTALL_DIR}/vspa_mbox"
