/* include ------------------------------------------------------------------ */
#include "eventos.h"
#include "stm32g070xx.h"
#include "test.h"

static uint8_t stack_idle[256];

/* main function ------------------------------------------------------------ */
int main(void)
{
    if (SysTick_Config(SystemCoreClock / 10000) != 0)
        while (1);
    
    eos_init(stack_idle, sizeof(stack_idle));       // EventOS初始化
    
    test_start();

    eos_run();                                      // EventOS启动

    return 0;
}

uint8_t tick_count = 0;
void SysTick_Handler(void)
{
    tick_count ++;
    if (tick_count >= 10)
    {
        tick_count = 0;
        eos_tick();
    }
    
    eos_cpu_usage_monitor();
}

void HardFault_Handler(void)
{
    while (1)
    {
    }
}
