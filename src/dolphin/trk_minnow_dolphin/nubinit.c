/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/types.h>

BOOL gTRKBigEndian;
extern volatile u8* gTRKInputPendingPtr;
void TRK_board_display(const char* message);
void TRKTerminateSerialHandler(void);
void usr_put_initialize(void);
int TRKInitializeEventQueue(void);
int TRKInitializeMessageBuffers(void);
int TRKInitializeDispatcher(void);
int TRKInitializeIntDrivenUART(u32 address, u32 channel, u32 unused,
                               volatile u8** input_pending);
void TRKTargetSetInputPendingPtr(volatile u8* input_pending);
int TRKInitializeSerialHandler(void);
int TRKInitializeTarget(void);

static inline BOOL TRKInitializeEndian(void) {
  u8 bendian[4];
  BOOL result = FALSE;

  gTRKBigEndian = TRUE;

  bendian[0] = 0x12;
  bendian[1] = 0x34;
  bendian[2] = 0x56;
  bendian[3] = 0x78;

  if (*(u32*)bendian == 0x12345678) {
    gTRKBigEndian = TRUE;
  } else if (*(u32*)bendian == 0x78563412) {
    gTRKBigEndian = FALSE;
  } else {
    result = TRUE;
  }

  return result;
}

void TRKNubWelcome(void) {
  TRK_board_display("MetroTRK for GAMECUBE v0.10");
}

int TRKTerminateNub(void) {
  TRKTerminateSerialHandler();
  return 0;
}

int TRKInitializeNub(void) {
  int error;
  int uart_error;

  error = TRKInitializeEndian();
  if (error == 0) {
    usr_put_initialize();
  }
  if (error == 0) {
    error = TRKInitializeEventQueue();
  }
  if (error == 0) {
    error = TRKInitializeMessageBuffers();
  }
  if (error == 0) {
    error = TRKInitializeDispatcher();
  }
  if (error == 0) {
    uart_error = TRKInitializeIntDrivenUART(0xE100, 1, 0, &gTRKInputPendingPtr);
    TRKTargetSetInputPendingPtr(gTRKInputPendingPtr);
    if (uart_error != 0) {
      error = uart_error;
    }
  }
  if (error == 0) {
    error = TRKInitializeSerialHandler();
  }
  if (error == 0) {
    error = TRKInitializeTarget();
  }
  return error;
}
