# SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
#Copyright 2019, 2024 NXP
CFLAGS  +=  -g -O0 -Wall -Werror

SRCS_TEST := mem_rw.c
OBJS_TEST := $(SRCS_TEST:.c =.o)
BIN_TEST := mem_rw

all: $(BIN_TEST)

$(BIN_TEST): ${OBJS_TEST}
	${CC} ${CFLAGS} ${LDFLAGS} -o $(BIN_TEST) ${OBJS_TEST} $(INCLUDES)

%.o: %.c
	${CC} -c ${CFLAGS} ${INCLUDES}  $< -o $@

clean:
	rm -rf *.o $(BIN_TEST)

install:
	install -D $(BIN_TEST) ${BIN_INSTALL_DIR}/$(BIN_TEST)
