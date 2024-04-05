#SPDX-License-Identifier: BSD-3-Clause
#Copyright 2024 NXP

CFLAGS  += -Wall -Werror -g -O -fPIC
LDFLAGS	 += -L${LIB_INSTALL_DIR}

INCLUDES += -I${API_DIR}
INCLUDES += -I${UAPI_DIR}
INCLUDES += -I${LA9310_COMMON_HEADERS}

SRCS_TEST := la9310_tti_lib.c
OBJ_LIB := $(SRCS_TEST:.c =.o)
OUT_LIB := libla9310tti.so

all: $(OUT_LIB)

$(OUT_LIB): ${OBJ_LIB}
	${CC} -shared -o $(OUT_LIB) ${OBJ_LIB} ${INCLUDES} ${CFLAGS} ${LDFLAGS}
	install -D $(OUT_LIB) ${LIB_INSTALL_DIR}/$(OUT_LIB)

%.o: %.c
	${CC} -c ${CFLAGS} ${INCLUDES}  $< -o $@

clean:
	rm -rf *.o *.so

install:
	install -D $(OUT_LIB) ${LIB_INSTALL_DIR}/$(OUT_LIB)
