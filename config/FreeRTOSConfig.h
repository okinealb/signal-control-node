#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <assert.h>

#define configTICK_RATE_HZ 100
#define configUSE_PREEMPTION 1
#define configUSE_TIME_SLICING 0
#define configMAX_PRIORITIES 4
#define configMINIMAL_STACK_SIZE 256
#define configMAX_TASK_NAME_LEN 16
#define configTICK_TYPE_WIDTH_IN_BITS TICK_TYPE_WIDTH_32_BITS
#define configIDLE_SHOULD_YIELD 1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0

#define configSUPPORT_STATIC_ALLOCATION 1
#define configSUPPORT_DYNAMIC_ALLOCATION 0
#define configKERNEL_PROVIDED_STATIC_MEMORY 1
#define configUSE_TIMERS 0
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define configCHECK_FOR_STACK_OVERFLOW 0
#define configQUEUE_REGISTRY_SIZE 0

#define INCLUDE_vTaskDelay 1
#define configASSERT(condition) assert(condition)

#endif
