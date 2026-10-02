#ifndef _DOLPHIN_GXPRIV
#define _DOLPHIN_GXPRIV

#include <dolphin/gx.h>
#include <dolphin/hw_regs.h>

typedef struct __GXData_struct {
  u16 vNumNot;
  u16 bpSentNot;
  u16 vNum;
  u16 vLim;
  u32 cpEnable;
  u32 cpStatus;
  u32 cpClr;
  u32 vcdLo;
  u32 vcdHi;
  u32 vatA[8];
  u32 vatB[8];
  u32 vatC[8];
  u32 lpSize;
  u32 matIdxA;
  u32 matIdxB;
  u32 indexBase[4];
  u32 indexStride[4];
  u32 ambColor[2];
  u32 matColor[2];
  u32 suTs0[8];
  u32 suTs1[8];
  u32 suScis0;
  u32 suScis1;
  u32 tref[8];
  u32 iref;
  u32 bpMask;
  u32 IndTexScale0;
  u32 IndTexScale1;
  u32 tevc[16];
  u32 teva[16];
  u32 tevKsel[8];
  u32 cmode0;
  u32 cmode1;
  u32 zmode;
  u32 peCtrl;
  u32 cpDispSrc;
  u32 cpDispSize;
  u32 cpDispStride;
  u32 cpDisp;
  u32 cpTexSrc;
  u32 cpTexSize;
  u32 cpTexStride;
  u32 cpTex;
  GXBool cpTexZ;
  u32 genMode;
  GXTexRegion TexRegions[8];
  GXTexRegion TexRegionsCI[4];
  u32 nextTexRgn;
  u32 nextTexRgnCI;
  GXTlutRegion TlutRegions[20];
  GXTexRegion* (*texRegionCallback)(const GXTexObj*, GXTexMapID);
  GXTlutRegion* (*tlutRegionCallback)(u32);
  GXAttrType nrmType;
  GXBool hasNrms;
  GXBool hasBiNrms;
  u32 projType;
  f32 projMtx[6];
  f32 vpLeft;
  f32 vpTop;
  f32 vpWd;
  f32 vpHt;
  f32 vpNearz;
  f32 vpFarz;
  u8 fgRange;
  f32 fgSideX;
  u32 tImage0[8];
  u32 tMode0[8];
  u32 texmapId[16];
  u32 tcsManEnab;
  u32 tevTcEnab;
  GXPerf0 perf0;
  GXPerf1 perf1;
  u32 perfSel;
  GXBool inDispList;
  GXBool dlSaveContext;
  u8 dirtyVAT;
  u32 dirtyState;
} GXData;

extern GXData* gx;
#define __GXData gx

extern void* __cpReg;
extern void* __peReg;

#define GX_WRITE_U8(v) (__GXWGFifo.u8 = (u8)(v))
#define GX_WRITE_U16(v) (__GXWGFifo.u16 = (u16)(v))
#define GX_WRITE_U32(v) (__GXWGFifo.u32 = (u32)(v))
#define GX_WRITE_RAS_REG(v) \
  do { \
    GX_WRITE_U8(0x61); \
    GX_WRITE_U32(v); \
  } while (0)
#define GX_GET_CP_REG(offset) (((volatile u16*)__cpReg)[offset])
#define GX_GET_PE_REG(offset) (((volatile u16*)__peReg)[offset])
#define GX_SET_CP_REG(offset, value) (GX_GET_CP_REG(offset) = (value))
#define GX_SET_PE_REG(offset, value) (GX_GET_PE_REG(offset) = (value))

#define SET_REG_FIELD(reg, size, shift, val) \
  do { \
    (reg) = ((u32)(reg) & ~(((1 << (size)) - 1) << (shift))) | ((u32)(val) << (shift)); \
  } while (0)

void __GXSendFlushPrim(void);
void __GXSetGenMode(void);
void __GXFlushTextureState(void);

#define GET_REG_FIELD(reg, size, shift) ((int)((reg) >> (shift)) & ((1 << (size)) - 1))
#define GX_WRITE_XF_REG(addr, value) \
  do { \
    GX_WRITE_U8(0x10); \
    GX_WRITE_U32(0x1000 + (addr)); \
    GX_WRITE_U32(value); \
  } while (0)
#define GX_WRITE_SOME_REG4(command, address, value, index) \
  do { \
    GX_WRITE_U8(command); \
    GX_WRITE_U8(address); \
    GX_WRITE_U32(value); \
  } while (0)

void __GetImageTileCount(GXTexFmt fmt, u16 wd, u16 ht, u32* rowTiles, u32* colTiles, u32* cmpTiles);

#define GX_WRITE_SOME_REG2(command, address, value, index) \
  GX_WRITE_SOME_REG4(command, address, value, index)
#define GX_WRITE_SOME_REG3(command, address, value, index) \
  GX_WRITE_SOME_REG4(command, address, value, index)
#define GX_WRITE_F32(v) (__GXWGFifo.f32 = (f32)(v))

void __GXSetMatrixIndex(GXAttr matIdxAttr);

void __GXSetDirtyState(void);
void __GXSetSUTexRegs(void);
void __GXUpdateBPMask(void);
void __GXSetVCD(void);
void __GXSetVAT(void);
void __GXSetRange(f32 nearz, f32 sidex);

#endif
