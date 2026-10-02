/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/os.h>
#include <dolphin/os/OSBootInfo.h>
#include <dolphin/db.h>
#include <dolphin/exi.h>
#include <dolphin/si.h>
#include <dolphin/os/OSAlarm.h>
#include <dolphin/os/OSException.h>
#include <dolphin/os/OSInterrupt.h>
#include <dolphin/hw_regs.h>
#include <dolphin/asm_sequences.inc>

static OSBootInfo* BootInfo;
static u32* BI2DebugFlag;
static u32 BI2DebugFlagHolder;
static BOOL AreWeInitialized;
static __OSExceptionHandler* OSExceptionTable;
OSTime __OSStartTime;
BOOL __OSInIPL;

u32 OSGetConsoleType(void) {
  if (BootInfo == NULL || BootInfo->consoleType == 0) {
    return OS_CONSOLE_ARTHUR;
  }
  return BootInfo->consoleType;
}

u32 BOOT_REGION_START AT_ADDRESS(0x812FDFF0);
u32 BOOT_REGION_END AT_ADDRESS(0x812FDFEC);
u32 OSGetResetCode(void);
void* memset(void* destination, int value, unsigned long length);

static void ClearArena(void) {
  void* start;
  void* end;

  if ((u32)(OSGetResetCode() + 0x80000000) != 0U) {
    memset(OSGetArenaLo(), 0, (u32)OSGetArenaHi() - (u32)OSGetArenaLo());
    return;
  }
  start = (void*)BOOT_REGION_START;
  end = (void*)BOOT_REGION_END;
  if (start == NULL) {
    memset(OSGetArenaLo(), 0, (u32)OSGetArenaHi() - (u32)OSGetArenaLo());
    return;
  }
  if ((u32)OSGetArenaLo() < (u32)start) {
    if ((u32)OSGetArenaHi() <= (u32)start) {
      memset(OSGetArenaLo(), 0, (u32)OSGetArenaHi() - (u32)OSGetArenaLo());
      return;
    }
    memset(OSGetArenaLo(), 0, (u32)start - (u32)OSGetArenaLo());
    if ((u32)OSGetArenaHi() > (u32)end) {
      memset(end, 0, (u32)OSGetArenaHi() - (u32)end);
    }
  }
}

extern u32 __DVDLongFileNameFlag;
extern u32 __PADSpec;
extern u8 __ArenaLo[];
extern u8 __ArenaHi[];
extern u8 _stack_addr[];

void __OSInitSystemCall(void);
void __OSModuleInit(void);
void __OSInterruptInit(void);
void __OSContextInit(void);
void __OSCacheInit(void);
void __OSInitSram(void);
void __OSThreadInit(void);
void __OSInitAudioSystem(void);
void __OSInitMemoryProtection(void);
void EnableMetroTRKInterrupts(void);
void __OSResetSWInterruptHandler(__OSInterrupt interrupt, OSContext* context);
u32 PPCMfhid2(void);
void PPCMthid2(u32 value);

void DCFlushRangeNoSync(void* address, u32 length);
void ICInvalidateRange(void* address, u32 length);
BOOL __DBIsExceptionMarked(__OSException exception);
__OSExceptionHandler __OSSetExceptionHandler(__OSException exception,
                                           __OSExceptionHandler handler);
void OSDefaultExceptionHandler(__OSException exception, OSContext* context);
void* memcpy(void* destination, const void* source, unsigned long length);

void __OSEVStart(void);
void __OSEVEnd(void);
void __OSEVSetNumber(void);
void __DBVECTOR(void);
void __OSDBINTSTART(void);
void __OSDBJUMPSTART(void);
void __OSDBJUMPEND(void);

static void OSExceptionInit(void);


