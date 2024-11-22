# SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
#Copyright 2024 NXP

CFLAGS  +=  -g -O3 -Wall -D_GNU_SOURCE -Werror -Wno-unused-variable
LDFLAGS += -pthread -lpthread -lrt 
LDFLAGS  += -L${LIB_INSTALL_DIR} -static

INCLUDES += -I../common 
INCLUDES += -I${API_DIR}

SRCS_TEST := iq_app.c  ../common/l1-trace-host.c ../common/imx8-host.c
OBJS_TEST := $(SRCS_TEST:.c =.o)
BIN_TEST := iq_app
LIB_FILE := iqplayer

BIN_INSTALL_DIR?=/usr/bin

.PHONY: all

all: $(BIN_TEST) 

$(BIN_TEST): ${OBJS_TEST}
	${CC} ${CFLAGS} ${LDFLAGS} -o $(BIN_TEST) ${OBJS_TEST} -l${LIB_FILE} $(INCLUDES)

%.o: %.c 
	${CC} -c ${CFLAGS} ${INCLUDES}  $< -o $@

clean:
	rm -rf *.o $(BIN_TEST) 

install:
	install -D $(BIN_TEST) ${BIN_INSTALL_DIR}/$(BIN_TEST)

