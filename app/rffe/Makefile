# SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
# Copyright 2025 NXP

CFLAGS  +=  -g -O3 -Wall -D_GNU_SOURCE -Werror -Wno-unused-variable

SRCS_TEST := rffe.c main.c HAL-imx8mp.c
DEPS := HAL-imx8mp.h SKY58281-11.h rffe.h
OBJS_TEST := $(SRCS_TEST:.c =.o)
BIN_TEST := rffe_app

all: $(BIN_TEST)

$(BIN_TEST): ${OBJS_TEST}
	 ${CC} ${CFLAGS} ${LDFLAGS} -o $(BIN_TEST) ${OBJS_TEST} $(INCLUDES)

%.o: %.c $(DEPS)
	 ${CC} -c ${CFLAGS} ${INCLUDES}  $< -o $@

clean:
	rm -rf *.o $(BIN_TEST)

install:
	install -D $(BIN_TEST) ${BIN_INSTALL_DIR}/$(BIN_TEST)
