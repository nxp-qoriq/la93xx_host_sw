# SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
#Copyright 2024 NXP

CFLAGS  +=  -g -O3 -Wall -D_GNU_SOURCE -Werror 
LDFLAGS += -pthread -lpthread -lrt

SRCS_TEST := iq_streamer.c pci_imx8mp.c stats.c 
OBJS_TEST := $(SRCS_TEST:.c =.o)
BIN_TEST := iq_streamer
TONE_TEST := *
SCRIPTS_INSTALL_DIR?=/home/root/host-utils
BIN_INSTALL_DIR?=/usr/bin

all: vspa_exported_symbols.h vspa_trace_enum.h $(BIN_TEST)

$(BIN_TEST): ${OBJS_TEST}
	${CC} ${CFLAGS} ${LDFLAGS} -o $(BIN_TEST) ${OBJS_TEST} $(INCLUDES)

vspa_exported_symbols.h: iq_streamer.c
	../fw_iqplayer/vspa_symbols_extract.sh ../fw_iqplayer/apm-iqplayer.eld

vspa_trace_enum.h: ./l1-trace.h ./vspa_trace_code_extract.sh 

%.o: %.c
	${CC} -c ${CFLAGS} ${INCLUDES}  $< -o $@

clean:
	rm -rf *.o $(BIN_TEST)

install:
	install -D $(BIN_TEST) ${BIN_INSTALL_DIR}/$(BIN_TEST)
	install -m 0755 -d ${SCRIPTS_INSTALL_DIR}
	install -D $(TONE_TEST) ${SCRIPTS_INSTALL_DIR}
	rm -rf "${SCRIPTS_INSTALL_DIR}/Makefile"
	rm -rf "${SCRIPTS_INSTALL_DIR}/*.c"
	rm -rf "${SCRIPTS_INSTALL_DIR}/iq_streamer"
