/*
 * EventOS Basic
 * Copyright (c) 2021, EventOS Team, <event-os@outlook.com>
 *
 * SPDX-License-Identifier: MIT
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the 'Software'), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS 
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.event-os.cn
 * https://github.com/event-os/eventos-basic
 * https://gitee.com/event-os/eventos-basic
 * 
 * Change Logs:
 * Date           Author        Notes
 * 2021-11-23     DogMing       V0.1.0
 */

/* include ------------------------------------------------------------------ */
#include "eventos.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* assert ------------------------------------------------------------------- */
#if (EOS_USE_ASSERT != 0)
#define EOS_ASSERT(test_)                                                      \
    do {                                                                       \
        if (!(test_))                                                          \
        {                                                                      \
            eos_critical_enter();                                              \
            eos_port_assert(__LINE__);                                         \
        }                                                                      \
    } while (0)
#else
#define EOS_ASSERT(test_)               ((void)0)
#endif

/* eos task ----------------------------------------------------------------- */
eos_task_t *volatile eos_current;
eos_task_t *volatile eos_next;

enum
{
    EosTaskState_Ready = 0,
    EosTaskState_Running,
    EosTaskState_Blocked,
    EosTaskState_Suspended,

    EosTaskState_Max,
};

typedef struct eos_tag
{
    eos_task_t * list;
    eos_timer_t * timers;
    uint8_t id_count;

    uint32_t time;
    uint32_t time_out_min;
    uint32_t time_offset;
    uint32_t cpu_usage_count;
    uint32_t timer_count                : 16;
    uint32_t timer_id_count             : 10;
} eos_t;

eos_t eos;
static eos_task_t task_idle;

/* macro -------------------------------------------------------------------- */
#define EOS_MS_NUM_30DAY                (2592000000U)
#define EOS_MS_NUM_15DAY                (1296000000U)

/* private function --------------------------------------------------------- */
static void eos_sheduler(void);
static void task_entry_idle(void *parameter);
static void eos_critical_enter(void);
static void eos_critical_exit(void);

/* public function ---------------------------------------------------------- */
void eos_init(void *stack_idle, uint32_t size)
{
    eos_critical_enter();

    // Set PendSV to be the lowest priority.
    *(uint32_t volatile *)0xE000ED20 |= (0xFFU << 16U);

    eos.time = 0;
    eos.time_offset = 0;
    eos.timer_id_count = 0;
    eos.timers = (eos_timer_t *)0;
    eos.time_out_min = UINT32_MAX;
    eos.list = NULL;

    eos_current = NULL;
    eos_next = &task_idle;

    eos_critical_exit();

    // Start the idle task.
    eos_task_start(&task_idle, task_entry_idle, 1, stack_idle, size, NULL);
}

