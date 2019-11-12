#ifndef __thermopile_H
#define __thermopile_H

#include "stdint.h"
#include "stdint.h"
#include "cmsis_os.h"

osThreadId_t thermopileTaskHandle;

//#ifdef __cplusplus
// extern "C" {
// }

void ThermopileTask(void);
void Setup_LMP91051(void);

//#endif
#endif
