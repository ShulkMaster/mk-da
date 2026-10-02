/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/os.h>
#include <dolphin/asm_sequences.inc>

void* memcpy(void* destination, const void* source, unsigned long length);

void __OSSystemCallVectorStart(void);
void __OSSystemCallVectorEnd(void);
void DCFlushRangeNoSync(void* address, u32 length);
void ICInvalidateRange(void* address, u32 length);

static asm void SystemCallVector(void) { SEQ_SystemCallVector(); }

void __OSInitSystemCall(void) {
  void* address = OSPhysicalToCached(0xC00);

  memcpy(address, __OSSystemCallVectorStart,
         (u32)__OSSystemCallVectorEnd - (u32)__OSSystemCallVectorStart);
  DCFlushRangeNoSync(address, 0x100);
  __sync();
  ICInvalidateRange(address, 0x100);
}
