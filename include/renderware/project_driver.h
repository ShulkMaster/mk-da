#ifndef MKDA_RENDERWARE_PROJECT_DRIVER_H
#define MKDA_RENDERWARE_PROJECT_DRIVER_H

#include <dolphin/types.h>

extern u32 _RwDlStateCache[0x1D];
extern s32 _RwGameCubeRasterExtOffset;
extern u16 _RwDlTokenCurrent;
void _rwDlTextureRasterFlush(void);
void _rwDlRenderStateSetAlphaComp(s32 arg0);
extern s32 _RwDlFSAA;
extern s32 _RwDlFSAATop;
extern s32 _RwDlHalfHeight;
extern u8* _RwDlRenderMode;

#endif
