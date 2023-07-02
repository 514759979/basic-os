#include "basic_os.h"
#include "elab_log.h"
#include "elab_assert.h"

ELAB_TAG("eLabCallBack");

void bos_port_assert(uint32_t error_id)
{
    elog_error("eLog error id: %u.", error_id);
    while (1)
    {
        bos_delay_ms(10);
    }
}

uint32_t count_idle = 0;
void bos_hook_idle(void)
{
    count_idle ++;
}

void bos_hook_start(void)
{

}

void SysTick_Handler(void)
{
    bos_tick();
}

/* ----------------------------- end of file -------------------------------- */
