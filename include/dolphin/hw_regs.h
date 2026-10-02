#ifndef _DOLPHIN_HW_REGS
#define _DOLPHIN_HW_REGS

#include <dolphin/types.h>

typedef union PPCWGPipe {
  u8 u8;
  u16 u16;
  u32 u32;
  f32 f32;
} PPCWGPipe;

#ifdef __MWERKS__
volatile u16 __DSPRegs[] : 0xCC005000;
volatile u32 __AIRegs[] : 0xCC006C00;
volatile u32 __DIRegs[] : 0xCC006000;
volatile u32 __SIRegs[] : 0xCC006400;
volatile u32 __PIRegs[] : 0xCC003000;
volatile u32 __EXIRegs[] : 0xCC006800;
volatile PPCWGPipe __GXWGFifo : 0xCC008000;
volatile u16 __MEMRegs[] : 0xCC004000;
volatile u16 __VIRegs[] : 0xCC002000;
#else
#define __DSPRegs ((volatile u16*)0xCC005000)
#define __AIRegs ((volatile u32*)0xCC006C00)
#define __DIRegs ((volatile u32*)0xCC006000)
#define __SIRegs ((volatile u32*)0xCC006400)
#define __PIRegs ((volatile u32*)0xCC003000)
#define __EXIRegs ((volatile u32*)0xCC006800)
#define __GXWGFifo (*(volatile PPCWGPipe*)0xCC008000)
#define __MEMRegs ((volatile u16*)0xCC004000)
#define __VIRegs ((volatile u16*)0xCC002000)
#endif

#endif
