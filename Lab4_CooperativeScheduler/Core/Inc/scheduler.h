#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

#define TICK_MS UINT32_C(10)
#define SCH_MAX_TASKS 16U
#define NO_TASK_ID UINT32_C(0)
#define SCH_MAX_INTERVAL_TICKS UINT32_C(0x7FFFFFFF)

#define SCH_OK UINT8_C(0)
#define SCH_ERR_TASK_NOT_FOUND UINT8_C(1)

static inline uint32_t MS_TO_TICK(uint32_t milliseconds)
{
    return milliseconds / TICK_MS + (milliseconds % TICK_MS != 0U);
}

void SCH_Init(void);
void SCH_Update(void);
void SCH_Dispatch_Tasks(void);

uint32_t SCH_Add_Task(void (*pFunction)(void),
                      uint32_t DELAY,
                      uint32_t PERIOD);

uint8_t SCH_Delete_Task(uint32_t taskID);
uint32_t SCH_Get_Time(void);

#endif
