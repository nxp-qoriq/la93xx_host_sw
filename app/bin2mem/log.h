// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2023-2024 NXP
 */

#ifndef LOG_H_
#define LOG_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>



#define PRINT_TO_CONSOLE 0x1

/* Macros for Log types */
#define ERR_LEVEL  (1<<0)
#define WARN_LEVEL (1<<1)
#define INFO_LEVEL (1<<2)
#define DBG_LEVEL  (1<<3)
#define PKT_PRINT  (1<<4)
/** Logging type, currently only PRINT_TO_CONSOLE supported*/
extern int debug_cfg;

/** Bitmap with enabled Log levels. Use X_LEVEL macros */
extern int debug_type;

void log_init();

/********************************************************************************
Similar with printf syntax + 1 extra argument at the beginning.
Eg: DBG_PRINT(INFO_LEVEL, "Message X, %d", number);
Printing is done if the X_LEVEL set is set in the debug_type bitmap and if
debug_cfg is set to PRINT_TO_CONSOLE
**********************************************************************************/
void DBG_PRINT(int type, const char *fmt, ...);

#endif //#ifndef LOG_H_

