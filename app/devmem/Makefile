# SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
#Copyright 2023-2024 NXP

CFLAGS  += -Wall -Werror

INCLUDES += -I${UAPI_DIR}

SRCS_TEST := devmem.c
OBJS_TEST := $(SRCS_TEST:.c =.o)
BIN_TEST := devmem
BIN_TEST_DEVMEM5 := devmem5

all: $(BIN_TEST)

$(BIN_TEST): ${OBJS_TEST}
	${CC} ${CFLAGS} ${LDFLAGS} -o $(BIN_TEST) ${OBJS_TEST} $(INCLUDES)

%.o: %.c
	${CC} -c ${CFLAGS} ${INCLUDES}  $< -o $@

clean:
	rm -rf *.o $(BIN_TEST)

install:
	install -D $(BIN_TEST) ${BIN_INSTALL_DIR}/$(BIN_TEST)
	install -D $(BIN_TEST_DEVMEM5) ${BIN_INSTALL_DIR}/$(BIN_TEST_DEVMEM5)
