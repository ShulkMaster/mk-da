/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/dvd.h>
#include <dolphin/os.h>
#include <dolphin/os/OSContext.h>
#include <dolphin/os/OSInterrupt.h>
#include <dolphin/asm_sequences.inc>

typedef struct ApploaderHeader {
  char date[16];
  u32 entry;
  u32 size;
  u32 rebootSize;
  u32 reserved2;
} ApploaderHeader;

static ApploaderHeader Header ATTRIBUTE_ALIGN(32);
static void* SaveStart = NULL;
static void* SaveEnd = NULL;
static volatile BOOL Prepared = FALSE;

volatile u8 OS_REBOOT_BOOL AT_ADDRESS(0x800030E2);
u32 BOOT_REGION_START AT_ADDRESS(0x812FDFF0);
u32 BOOT_REGION_END AT_ADDRESS(0x812FDFEC);
u32 UNK_HOT_RESET1 AT_ADDRESS(0x817FFFF8);
u32 UNK_HOT_RESET2 AT_ADDRESS(0x817FFFFC);

void __DVDPrepareResetAsync(DVDCBCallback callback);
BOOL DVDCheckDisk(void);
BOOL DVDReadAbsAsyncForBS(DVDCommandBlock* block, void* address, s32 length,
                          s32 offset, DVDCBCallback callback);
void __OSDoHotReset(s32 resetCode);
void DVDInit(void);
BOOL DVDSetAutoInvalidation(BOOL autoInval);
void ICInvalidateRange(void* address, u32 length);
void ICFlashInvalidate(void);

static asm void Run(void* entrypoint) { SEQ_Run(); }

static void Callback(s32 result, DVDCommandBlock *block) { Prepared = TRUE; }

static inline void ReadApploader(void *addr, s32 length, s32 offset) {
  DVDCommandBlock block;

  while (!Prepared) {
  }

  DVDReadAbsAsyncForBS(&block, addr, length, offset + 0x2440, NULL);
  while (1) {
    switch (block.state) {
    case 0:
      return;
    case 1:
      break;
    case -1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
      __OSDoHotReset(UNK_HOT_RESET2);
      break;
    default:
      break;
    }
  }
}

void __OSReboot(u32 resetCode, u32 bootDol) {
  OSContext exceptionContext;
  u32 numBytes;
  u32 offset;

  OSDisableInterrupts();
  UNK_HOT_RESET2 = resetCode;
  UNK_HOT_RESET1 = 0;
  OS_REBOOT_BOOL = 1;
  BOOT_REGION_START = (u32)SaveStart;
  BOOT_REGION_END = (u32)SaveEnd;
  OSClearContext(&exceptionContext);
  OSSetCurrentContext(&exceptionContext);
  DVDInit();
  DVDSetAutoInvalidation(TRUE);
  __DVDPrepareResetAsync(Callback);
  if (!DVDCheckDisk()) {
    __OSDoHotReset(UNK_HOT_RESET2);
  }

  __OSMaskInterrupts(~0x1F);
  __OSUnmaskInterrupts(0x400);
  OSEnableInterrupts();

  ReadApploader(&Header, 32, 0);
  offset = Header.size + 0x20;
  numBytes = OSRoundUp32B(Header.rebootSize);
  ReadApploader((void *)0x81300000, numBytes, offset);

  ICInvalidateRange((void *)0x81300000, numBytes);
  Run((void *)0x81300000);
}
