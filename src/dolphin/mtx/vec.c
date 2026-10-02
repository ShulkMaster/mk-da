/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/mtx.h>
#include <dolphin/asm_sequences.inc>

asm void PSVECAdd(const Vec* a, const Vec* b, Vec* ab) { SEQ_PSVECAdd(); }

asm void PSVECSubtract(const Vec* a, const Vec* b, Vec* a_b) { SEQ_PSVECSubtract(); }

asm void PSVECScale(const Vec* src, Vec* dst, f32 scale) { SEQ_PSVECScale(); }

static const f32 c_half = 0.5f;
static const f32 c_three = 3.0f;

asm void PSVECNormalize(const Vec* src, Vec* unit) { SEQ_PSVECNormalize(); }

asm f32 PSVECMag(const Vec* v) { SEQ_PSVECMag(); }

asm f32 PSVECDotProduct(const Vec* a, const Vec* b) { SEQ_PSVECDotProduct(); }

asm void PSVECCrossProduct(const Vec* a, const Vec* b, Vec* axb) { SEQ_PSVECCrossProduct(); }
