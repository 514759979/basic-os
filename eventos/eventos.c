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
#define EOS_ASSERT(test_) do { if (!(test_)) {                                 \
        eos_port_critical_enter();                                             \
        eos_port_assert(__LINE__);                                             \
    } } while (0)
#else
#define EOS_ASSERT(test_)               ((void)0)
#endif

/* eos task ----------------------------------------------------------------- */
eos_task_t *volatile eos_current;
eos_task_t *volatile eos_next;

typedef struct eos_tag {
    eos_task_t * task[EOS_MAX_TASKS];

    uint32_t delay;
    uint32_t exist;
    uint32_t time;
    uint64_t time_offset;
} eos_t;

eos_t eos;

static eos_task_t task_idle;

/* macro -------------------------------------------------------------------- */
#define LOG2(x) (32U - __builtin_clz(x))

#define EOS_MS_NUM_30DAY                (2592000000U)
#define EOS_MS_NUM_15DAY                (1296000000U)

/* private function --------------------------------------------------------- */
static void eos_sheduler(void);
static void task_entry_idle(void);

/* public function ---------------------------------------------------------- */
void eos_init(void *stack_idle, uint32_t size)
{
    eos_port_critical_enter();

    // Set PendSV to be the lowest priority.
    *(uint32_t volatile *)0xE000ED20 |= (0xFFU << 16U);

    eos.time = 0;
    eos.delay = 0;
    eos.time_offset = 0;
    eos_current = (void *)0;
    eos_next = &task_idle;
    
    for (int i = 0; i < EOS_MAX_TASKS; i ++) {
        eos.task[i] = (void *)0;
    }
    eos_port_critical_exit();

    eos_task_start(&task_idle, task_entry_idle, 0, stack_idle, size);
}

void eos_task_start(  eos_task_t * const me,
                        eos_func_t func,
                        uint8_t priority,
                        void *stack_addr,
                        uint32_t stack_size)
{
    EOS_ASSERT(eos.task[priority] == (void *)0);
    
    /* round down the stack top to the 8-byte boundary
     * NOTE: ARM Cortex-M stack grows down from hi -> low memory
     */
    uint32_t *sp = (uint32_t *)((((uint32_t)stack_addr + stack_size) >> 3U) << 3U);
    uint32_t *stk_limit;

    *(-- sp) = (uint32_t)(1 << 24);            /* xPSR, Set Bit24(Thumb Mode) to 1. */
    *(-- sp) = (uint32_t)func;                 /* the entry function (PC) */
    *(-- sp) = (uint32_t)func;                 /* R14(LR) */
    *(-- sp) = (uint32_t)0x12121212u;          /* R12 */
    *(-- sp) = (uint32_t)0x03030303u;          /* R3 */
    *(-- sp) = (uint32_t)0x02020202u;          /* R2 */
    *(-- sp) = (uint32_t)0x01010101u;          /* R1 */
    *(-- sp) = (uint32_t)0x00000000u;          /* R0 */
    /* additionally, fake registers R4-R11 */
    *(-- sp) = (uint32_t)0x11111111u;          /* R11 */
    *(-- sp) = (uint32_t)0x10101010u;          /* R10 */
    *(-- sp) = (uint32_t)0x09090909u;          /* R9 */
    *(-- sp) = (uint32_t)0x08080808u;          /* R8 */
    *(-- sp) = (uint32_t)0x07070707u;          /* R7 */
    *(-- sp) = (uint32_t)0x06060606u;          /* R6 */
    *(-- sp) = (uint32_t)0x05050505u;          /* R5 */
    *(-- sp) = (uint32_t)0x04040404u;          /* R4 */

    /* save the top of the stack in the task's attibute */
    me->sp = sp;

    /* round up the bottom of the stack to the 8-byte boundary */
    stk_limit = (uint32_t *)(((((uint32_t)stack_addr - 1U) >> 3U) + 1U) << 3U);

    /* pre-fill the unused part of the stack with 0xDEADBEEF */
    for (sp = sp - 1U; sp >= stk_limit; --sp) {
        *sp = 0xDEADBEEFU;
    }

    eos_port_critical_enter();
    me->priority = priority;
    eos.task[priority] = me;
    eos.exist |= (1 << priority);
    
    if (eos_current == &task_idle) {
        eos_port_critical_exit();
        eos_sheduler();
    }
    eos_port_critical_exit();
}

void eos_run(void)
{
    eos_hook_start();
    
    eos_sheduler();
}

void eos_tick(void)
{
    eos_port_critical_enter();
    eos.time += EOS_TICK_MS;
    if (eos.time >= EOS_MS_NUM_15DAY) {
        for (uint32_t i = 0; i < EOS_MAX_TASKS; i ++) {
            if (eos.task[i] != (void *)0) {
                eos.task[i]->timeout -= eos.time;
            }
        }
        eos.time_offset += eos.time;
        eos.time = 0;
    }
    
    /* check all the time-events are timeout or not */
    uint32_t working_set, bit;
    working_set = eos.delay;
    while (working_set != 0U) {
        eos_task_t *t = eos.task[LOG2(working_set) - 1];
        EOS_ASSERT(t != (eos_task_t *)0);
        EOS_ASSERT(((eos_task_t *)t)->timeout != 0U);

        bit = (1U << (((eos_task_t *)t)->priority));
        if (eos.time >= ((eos_task_t *)t)->timeout) {
            eos.delay &= ~bit;              /* remove from set */
        }
        working_set &=~ bit;                /* remove from working set */
    }
    eos_port_critical_exit();
    
    if (eos_current == &task_idle) {
        eos_sheduler();
    }
}

void eos_delay_ms(uint32_t time_ms)
{
    EOS_ASSERT(time_ms <= EOS_MS_NUM_30DAY);

    /* never call eos_delay_ms and eos_delay_ticks in the idle task */
    EOS_ASSERT(eos_current != &task_idle);

    eos_port_critical_enter();
    eos_current->timeout = eos.time + time_ms;
    eos.delay |= (1U << (eos_current->priority));
    eos_port_critical_exit();
    
    eos_sheduler();
}

void eos_exit(void)
{
    eos_port_critical_enter();
    eos.task[eos_current->priority] = (void *)0;
    eos.exist &= ~(1 << eos_current->priority);
    eos_port_critical_exit();
    
    eos_sheduler();
}

static void eos_sheduler(void)
{
    eos_port_critical_enter();
    /* eos_next = ... */
    eos_next = eos.task[LOG2(eos.exist & (~eos.delay)) - 1];
    /* trigger PendSV, if needed */
    if (eos_next != eos_current) {
        *(uint32_t volatile *)0xE000ED04 = (1U << 28);
    }
    eos_port_critical_exit();
}

uint64_t eos_time(void)
{
    eos_port_critical_enter();
    uint64_t time_offset = eos.time_offset;
    eos_port_critical_enter();

    return (time_offset + eos.time);
}

/* private function --------------------------------------------------------- */
static void task_entry_idle(void)
{
    while (1) {
        eos_hook_idle();
    }
}

#ifdef __cplusplus
}
#endif
