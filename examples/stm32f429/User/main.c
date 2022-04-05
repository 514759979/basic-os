/* include ------------------------------------------------------------------ */
#include "stm32f4xx.h"
#include "eventos.h"

/* task entry --------------------------------------------------------------- */
static void task_entry_led(void);
static void task_entry_count(void);
static void task_entry_test_exit(void);
static void eos_delay_block(uint32_t time_ms);
static void eos_delay_10ms(void);
static void eos_delay_50ms(void);

/* data --------------------------------------------------------------------- */
uint8_t stack_idle[256];

uint8_t stack_led[256];
eos_task_t led;

uint8_t stack_count[256];
eos_task_t count;

uint64_t stack_test_exit[32];
eos_task_t test_exit;

/* main function ------------------------------------------------------------ */
int main(void)
{
    if (SysTick_Config(SystemCoreClock / 10000) != 0)
        while (1);
    
    eos_init(stack_idle, sizeof(stack_idle));       // EventOS初始化
    
    // 启动LED闪烁任务
    eos_task_start(&led, task_entry_led, 1, stack_led, sizeof(stack_led));

    // 启动计数任务
    eos_task_start(&count, task_entry_count, 2, stack_count, sizeof(stack_count));

    eos_run();                                      // EventOS启动

    return 0;
}

uint8_t led_status = 0;
static void task_entry_led(void)
{
    eos_task_start(&test_exit, task_entry_test_exit, 3, stack_test_exit, sizeof(stack_test_exit));
    
    while (1) {
        led_status = 0;
        eos_delay_50ms();
        eos_delay_ms(450);
        led_status = 1;
        eos_delay_50ms();
        eos_delay_ms(450);
    }
}

uint32_t count_num = 0;
static void task_entry_count(void)
{
    while (1) {
        count_num ++;
        eos_delay_block(2);
        eos_delay_ms(8);
    }
}

uint32_t count_num_exit = 0;
static void task_entry_test_exit(void)
{
    for (int i = 0; i < 10 ; i ++) {
        count_num_exit ++;
        eos_delay_ms(10);
    }
    
    eos_task_exit();
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

static void eos_delay_10ms(void)
{
    eos_delay_block(10);
}

static void eos_delay_50ms(void)
{
    for (uint8_t i = 0; i < 5; i ++) {
        eos_delay_10ms();
    }
}

uint8_t systick_count = 0;
uint8_t usage_stack_max[4];
uint8_t uasge_cpu[4];
void SysTick_Handler(void)
{
    systick_count ++;
    if (systick_count >= 10) {
        systick_count = 0;
        eos_tick();
    }
    
    eos_cpu_usage_monitor();
    if (eos_time() > 100) {
        for (uint8_t i = 0; i < 3; i ++) {
            usage_stack_max[i] = eos_task_stack_usage(i);
            uasge_cpu[i] = eos_task_cpu_usage(i);
        }
    }
}

void HardFault_Handler(void)
{
    while (1) {
    }
}
