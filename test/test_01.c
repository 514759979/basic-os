#include "test.h"
#include "eventos.h"

static uint8_t stack_1[256];
static uint8_t stack_2[256];
static uint8_t stack_3[256];
static uint8_t stack_4[256];
static uint8_t stack_5[256];
static uint8_t stack_6[256];
static uint8_t stack_7[256];
static uint8_t stack_8[256];

eos_task_t task1;
eos_task_t task2;
eos_task_t task3;
eos_task_t task4;
eos_task_t task5;
eos_task_t task6;
eos_task_t task7;
eos_task_t task8;

static void task_entry_yield_1(void);
static void task_entry_yield_2(void);
static void task_entry_yield_3(void);
static void task_entry_yield_4(void);
static void task_entry_yield_5(void);
static void task_entry_yield_6(void);
static void task_entry_yield_7(void);
static void task_entry_yield_8(void);

uint32_t count_task[8];

void test_start(void)
{
    eos_task_start(&task1, task_entry_yield_1, 2, stack_1, sizeof(stack_1));
    eos_task_start(&task2, task_entry_yield_2, 2, stack_2, sizeof(stack_2));
    eos_task_start(&task3, task_entry_yield_3, 2, stack_3, sizeof(stack_3));
    eos_task_start(&task4, task_entry_yield_4, 3, stack_4, sizeof(stack_4));
    eos_task_start(&task5, task_entry_yield_5, 2, stack_5, sizeof(stack_5));
    eos_task_start(&task6, task_entry_yield_6, 2, stack_6, sizeof(stack_6));
    eos_task_start(&task7, task_entry_yield_7, 2, stack_7, sizeof(stack_7));
    eos_task_start(&task8, task_entry_yield_8, 1, stack_8, sizeof(stack_8));
    
    for (uint32_t i = 0; i < 8; i ++)
    {
        count_task[i] = 0;
    }
}

uint32_t count_yelid = 0;
uint32_t count_sec = 0;
void task_entry_yield_1(void)
{
    while (1)
    {
        count_yelid ++;
        count_task[0] ++;
        count_sec = count_yelid / eos_time();
        eos_task_yield();
    }
}

void task_entry_yield_2(void)
{
    while (1)
    {
        count_yelid ++;
        count_task[1] ++;
        eos_task_yield();
    }
}

void task_entry_yield_3(void)
{
    while (1)
    {
        count_yelid ++;
        count_task[2] ++;
        eos_task_yield();
    }
}

void task_entry_yield_4(void)
{
    while (1)
    {
        count_yelid ++;
        count_task[3] ++;
        
        eos_task_yield();
        
        eos_delay_ms(100000);
    }
}

void task_entry_yield_5(void)
{
    while (1)
    {
        count_yelid ++;
        count_task[4] ++;
        eos_task_yield();
    }
}

void task_entry_yield_6(void)
{
    while (1)
    {
        count_yelid ++;
        count_task[5] ++;
        eos_task_yield();
    }
}

void task_entry_yield_7(void)
{
    while (1)
    {
        count_yelid ++;
        count_task[6] ++;
        eos_task_yield();
    }
}

void task_entry_yield_8(void)
{
    while (1)
    {
        count_yelid ++;
        count_task[7] ++;
        
        eos_task_yield();
        
        eos_delay_ms(100000);
    }
}

