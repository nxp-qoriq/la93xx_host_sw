# SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
#Copyright 2024 NXP

CFLAGS  +=  -g -O3 -Wall -D_GNU_SOURCE -Werror
LDFLAGS += -pthread -lpthread -lrt

SRCS_TEST := iq_streamer.c pci_imx8mp.c stats.c l1-trace-host.c 
OBJS_TEST := $(SRCS_TEST:.c =.o)
BIN_TEST := iq_streamer
BIN_INSTALL_DIR?=/usr/bin
FIRMWARE_DIR := $(PWD)/app/host_utils/fw_iqplayer/

.PHONY: all

all: $(BIN_TEST)

$(BIN_TEST): ${OBJS_TEST}
	${CC} ${CFLAGS} ${LDFLAGS} -o $(BIN_TEST) ${OBJS_TEST} $(INCLUDES)

%.o: %.c 
	${CC} -c ${CFLAGS} ${INCLUDES}  $< -o $@

clean:
	rm -rf *.o $(BIN_TEST)

install:

	mkdir -p ${FIRMWARE_INSTALL_DIR}/
	install -D ${FIRMWARE_DIR}/*.eld ${FIRMWARE_INSTALL_DIR}/
	install -D $(BIN_TEST) ${BIN_INSTALL_DIR}/$(BIN_TEST)

