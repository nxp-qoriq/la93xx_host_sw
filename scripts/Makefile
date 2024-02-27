#Copyright 2017-2019, 2021-2024 NXP
SCRIPT_TEST := reset_la9310.sh scratch_buff_base.sh
SCRIPT_TEST += rfnm_load_la9310_drivers rfnm_enable_la9310_uart_ext_port
SCRIPT_TEST += rfnm_en_lms rfnm_init_rba_gpio rfnm_en_daughterboard_power

.PHONY: all

all:
clean:
install:
	install -m 0755 -d ${SCRIPTS_INSTALL_DIR}
	install -D $(SCRIPT_TEST) ${SCRIPTS_INSTALL_DIR}/$(SCRIPTS_TEST)
