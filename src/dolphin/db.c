/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/db.h>
#include <dolphin/os.h>
#include <dolphin/asm_sequences.inc>

typedef u8 __OSException;

DBInterface* __DBInterface = NULL;
int DBVerbose;

void __DBExceptionDestination(void);
void PPCHalt(void);

void DBInit(void) {
  __DBInterface = (DBInterface*)OSPhysicalToCached(0x40);
  __DBInterface->ExceptionDestination = (void (*)())OSCachedToPhysical(__DBExceptionDestination);
  DBVerbose = TRUE;
}

void __DBExceptionDestinationAux(void) {
  u32* contextAddr = (void*)0x00C0;
  OSContext* context = (OSContext*)OSPhysicalToCached(*contextAddr);

  OSReport("DBExceptionDestination\n");
  OSDumpContext(context);
  PPCHalt();
}

asm void __DBExceptionDestination(void) { SEQ___DBExceptionDestination(); }

BOOL __DBIsExceptionMarked(__OSException exception) {
  u32 mask = 1 << exception;

  return (BOOL)(__DBInterface->exceptionMask & mask);
}

void DBPrintf(char* format, ...) {}
