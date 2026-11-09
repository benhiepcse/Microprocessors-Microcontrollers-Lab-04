#include "tasks.h"
#include "main.h"
#include "scheduler.h"
#include "usart.h"

#include <inttypes.h>
#include <stdio.h>

#define TASKS_UART_TIMEOUT_MS 5U

static volatile uint32_t log_format_error_count;
static volatile uint32_t log_uart_error_count;
static volatile HAL_StatusTypeDef log_last_uart_status = HAL_OK;

void Tasks_LogTimestamp(const char *task_name)
{
    uint32_t timestamp_ms = SCH_Get_Time();
    char message[40];

    int length = snprintf(message, sizeof(message),
                          "[t=%" PRIu32 " ms] %s\r\n",
                          timestamp_ms, task_name);

    if (length < 0 || (size_t)length >= sizeof(message))
    {
        ++log_format_error_count;
        return;
    }

    HAL_StatusTypeDef status = HAL_UART_Transmit(
        &huart1, (uint8_t *)message, (uint16_t)length,
        TASKS_UART_TIMEOUT_MS);

    log_last_uart_status = status;

    if (status != HAL_OK)
    {
        ++log_uart_error_count;
    }
}

void Tasks_RedLed(void)
{
    HAL_GPIO_TogglePin(RED_GPIO_Port, RED_Pin);
    Tasks_LogTimestamp("T1");
}
