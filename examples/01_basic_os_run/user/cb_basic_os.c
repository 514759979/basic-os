#include "basic_os.h"
#include "elab_log.h"
#include "elab_assert.h"

ELAB_TAG("eLabCallBack");

void eos_port_assert(uint32_t error_id)
{
    elog_error("eLog error id: %u.", error_id);
    while (1)
    {
        eos_delay_ms(10);
    }
}

void eos_hook_idle(void)
{

}

void eos_hook_start(void)
{

}

void SysTick_Handler(void)
{
    eos_tick();
}

/* ----------------------------- end of file -------------------------------- */
