/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/types.h>

typedef struct MessageBuffer MessageBuffer;
typedef int (*DispatchFunction)(MessageBuffer* message);

int TRKSetBufferPosition(MessageBuffer* message, u32 position);
int TRKReadBuffer1_ui8(MessageBuffer* message, u8* value);
int TRKDoUnsupported(MessageBuffer* message);
int TRKDoConnect(MessageBuffer* message);
int TRKDoDisconnect(MessageBuffer* message);
int TRKDoReset(MessageBuffer* message);
int TRKDoVersions(MessageBuffer* message);
int TRKDoSupportMask(MessageBuffer* message);
int TRKDoCPUType(MessageBuffer* message);
int TRKDoReadMemory(MessageBuffer* message);
int TRKDoWriteMemory(MessageBuffer* message);
int TRKDoReadRegisters(MessageBuffer* message);
int TRKDoWriteRegisters(MessageBuffer* message);
int TRKDoFlushCache(MessageBuffer* message);
int TRKDoSetOption(MessageBuffer* message);
int TRKDoContinue(MessageBuffer* message);
int TRKDoStep(MessageBuffer* message);
int TRKDoStop(MessageBuffer* message);

u32 gTRKDispatchTableSize;
DispatchFunction gTRKDispatchTable[33] = {
  TRKDoUnsupported,
  TRKDoConnect,
  TRKDoDisconnect,
  TRKDoReset,
  TRKDoVersions,
  TRKDoSupportMask,
  TRKDoCPUType,
  TRKDoUnsupported,
  TRKDoUnsupported,
  TRKDoUnsupported,
  TRKDoUnsupported,
  TRKDoUnsupported,
  TRKDoUnsupported,
  TRKDoUnsupported,
  TRKDoUnsupported,
  TRKDoUnsupported,
  TRKDoReadMemory,
  TRKDoWriteMemory,
  TRKDoReadRegisters,
  TRKDoWriteRegisters,
  TRKDoUnsupported,
  TRKDoUnsupported,
  TRKDoFlushCache,
  TRKDoSetOption,
  TRKDoContinue,
  TRKDoStep,
  TRKDoStop,
  TRKDoUnsupported,
  TRKDoUnsupported,
  TRKDoUnsupported,
  TRKDoUnsupported,
  TRKDoUnsupported,
  0,
};

int TRKDispatchMessage(MessageBuffer* message) {
  int error = 0x500;
  u8 command;

  TRKSetBufferPosition(message, 0);
  TRKReadBuffer1_ui8(message, &command);
  command &= 0xFF;
  if (command < gTRKDispatchTableSize) {
    error = gTRKDispatchTable[command](message);
  }
  return error;
}

int TRKInitializeDispatcher(void) {
  gTRKDispatchTableSize = 32;
  return 0;
}
