/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/db.h>

typedef u8 __OSException;

DBInterface* __DBInterface = NULL;
int DBVerbose;

BOOL __DBIsExceptionMarked(__OSException exception) {
  u32 mask = 1 << exception;

  return (BOOL)(__DBInterface->exceptionMask & mask);
}

void DBPrintf(char* format, ...) {}
