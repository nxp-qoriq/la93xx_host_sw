/* SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 * Copyright 2024 NXP
 */

/*function to initialize phc 1588 registers for user space access */
int init_1588(void);
/*function to de-initialize phc 1588 registers for user space access */
void deinit_1588(void);
/*function to read phc 1588 registers for user space access */
int tmr_cnt_read(uint32_t *seconds, uint32_t *nseconds);
/*function to write/update phc 1588 registers for user space access */
int tmr_cnt_write(uint32_t seconds, uint32_t nseconds);
/*function to read phc 1588 registers for user space access for debug only */
uint32_t tmr_read_reg(uint32_t reg_addr);
