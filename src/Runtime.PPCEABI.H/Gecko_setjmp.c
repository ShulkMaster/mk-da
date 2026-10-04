#include <dolphin/asm_sequences.inc>

asm int __setjmp(register void* env) { SEQ___setjmp(); }

asm void longjmp(register void* env, register int val) { SEQ_longjmp(); }
