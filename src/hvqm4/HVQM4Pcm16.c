#include "hvqm4snd.h"

void HVQM4DecodePcm16Ch1(const u8* code, u8* out, u32 flags, int samples,
                       int track) {
  int bytes = samples * 2;
  bytes *= track;
  code += bytes;
  for (; samples > 0; samples--) {
    out[0] = out[2] = code[0];
    out[1] = out[3] = code[1];
    code += 2;
    out += 4;
  }
}

void HVQM4DecodePcm16Ch2(const u8* code, u8* out, u32 flags, int samples,
                       int track) {
  int bytes = samples * 4;
  bytes *= track;
  code += bytes;
  for (; samples > 0; samples--) {
    out[0] = code[0];
    out[1] = code[1];
    out[2] = code[2];
    out[3] = code[3];
    code += 4;
    out += 4;
  }
}
