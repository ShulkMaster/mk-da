#ifndef _DOLPHIN_GX
#define _DOLPHIN_GX

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef u8 GXBool;

typedef enum _GXCompare {
  GX_NEVER,
  GX_LESS,
  GX_EQUAL,
  GX_LEQUAL,
  GX_GREATER,
  GX_NEQUAL,
  GX_GEQUAL,
  GX_ALWAYS,
} GXCompare;

typedef enum _GXAlphaReadMode {
  GX_READ_00 = 0,
  GX_READ_FF = 1,
  GX_READ_NONE = 2,
} GXAlphaReadMode;

typedef enum _GXTexFmt {
  GX_TF_I4 = 0x0,
  GX_TF_I8 = 0x1,
  GX_TF_IA4 = 0x2,
  GX_TF_IA8 = 0x3,
  GX_TF_RGB565 = 0x4,
  GX_TF_RGB5A3 = 0x5,
  GX_TF_RGBA8 = 0x6,
  GX_TF_CMPR = 0xE,

  GX_CTF_R4 = 0x0 | 0x20,
  GX_CTF_RA4 = 0x2 | 0x20,
  GX_CTF_RA8 = 0x3 | 0x20,
  GX_CTF_YUVA8 = 0x6 | 0x20,
  GX_CTF_A8 = 0x7 | 0x20,
  GX_CTF_R8 = 0x8 | 0x20,
  GX_CTF_G8 = 0x9 | 0x20,
  GX_CTF_B8 = 0xA | 0x20,
  GX_CTF_RG8 = 0xB | 0x20,
  GX_CTF_GB8 = 0xC | 0x20,

  GX_TF_Z8 = 0x1 | 0x10,
  GX_TF_Z16 = 0x3 | 0x10,
  GX_TF_Z24X8 = 0x6 | 0x10,

  GX_CTF_Z4 = 0x0 | 0x10 | 0x20,
  GX_CTF_Z8M = 0x9 | 0x10 | 0x20,
  GX_CTF_Z8L = 0xA | 0x10 | 0x20,
  GX_CTF_Z16L = 0xC | 0x10 | 0x20,

  GX_TF_A8 = GX_CTF_A8,
} GXTexFmt;

typedef struct _GXTexObj {
  u32 dummy[8];
} GXTexObj;

typedef struct _GXLightObj {
  u32 dummy[16];
} GXLightObj;

void GXPokeAlphaMode(GXCompare func, u8 threshold);
void GXPokeAlphaRead(GXAlphaReadMode mode);
void GXPokeAlphaUpdate(GXBool update_enable);
void GXInitLightAttn(GXLightObj* lt_obj, f32 a0, f32 a1, f32 a2, f32 k0, f32 k1, f32 k2);
void GXInitLightAttnA(GXLightObj* lt_obj, f32 a0, f32 a1, f32 a2);
void GXInvalidateVtxCache(void);
GXTexFmt GXGetTexObjFmt(const GXTexObj* to);
void GXClearGPMetric(void);

#ifdef __cplusplus
}
#endif

#endif
