#include <dolphin/asm_sequences.inc>
#include <dolphin/os.h>
#include <dolphin/os/OSInterrupt.h>
#include <dolphin/exi.h>
#include <dolphin/trk.h>

#define BUFF_LEN 4362

typedef int (*DBCommFunc)();
typedef int (*DBCommInitFunc)(void*, __OSInterruptHandler);
typedef int (*DBCommReadFunc)(u8*, int);
typedef int (*DBCommWriteFunc)(const u8*, int);

typedef struct DBCommTable {
  DBCommInitFunc initialize_func;
  DBCommFunc init_interrupts_func;
  DBCommFunc peek_func;
  DBCommReadFunc read_func;
  DBCommWriteFunc write_func;
  DBCommFunc open_func;
  DBCommFunc close_func;
} DBCommTable;

extern void TRKInterruptHandler(void);
extern int Hu_IsStub(void);
extern int AMC_IsStub(void);
extern void DBInitComm(volatile u8** input_pending, EXICallback monitor_callback);
extern void DBInitInterrupts(void);
extern int DBQueryData(void);
extern int DBRead(void* buffer, u32 count);
extern int DBWrite(const void* src, int size);
extern void DBOpen(void);
extern void DBClose(void);
extern void EXI2_Init(volatile unsigned char** inputPendingPtrRef, EXICallback monitorCallback);
extern void EXI2_EnableInterrupts(void);
extern int EXI2_Poll(void);
extern int EXI2_ReadN(void* bytes, unsigned long length);
extern int EXI2_WriteN(const void* bytes, unsigned long length);
extern void EXI2_Reserve(void);
extern void EXI2_Unreserve(void);

u8 gWriteBuf[BUFF_LEN];
u8 gReadBuf[BUFF_LEN];
s32 _MetroTRK_Has_Framing;
s32 gReadCount;
s32 gReadPos;
s32 gWritePos;

DBCommTable gDBCommTable = {};

void TRKEXICallBack(__OSInterrupt interrupt, OSContext* ctx);

asm void TRKLoadContext(OSContext* ctx, u32) { SEQ_TRKLoadContext(); }

inline int TRKPollUART(void) { return gDBCommTable.peek_func(); }

inline int TRKReadUARTN(void* bytes, u32 length) {
  int readErr = gDBCommTable.read_func(bytes, length);
  return readErr == 0 ? 0 : -1;
}

inline int TRKWriteUARTN(const void* bytes, u32 length) {
  int writeErr = gDBCommTable.write_func(bytes, length);
  return writeErr == 0 ? 0 : -1;
}

void TRKEXICallBack(__OSInterrupt interrupt, OSContext* ctx) {
  OSEnableScheduler();
  TRKLoadContext(ctx, 0x500);
}

int InitMetroTRKCommTable(int hwId) {
  int result;

  if (hwId == 1) {
    OSReport("MetroTRK : Set to GDEV hardware\n");
    result = Hu_IsStub();

    gDBCommTable.initialize_func = (DBCommInitFunc)DBInitComm;
    gDBCommTable.init_interrupts_func = (DBCommFunc)DBInitInterrupts;
    gDBCommTable.peek_func = (DBCommFunc)DBQueryData;
    gDBCommTable.read_func = (DBCommReadFunc)DBRead;
    gDBCommTable.write_func = (DBCommWriteFunc)DBWrite;
    gDBCommTable.open_func = (DBCommFunc)DBOpen;
    gDBCommTable.close_func = (DBCommFunc)DBClose;
  } else {
    OSReport("MetroTRK : Set to AMC DDH hardware\n");
    result = AMC_IsStub();

    gDBCommTable.initialize_func = (DBCommInitFunc)EXI2_Init;
    gDBCommTable.init_interrupts_func = (DBCommFunc)EXI2_EnableInterrupts;
    gDBCommTable.peek_func = (DBCommFunc)EXI2_Poll;
    gDBCommTable.read_func = (DBCommReadFunc)EXI2_ReadN;
    gDBCommTable.write_func = (DBCommWriteFunc)EXI2_WriteN;
    gDBCommTable.open_func = (DBCommFunc)EXI2_Reserve;
    gDBCommTable.close_func = (DBCommFunc)EXI2_Unreserve;
  }

  return result;
}

DSError TRKInitializeIntDrivenUART(u32 baudRate, u32 flags, u32 unused, void* pendingInput) {
  gDBCommTable.initialize_func(pendingInput, TRKEXICallBack);
  return DS_NoError;
}

void EnableEXI2Interrupts(void) { gDBCommTable.init_interrupts_func(); }

int WriteUARTFlush(void) {
  int error = 0;

  while (gWritePos < 0x800) {
    gWriteBuf[gWritePos] = 0;
    gWritePos++;
  }

  if (gWritePos != 0) {
    error = TRKWriteUARTN(gWriteBuf, gWritePos);
    gWritePos = 0;
  }

  return error;
}

int WriteUART1(s8 byte) {
  gWriteBuf[gWritePos++] = byte;
  return 0;
}

int TRKReadUARTPoll(s8* byte) {
  int error = 4;
  s32 cnt;

  if (gReadPos >= gReadCount) {
    gReadPos = 0;
    cnt = gReadCount = TRKPollUART();

    if (cnt > 0) {
      if (cnt > BUFF_LEN) {
        gReadCount = BUFF_LEN;
      }

      error = TRKReadUARTN(gReadBuf, gReadCount);
      if (error != 0) {
        gReadCount = 0;
      }
    }
  }

  if (gReadPos < gReadCount) {
    *byte = gReadBuf[gReadPos++];
    error = 0;
  }

  return error;
}

void ReserveEXI2Port(void) { gDBCommTable.open_func(); }

void UnreserveEXI2Port(void) { gDBCommTable.close_func(); }

void TRK_board_display(char* str) { OSReport(str); }

void TRKUARTInterruptHandler(void) {}
