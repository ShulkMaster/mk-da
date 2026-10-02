/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/types.h>

static u8 bUseSerialIO;

u8 GetUseSerialIO(void) {
  return bUseSerialIO;
}

void SetUseSerialIO(u8 serial_io) {
  bUseSerialIO = serial_io;
}
