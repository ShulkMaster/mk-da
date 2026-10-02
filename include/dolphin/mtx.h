#ifndef _DOLPHIN_MTX
#define _DOLPHIN_MTX

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Vec {
  f32 x, y, z;
} Vec, *VecPtr, Point3d, *Point3dPtr;

typedef struct Quaternion {
  f32 x, y, z, w;
} Quaternion, *QuaternionPtr, Qtrn, *QtrnPtr;

typedef f32 Mtx[3][4];
typedef f32 (*MtxPtr)[4];

void PSMTXScale(Mtx m, f32 xS, f32 yS, f32 zS);
void PSMTXQuat(Mtx m, const Quaternion* q);
void PSMTXMultVec(const Mtx m, const Vec* src, Vec* dst);

void PSVECAdd(const Vec* a, const Vec* b, Vec* ab);
void PSVECSubtract(const Vec* a, const Vec* b, Vec* a_b);
void PSVECScale(const Vec* src, Vec* dst, f32 scale);
void PSVECNormalize(const Vec* src, Vec* unit);
f32 PSVECMag(const Vec* v);
f32 PSVECDotProduct(const Vec* a, const Vec* b);
void PSVECCrossProduct(const Vec* a, const Vec* b, Vec* axb);

void PSQUATMultiply(const Quaternion* p, const Quaternion* q, Quaternion* pq);
void PSQUATNormalize(const Quaternion* src, Quaternion* unit);

#ifdef __cplusplus
}
#endif

#endif
