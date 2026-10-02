/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/dvd.h>
#include <dolphin/hw_regs.h>
#include <dolphin/os.h>

extern volatile BOOL ResetRequired_8041CCC0;
extern vu32 ResumeFromHere_8041CCB0;

void DVDLowReset(void);

void DVDReset(void) {
  DVDLowReset();
  __DIRegs[0] = 0x2a;
  __DIRegs[1] = __DIRegs[1];
  ResetRequired_8041CCC0 = FALSE;
  ResumeFromHere_8041CCB0 = 0;
}

s32 DVDGetCommandBlockStatus(const DVDCommandBlock* block) {
  BOOL enabled;
  s32 retVal;

  enabled = OSDisableInterrupts();

  if (block->state == 3) {
    retVal = 1;
  } else {
    retVal = block->state;
  }

  OSRestoreInterrupts(enabled);

  return retVal;
}
