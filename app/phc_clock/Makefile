# SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
#Copyright 2024 NXP


CFLAGS  +=  -g -O0 -Wall -Werror
LDFLAGS  += -L${LIB_INSTALL_DIR} -static

INCLUDES += -I${UAPI_DIR}

SRCS_TEST := phc_test.c
OBJS_TEST := $(SRCS_TEST:.c =.o)
BIN_TEST := phc_test
LIB_FILE := phc_clock

all: $(BIN_TEST)

$(BIN_TEST): ${OBJS_TEST}
	${CC} ${CFLAGS} ${LDFLAGS} -o $(BIN_TEST) ${OBJS_TEST} -l${LIB_FILE} $(INCLUDES)

%.o: %.c
	${CC} -c ${CFLAGS} ${INCLUDES}  $< -o $@

clean:
	rm -rf *.o $(BIN_TEST)

install:
	install -D $(BIN_TEST) ${BIN_INSTALL_DIR}/$(BIN_TEST)
