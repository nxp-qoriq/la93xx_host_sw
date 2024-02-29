# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2024 NXP

CFLAGS  += -Wall -Werror -I${COMMON_HEADERS_DIR}
LDFLAGS  += -L${LIB_INSTALL_DIR} -static

INCLUDES += -I${API_DIR}
INCLUDES += -I${UAPI_DIR}

SRCS_TEST := la9310_tvd_app.c
OBJS_TEST := $(SRCS_TEST:.c =.o)
BIN_TEST := la9310_tvd_testapp

all: $(BIN_TEST)

$(BIN_TEST): ${OBJS_TEST}
	${CC} ${CFLAGS} ${LDFLAGS} -o $(BIN_TEST) ${OBJS_TEST}  $(INCLUDES) -lpthread

%.o: %.c
	${CC} -c ${CFLAGS} ${INCLUDES}  $< -o $@

clean:
	 rm -rf *.o $(BIN_TEST)

install:
	install -D $(BIN_TEST) ${BIN_INSTALL_DIR}/$(BIN_TEST)

