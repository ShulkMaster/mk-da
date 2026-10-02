/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/asm_sequences.inc>

extern "C" {
void PPCHalt(void);
void __OSPSInit(void);
void __OSCacheInit(void);

__declspec(section ".init") asm void __init_hardware(void) { SEQ___init_hardware(); }

__declspec(section ".init") asm void __flush_cache(register void* address,
                                                  register unsigned int size) {
  SEQ___flush_cache();
}

typedef void (*VoidFunction)(void);
extern VoidFunction _ctors[];

static void __init_cpp(void);

void __init_user(void) {
  __init_cpp();
}

static void __init_cpp(void) {
  /* Call static initializers. */
  for (VoidFunction* constructor = _ctors; *constructor; constructor++) {
    (*constructor)();
  }
}

void _ExitProcess(void) {
  PPCHalt();
}
}
