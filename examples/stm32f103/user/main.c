/* include ------------------------------------------------------------------ */
#include "stm32f10x.h"
#include "eventos.h"

/* task entry --------------------------------------------------------------- */
static void task_entry_led(void);
static void task_entry_count(void);
static void task_entry_test_exit(void);
static void timer_handler_10ms(void);

/* data structure ----------------------------------------------------------- */
typedef struct timer_test {
    int32_t id;
    uint32_t count;
    uint8_t pause;
    uint8_t continue_;
    uint8_t reset;
    uint8_t delete_;
} timer_test_t;

timer_test_t timer_test;

/* data --------------------------------------------------------------------- */
static uint8_t stack_led[512];
eos_task_t led;

static uint8_t stack_count[512];
eos_task_t count;

static uint8_t stack_test_exit[512];
eos_task_t test_exit;

eos_timer_t timer;

/* main function ------------------------------------------------------------ */
int main(void)
{
    if (SysTick_Config(SystemCoreClock / 1000) != 0)
        while (1);
    
    static uint8_t stack_idle[512];
    eos_init(stack_idle, sizeof(stack_idle));       // EventOS初始化
    
    // 启动LED闪烁任务
    eos_task_start(&led, task_entry_led, 2, stack_led, sizeof(stack_led));

    // 启动计数任务
    eos_task_start(&count, task_entry_count, 3, stack_count, sizeof(stack_count));

    eos_run();                                      // EventOS启动

    return 0;
}

uint8_t led_status = 0;


static void task_entry_led(void)
{
    eos_task_start(&test_exit, task_entry_test_exit, 4, stack_test_exit, sizeof(stack_test_exit));
    
    // 启动定时器
    timer_test.id = eos_timer_start(&timer, 10, false, timer_handler_10ms);
    
    while (1) {
        led_status = 0;
        eos_delay_ms(500);
        led_status = 1;
        eos_delay_ms(500);
    }
}

uint32_t count_num = 0;
static void task_entry_count(void)
{
    while (1) {
        count_num ++;
        eos_delay_ms(10);
        
        if (timer_test.pause != 0) {
            timer_test.pause = 0;
            eos_timer_pause(timer_test.id);
        }
        
        if (timer_test.continue_ != 0) {
            timer_test.continue_ = 0;
            eos_timer_continue(timer_test.id);
        }
        
        if (timer_test.reset != 0) {
            timer_test.reset = 0;
            eos_timer_reset(timer_test.id);
        }
        
        if (timer_test.delete_ != 0) {
            timer_test.delete_ = 0;
            eos_timer_delete(timer_test.id);
        }
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


static void timer_handler_10ms(void)
{
    timer_test.count ++;
}
