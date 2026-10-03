#ifndef MKDA_RENDERWARE_PROJECT_VECTOR_H
#define MKDA_RENDERWARE_PROJECT_VECTOR_H

#include <dolphin/types.h>

f32 _rwSqrt(f32 arg0);
f32 _rwInvSqrt(f32 arg0);
void _rwV3dNormalize(f32* arg0, const f32* arg1);
f32* RwV3dTransformPoints(f32* arg0, const f32* arg1, s32 arg2, const f32* arg3);

#endif
