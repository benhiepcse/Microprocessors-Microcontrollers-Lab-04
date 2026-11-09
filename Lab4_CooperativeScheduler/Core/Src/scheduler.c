#include "scheduler.h"
#include "main.h"

#include <stddef.h>

#define SCH_NO_SLOT UINT8_MAX
#define SCH_HALF_TICK_RANGE UINT32_C(0x80000000)

typedef enum
{
    SLOT_FREE,
    SLOT_QUEUED,
    SLOT_RUNNING,
    SLOT_CANCELLED
} SlotState;

typedef struct
{
    void (*callback)(void);
    uint32_t task_id;
    uint32_t deadline_tick;
    uint32_t period_ticks;
    uint8_t next_slot;
    SlotState state;
} SchedulerTask;

static SchedulerTask task_pool[SCH_MAX_TASKS];
static volatile uint32_t current_tick;
static uint32_t next_task_id = UINT32_C(1);
static uint8_t head_slot = SCH_NO_SLOT;
static uint8_t running_slot = SCH_NO_SLOT;

static uint32_t read_tick(void)
{
    uint32_t saved_primask = __get_PRIMASK();
    __disable_irq();

    uint32_t tick = current_tick;

    __set_PRIMASK(saved_primask);
    return tick;
}

static void release_slot(uint8_t slot)
{
    task_pool[slot] = (SchedulerTask){
        .next_slot = SCH_NO_SLOT,
        .state = SLOT_FREE
    };
}

static void insert_task(uint8_t slot)
{
    SchedulerTask *task = &task_pool[slot];
    uint8_t *link = &head_slot;

    while (*link != SCH_NO_SLOT)
    {
        SchedulerTask *queued = &task_pool[*link];

        if (task->deadline_tick == queued->deadline_tick)
        {
            if (task->task_id < queued->task_id)
            {
                break;
            }
        }
        else if (queued->deadline_tick - task->deadline_tick <
                 SCH_HALF_TICK_RANGE)
        {
            break;
        }

        link = &queued->next_slot;
    }

    task->next_slot = *link;
    task->state = SLOT_QUEUED;
    *link = slot;
}

void SCH_Init(void)
{
    current_tick = 0U;
    next_task_id = UINT32_C(1);
    head_slot = SCH_NO_SLOT;
    running_slot = SCH_NO_SLOT;

    for (uint8_t slot = 0U; slot < SCH_MAX_TASKS; ++slot)
    {
        release_slot(slot);
    }
}

void SCH_Update(void)
{
    ++current_tick;
}

uint32_t SCH_Add_Task(void (*pFunction)(void),
                      uint32_t DELAY,
                      uint32_t PERIOD)
{
    if (pFunction == NULL || DELAY > SCH_MAX_INTERVAL_TICKS ||
        PERIOD > SCH_MAX_INTERVAL_TICKS || next_task_id == NO_TASK_ID)
    {
        return NO_TASK_ID;
    }

    for (uint8_t slot = 0U; slot < SCH_MAX_TASKS; ++slot)
    {
        if (task_pool[slot].state == SLOT_FREE)
        {
            uint32_t task_id = next_task_id++;

            task_pool[slot].callback = pFunction;
            task_pool[slot].task_id = task_id;
            task_pool[slot].deadline_tick = read_tick() + DELAY;
            task_pool[slot].period_ticks = PERIOD;

            insert_task(slot);
            return task_id;
        }
    }

    return NO_TASK_ID;
}

uint8_t SCH_Delete_Task(uint32_t taskID)
{
    if (taskID == NO_TASK_ID)
    {
        return SCH_ERR_TASK_NOT_FOUND;
    }

    if (running_slot != SCH_NO_SLOT)
    {
        SchedulerTask *running = &task_pool[running_slot];

        if (running->task_id == taskID && running->state == SLOT_RUNNING)
        {
            running->state = SLOT_CANCELLED;
            return SCH_OK;
        }
    }

    uint8_t *link = &head_slot;

    while (*link != SCH_NO_SLOT)
    {
        uint8_t slot = *link;
        SchedulerTask *task = &task_pool[slot];

        if (task->task_id == taskID)
        {
            *link = task->next_slot;
            release_slot(slot);
            return SCH_OK;
        }

        link = &task->next_slot;
    }

    return SCH_ERR_TASK_NOT_FOUND;
}

void SCH_Dispatch_Tasks(void)
{
    if (head_slot == SCH_NO_SLOT)
    {
        return;
    }

    uint8_t slot = head_slot;
    SchedulerTask *task = &task_pool[slot];
    uint32_t now = read_tick();

    if (now - task->deadline_tick >= SCH_HALF_TICK_RANGE)
    {
        return;
    }

    head_slot = task->next_slot;
    task->next_slot = SCH_NO_SLOT;
    task->state = SLOT_RUNNING;
    running_slot = slot;

    task->callback();

    running_slot = SCH_NO_SLOT;

    if (task->state == SLOT_CANCELLED || task->period_ticks == 0U)
    {
        release_slot(slot);
        return;
    }

    task->deadline_tick += task->period_ticks;
    insert_task(slot);
}

uint32_t SCH_Get_Time(void)
{
    return read_tick() * TICK_MS;
}
