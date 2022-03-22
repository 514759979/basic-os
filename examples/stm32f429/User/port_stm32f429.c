
#include "eventos.h"

__asm void PendSV_Handler(void)
{
    IMPORT        eos_current           /* extern variable */
    IMPORT        eos_next              /* extern variable */

#if (__TARGET_ARCH_THUMB == 3)          /* Cortex-M0/M0+/M1 (v6-M, v6S-M)? */
    CPSID   i                           /* disable interrupts (set PRIMASK) */
#else
    MOVS    r0,#0x3F
    CPSID   i                           /* selectively disable interrutps with BASEPRI */
    MSR     BASEPRI,r0                  /* apply the workaround the Cortex-M7 erraturm */
    CPSIE   i                           /* 837070, see SDEN-1068427. */
#endif                                  /* M3/M4/M7 */

    LDR           r1,=eos_current       /* if (eos_current != 0) { */
    LDR           r1,[r1,#0x00]
    CBZ           r1,PendSV_restore

    PUSH          {r4-r11}              /*     push r4-r11 into stack */
    LDR           r1,=eos_current       /*     eos_current->sp = sp; */
    LDR           r1,[r1,#0x00]
    STR           sp,[r1,#0x00]         /* } */
    
PendSV_restore
    LDR           r1,=eos_next          /* sp = eos_next->sp; */
    LDR           r1,[r1,#0x00]
    LDR           sp,[r1,#0x00]

    LDR           r1,=eos_next          /* eos_current = eos_next; */
    LDR           r1,[r1,#0x00]
    LDR           r2,=eos_current
    STR           r1,[r2,#0x00]
    POP           {r4-r11}              /* pop registers r4-r11 */
#if (__TARGET_ARCH_THUMB == 3)          /* Cortex-M0/M0+/M1 (v6-M, v6S-M)? */
    CPSIE   i                           /* enable interrupts (clear PRIMASK) */
#else                                   /* M3/M4/M7 */
    MOVS    r0,#0
    MSR     BASEPRI,r0                  /* enable interrupts (clear BASEPRI) */
    DSB                                 /* ARM Erratum 838869 */
#endif                                  /* M3/M4/M7 */
    BX            lr                    /* return to the next task */
}

static int critical_count = 0;
void eos_port_critical_enter(void)
{
    __disable_irq();
    critical_count ++;
}

void eos_port_critical_exit(void)
{
    critical_count --;
    if (critical_count <= 0) {
        critical_count = 0;
        __enable_irq();
    }
}

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
    eos_delay_block(10000);
}

void eos_hook_start(void)
{

}
