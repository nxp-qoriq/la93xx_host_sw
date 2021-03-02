#Copyright 2021 NXP

CFLAGS  += -Wall -Werror -g -O -fPIC
LDFLAGS  += -L${LIB_INSTALL_DIR}

INCLUDES += -I${API_DIR}
INCLUDES += -I${UAPI_DIR}

SRCS_TEST := la9310_wdog_lib.c
OBJS_TEST := $(SRCS_TEST:.c =.o)
BIN_TEST := libla9310wdog.so

all: $(BIN_TEST)

$(BIN_TEST): ${OBJS_TEST}
	${CC} -shared -o $(BIN_TEST) ${OBJS_TEST} ${INCLUDES} ${CFLAGS} ${LDFLAGS}
	install -D $(BIN_TEST) ${LIB_INSTALL_DIR}/$(BIN_TEST)

%.o: %.c
	${CC} -c ${CFLAGS} ${INCLUDES}  $< -o $@

clean:
	rm -rf *.o *.so

install:
	install -D $(BIN_TEST) ${LIB_INSTALL_DIR}/$(BIN_TEST)

