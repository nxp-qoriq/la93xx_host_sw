# SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
#Copyright 2024 NXP

CFLAGS  +=  -g -O3 -Wall -D_GNU_SOURCE -Werror -Wno-unused-variable
LDFLAGS += -pthread -lpthread -lrt
LDFLAGS  += -L${LIB_INSTALL_DIR} -static

INCLUDES += -I../common 
INCLUDES += -I${API_DIR}
INCLUDES += -I${LIB_DIR}/dma

SRCS_TEST := iq_streamer.c pci_imx8.c l1-trace-host.c l1-trace-vspa.c 
OBJS_TEST := $(SRCS_TEST:.c =.o)
BIN_TEST := iq_streamer
BIN_TEST_2R := iq_streamer_2R
BIN_TEST_4R := iq_streamer_4R
LIB_FILE := edma

BIN_INSTALL_DIR?=/usr/bin
FIRMWARE_DIR := $(PWD)/app/host_utils/fw_iqplayer/

.PHONY: all

all: $(BIN_TEST) $(BIN_TEST_2R) $(BIN_TEST_4R)

$(BIN_TEST): ${OBJS_TEST}
	${CC} ${CFLAGS} ${LDFLAGS} -DIQMOD_RX_1R -o $(BIN_TEST) ${OBJS_TEST} -l${LIB_FILE} $(INCLUDES)

$(BIN_TEST_2R): ${OBJS_TEST}
	${CC} ${CFLAGS} ${LDFLAGS} -DIQMOD_RX_2R -o $(BIN_TEST_2R) ${OBJS_TEST} -l${LIB_FILE} $(INCLUDES)

$(BIN_TEST_4R): ${OBJS_TEST}
	${CC} ${CFLAGS} ${LDFLAGS} -DIQMOD_RX_4R -o $(BIN_TEST_4R) ${OBJS_TEST} -l${LIB_FILE} $(INCLUDES)

%.o: %.c 
	${CC} -c ${CFLAGS} ${INCLUDES}  $< -o $@

clean:
	rm -rf *.o $(BIN_TEST) $(BIN_TEST_2R) $(BIN_TEST_4R)

install:
	mkdir -p ${FIRMWARE_INSTALL_DIR}/
	install -D ${FIRMWARE_DIR}/*.eld ${FIRMWARE_INSTALL_DIR}/
	install -D ${FIRMWARE_DIR}/*.elf ${FIRMWARE_INSTALL_DIR}/
	install -D $(BIN_TEST) ${BIN_INSTALL_DIR}/$(BIN_TEST)
	install -D $(BIN_TEST_2R) ${BIN_INSTALL_DIR}/$(BIN_TEST_2R)
	install -D $(BIN_TEST_4R) ${BIN_INSTALL_DIR}/$(BIN_TEST_4R)

