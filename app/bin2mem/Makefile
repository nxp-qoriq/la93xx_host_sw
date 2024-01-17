#Copyright 2024 NXP


CFLAGS  +=  -g -O0 -Wall -Werror

INCLUDES += -I${UAPI_DIR}

SRCS_TEST := log.c physical_mem.c bin2mem.c
OBJS_TEST := $(SRCS_TEST:.c =.o)
BIN_TEST := bin2mem

all: $(BIN_TEST)

$(BIN_TEST): ${OBJS_TEST}
	${CC} ${CFLAGS} ${LDFLAGS} -o $(BIN_TEST) ${OBJS_TEST} $(INCLUDES)

%.o: %.c
	${CC} -c ${CFLAGS} ${INCLUDES}  $< -o $@

clean:
	rm -rf *.o $(BIN_TEST)

install:
	install -D $(BIN_TEST) ${BIN_INSTALL_DIR}/$(BIN_TEST)
