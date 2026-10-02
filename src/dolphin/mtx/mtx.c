/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/mtx.h>
#include <dolphin/asm_sequences.inc>

/* Extern linkage: MWCC lays out static constants by first use (0.0f first); retail has 1.0f first. */
const f32 c_one = 1.0f;
const f32 c_zero = 0.0f;

asm void PSMTXScale(Mtx m, f32 xS, f32 yS, f32 zS) { SEQ_PSMTXScale(); }

asm void PSMTXQuat(Mtx m, const Quaternion* q) { SEQ_PSMTXQuat(); }
