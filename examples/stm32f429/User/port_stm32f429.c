
#include "eventos.h"

uint32_t eos_error_id = 0;
void eos_port_assert(uint32_t error_id)
{
    eos_error_id = error_id;

    while (1) {
    }
}

static void eos_delay_block(uint32_t time_ms)
{
    for (uint32_t i = 0; i < time_ms; i ++) {
        uint32_t time_count = 36000;
        while (time_count --) {
            __nop();
        }
    }
}

void eos_hook_idle(void)
{
    // 设置这个运行10S的函数，是为了测试其他任务的抢占功能。
    eos_delay_block(10000);
}

void eos_hook_start(void)
{

}