void eos_task_start(eos_task_t * const me,
                    eos_func_t func,
                    uint8_t priority,
                    void *stack_addr,
                    uint32_t stack_size,
                    void *parameter)
{
    EOS_ASSERT(priority <= EOS_MAX_PRIORITY && priority != 0);
    EOS_ASSERT(eos_current != &task_idle);
    
    eos_critical_enter();
    uint32_t mod = (uint32_t)me->stack % 4;
    if (mod == 0)
    {
        me->stack = stack_addr;
        me->size = stack_size;
    }
    else
    {
        me->stack = (void *)((uint32_t)stack_addr - mod);
        me->size = stack_size - 4;
    }
    /* pre-fill the unused part of the stack with 0xDEADBEEF */
    for (uint32_t i = 0; i < (me->size / 4); i ++)
    {
        ((uint32_t *)me->stack)[i] = 0xDEADBEEFU;
    }
    eos_critical_exit();
    
    /* round down the stack top to the 8-byte boundary
     * NOTE: ARM Cortex-M stack grows down from hi -> low memory
     */
    uint32_t *sp = (uint32_t *)((((uint32_t)stack_addr + stack_size) >> 3U) << 3U);

    *(-- sp) = (uint32_t)(1 << 24);            /* xPSR, Set Bit24(Thumb Mode) to 1. */
    *(-- sp) = (uint32_t)func;                 /* the entry function (PC) */
    *(-- sp) = (uint32_t)func;                 /* R14(LR) */
    *(-- sp) = (uint32_t)0x12121212u;          /* R12 */
    *(-- sp) = (uint32_t)0x03030303u;          /* R3 */
    *(-- sp) = (uint32_t)0x02020202u;          /* r2 */
    *(-- sp) = (uint32_t)0x01010101u;          /* R1 */
    *(-- sp) = (uint32_t)parameter;            /* r0 */
    /* additionally, fake registers r4-r11 */
    *(-- sp) = (uint32_t)0x11111111u;          /* r11 */
    *(-- sp) = (uint32_t)0x10101010u;          /* r10 */
    *(-- sp) = (uint32_t)0x09090909u;          /* r9 */
    *(-- sp) = (uint32_t)0x08080808u;          /* r8 */
    *(-- sp) = (uint32_t)0x07070707u;          /* r7 */
    *(-- sp) = (uint32_t)0x06060606u;          /* r6 */
    *(-- sp) = (uint32_t)0x05050505u;          /* r5 */
    *(-- sp) = (uint32_t)0x04040404u;          /* r4 */

    /* save the top of the stack in the task's attibute */
    me->sp = sp;

    eos_critical_enter();
    me->state = EosTaskState_Ready;
    me->state_bkp = EosTaskState_Max;
    
    eos_task_t *list = eos.list;
    
    if (me == &task_idle)
    {
        me->priority = 0;
        eos.list = me;
        me->next = NULL;
        me->prev = NULL;
    }
    else
    {
        me->priority = priority;
        
        // Insert into the task list from high-priority to low.
        while (list != NULL)
        {
            /*  The first task's priority is low than the current task, insert
                it at the head of the list. */
            if (list->priority < priority && list->prev == NULL)
            {
                me->next = list;
                me->prev = list->prev;
                list->prev = me;
                eos.list = me;
                break;
            }

            /* Insert the task at the tail of the list. */
            if ((list->priority >= priority && list->next->priority < priority))
            {
                me->next = list->next;
                list->next = me;
                me->next->prev = me;
                me->prev = list;
                break;
            }

            list = list->next;
        }
    }
    
    eos_critical_exit();
}

void eos_run(void)
{
    eos_hook_start();
    
    eos_sheduler();
}

void eos_tick(void)
{
    eos_critical_enter();
    eos.time += EOS_TICK_MS;
    eos_critical_exit();
}

void eos_delay_ms(uint32_t time_ms)
{
    if (time_ms == 0)
    {
        return;
    }
    
    EOS_ASSERT(time_ms <= EOS_MS_NUM_30DAY);

    /* never call eos_delay_ms in the idle task. */
    EOS_ASSERT(eos_current != &task_idle);

    eos_critical_enter();
    eos_current->timeout = eos.time + time_ms;
    eos_current->state = EosTaskState_Blocked;
    eos_critical_exit();
    
    eos_sheduler();
}

void eos_task_exit(void)
{
    eos_critical_enter();
    // Find the task in the list
    eos_task_t *list = eos.list;
    eos_task_t *former = NULL;
    while (list != NULL)
    {
        if (list == eos_current)
        {
            if (former == NULL)
            {
                eos.list = list->next;
            }
            else
            {
                former->next = list->next;
            }
            break;
        }
        list = list->next;
    }
    eos_critical_exit();
    
    eos_sheduler();
}

