#include <dolphin/asm_sequences.inc>
#include <dolphin/trk.h>

extern u8 _db_stack_addr[];
extern u8 gTRKInterruptVectorTable[];
extern void OSResetSystem(int reset, u32 resetCode, int forceMenu);
extern void EnableEXI2Interrupts(void);
extern void TRKSaveExtended1Block(void);
extern int InitMetroTRKCommTable(int hardware_id);
extern int TRK_main(void);
extern void TRK_flush_cache(void* addr, int nBytes);

enum {
  PPC_SystemReset = 0x100,
  PPC_MachineCheck = 0x200,
  PPC_DataStorage = 0x300,
  PPC_InstructionStorage = 0x400,
  PPC_ExternalInterrupt = 0x500,
  PPC_Alignment = 0x600,
  PPC_Program = 0x700,
  PPC_FloatingPointUnavaiable = 0x800,
  PPC_Decrementer = 0x900,
  PPC_SystemCall = 0xC00,
  PPC_Trace = 0xD00,
  PPC_PerformanceMonitor = 0xF00,
  PPC_InstructionAddressBreakpoint = 0x1300,
  PPC_SystemManagementInterrupt = 0x1400,
  PPC_ThermalManagementInterrupt = 0x1700,
};

static u32 lc_base;

static u32 TRK_ISR_OFFSETS[15] = { PPC_SystemReset,
                                   PPC_MachineCheck,
                                   PPC_DataStorage,
                                   PPC_InstructionStorage,
                                   PPC_ExternalInterrupt,
                                   PPC_Alignment,
                                   PPC_Program,
                                   PPC_FloatingPointUnavaiable,
                                   PPC_Decrementer,
                                   PPC_SystemCall,
                                   PPC_Trace,
                                   PPC_PerformanceMonitor,
                                   PPC_InstructionAddressBreakpoint,
                                   PPC_SystemManagementInterrupt,
                                   PPC_ThermalManagementInterrupt };

void __TRK_copy_vectors(void);
__declspec(section ".init") void __TRK_reset(void) { __TRK_copy_vectors(); }

asm void InitMetroTRK(void) { SEQ_InitMetroTRK(); }

void EnableMetroTRKInterrupts(void) { EnableEXI2Interrupts(); }

u32 TRKTargetTranslate(u32 addr) {
  if (addr >= lc_base && addr < lc_base + 0x4000) {
    if ((gTRKCPUState.Extended1.DBAT3U & 3) != 0) {
      return addr;
    }
  }

  return addr & 0x3FFFFFFF | 0x80000000;
}

inline void TRK_copy_vector(u32 offset) {
  void* destPtr = (void*)TRKTargetTranslate(offset);
  TRK_memcpy(destPtr, gTRKInterruptVectorTable + offset, 0x100);
  TRK_flush_cache(destPtr, 0x100);
}

inline void __TRK_copy_vectors(void) {
  int i;
  u32 mask;

  mask = *(u32*)TRKTargetTranslate(0x44);

  for (i = 0; i <= 14; ++i) {
    if (mask & (1 << i)) {
      TRK_copy_vector(TRK_ISR_OFFSETS[i]);
    }
  }
}

DSError TRKInitializeTarget(void) {
  gTRKState.isStopped = TRUE;
  gTRKState.msr = __TRK_get_MSR();
  lc_base = 0xE0000000;
  return DS_NoError;
}