void OSInit(void) {
  u32 consoleType;
  u32* bi2StartAddr;

  if (!AreWeInitialized) {
    AreWeInitialized = TRUE;
    __OSStartTime = __OSGetSystemTime();
    OSDisableInterrupts();
    BI2DebugFlag = NULL;
    BootInfo = (OSBootInfo*)OSPhysicalToCached(0);
    __DVDLongFileNameFlag = 0;
    bi2StartAddr = *(u32**)OSPhysicalToCached(0xF4);
    if (bi2StartAddr) {
      BI2DebugFlag = &bi2StartAddr[3];
      __PADSpec = bi2StartAddr[9];
      *(u8*)0x800030E8 = (u8)*BI2DebugFlag;
      *(u8*)0x800030E9 = (u8)__PADSpec;
    } else if (BootInfo->arenaHi) {
      BI2DebugFlagHolder = *(u8*)0x800030E8;
      BI2DebugFlag = &BI2DebugFlagHolder;
      __PADSpec = *(u8*)0x800030E9;
    }
    __DVDLongFileNameFlag = TRUE;
    OSSetArenaLo(!BootInfo->arenaLo ? __ArenaLo : BootInfo->arenaLo);
    if (!BootInfo->arenaLo && BI2DebugFlag &&
        *BI2DebugFlag < 2) {
      OSSetArenaLo((void*)(((u32)_stack_addr + 31) & ~31));
    }
    OSSetArenaHi(!BootInfo->arenaHi ? __ArenaHi : BootInfo->arenaHi);
    OSExceptionInit();
    __OSInitSystemCall();
    OSInitAlarm();
    __OSModuleInit();
    __OSInterruptInit();
    __OSSetInterruptHandler(__OS_INTERRUPT_PI_RSW, __OSResetSWInterruptHandler);
    __OSContextInit();
    __OSCacheInit();
    EXIInit();
    SIInit();
    __OSInitSram();
    __OSThreadInit();
    __OSInitAudioSystem();
    PPCMthid2(PPCMfhid2() & ~0x40000000);
    if (BootInfo->consoleType & OS_CONSOLE_DEVELOPMENT) {
      BootInfo->consoleType = OS_CONSOLE_DEVHW1;
    } else {
      BootInfo->consoleType = OS_CONSOLE_RETAIL1;
    }
    BootInfo->consoleType += (__PIRegs[11] & 0xF0000000) >> 28;
    if (!__OSInIPL) {
      __OSInitMemoryProtection();
    }
    OSReport("\nDolphin OS $Revision: 49 $.\n");
    OSReport("Kernel built : %s %s\n", "Dec 17 2001", "18:46:45");
    OSReport("Console Type : ");
    consoleType = OSGetConsoleType();
    if (!(consoleType & OS_CONSOLE_DEVELOPMENT)) {
      OSReport("Retail %d\n", consoleType);
    } else {
      switch (consoleType) {
      case OS_CONSOLE_EMULATOR:
        OSReport("Mac Emulator\n");
        break;
      case OS_CONSOLE_PC_EMULATOR:
        OSReport("PC Emulator\n");
        break;
      case OS_CONSOLE_ARTHUR:
        OSReport("EPPC Arthur\n");
        break;
      case OS_CONSOLE_MINNOW:
        OSReport("EPPC Minnow\n");
        break;
      default:
        OSReport("Development HW%d\n", consoleType - 0x10000000 - 3);
        break;
      }
    }
    OSReport("Memory %d MB\n", BootInfo->memorySize >> 20);
    OSReport("Arena : 0x%x - 0x%x\n", OSGetArenaLo(), OSGetArenaHi());
    if (BI2DebugFlag && *BI2DebugFlag >= 2) {
      EnableMetroTRKInterrupts();
    }
    ClearArena();
    OSEnableInterrupts();
  }
}

static u32 __OSExceptionLocations[] = {
  0x100, 0x200, 0x300, 0x400, 0x500, 0x600, 0x700, 0x800,
  0x900, 0xC00, 0xD00, 0xF00, 0x1300, 0x1400, 0x1700,
};

