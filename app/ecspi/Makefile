# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2024 NXP

CFLAGS  += -Wall -Werror
LDFLAGS  += -L${LIB_INSTALL_DIR} -static

INCLUDES += -I${LIB_DIR}/ecspi/

SRCS_TEST := ecspi_test_app.c
OBJS_TEST := $(SRCS_TEST:.c =.o)
BIN_TEST := ecspi_test_app
LIB_FILE := ecspi

all: $(BIN_TEST)

$(BIN_TEST): ${OBJS_TEST}
	${CC} ${CFLAGS} ${LDFLAGS} -o $(BIN_TEST) ${OBJS_TEST} -l${LIB_FILE}  $(INCLUDES)

%.o: %.c
	${CC} -c ${CFLAGS} ${INCLUDES}  $< -o $@

clean:
	 rm -rf *.o $(BIN_TEST)

install:
	install -D $(BIN_TEST) ${BIN_INSTALL_DIR}/$(BIN_TEST)

