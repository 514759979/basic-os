/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* private variables -------------------------------------------------------- */
static uint32_t sys_time_ms = 0;

/* public functions --------------------------------------------------------- */
/**
  * @brief  SysTick ISR function.
  * @retval None
  */
uint32_t elab_time_ms(void)
{
    return sys_time_ms;
}

/* private functions -------------------------------------------------------- */
/**
  * @brief  SysTick ISR function.
  * @retval None
  */
void SysTick_Handler(void)
{
    sys_time_ms ++;
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
