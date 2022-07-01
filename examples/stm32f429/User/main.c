/* include ------------------------------------------------------------------ */
#include "stm32f4xx.h"
#include "eventos.h"
#include "test.h"

static uint8_t stack_idle[256];

/* main function ------------------------------------------------------------ */
int main(void)
{
    if (SysTick_Config(SystemCoreClock / 1000) != 0)
    {
        while (1);
    }
    
    eos_init(stack_idle, sizeof(stack_idle));       // EventOS初始化
    
    test_start();

    eos_run();                                      // EventOS启动

    return 0;
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
