/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/os/OSContext.h>

volatile OSContext* __OSFPUContext AT_ADDRESS(0x800000D8);

void OSClearContext(OSContext* context) {
  context->mode = 0;
  context->state = 0;
  if (context == __OSFPUContext)
    __OSFPUContext = NULL;
}
