#Copyright 2017-2019, 2021-2024 NXP
SCRIPT_TEST := *

.PHONY: all

all:
clean:
install:
	install -m 0755 -d ${SCRIPTS_INSTALL_DIR}
	install -D $(SCRIPT_TEST) ${SCRIPTS_INSTALL_DIR}/$(SCRIPTS_TEST)
	rm -rf "${SCRIPTS_INSTALL_DIR}/Makefile"
	rm -rf "${SCRIPTS_INSTALL_DIR}/memory_calc.py"
