/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/ar.h>
#include <dolphin/hw_regs.h>
#include <dolphin/os.h>
#include <dolphin/os/OSContext.h>
#include <dolphin/os/OSInterrupt.h>

extern ARCallback __AR_Callback_8041CBB0;
extern u32 __AR_Size_8041CBB4;
extern u32 __AR_InternalSize_8041CBB8;
extern u32 __AR_ExpansionSize_8041CBBC;
extern u32 __AR_StackPointer_8041CBC0;
extern u32 __AR_FreeBlocks_8041CBC4;
extern u32 *__AR_BlockLength_8041CBC8;
extern BOOL __AR_init_flag_8041CBCC;

static void __ARHandler(__OSInterrupt interrupt, OSContext *context);
static void __ARChecksize(void);
extern void DCFlushRange(void *address, u32 length);
extern void DCInvalidateRange(void *address, u32 length);
extern void PPCSync(void);
extern void *memset(void *address, int value, u32 length);
#define RoundUP32(x) (((u32)(x) + 31) & ~31)

ARCallback ARRegisterDMACallback(ARCallback callback) {
  ARCallback oldCb;
  BOOL enabled;
  oldCb = __AR_Callback_8041CBB0;
  enabled = OSDisableInterrupts();
  __AR_Callback_8041CBB0 = callback;
  OSRestoreInterrupts(enabled);
  return oldCb;
}

void ARStartDMA(u32 type, u32 mainmem_addr, u32 aram_addr, u32 length) {
  BOOL enabled;

  enabled = OSDisableInterrupts();

  __DSPRegs[16] = (u16)(__DSPRegs[16] & ~0x3ff) | (u16)(mainmem_addr >> 16);
  __DSPRegs[17] = (u16)(__DSPRegs[17] & ~0xffe0) | (u16)(mainmem_addr & 0xffff);
  __DSPRegs[18] = (u16)(__DSPRegs[18] & ~0x3ff) | (u16)(aram_addr >> 16);
  __DSPRegs[19] = (u16)(__DSPRegs[19] & ~0xffe0) | (u16)(aram_addr & 0xffff);
  __DSPRegs[20] = (u16)((__DSPRegs[20] & ~0x8000) | (type << 15));
  __DSPRegs[20] = (u16)(__DSPRegs[20] & ~0x3ff) | (u16)(length >> 16);
  __DSPRegs[21] = (u16)(__DSPRegs[21] & ~0xffe0) | (u16)(length & 0xffff);
  OSRestoreInterrupts(enabled);
}

u32 ARInit(u32 *stack_index_addr, u32 num_entries) {

  BOOL old;
  u16 refresh;

  if (__AR_init_flag_8041CBCC == TRUE) {
    return 0x4000;
  }

  old = OSDisableInterrupts();

  __AR_Callback_8041CBB0 = NULL;

  __OSSetInterruptHandler(6, __ARHandler);
  __OSUnmaskInterrupts(0x02000000);

  __AR_StackPointer_8041CBC0 = 0x4000;
  __AR_FreeBlocks_8041CBC4 = num_entries;
  __AR_BlockLength_8041CBC8 = stack_index_addr;

  refresh = (u16)(__DSPRegs[13] & 0x000000ff);

  __DSPRegs[13] = (u16)((__DSPRegs[13] & ~0x000000ff) | (refresh & 0x000000ff));

  __ARChecksize();

  __AR_init_flag_8041CBCC = TRUE;

  OSRestoreInterrupts(old);

  return __AR_StackPointer_8041CBC0;
}

u32 ARGetBaseAddress(void) { return 0x4000; }

u32 ARGetSize() { return __AR_Size_8041CBB4; }

static void __ARHandler(__OSInterrupt interrupt, OSContext *context) {

  OSContext exceptionContext;
  u16 tmp;

  tmp = __DSPRegs[5];
  tmp = (u16)((tmp & ~0x00000088) | 0x20);
  __DSPRegs[5] = tmp;

  OSClearContext(&exceptionContext);
  OSSetCurrentContext(&exceptionContext);

  if (__AR_Callback_8041CBB0) {
    (*__AR_Callback_8041CBB0)();
  }

  OSClearContext(&exceptionContext);
  OSSetCurrentContext(context);
}

static inline void __ARClearInterrupt(void) {

  u16 tmp;
  tmp = __DSPRegs[5];
  tmp = (u16)((tmp & ~(0x00000080 | 0x00000008)) | 0x00000020);
  __DSPRegs[5] = tmp;
}

static inline void __ARWaitForDMA(void) {

  while (__DSPRegs[5] & 0x0200) {
  }
}

static inline void __ARWriteDMA(u32 mmem_addr, u32 aram_addr, u32 length) {

  __DSPRegs[16] = (u16)((__DSPRegs[16] & ~0x03ff) | (u16)(mmem_addr >> 16));
  __DSPRegs[16 + 1] = (u16)((__DSPRegs[16 + 1] & ~0xffe0) | (u16)(mmem_addr & 0xffff));

  __DSPRegs[18] = (u16)((__DSPRegs[18] & ~0x03ff) | (u16)(aram_addr >> 16));
  __DSPRegs[18 + 1] = (u16)((__DSPRegs[18 + 1] & ~0xffe0) | (u16)(aram_addr & 0xffff));

  __DSPRegs[20] = (u16)(__DSPRegs[20] & ~0x8000);

  __DSPRegs[20] = (u16)((__DSPRegs[20] & ~0x03ff) | (u16)(length >> 16));
  __DSPRegs[20 + 1] = (u16)((__DSPRegs[20 + 1] & ~0xffe0) | (u16)(length & 0xffff));

  __ARWaitForDMA();

  __ARClearInterrupt();
}

