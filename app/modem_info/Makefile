# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2024 NXP
#
CFLAGS  += -Wall -Werror
LDFLAGS  += -L${LIB_INSTALL_DIR}

INCLUDES  += -I${LA9310_COMMON_HEADERS} -I${UAPI_DIR}

SRCS_TEST := modem_info.c
OBJS := $(SRCS_TEST:.c =.o)
BIN_MOD_INFO := la9310_modem_info

SRCS_SHBUF := la9310_shbuf_test.c
OBJS_SHBUF := $(SRCS_SHBUF:.c =.o)
BIN_SHBUF := la9310_shbuf_test

all: $(BIN_MOD_INFO) $(BIN_SHBUF)

$(BIN_MOD_INFO): ${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o $(BIN_MOD_INFO) ${OBJS} $(INCLUDES)

$(BIN_SHBUF): ${OBJS_SHBUF}
	${CC} ${CFLAGS} ${LDFLAGS} -o $(BIN_SHBUF) ${OBJS_SHBUF} $(INCLUDES)

%.o: %.c
	${CC} -c ${CFLAGS} ${INCLUDES}  $< -o $@

clean:
	rm -rf *.o $(BIN_MOD_INFO) $(BIN_SHBUF)

install:
	install -D $(BIN_MOD_INFO) ${BIN_INSTALL_DIR}/$(BIN_MOD_INFO)
	install -D $(BIN_SHBUF) ${BIN_INSTALL_DIR}/$(BIN_SHBUF)
