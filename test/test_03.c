#include "test.h"
#include "eventos.h"

#if (TEST_EN_03 != 0)

#define TEST_03_TASK_MAX                    (3)
#define TEST_03_STACK_SIZE                  (1024)

typedef struct task_info
{
    eos_task_t task;
    eos_func_t func;
    uint8_t stack[TEST_03_STACK_SIZE];
    uint32_t count;
    uint8_t delay_ms;
    uint8_t stack_usage;
} task_info_t;

task_info_t task_info[TEST_03_TASK_MAX];

static void task_entry_0(void *parameter);
static void task_entry_1(void *parameter);
static void task_entry_2(void *parameter);

void test_start(void)
{
    task_info[0].func = task_entry_0;
    task_info[1].func = task_entry_1;
    task_info[2].func = task_entry_2;

    for (uint8_t i = 0; i < TEST_03_TASK_MAX; i ++)
    {
        task_info[i].delay_ms = (1 + i);
        eos_task_start(&task_info[i].task,
                       task_info[i].func,
                       1,
                       task_info[i].stack,
                       sizeof(task_info[i].stack),
                       &task_info[i]);
    }
}

static void task_entry_0(void *parameter)
{
    task_info_t *info = (task_info_t *)parameter;

    while (1)
    {
        uint8_t data[32];
        for (uint32_t i = 0; i < sizeof(data); i ++)
        {
            data[i] = info->stack_usage;
        }

        info->count += (data[0] - data[1] + 1);
        info->stack_usage = eos_task_stack_usage(eos_get_task());

        eos_delay_ms(info->delay_ms);
    }
}

static void task_entry_1(void *parameter)
{
    task_info_t *info = (task_info_t *)parameter;

    while (1)
    {
        uint8_t data[128];
        for (uint32_t i = 0; i < sizeof(data); i ++)
        {
            data[i] = info->stack_usage;
        }

        info->count += (data[0] - data[1] + 1);
        info->stack_usage = eos_task_stack_usage(eos_get_task());

        eos_delay_ms(info->delay_ms);
    }
}

static void task_entry_2(void *parameter)
{
    task_info_t *info = (task_info_t *)parameter;

    while (1)
    {
        uint8_t data[256];
        for (uint32_t i = 0; i < sizeof(data); i ++)
        {
            data[i] = info->stack_usage;
        }

        info->count += (data[0] - data[1] + 1);
        info->stack_usage = eos_task_stack_usage(eos_get_task());

        eos_delay_ms(info->delay_ms);
    }
}

#endif
