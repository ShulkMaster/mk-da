/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/gx/GXPriv.h>

void GXInvalidateVtxCache(void) { GX_WRITE_U8(0x48); }