static inline void __ARReadDMA(u32 mmem_addr, u32 aram_addr, u32 length) {

  __DSPRegs[16] = (u16)((__DSPRegs[16] & ~0x03ff) | (u16)(mmem_addr >> 16));
  __DSPRegs[16 + 1] = (u16)((__DSPRegs[16 + 1] & ~0xffe0) | (u16)(mmem_addr & 0xffff));

  __DSPRegs[18] = (u16)((__DSPRegs[18] & ~0x03ff) | (u16)(aram_addr >> 16));
  __DSPRegs[18 + 1] = (u16)((__DSPRegs[18 + 1] & ~0xffe0) | (u16)(aram_addr & 0xffff));

  __DSPRegs[20] = (u16)(__DSPRegs[20] | 0x8000);

  __DSPRegs[20] = (u16)((__DSPRegs[20] & ~0x03ff) | (u16)(length >> 16));
  __DSPRegs[20 + 1] = (u16)((__DSPRegs[20 + 1] & ~0xffe0) | (u16)(length & 0xffff));

  __ARWaitForDMA();

  __ARClearInterrupt();
}

static void __ARChecksize(void) {
  u8 test_data_pad[63];
  u8 dummy_data_pad[63];
  u8 buffer_pad[63];
  u32 *test_data;
  u32 *dummy_data;
  u32 *buffer;
  u16 ARAM_mode;
  u32 ARAM_size;
  u32 i;

  while (!(__DSPRegs[11] & 1)) {
  }
  ARAM_mode = 3;
  ARAM_size = __AR_InternalSize_8041CBB8 = 0x1000000;
  __DSPRegs[9] = (u16)((__DSPRegs[9] & ~0x3F) | 0x23);
  test_data = (u32 *)RoundUP32(test_data_pad);
  dummy_data = (u32 *)RoundUP32(dummy_data_pad);
  buffer = (u32 *)RoundUP32(buffer_pad);
  for (i = 0; i < 8; i++) {
    test_data[i] = 0xDEADBEEF;
    dummy_data[i] = 0xBAD0BAD0;
  }
  DCFlushRange(test_data, 0x20);
  DCFlushRange(dummy_data, 0x20);
  __AR_ExpansionSize_8041CBBC = 0;
  __ARWriteDMA((u32)dummy_data, ARAM_size, 0x20U);
  __ARWriteDMA((u32)dummy_data, ARAM_size + 0x200000, 0x20U);
  __ARWriteDMA((u32)dummy_data, ARAM_size + 0x01000000, 0x20U);
  __ARWriteDMA((u32)dummy_data, ARAM_size + 0x200, 0x20U);
  __ARWriteDMA((u32)dummy_data, ARAM_size + 0x400000, 0x20U);
  memset(buffer, 0, 0x20);
  DCFlushRange(buffer, 0x20);
  __ARWriteDMA((u32)test_data, ARAM_size, 0x20U);
  DCInvalidateRange(buffer, 0x20);
  __ARReadDMA((u32)buffer, ARAM_size, 0x20U);
  PPCSync();
  if (*buffer == *test_data) {
    memset(buffer, 0, 0x20);
    DCFlushRange(buffer, 0x20);
    __ARReadDMA((u32)buffer, ARAM_size + 0x200000, 0x20U);
    PPCSync();
    if (*buffer == *test_data) {
      ARAM_size += 0x200000;
      __AR_ExpansionSize_8041CBBC = 0x200000;
    } else {
      memset(buffer, 0, 0x20);
      DCFlushRange(buffer, 0x20);
      __ARReadDMA((u32)buffer, ARAM_size + 0x01000000, 0x20U);
      PPCSync();
      if (*buffer == *test_data) {
        ARAM_mode |= 8;
        ARAM_size += 0x400000;
        __AR_ExpansionSize_8041CBBC = 0x400000;
      } else {
        memset(buffer, 0, 0x20);
        DCFlushRange(buffer, 0x20);
        __ARReadDMA((u32)buffer, ARAM_size + 0x200, 0x20U);
        PPCSync();
        if (*buffer == *test_data) {
          ARAM_mode |= 0x10;
          ARAM_size += 0x800000;
          __AR_ExpansionSize_8041CBBC = 0x800000;
        } else {
          memset(buffer, 0, 0x20);
          DCFlushRange(buffer, 0x20);
          __ARReadDMA((u32)buffer, ARAM_size + 0x400000, 0x20U);
          PPCSync();
          if (*buffer == *test_data) {
            ARAM_mode |= 0x18;
            ARAM_size += 0x01000000;
            __AR_ExpansionSize_8041CBBC = 0x01000000;
          } else {
            ARAM_mode |= 0x20;
            ARAM_size += 0x02000000;
            __AR_ExpansionSize_8041CBBC = 0x02000000;
          }
        }
      }
    }

    __DSPRegs[9] = ((u16)(__DSPRegs[9] & 0xFFFFFFC0) | ARAM_mode);
  }
  *(u32 *)OSPhysicalToUncached(0xD0) = ARAM_size;
  __AR_Size_8041CBB4 = ARAM_size;
}
