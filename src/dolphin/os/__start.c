/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/os.h>
#include <dolphin/asm_sequences.inc>

typedef struct RomCopyInfo {
  void* rom;
  void* address;
  unsigned long size;
} RomCopyInfo;

typedef struct BssInitInfo {
  void* address;
  unsigned long size;
} BssInitInfo;

extern RomCopyInfo _rom_copy_info[];
extern BssInitInfo _bss_init_info[];
void* memcpy(void* destination, const void* source, unsigned long size);
void* memset(void* destination, int value, unsigned long size);
void __flush_cache(void* address, unsigned long size);

static inline void __copy_rom_section(void* destination, const void* source,
                                      unsigned long size) {
  if (size && destination != source) {
    memcpy(destination, source, size);
    __flush_cache(destination, size);
  }
}

static inline void __init_bss_section(void* destination, unsigned long size) {
  if (size) {
    memset(destination, 0, size);
  }
}

__declspec(section ".init") extern char _stack_addr[];
__declspec(section ".init") extern char _SDA_BASE_[];
__declspec(section ".init") extern char _SDA2_BASE_[];

void InitMetroTRK(void);
void DBInit(void);
void __init_user(void);
void __init_hardware(void);
int main(int argc, char* argv[]);
void exit(int status);
static void __init_registers(void);
static void __init_data(void);

__declspec(section ".init") static void __check_pad3(void) {
  if ((*(u16*)0x800030E4 & 0x0EEF) == 0x0EEF) {
    OSResetSystem(0, 0, FALSE);
  }
  return;
}

__declspec(section ".init") __declspec(weak) asm void __start(void) { SEQ___start(); }

__declspec(section ".init") static asm void __init_registers(void) { SEQ___init_registers(); }

#pragma scheduling off
__declspec(section ".init") static void __init_data(void) {
  RomCopyInfo* copy;
  BssInitInfo* bss;

  copy = _rom_copy_info;
  while (1) {
    if (copy->size == 0) {
      break;
    }
    __copy_rom_section(copy->address, copy->rom, copy->size);
    copy++;
  }

  bss = _bss_init_info;
  while (1) {
    if (bss->size == 0) {
      break;
    }
    __init_bss_section(bss->address, bss->size);
    bss++;
  }
}