void eos_task_yield(void)
{
    eos_critical_enter();
    
    eos_task_t *head = NULL, *tail = NULL;
    eos_task_t *list = eos_current->next;
    eos_task_t *next = eos_current;
    tail = eos_current;
    bool next_found = false;
    while (list->priority == eos_current->priority)
    {
        if (list->state == EosTaskState_Ready &&
            list->priority == eos_current->priority &&
            next_found == false)
        {
            next = list;
            next_found = true;
        }
        if (list->next->priority != eos_current->priority)
        {
            tail = list;
            break;
        }

        list = list->next;
    }

    list = eos_current->prev;
    head = eos_current;
    while (list != NULL && list->priority == eos_current->priority)
    {
        if (list->state == EosTaskState_Ready &&
            list->priority == eos_current->priority &&
            next_found == false)
        {
            next = list;
        }
        if (list->prev->priority != eos_current->priority)
        {
            head = list;
            break;
        }
        if (list->prev == NULL)
        {
            head = list;
            break;
        }

        list = list->prev;
    }

    list = head;
    do
    {
        list->state_bkp = list->state;
        list->state = EosTaskState_Suspended;
        list = list->next;
    } while (list->prev != tail);

    next->state = next->state_bkp;

    eos_sheduler();

    list = head;
    do
    {
        list->state = list->state_bkp;
        list = list->next;
    } while (list->prev != tail);

    eos_critical_exit();
}

eos_task_t *eos_get_task(void)
{
    return eos_current;
}

static void eos_sheduler(void)
{
    eos_critical_enter();
    eos_task_t *list = eos.list;
    while (list != NULL)
    {
        if (list->state == EosTaskState_Ready)
        {
            eos_next = list;
            if (eos_next != eos_current)
            {
                *(uint32_t volatile *)0xE000ED04 = (1U << 28);
            }
            break;
        }
        list = list->next;
    }
    eos_critical_exit();
}

uint32_t eos_time(void)
{
    eos_critical_enter();
    uint32_t time_offset = eos.time_offset;
    eos_critical_exit();

    return (time_offset + eos.time);
}

/* Soft timer --------------------------------------------------------------- */
int32_t eos_timer_start(eos_timer_t * const me,
                        uint32_t time_ms,
                        bool oneshoot,
                        eos_func_t callback,
                        void *parameter)
{
    EOS_ASSERT(time_ms <= EOS_MS_NUM_30DAY);

    eos_critical_enter();

    // Check the timer is repeated.
    eos_timer_t *list = eos.timers;
    while (list != (eos_timer_t *)0)
    {
        EOS_ASSERT(list != me);
        EOS_ASSERT(list->callback != callback);
        list = list->next;
    }

    // Timer data.
    me->time = time_ms;
    me->time_out = eos.time + time_ms;
    me->callback = callback;
    me->id = eos.timer_id_count ++;
    me->oneshoot = oneshoot == false ? 0 : 1;
    me->running = 1;
    me->parameter = parameter;

    // Add the timer to the list.
    me->next = eos.timers;
    eos.timers = me;
    
    if (eos.time_out_min > me->time_out)
    {
        eos.time_out_min = me->time_out;
    }

    eos_critical_exit();

    return EOS_OK;
}

void eos_timer_pause(uint16_t timer_id)
{
    eos_critical_enter();

    eos_timer_t *list = eos.timers;
    uint32_t time_out_min = UINT32_MAX;
    bool timer_id_existed = false;
    while (list != (eos_timer_t *)0)
    {
        if (list->id == timer_id)
        {
            list->running = 0;
            timer_id_existed = true;
        }
        else if (list->running != 0 && time_out_min > list->time_out)
        {
            time_out_min = list->time_out;
        }
        list = list->next;
    }
    eos.time_out_min = time_out_min;
    eos_critical_exit();
    
    // not found.
    EOS_ASSERT(timer_id_existed);
}

void eos_timer_continue(uint16_t timer_id)
{
    eos_critical_enter();

    eos_timer_t *list = eos.timers;
    while (list != (eos_timer_t *)0)
    {
        if (list->id == timer_id)
        {
            list->running = 1;
            if (eos.time_out_min > list->time_out)
            {
                eos.time_out_min = list->time_out;
            }
            eos_critical_exit();
            return;
        }
        list = list->next;
    }

    // not found.
    EOS_ASSERT(0);
}

