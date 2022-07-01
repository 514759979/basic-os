
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
 * 2022-03-21     DogMing       V0.0.1
 */

#ifndef EVENTOS_H_
#define EVENTOS_H_

/* include ------------------------------------------------------------------ */
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Config ------------------------------------------------------------------- */
// 支持的最大的线程数
#define EOS_MAX_TASKS                           32

// 支持的最大优先级数
#define EOS_MAX_PRIORITY                        8

// 时钟滴答的毫秒数
#define EOS_TICK_MS                             1

// 是否使用断言
#define EOS_USE_ASSERT                          1

// 是否统计堆栈使用率
#define EOS_USE_STACK_USAGE                     1

// 是否统计CPU使用率
#define EOS_USE_CPU_USAGE                       1

#if (EOS_MAX_TASKS > 32)
#error The number of tasks can NOT be larger than 32 !
#endif

/* Data structure ----------------------------------------------------------- */
enum {
    EOS_OK                          = 0,

};

typedef void (* eos_func_t)(void *parameter);

// Task
typedef struct eos_task
{
    uint32_t *sp;
    struct eos_task *next;
    struct eos_task *prev;
    void *stack;
    const char *name;
    uint32_t timeout;
    uint32_t size                   : 16;
    uint32_t usage                  : 8;
    uint32_t cpu_usage              : 8;
    uint32_t cpu_usage_count        : 16;
    uint32_t priority               : 5;
    uint32_t state;
    uint32_t state_bkp;
} eos_task_t;

// Timer
typedef struct eos_timer
{
    struct eos_timer *next;
    uint32_t time;
    uint32_t time_out;
    eos_func_t callback;
    void *parameter;
    uint32_t id                     : 10;
    uint32_t domain                 : 8;
    uint32_t oneshoot               : 1;
    uint32_t running                : 1;
} eos_timer_t;

/* 任务相关 ------------------------------------------------------------------ */
// 初始化，建议在main函数中调用。
void eos_init(void *stack_idle, uint32_t size);
// 启动系统
void eos_run(void);
// 系统当前时间
uint32_t eos_time(void);
// 系统滴答，建议在SysTick中断里调用，也可在普通定时器中断中调用。
void eos_tick(void);
// 任务内延时，任务函数中调用，不允许在定时器的回调函数调用，不允许在空闲回调函数中调用。
void eos_delay_ms(uint32_t time_ms);
// 退出任务，任务函数中调用。
void eos_task_exit(void);
// 任务切换
void eos_task_yield(void);
// 获取当前任务句柄
eos_task_t *eos_get_task(void);
// 启动任务，main函数或者任务函数中调用。
void eos_task_start(eos_task_t * const me,
                    eos_func_t func,
                    uint8_t priority,
                    void *stack_addr,
                    uint32_t stack_size,
                    void *parameter);

/* 软定时器 ------------------------------------------------------------------ */
// 启动软定时器，允许在中断中调用。
int32_t eos_timer_start(eos_timer_t * const me,
                        uint32_t time_ms,
                        bool oneshoot,
                        eos_func_t callback,
                        void *parameter);
// 删除软定时器，允许在中断中调用。
void eos_timer_delete(uint16_t timer_id);
// 暂停软定时器，允许在中断中调用。
void eos_timer_pause(uint16_t timer_id);
// 继续软定时器，允许在中断中调用。
void eos_timer_continue(uint16_t timer_id);
// 重启软定时器的定时，允许在中断中调用。
void eos_timer_reset(uint16_t timer_id);

/* 统计功能 ------------------------------------------------------------------ */
#if (EOS_USE_STACK_USAGE != 0)
// 任务的堆栈使用率
uint8_t eos_task_stack_usage(eos_task_t * const me);
#endif

#if (EOS_USE_CPU_USAGE != 0)
// 任务的CPU使用率
uint8_t eos_task_cpu_usage(eos_task_t * const me);
// 监控函数，放进一个单独的定时器中断函数，中断频率为SysTick的10-20倍。
void eos_cpu_usage_monitor(void);
#endif

/* port --------------------------------------------------------------------- */
void eos_port_assert(uint32_t error_id);

/* hook --------------------------------------------------------------------- */
// 空闲回调函数
void eos_hook_idle(void);

// 启动EventOS Basic的时候，所调用的回调函数
void eos_hook_start(void);

#ifdef __cplusplus
}
#endif

#endif
