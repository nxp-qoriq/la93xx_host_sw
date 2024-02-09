# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2022-2024 NXP
#

CFLAGS  += -Wall -Werror -I${LA9310_COMMON_HEADERS}

OBJS := modem_info.o
BIN_MOD_INFO := modem_info

all: $(BIN_MOD_INFO)

$(BIN_MOD_INFO): ${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o $(BIN_MOD_INFO) ${OBJS} $(INCLUDES)


%.o: %.c
	${CC} -c ${CFLAGS} ${INCLUDES}  $< -o $@

clean:
	rm -rf *.o $(BIN_MOD_INFO)

install:
	install -D $(BIN_MOD_INFO) ${BIN_INSTALL_DIR}/$(BIN_MOD_INFO)
