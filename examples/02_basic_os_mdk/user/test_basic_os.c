/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "basic_os.h"

uint32_t count_poll1 = 0;
uint32_t count_poll2 = 0;
uint32_t count_poll3 = 0;
uint32_t count_poll4 = 0;
uint32_t count_poll5 = 0;
uint32_t count_poll6 = 0;

uint32_t count_total = 0;

/* private functions -------------------------------------------------------- */
void _func_poll_1(void *parameter)
{
    while (1)
    {
        count_poll1 ++;
        count_total ++;
        bos_delay_ms(10);
    }
}

void _func_poll_2(void *parameter)
{
    while (1)
    {
        count_poll2 ++;
        count_total ++;
        bos_task_yield();
    }
}

void _func_poll_3(void *parameter)
{
    while (1)
    {
        count_poll3 ++;
        count_total ++;
        bos_task_yield();
    }
}

void _func_poll_4(void *parameter)
{
    while (1)
    {
        count_poll4 ++;
        count_total ++;
        bos_task_yield();
    }
}

void _func_poll_5(void *parameter)
{
    while (1)
    {
        count_poll5 ++;
        count_total ++;
        bos_task_yield();
    }
}

void _func_poll_6(void *parameter)
{
    while (1)
    {
        count_poll6 ++;
        count_total ++;
        bos_delay_ms(10);
    }
}

bos_task_export(poll1, _func_poll_1, 2, NULL);
bos_task_export(poll2, _func_poll_2, 2, NULL);
bos_task_export(poll3, _func_poll_3, 2, NULL);
bos_task_export(poll4, _func_poll_4, 2, NULL);
bos_task_export(poll5, _func_poll_5, 2, NULL);
bos_task_export(poll6, _func_poll_6, 2, NULL);

/* ----------------------------- end of file -------------------------------- */
