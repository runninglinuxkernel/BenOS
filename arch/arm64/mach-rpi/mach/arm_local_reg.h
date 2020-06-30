#ifndef MACH_ARM_LOCAL_REG_H
#define  MACH_ARM_LOCAL_REG_H

#include "base.h"

/*
 * ARM Local register for PI
 * see <BCM2711 ARM Peripherals> 6.5.2
 */

#define ARM_CONTROL (0x0)
#define CORE_IRQ_CONTROL (0xc)
#define PMU_CONTROL_SET (0x10)
#define PMU_CONTROL_CLR (0x14)
#define PERI_IRQ_ROUTE0 (0x24)
#define AXI_QUIET_TIME (0x30)
#define LOCAL_TIMER_CONTROL (0x34)
#define LOCAL_TIMER_IRQ (0x38)

#define TIMER_CNTRL0 (0x40)
#define TIMER_CNTRL1 (0x44)
#define TIMER_CNTRL2 (0x48)
#define TIMER_CNTRL3 (0x4c)
/* Secure Physical Timer Event for IRQ */
#define CNT_PS_IRQ (1 << 0)
/* Nonsecure Physical Timer Event for IRQ */
#define CNT_PNS_IRQ (1 << 1)
/* Hypervisor Physical Timer Event for IRQ */
#define CNT_HP_IRQ (1 << 2)
/* Virtual Timer Event for IRQ */
#define CNT_V_IRQ (1 << 3)
/* Secure Physical Timer Event for FIQ */
#define CNT_PS_IRQ_FIQ (1 << 4)
/* Nonsecure Physical Timer Event for FIQ */
#define CNT_PNS_IRQ_FIQ (1 << 5)
/* Hypervisor Physical Timer Event for FIQ */
#define CNT_HP_IRQ_FIQ (1 << 6)
/* Virtual Timer Event for FIQ */
#define CNT_V_IRQ_FIQ (1 << 7)

#define ARM_LOCAL_IRQ_SOURCE0 (0x60)
#define ARM_LOCAL_IRQ_SOURCE1 (0x64)
#define ARM_LOCAL_IRQ_SOURCE2 (0x68)
#define ARM_LOCAL_IRQ_SOURCE3 (0x6c)
#define MAILBOX_IRQ_SHIFT 4
#define CORE_IRQ (1 << 8)
#define PMU_IRQ (1 << 9)
#define AXI_QUIET (1 << 10)
#define TIMER_IRQ (1 << 11)
#define AXI_IRQ (1 << 30)

#define ARM_LOCAL_FRQ_SOURCE0 (0x70)
#define ARM_LOCAL_FRQ_SOURCE1 (0x74)
#define ARM_LOCAL_FRQ_SOURCE2 (0x78)
#define ARM_LOCAL_FRQ_SOURCE3 (0x7c)

/* GIC V2*/
#define GIC_V2_DISTRIBUTOR_BASE     (ARM_LOCAL_BASE + 0x00041000)
#define GIC_V2_CPU_INTERFACE_BASE   (ARM_LOCAL_BASE + 0x00042000)
#define GIC_V2_HYPERVISOR_BASE      (ARM_LOCAL_BASE + 0x00044000)
#define GIC_V2_VIRTUAL_CPU_BASE     (ARM_LOCAL_BASE + 0x00046000)

#endif /*MACH_ARM_LOCAL_REG_H*/
