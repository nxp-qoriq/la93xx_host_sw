# SPDX-License-Identifier: BSD-3-Clause
#Copyright 2024 NXP

CFLAGS  += -Wall -Werror
LDFLAGS	 += -L${LIB_INSTALL_DIR}

INCLUDES += -I${API_DIR}
INCLUDES += -I${UAPI_DIR}
INCLUDES  += -I${LA9310_COMMON_HEADERS}
INCLUDES += -I${LIB_DIR}/dma

SRCS_TEST := la9310_edma_app.c
OBJS_TEST := $(SRCS_TEST:.c =.o)
BIN_TEST := la9310_edma_app
LIB_FILE := edma

all: $(BIN_TEST)

$(BIN_TEST): ${OBJS_TEST}
	${CC} ${CFLAGS} ${LDFLAGS} -o $(BIN_TEST) ${OBJS_TEST} -l${LIB_FILE} $(INCLUDES)

%.o: %.c
	${CC} -c ${CFLAGS} ${INCLUDES}  $< -o $@

clean:
	rm -rf *.o $(BIN_TEST)

install:
	install -D $(BIN_TEST) ${BIN_INSTALL_DIR}/$(BIN_TEST)
