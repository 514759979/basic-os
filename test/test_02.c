#include "test.h"
#include "eventos.h"

#if (TEST_EN_02 != 0)

#define TEST_02_TASK_MAX                    (4)
#define TEST_02_STACK_SIZE                  (256)

typedef struct task_info
{
    eos_task_t task;
    uint8_t stack[TEST_02_STACK_SIZE];
    uint32_t count;
    uint8_t delay_ms;
    uint8_t cpu_usage;
} task_info_t;

task_info_t task_info[TEST_02_TASK_MAX];
static void task_entry_yield(void *parameter);
static void eos_delay_block(uint32_t time_ms);

void test_start(void)
{
    for (uint8_t i = 0; i < TEST_02_TASK_MAX; i ++)
    {
        task_info[i].delay_ms = (1 + i);
        eos_task_start(&task_info[i].task,
                       task_entry_yield,
                       1,
                       task_info[i].stack,
                       sizeof(task_info[i].stack),
                       &task_info[i]);
    }
}

static void task_entry_yield(void *parameter)
{
    task_info_t *info = (task_info_t *)parameter;
    while (1)
    {
        eos_delay_block(info->delay_ms); 
        info->count ++;
        info->cpu_usage = eos_task_cpu_usage(eos_get_task());
        eos_task_yield();
    }
}

static void eos_delay_block(uint32_t time_ms)
{
    for (uint32_t i = 0; i < time_ms; i ++) {
        uint32_t time_count = 45000;
        while (time_count --) {
#if (defined __CC_ARM)
            __nop();
#endif
        }
    }
}

#endif
