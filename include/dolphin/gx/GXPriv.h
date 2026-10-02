#ifndef _DOLPHIN_GXPRIV
#define _DOLPHIN_GXPRIV

#include <dolphin/gx.h>
#include <dolphin/hw_regs.h>

extern void* __cpReg;
extern void* __peReg;

#define GX_WRITE_U8(v) (__GXWGFifo = (u8)(v))
#define GX_GET_CP_REG(offset) (((volatile u16*)__cpReg)[offset])
#define GX_GET_PE_REG(offset) (((volatile u16*)__peReg)[offset])
#define GX_SET_CP_REG(offset, value) (GX_GET_CP_REG(offset) = (value))
#define GX_SET_PE_REG(offset, value) (GX_GET_PE_REG(offset) = (value))

#define SET_REG_FIELD(reg, size, shift, val) \
  do { \
    (reg) = ((u32)(reg) & ~(((1 << (size)) - 1) << (shift))) | ((u32)(val) << (shift)); \
  } while (0)

#endif
