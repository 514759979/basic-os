/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "stm32g0xx_hal.h"


extern UART_HandleTypeDef huart;

/* public functions --------------------------------------------------------- */
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
    while (1)
    {
    }
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
    while (1)
    {
    }
}

/**
  * @brief This function handles USART3 and USART4 interrupts.
  */
void USART3_4_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart);
}

/* ----------------------------- end of file -------------------------------- */
