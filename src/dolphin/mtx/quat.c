#include <dolphin/mtx.h>
#include <dolphin/asm_sequences.inc>

asm void PSQUATMultiply(const Quaternion* p, const Quaternion* q, Quaternion* pq) { SEQ_PSQUATMultiply(); }

static const f32 epsilon = 0.00001f;
static const f32 c_half = 0.5f;
static const f32 c_three = 3.0f;

asm void PSQUATNormalize(const Quaternion* src, Quaternion* unit) { SEQ_PSQUATNormalize(); }
