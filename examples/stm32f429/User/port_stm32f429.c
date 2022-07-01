
#include "eventos.h"

uint32_t eos_error_id = 0;
void eos_port_assert(uint32_t error_id)
{
    eos_error_id = error_id;

    while (1) {
    }
}

uint32_t count_idle = 0;
void eos_hook_idle(void)
{
    count_idle ++;
}

void eos_hook_start(void)
{

}