void eos_timer_delete(uint16_t timer_id)
{
    eos_critical_enter();

    eos_timer_t *list = eos.timers;
    eos_timer_t *last = (eos_timer_t *)0;
    while (list != (eos_timer_t *)0)
    {
        if (list->id == timer_id)
        {
            list->running = 1;
            if (last == (eos_timer_t *)0)
            {
                eos.timers = list->next;
            }
            else
            {
                last->next = list->next;
            }
            eos_critical_exit();
            return;
        }
        last = list;
        list = list->next;
    }

    // not found.
    EOS_ASSERT(0);
}

void eos_timer_reset(uint16_t timer_id)
{
    eos_critical_enter();

    eos_timer_t *list = eos.timers;
    while (list != (eos_timer_t *)0)
    {
        if (list->id == timer_id)
        {
            list->running = 1;
            list->time_out = eos.time + list->time;
            eos_critical_exit();
            return;
        }
        list = list->next;
    }

    // not found.
    EOS_ASSERT(0);
}

/* 统计功能 ------------------------------------------------------------------ */
// 任务的堆栈使用率
#if (EOS_USE_STACK_USAGE != 0)
uint8_t eos_task_stack_usage(eos_task_t * const me)
{
    EOS_ASSERT(me != NULL);

    return me->usage;
}
#endif

// 任务的CPU使用率
#if (EOS_USE_CPU_USAGE != 0)
uint8_t eos_task_cpu_usage(eos_task_t * const me)
{
    EOS_ASSERT(me != NULL);

    return me->cpu_usage;
}

// 监控函数，放进一个单独的定时器中断函数，中断频率为SysTick的10-20倍。
void eos_cpu_usage_monitor(void)
{
    // CPU使用率的计算
    eos.cpu_usage_count ++;
    eos_current->cpu_usage_count ++;
    if (eos.cpu_usage_count >= 10000)
    {
        eos_task_t *list = eos.list;
        while (list != NULL)
        {
            list->cpu_usage = list->cpu_usage_count * 100 / eos.cpu_usage_count;
            list->cpu_usage_count = 0;
            list = list->next;
        }
        eos.cpu_usage_count = 0;
    }
}
#endif

