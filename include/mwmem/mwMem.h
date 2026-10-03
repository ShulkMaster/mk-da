#ifndef MKDA_MWMEM_MWMEM_H
#define MKDA_MWMEM_MWMEM_H

#include <dolphin/types.h>

typedef struct mwMemHeapParams {
  u32 unk00;
  u32 unk04;
  u32 unk08;
  u8 unk0C;
  u8 unk0D;
  u8 unk0E;
  u32 unk10;
  u32 unk14;
} mwMemHeapParams;

#ifdef __cplusplus
extern "C" {
#endif

int mwMemHeapGetDefaultParams(mwMemHeapParams* params);

#ifdef __cplusplus
}
#endif

#endif
