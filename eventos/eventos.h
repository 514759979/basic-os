
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
#define EOS_MAX_TASKS                           4

// 支持的最大的软定时器数
#define EOS_MAX_TIMERS                          4

#define EOS_TICK_MS                             1

#define EOS_USE_ASSERT                          1

#if (EOS_MAX_TASKS > 32)
#error The number of tasks can NOT be larger than 32 !
#endif

/* Data structure ----------------------------------------------------------- */
typedef void (* eos_func_t)(void);

// Task
typedef struct eos_actor {
    uint32_t *sp;
    uint32_t timeout;
    uint32_t priority              : 5;
} eos_task_t;

typedef struct eos_timer {
    struct eos_timer *next;
    uint32_t time;
    uint32_t time_out;
    eos_func_t callback;
    uint32_t domain                 : 8;
    uint32_t oneshoot               : 1;
} eos_timer_t;

/* 任务相关 ------------------------------------------------------------------ */
// 初始化
void eos_init(void *stack_idle, uint32_t size);
// 启动系统
void eos_run(void);
// 系统当前时间
uint64_t eos_time(void);
// 系统滴答
void eos_tick(void);
// 任务内延时
void eos_delay_ms(uint32_t time_ms);
// 退出任务
void eos_exit(void);
// 启动任务
void eos_task_start(eos_task_t * const me,
                    eos_func_t func,
                    uint8_t priority,
                    void *stack_addr,
                    uint32_t stack_size);

/* 软定时器 ------------------------------------------------------------------ */
// 启动软定时器
int32_t eos_timer_start(eos_timer_t const *me,
                        uint32_t time,
                        bool oneshoot,
                        eos_func_t callback);
// 暂停
void eos_timer_pause(uint16_t timer_id);
// 继续
void eos_timer_continue(uint16_t timer_id);
// 停止
void eos_timer_stop(uint16_t timer_id);
// 重启
void eos_timer_reset(uint16_t timer_id);

/* port --------------------------------------------------------------------- */
void eos_port_assert(uint32_t error_id);

/* hook --------------------------------------------------------------------- */
// 空闲回调函数
void eos_hook_idle(void);

// 启动EventOS Nano的时候，所调用的回调函数
void eos_hook_start(void);

#ifdef __cplusplus
}
#endif

#endif
