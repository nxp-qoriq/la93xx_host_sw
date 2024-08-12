# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2024 NXP

CFLAGS  += -Wall -Werror -g -O -fPIC

ifeq (${IMX_RFMT3812}, 1)
CFLAGS += -DIMX_RFMT3812=1
endif

LDFLAGS  += -L${LIB_INSTALL_DIR}

INCLUDES += -I${API_DIR}

OBJ_LIB := imx_ecspi_lib.o diora_ecspi_lib.o
ifeq (${IMX_RFMT3812}, 1)
	OBJ_LIB := imx_ecspi_lib.o diora_ecspi_lib.o
else
	OBJ_LIB := imx_ecspi_lib.o
endif

OUT_LIB := libecspi.a

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

