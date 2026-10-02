/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/os.h>
#include <dolphin/gx.h>
#include <dolphin/gx/GXPriv.h>

static __GXFifoObj DisplayListFifo;
static volatile __GXFifoObj *OldCPUFifo;
static GXData __savedGXdata;

void *memcpy(void *destination, const void *source, u32 size);

void GXBeginDisplayList(void *list, u32 size) {
  __GXFifoObj *cpuFifo = (__GXFifoObj *)GXGetCPUFifo();

  if (__GXData->dirtyState != 0) {
    __GXSetDirtyState();
  }

  if (__GXData->dlSaveContext != 0) {
    memcpy(&__savedGXdata, __GXData, sizeof(__savedGXdata));
  }

  DisplayListFifo.base = (u8 *)list;
  DisplayListFifo.top = (u8 *)list + size - 4;
  DisplayListFifo.size = size;
  DisplayListFifo.count = 0;
  DisplayListFifo.rdPtr = list;
  DisplayListFifo.wrPtr = list;
  __GXData->inDispList = TRUE;
  GXSaveCPUFifo((GXFifoObj *)cpuFifo);
  OldCPUFifo = cpuFifo;
  GXSetCPUFifo((GXFifoObj *)&DisplayListFifo);
}

u32 GXEndDisplayList(void) {
  u32 ov;
  u32 reg;
  BOOL enabled;
  u32 cpenable;
  u8 unused[4];

  if (__GXData->dirtyState != 0) {
    __GXSetDirtyState();
  }
  reg = GX_GET_PI_REG(5);
  ov = (reg >> 26) & 1;
  __GXSaveCPUFifoAux(&DisplayListFifo);
  GXSetCPUFifo((GXFifoObj *)OldCPUFifo);
  if (__GXData->dlSaveContext != 0) {
    enabled = OSDisableInterrupts();
    cpenable = __GXData->cpEnable;
    memcpy(__GXData, &__savedGXdata, sizeof(*__GXData));
    __GXData->cpEnable = cpenable;
    OSRestoreInterrupts(enabled);
  }
  __GXData->inDispList = 0;
  if (!ov) {
    return DisplayListFifo.count;
  }
  return 0;
}

void GXCallDisplayList(const void *list, u32 nbytes) {
  if (__GXData->dirtyState != 0) {
    __GXSetDirtyState();
  }
  if (*(u32 *)&__GXData->vNumNot == 0) {
    __GXSendFlushPrim();
  }

  GX_WRITE_U8(0x40);
  GX_WRITE_U32((u32)list);
  GX_WRITE_U32(nbytes);
}