/* Interrupt service function ----------------------------------------------- */
#if (defined __CC_ARM)
__asm void PendSV_Handler(void)
{
    IMPORT        eos_current           /* extern variable */
    IMPORT        eos_next              /* extern variable */

    CPSID         i                     /* disable interrupts (set PRIMASK) */

    LDR           r1,=eos_current       /* if (eos_current != 0)
    { */
    LDR           r1,[r1,#0x00]
#if (__TARGET_ARCH_THUMB == 3)          /* Cortex-M0/M0+/M1 (v6-M, v6S-M)? */
    CMP           r1, #0
    BEQ           PendSV_restore
    NOP
    PUSH          {r4-r7}              /*     push r4-r11 into stack */
    MOV           r4, r8
    MOV           r5, r9
    MOV           r6, r10
    MOV           r7, r11
    PUSH          {r4-r7}
#else
    CBZ           r1,PendSV_restore
    PUSH          {r4-r11}              /*     push r4-r11 into stack */
#endif
  
    LDR           r1,=eos_current       /*     eos_current->sp = sp; */
    LDR           r1,[r1,#0x00]
#if (__TARGET_ARCH_THUMB == 3)          /* Cortex-M0/M0+/M1 (v6-M, v6S-M)? */
    MOV           r2, SP
    STR           r2,[r1,#0x00]         /* } */
#else
    STR           SP,[r1,#0x00]
#endif
    
PendSV_restore
    LDR           r1,=eos_next          /* sp = eos_next->sp; */
    LDR           r1,[r1,#0x00]
#if (__TARGET_ARCH_THUMB == 3)          /* Cortex-M0/M0+/M1 (v6-M, v6S-M)? */
    LDR           r0,[r1,#0x00]
    MOV           SP, r0
#else
    LDR           sp,[r1,#0x00]
#endif
    LDR           r1,=eos_next          /* eos_current = eos_next; */
    LDR           r1,[r1,#0x00]
    LDR           r2,=eos_current
    STR           r1,[r2,#0x00]
#if (__TARGET_ARCH_THUMB == 3)          /* Cortex-M0/M0+/M1 (v6-M, v6S-M)? */
    POP           {r4-r7}
    MOV           r8, r4
    MOV           r9, r5
    MOV           r10,r6
    MOV           r11,r7
    POP           {r4-r7}
#else
    POP           {r4-r11}              /* pop registers r4-r11 */
#endif
    CPSIE         i                     /* enable interrupts (clear PRIMASK) */
    BX            lr                    /* return to the next task */
}
#endif

/*******************************************************************************
* NOTE:
* The inline GNU assembler does not accept mnemonics MOVS, LSRS and ADDS,
* but for Cortex-M0/M0+/M1 the mnemonics MOV, LSR and ADD always set the
* condition flags in the PSR.
*******************************************************************************/
#if ((defined __GNUC__) || (defined __ICCARM__))
#if (defined __GNUC__)
__attribute__ ((naked))
#endif
#if ((defined __ICCARM__))
__stackless
#endif
void PendSV_Handler(void)
{
    __asm volatile
    (
    "CPSID         i                \n" /* disable interrupts (set PRIMASK) */
    "LDR           r1,=eos_current  \n"  /* if (eos_current != 0)
    { */
    "LDR           r1,[r1,#0x00]    \n"

#if (__TARGET_ARCH_THUMB == 3)          /* Cortex-M0/M0+/M1 (v6-M, v6S-M)? */
    "CMP           r1, #0           \n"
    "BEQ           restore          \n"
    "NOP                            \n"
    "PUSH          {r4-r7}          \n" /*      push r4-r11 into stack */
    "MOV           r4, r8           \n"
    "MOV           r5, r9           \n"
    "MOV           r6, r10          \n"
    "MOV           r7, r11          \n"
    "PUSH          {r4-r7}          \n"
#else
    "CBZ           r1,restore       \n"
    "PUSH          {r4-r11}         \n"
#endif

    "LDR           r1,=eos_current  \n"  /*     eos_current->sp = sp; */
    "LDR           r1,[r1,#0x00]    \n"

#if (__TARGET_ARCH_THUMB == 3)          /* Cortex-M0/M0+/M1 (v6-M, v6S-M)? */
    "MOV           r2, SP           \n"
    "STR           r2,[r1,#0x00]    \n"  /* } */
#else
    "STR           sp,[r1,#0x00]    \n"  /* } */
#endif

    "restore: LDR r1,=eos_next      \n"  /* sp = eos_next->sp; */
    "LDR           r1,[r1,#0x00]    \n"
#if (__TARGET_ARCH_THUMB == 3)          /* Cortex-M0/M0+/M1 (v6-M, v6S-M)? */
    "LDR           r0,[r1,#0x00]    \n"
    "MOV           SP, r0           \n"
#else
    "LDR           sp,[r1,#0x00]    \n"
#endif

    "LDR           r1,=eos_next     \n"  /* eos_current = eos_next; */
    "LDR           r1,[r1,#0x00]    \n"
    "LDR           r2,=eos_current  \n"
    "STR           r1,[r2,#0x00]    \n"
#if (__TARGET_ARCH_THUMB == 3)          /* Cortex-M0/M0+/M1 (v6-M, v6S-M)? */
    "POP           {r4-r7}          \n"
    "MOV           r8, r4           \n"
    "MOV           r9, r5           \n"
    "MOV           r10,r6           \n"
    "MOV           r11,r7           \n"
    "POP           {r4-r7}          \n"
#else
    "POP           {r4-r11}         \n"  /* pop registers r4-r11 */
#endif
    "CPSIE         i                \n"  /* enable interrupts (clear PRIMASK) */
    "BX            lr               \n"   /* return to the next task */
    );
}
#endif

/* private function --------------------------------------------------------- */
static void task_entry_idle(void *parameter)
{
    (void)parameter;
    
    eos_critical_enter();
    
    while (1)
    {
        eos_task_t *list;
#if (EOS_USE_STACK_USAGE != 0)
        // 堆栈使用率的计算
        uint8_t usage = 0;
        uint32_t *stack;
        uint32_t size_used = 0;
        list = eos.list;
        while (list != NULL)
        {
            size_used = 0;
            stack = (uint32_t *)list->stack;
            for (uint32_t m = 0; m < (list->size / 4); m ++)
            {
                if (stack[m] == 0xDEADBEEFU)
                {
                    size_used += 4;
                }
                else
                {
                    break;
                }
            }
            usage = 100 - (size_used * 100 / list->size);
            list->usage = usage;
            list = list->next;
        }
#endif
        /* check all the task are timeout or not. */
        list = eos.list;
        bool sheduler_en = false;
        while (list != NULL)
        {
            if (list->state == EosTaskState_Blocked)
            {
                if (eos.time >= list->timeout)
                {
                    list->state = EosTaskState_Ready;
                    sheduler_en = true;
                }
            }

            list = list->next;
        }
        if (sheduler_en == true)
        {
            eos_critical_exit();
            eos_sheduler();
            eos_critical_enter();
        }

        if (eos.time >= EOS_MS_NUM_15DAY)
        {
            list = eos.list;
            while (list != NULL)
            {
                if (list->state == EosTaskState_Blocked)
                {
                    list->timeout -= eos.time;
                }
                list = list->next;
            }
            
            // Adjust all timmer's timing.
            eos_timer_t *list_tim = eos.timers;
            while (list_tim != (eos_timer_t *)0)
            {
                if (list_tim->running != 0)
                {
                    list_tim->time_out -= eos.time;
                }
                list_tim = list_tim->next;
            }
            eos.time_out_min -= eos.time;

            eos.time_offset += eos.time;
            eos.time = 0;
        }

        // if any timer is timeout
        if (eos.time >= eos.time_out_min)
        {
            eos_timer_t *list;
            // Find the time-out timers and excute the handlers.
            list = eos.timers;
            while (list != (eos_timer_t *)0)
            {
                if (list->running != 0 && eos.time >= list->time_out)
                {
                    eos_critical_exit();
                    list->callback(list->parameter);
                    eos_critical_enter();
                    if (list->oneshoot == 0)
                    {
                        list->time_out += list->time;
                    }
                    else
                    {
                        list->running = 0;
                    }
                }
                list = list->next;
            }
            // Recalculate the minimum timeout value.
            list = eos.timers;
            uint32_t time_out_min = UINT32_MAX;
            while (list != (eos_timer_t *)0)
            {
                if (list->running != 0 && time_out_min > list->time_out)
                {
                    time_out_min = list->time_out;
                }
                list = list->next;
            }
            eos.time_out_min = time_out_min;
        }
        // if no timer is timeout
        else
        {
            eos_critical_exit();
            eos_hook_idle();
            eos_critical_enter();
        }
    }
}

static int32_t critical_count = 0;
#if (defined __CC_ARM)
inline void eos_critical_enter(void)
#elif ((defined __GNUC__) || (defined __ICCARM__))
__attribute__((always_inline)) inline void eos_critical_enter(void)
#endif
{
#if (defined __CC_ARM)
    __disable_irq();
#elif ((defined __GNUC__) || (defined __ICCARM__))
    __asm volatile ("cpsid i" : : : "memory");
#endif
    critical_count ++;
}

#if (defined __CC_ARM)
inline void eos_critical_exit(void)
#elif ((defined __GNUC__) || (defined __ICCARM__))
__attribute__((always_inline)) inline void eos_critical_exit(void)
#endif
{
    critical_count --;
    if (critical_count <= 0)
    {
        critical_count = 0;
#if (defined __CC_ARM)
        __enable_irq();
#elif ((defined __GNUC__) || (defined __ICCARM__))
        __asm volatile ("cpsie i" : : : "memory");
#endif
    }
}

#ifdef __cplusplus
}
#endif
