/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/mtx.h>
#include <dolphin/asm_sequences.inc>

asm void PSMTXMultVec(const Mtx m, const Vec* src, Vec* dst) { SEQ_PSMTXMultVec(); }
