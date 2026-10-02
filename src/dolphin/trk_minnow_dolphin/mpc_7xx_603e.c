#include <dolphin/types.h>
#include <dolphin/asm_sequences.inc>

extern u8 gTRKCPUState[];
extern u8 gTRKRestoreFlags[];

asm void TRKSaveExtended1Block(void) { SEQ_TRKSaveExtended1Block(); }

asm void TRKRestoreExtended1Block(void) { SEQ_TRKRestoreExtended1Block(); }

u8 TRKTargetCPUMinorType(void) {
  return 0x54;
}
