/* include ------------------------------------------------------------------ */
#include "stm32f4xx.h"
#include "eventos.h"

/* thread entry ------------------------------------------------------------- */
static void thread_entry_led(void);
static void thread_entry_count(void);
static void thread_entry_test_exit(void);

/* data --------------------------------------------------------------------- */
static uint64_t stack_led[32];
eos_thread_t led;

static uint64_t stack_count[32];
eos_thread_t count;

static uint64_t stack_test_exit[32];
eos_thread_t test_exit;

uint32_t count_tick = 0;
uint32_t time_consume = 0;
uint32_t time_bkp = 0;

    
/* main function ------------------------------------------------------------ */
int main(void)
{
    if (SysTick_Config(SystemCoreClock / 1000) != 0)
        while (1);
    
    eos_init();                                     // EventOS初始化
    
    eos_thread_start(&led, thread_entry_led, 1, stack_led, sizeof(stack_led));
    eos_thread_start(&count, thread_entry_count, 2, stack_count, sizeof(stack_count));

    eos_run();                                      // EventOS启动

    return 0;
}

void SysTick_Handler(void)
{
    eos_tick();
    
    count_tick ++;
}

void HardFault_Handler(void)
{
    while (1) {
    }
}

uint8_t led_status = 0;
static void thread_entry_led(void)
{
    eos_thread_start(&test_exit, thread_entry_test_exit, 3, stack_test_exit, sizeof(stack_test_exit));
    
    while (1) {
        led_status = 0;
        eos_delay_ms(500);
        led_status = 1;
        eos_delay_ms(500);
    }
}

uint32_t count_num = 0;
static void thread_entry_count(void)
{
    while (1) {
        count_num ++;
        eos_delay_ms(10);
    }
}

uint32_t count_num_exit = 0;
static void thread_entry_test_exit(void)
{
    for (int i = 0; i < 10 ; i ++) {
        count_num_exit ++;
        eos_delay_ms(10);
    }
    
    eos_exit();
}
