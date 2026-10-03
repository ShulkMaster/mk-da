#include <dolphin/types.h>
#include <dolphin/asm_sequences.inc>

asm void matrixASMMult(void* unk00, const void* unk04, const void* unk08) { SEQ_matrixASMMult(); }

asm void vectorASMMultPoint(void* unk00, const void* unk04, s32 unk08, const void* unk0C) {
  SEQ_vectorASMMultPoint();
}

asm void vectorASMMultVector(void* unk00, const void* unk04, s32 unk08, const void* unk0C) {
  SEQ_vectorASMMultVector();
}
