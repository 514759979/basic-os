/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "basic_os.h"

uint32_t count_timer1 = 0;
uint32_t count_timer2 = 0;
uint32_t count_timer3 = 0;
uint32_t count_timer4 = 0;
uint32_t count_timer5 = 0;
uint32_t count_timer6 = 0;

uint32_t count_timer_total = 0;

char timer_name[32];
char timer_name_id = 0;
int16_t timer_id = 0;
bool flag_timer_start = 0;
bool flag_timer_stop = 0;
bool flag_timer_reset = 0;

/* private functions -------------------------------------------------------- */
void _func_poll_7(void *parameter)
{
    while (1)
    {
        if (flag_timer_start || flag_timer_stop || flag_timer_reset)
        {
            memset(timer_name, 0, 32);
            sprintf(timer_name, "timer%d", timer_name_id);
            timer_id = bos_timer_get_id(timer_name);
        }
        
        if (flag_timer_start)
        {
            flag_timer_start = false;
            bos_timer_start(timer_id, 1000);
        }
        
        if (flag_timer_stop)
        {
            flag_timer_stop = false;
            bos_timer_stop(timer_id);
        }
        
        if (flag_timer_reset)
        {
            flag_timer_reset = false;
            bos_timer_reset(timer_id, 1000);
        }

        bos_delay_ms(10);
    }
}

//bos_task_export(poll7, _func_poll_7, 2, NULL);

void _timer_func_1(void *parameter)
{
    count_timer1 ++;
    count_timer_total ++;
}

void _timer_func_2(void *parameter)
{
    count_timer2 ++;
    count_timer_total ++;
}

void _timer_func_3(void *parameter)
{
    count_timer3 ++;
    count_timer_total ++;
}

void _timer_func_4(void *parameter)
{
    count_timer4 ++;
    count_timer_total ++;
}

void _timer_func_5(void *parameter)
{
    count_timer5 ++;
    count_timer_total ++;
}

void _timer_func_6(void *parameter)
{
    count_timer6 ++;
    count_timer_total ++;
}

bos_timer_export(timer1, _timer_func_1, false, NULL);
bos_timer_export(timer2, _timer_func_2, false, NULL);
bos_timer_export(timer3, _timer_func_3, false, NULL);
bos_timer_export(timer4, _timer_func_4, false, NULL);
bos_timer_export(timer5, _timer_func_5, false, NULL);
bos_timer_export(timer6, _timer_func_6, false, NULL);

/* ----------------------------- end of file -------------------------------- */
