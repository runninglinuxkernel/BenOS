#ifndef MACH_IRQ_H
#define MACH_IRQ_H

#include "base.h"
#include "arm_local_reg.h"

#define ARMC_BASE (PBASE + 0xb000)

/* ARMC for PI3B */
#define IRQ_BASIC_PENDING	(0x200)
#define IRQ_PENDING_1		(0x204)
#define IRQ_PENDING_2		(0x208)
#define FIQ_CONTROL		(0x20C)
#define ENABLE_IRQS_1		(0x210)
#define ENABLE_IRQS_2		(0x214)
#define ENABLE_BASIC_IRQS	(0x218)
#define DISABLE_IRQS_1		(0x21C)
#define DISABLE_IRQS_2		(0x220)
#define DISABLE_BASIC_IRQS	(0x224)

#define SYSTEM_TIMER_IRQ_0	(1 << 0)
#define SYSTEM_TIMER_IRQ_1	(1 << 1)
#define SYSTEM_TIMER_IRQ_2	(1 << 2)
#define SYSTEM_TIMER_IRQ_3	(1 << 3)

#define ARM_TIMER_IRQ (1 << 0)

/* ARMC for PI4B */
#define IRQ0_PENDING0 0x200
#define IRQ0_PENDING1 0x204
#define IRQ0_PENDING2 0x208
#define IRQ0_SET_EN_0 0x210
#define IRQ0_SET_EN_1 0x214
#define IRQ0_SET_EN_2 0x218
#define IRQ0_CLR_EN_0 0x220
#define IRQ0_CLR_EN_1 0x224
#define IRQ0_CLR_EN_2 0x228
#define IRQ_STATUS0   0x230
#define IRQ_STATUS1   0x234
#define IRQ_STATUS2   0x238

#define IRQ1_PENDING0 0x240
#define IRQ1_PENDING1 0x244
#define IRQ1_PENDING2 0x248
#define IRQ1_SET_EN_0 0x250
#define IRQ1_SET_EN_1 0x254
#define IRQ1_SET_EN_2 0x258
#define IRQ1_CLR_EN_0 0x260
#define IRQ1_CLR_EN_1 0x264
#define IRQ1_CLR_EN_2 0x268

#define SWIRQ_SET     0x3f0
#define SWIRQ_CLEAR   0x3f4

/*
 * Legacy interrupt for PI
 *
 *  0 ~ 63 :  VideoCore interrupts (ARMC)
 *              PI4: IRQ0_SET_EN_0, IRQ0_SET_EN_1
 *              PI3: IRQ_PENDING1, IRQ_PENDING2
 * 64 ~ 85 :  ARMC interrupts
 *              PI4: IRQ0_SET_EN_2
 *              PI3: IRQ_BASE_PENDING
 *
 * 86 ~ 118:  ARM Local interrupts (TIMER_CNTRL0)
 */

#ifdef CONFIG_ARM_GICV2 /* GICV2 irq assignment */
#define GENERIC_TIMER_IRQ 30

#else  /* Legacy irq assignment */

#define GENERIC_TIMER_IRQ  87
#endif

/* irq register for legacy irq chip driver */
#ifdef CONFIG_BOARD_PI3B
#define PI_IRQ_BASE_PENDING  IRQ_BASIC_PENDING
#define BAND0_STATUS   (1<<8)
#define BAND1_STATUS   (1<<9)
/* irq: 0~31 */
#define PI_IRQ_BANK0_PENDING  IRQ_PENDING_1
/* irq: 32~63 */
#define PI_IRQ_BANK1_PENDING IRQ_PENDING_2
/* irq: 64~85 */
#define PI_IRQ_BANK2_PENDING PI_IRQ_BASE_PENDING

/* irq disable*/
#define PI_IRQ_BANK0_DISABLE  DISABLE_IRQS_1
#define PI_IRQ_BANK1_DISABLE  DISABLE_IRQS_2
#define PI_IRQ_BANK2_DISABLE  DISABLE_BASIC_IRQS

/* irq enable*/
#define PI_IRQ_BANK0_ENABLE  ENABLE_IRQS_1
#define PI_IRQ_BANK1_ENABLE  ENABLE_IRQS_2
#define PI_IRQ_BANK2_ENABLE  ENABLE_BASIC_IRQS

#else /* for PI4B */
#define PI_IRQ_BASE_PENDING  IRQ0_PENDING2
#define BAND0_STATUS   (1<<24)
#define BAND1_STATUS   (1<<25)

/* irq: 0~31 */
#define PI_IRQ_BANK0_PENDING  IRQ0_PENDING0
/* irq: 32~63 */
#define PI_IRQ_BANK1_PENDING IRQ0_PENDING1
/* irq: 64~85 */
#define PI_IRQ_BANK2_PENDING IRQ0_PENDING2

/* irq disable*/
#define PI_IRQ_BANK0_DISABLE  IRQ0_CLR_EN_0
#define PI_IRQ_BANK1_DISABLE  IRQ0_CLR_EN_1
#define PI_IRQ_BANK2_DISABLE  IRQ0_CLR_EN_2

/* irq enable*/
#define PI_IRQ_BANK0_ENABLE  IRQ0_SET_EN_0
#define PI_IRQ_BANK1_ENABLE  IRQ0_SET_EN_1
#define PI_IRQ_BANK2_ENABLE  IRQ0_SET_EN_2

#endif

#endif /*MAC_IRQ_H*/
