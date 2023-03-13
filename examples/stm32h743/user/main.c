/* include ------------------------------------------------------------------ */
#include "stm32h7xx.h"
#include "eventos.h"

/* task entry --------------------------------------------------------------- */
static void task_entry_led(void);
static void task_entry_count(void);
static void task_entry_test_exit(void);

/* data --------------------------------------------------------------------- */
static uint64_t stack_led[32];
eos_task_t led;

static uint64_t stack_count[32];
eos_task_t count;

static uint64_t stack_test_exit[32];
eos_task_t test_exit;

/* main function ------------------------------------------------------------ */
int main(void)
{
    if (SysTick_Config(SystemCoreClock / 1000) != 0)
        while (1);
    
//    static uint64_t stack_idle[32];
//    eos_init(stack_idle, sizeof(stack_idle));       // EventOS初始化
//    
//    // 启动LED闪烁任务
//    eos_task_start(&led, task_entry_led, 1, stack_led, sizeof(stack_led));

//    // 启动计数任务
//    eos_task_start(&count, task_entry_count, 2, stack_count, sizeof(stack_count));

//    eos_run();                                      // EventOS启动
    
    while (1);

    return 0;
}

//uint8_t led_status = 0;
//static void task_entry_led(void)
//{
//    eos_task_start(&test_exit, task_entry_test_exit, 3, stack_test_exit, sizeof(stack_test_exit));
//    
//    while (1) {
//        led_status = 0;
//        eos_delay_ms(500);
//        led_status = 1;
//        eos_delay_ms(500);
//    }
//}

uint32_t count_num = 0;
static void task_entry_count(void)
{
    while (1) {
        count_num ++;
        eos_delay_ms(10);
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

void SysTick_Handler(void)
{
    eos_tick();
}

void HardFault_Handler(void)
{
    while (1) {
    }
}
