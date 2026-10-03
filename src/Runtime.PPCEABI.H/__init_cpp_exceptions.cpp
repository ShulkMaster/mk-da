/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/asm_sequences.inc>

typedef struct __eti_init_info {
  void* eti_start;
  void* eti_end;
  void* code_start;
  unsigned long code_size;
} __eti_init_info;

extern __eti_init_info _eti_init_info[];

extern "C" {
int __register_fragment(struct __eti_init_info* info, char* TOC);
void __unregister_fragment(int fragmentID);
void __destroy_global_chain(void);
void __init_cpp_exceptions(void);
void __fini_cpp_exceptions(void);
}

static int fragmentID = -2;

static asm char* GetR2() { SEQ_GetR2__Fv(); }

extern void __fini_cpp_exceptions(void) {
  if (fragmentID != -2) {
    __unregister_fragment(fragmentID);
    fragmentID = -2;
  }
}

extern void __init_cpp_exceptions(void) {
  char* R2;
  if (fragmentID == -2) {
    R2 = GetR2();
    fragmentID = __register_fragment(_eti_init_info, R2);
  }
}

__declspec(section ".ctors") static void* const __init_cpp_exceptions_reference = __init_cpp_exceptions;
__declspec(section ".dtors") static void* const __destroy_global_chain_reference = __destroy_global_chain;
__declspec(section ".dtors") static void* const __fini_cpp_exceptions_reference = __fini_cpp_exceptions;
