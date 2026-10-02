/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/gx.h>

typedef struct __GXTexObjInt_struct {
  u32 mode0;
  u32 mode1;
  u32 image0;
  u32 image3;
  void* userData;
  GXTexFmt fmt;
  u32 tlutName;
  u16 loadCnt;
  u8 loadFmt;
  u8 flags;
} __GXTexObjInt;

GXTexFmt GXGetTexObjFmt(const GXTexObj* to) {
  const __GXTexObjInt* t = (const __GXTexObjInt* )to;

  return t->fmt;
}
