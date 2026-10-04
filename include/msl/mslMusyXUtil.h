#ifndef MSL_MSLMUSYXUTIL_H
#define MSL_MSLMUSYXUTIL_H

#include <dolphin/types.h>

f32 dBToLinear(f32 db);
u16 mslMusyXScalePitch(u16 pitch, u16 scale);

f32 mkFpPan(u8 pan);
u8 mkMusyXPan(f32 pan);
f32 mkFpPitch(u16 pitch);
u16 mkMusyXPitch(f32 pitch);
void enableIRQ(void);
void disableIRQ(void);
u8 mslMusyXScale(u8 volume, u8 scale);
u8 mkMusyXVolume(f32 volume);

f32 mkFpVolume(u8 volume);
u8 mslMusyXScalePan(u8 pan, u8 scale);

#endif
