#include <dolphin/asm_sequences.inc>

asm void TRK_flush_cache(register void* addr, register int nBytes) {
  SEQ_TRK_flush_cache();
}
