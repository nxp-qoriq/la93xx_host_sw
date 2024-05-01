# SPDX-License-Identifier: BSD-3-Clause
#Copyright 2024 NXP

CFLAGS  += -Wall -Werror -D_GNU_SOURCE
LDFLAGS	 += -L${LIB_INSTALL_DIR}

INCLUDES += -I${API_DIR}
INCLUDES += -I${UAPI_DIR}
INCLUDES  += -I${LA9310_COMMON_HEADERS}

SRCS_TEST := la9310_tti_app.c
OBJS_TEST := $(SRCS_TEST:.c =.o)
BIN_TEST := la9310_tti_app
SO_FILE := la9310tti

all: $(BIN_TEST)

$(BIN_TEST): ${OBJS_TEST}
	${CC} ${CFLAGS} ${LDFLAGS} -o $(BIN_TEST) ${OBJS_TEST} -l${SO_FILE}  $(INCLUDES)

%.o: %.c
	${CC} -c ${CFLAGS} ${INCLUDES}  $< -o $@

clean:
	rm -rf *.o $(BIN_TEST)

install:
	install -D $(BIN_TEST) ${BIN_INSTALL_DIR}/$(BIN_TEST)