static void OSExceptionInit(void) {
  __OSException exception;
  void* destAddr;

  // These two vars change the exception number embedded in the handler code.
  u32* opCodeAddr;
  u32 oldOpCode;
  u8* handlerStart;
  u32 handlerSize;

  opCodeAddr = (u32*)__OSEVSetNumber;
  oldOpCode = *opCodeAddr;
  handlerStart = (u8*)__OSEVStart;
  handlerSize = (u32)((u8*)__OSEVEnd - (u8*)__OSEVStart);
  // Install the debugger integrator only on the first OSInit after BS2.
  destAddr = OSPhysicalToCached(0x60);
  if (*(u32*)destAddr == 0) {
    DBPrintf("Installing OSDBIntegrator\n");
    memcpy(destAddr, (void*)__OSDBINTSTART,
           (u32)__OSDBJUMPSTART - (u32)__OSDBINTSTART);
    DCFlushRangeNoSync(destAddr, (u32)__OSDBJUMPSTART - (u32)__OSDBINTSTART);
    __sync();
    ICInvalidateRange(destAddr, (u32)__OSDBJUMPSTART - (u32)__OSDBINTSTART);
  }
  for (exception = 0; exception < 15; exception++) {
    if (BI2DebugFlag && *BI2DebugFlag >= 2 &&
        __DBIsExceptionMarked(exception)) {
      DBPrintf(">>> OSINIT: exception %d commandeered by TRK\n", exception);
      continue;
    }
    *opCodeAddr = oldOpCode | exception;
    if (__DBIsExceptionMarked(exception)) {
      DBPrintf(">>> OSINIT: exception %d vectored to debugger\n", exception);
      memcpy((void*)__DBVECTOR, (void*)__OSDBJUMPSTART,
             (u32)__OSDBJUMPEND - (u32)__OSDBJUMPSTART);
    } else {
      u32* ops = (u32*)__DBVECTOR;
      int cb;
      for (cb = 0; cb < (u32)__OSDBJUMPEND - (u32)__OSDBJUMPSTART; cb += sizeof(u32)) {
        *ops++ = 0x60000000;
      }
    }
    destAddr = OSPhysicalToCached(__OSExceptionLocations[exception]);
    memcpy(destAddr, handlerStart, handlerSize);
    DCFlushRangeNoSync(destAddr, handlerSize);
    __sync();
    ICInvalidateRange(destAddr, handlerSize);
  }
  OSExceptionTable = OSPhysicalToCached(0x3000);
  for (exception = 0; exception < 15; exception++) {
    __OSSetExceptionHandler(exception, OSDefaultExceptionHandler);
  }
  // Restore the opcode so an application can restart without reloading text.
  *opCodeAddr = oldOpCode;
  DBPrintf("Exceptions initialized...\n");
}

static asm void __OSDBIntegrator(void) { SEQ___OSDBIntegrator(); }

static asm void __OSDBJump(void) { SEQ___OSDBJump(); }


__OSExceptionHandler __OSSetExceptionHandler(__OSException exception,
                                            __OSExceptionHandler handler) {
  __OSExceptionHandler oldHandler = OSExceptionTable[exception];
  OSExceptionTable[exception] = handler;
  return oldHandler;
}

__OSExceptionHandler __OSGetExceptionHandler(__OSException exception) {
  return OSExceptionTable[exception];
}

static asm void OSExceptionVector(void) { SEQ_OSExceptionVector(); }

asm void OSDefaultExceptionHandler(register __OSException exception,
                                   register OSContext* context) {
  SEQ_OSDefaultExceptionHandler();
}

extern u32 PPCMfhid2(void);
extern void PPCMthid2(u32 value);
extern void ICFlashInvalidate(void);

void __OSPSInit(void) {
  PPCMthid2(PPCMfhid2() | 0xA0000000);
  ICFlashInvalidate();
  __sync();
  asm { SEQ___OSPSInitGQR() }
}

u32 __OSGetDIConfig(void) {
  return __DIRegs[9] & 0xFF;
}
