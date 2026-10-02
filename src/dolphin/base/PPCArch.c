/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/base/PPCArch.h>
#include <dolphin/asm_sequences.inc>

asm u32 PPCMfmsr(void) { SEQ_PPCMfmsr(); }

asm void PPCMtmsr(u32 newMSR) { SEQ_PPCMtmsr(); }

asm u32 PPCMfhid0(void) { SEQ_PPCMfhid0(); }

asm u32 PPCMfl2cr(void) { SEQ_PPCMfl2cr(); }

asm void PPCMtl2cr(u32 newL2cr) { SEQ_PPCMtl2cr(); }

__declspec(weak) asm void PPCMtdec(u32 newDec) { SEQ_PPCMtdec(); }

asm void PPCSync(void) { SEQ_PPCSync(); }

__declspec(weak) asm void PPCHalt(void) { SEQ_PPCHalt(); }

asm u32 PPCMfhid2(void) { SEQ_PPCMfhid2(); }

asm void PPCMthid2(u32 newhid2) { SEQ_PPCMthid2(); }

asm u32 PPCMfwpar(void) { SEQ_PPCMfwpar(); }

asm void PPCMtwpar(u32 newwpar) { SEQ_PPCMtwpar(); }
