#ifndef WATCHDOG_TASK_H
#define WATCHDOG_TASK_H

#include "stdint.h"
#include "cmsis_os.h"
#include "captivate_config.h"

void watchdogTask(void *argument);

#endif
