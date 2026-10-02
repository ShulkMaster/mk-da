/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/gx.h>

typedef struct GXLightObjInt {
  u32 padding[3];
  u32 color;
  float a0;
  float a1;
  float a2;
  float k0;
  float k1;
  float k2;
  float px;
  float py;
  float pz;
  float nx;
  float ny;
  float nz;
} GXLightObjInt;

void GXInitLightAttn(GXLightObj* lt_obj, f32 a0, f32 a1, f32 a2, f32 k0, f32 k1, f32 k2) {
  GXLightObjInt* obj = (GXLightObjInt* )lt_obj;
  obj->a0 = a0;
  obj->a1 = a1;
  obj->a2 = a2;
  obj->k0 = k0;
  obj->k1 = k1;
  obj->k2 = k2;
}

void GXInitLightAttnA(GXLightObj* lt_obj, f32 a0, f32 a1, f32 a2) {
  GXLightObjInt* obj = (GXLightObjInt* )lt_obj;
  obj->a0 = a0;
  obj->a1 = a1;
  obj->a2 = a2;
}
