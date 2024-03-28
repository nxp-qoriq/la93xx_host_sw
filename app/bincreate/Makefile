#Copyright 2017, 2021 NXP

CFLAGS  += -Wall -Werror
LDFLAGS	 += -L${LIB_INSTALL_DIR}

INCLUDES += -I${API_DIR}/bincreate
INCLUDES += -I${UAPI_DIR}

SRCS_TEST := bin_create.c
OBJS_TEST := $(SRCS_TEST:.c =.o)
BIN_TEST := bin_create

all: $(BIN_TEST)

$(BIN_TEST): ${OBJS_TEST}
	${CC} ${CFLAGS} ${LDFLAGS} -o $(BIN_TEST) ${OBJS_TEST} ${LIBS} $(INCLUDES)

%.o: %.c
	${CC} -c ${CFLAGS} ${INCLUDES}  $< -o $@

clean:
	rm -rf *.o $(BIN_TEST)

install:
	install -D $(BIN_TEST) ${BIN_INSTALL_DIR}/$(BIN_TEST)
