#ifndef _DOLPHIN_HW_REGS
#define _DOLPHIN_HW_REGS

#include <dolphin/types.h>

#ifdef __MWERKS__
volatile u16 __DSPRegs[] : 0xCC005000;
#else
#define __DSPRegs ((volatile u16*)0xCC005000)
#endif

#endif
