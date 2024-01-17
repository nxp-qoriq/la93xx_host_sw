// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2023-2024 NXP
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "log.h"

#ifdef THREADING
#include <pthread.h>
#else
#define pthread_spin_init(...)
#define pthread_spin_lock(...)
#define pthread_spin_unlock(...)
#define pthread_spinlock_t int
#endif

pthread_spinlock_t dbgPrintSpinlock;



int debug_cfg   = PRINT_TO_CONSOLE;
int debug_type  = ERR_LEVEL | WARN_LEVEL | INFO_LEVEL | DBG_LEVEL | PKT_PRINT;
char dbg_print_buff[255];

void log_init() {
	pthread_spin_init(&dbgPrintSpinlock, PTHREAD_PROCESS_PRIVATE);
}
void DBG_PRINT(int type, const char *fmt, ...) {
  va_list args;


  if (debug_type & type) {
    va_start(args, fmt);

    pthread_spin_lock(&dbgPrintSpinlock);

    vsprintf(dbg_print_buff, fmt, args);
    va_end(args);
    if (debug_cfg & PRINT_TO_CONSOLE) {
      printf("%s", dbg_print_buff);
    }

    pthread_spin_unlock(&dbgPrintSpinlock);
  }
}
