# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2024 NXP

CFLAGS  += -Wall -Werror -g -O -fPIC

LDFLAGS  += -L${LIB_INSTALL_DIR}

INCLUDES += -I${API_DIR} -I../../app/host_utils/common

OBJ_LIB := libiqplayer.o

OUT_LIB := libiqplayer.a

all: $(OUT_LIB)

$(OUT_LIB): ${OBJ_LIB}
	echo ${OBJ_LIB}
	${AR} rcs  $(OUT_LIB) ${OBJ_LIB}
	install -D $(OUT_LIB) ${LIB_INSTALL_DIR}/$(OUT_LIB)

%.o: %.c
	${CC} -c ${CFLAGS} ${INCLUDES}  $< -o $@

clean:
	rm -rf *.o *.a

install:
	install -D $(OUT_LIB) ${LIB_INSTALL_DIR}/$(OUT_LIB)
