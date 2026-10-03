#include "hvqm4snd.h"

#include <dolphin/os/OSAlloc.h>

HVQM4SoundContext* HVQM4DecSoundCreate(const HVQM4SoundInfo* info) {
  /* Retail reserves 0x20 bytes; only the 0x18-byte header/state is accessed. */
  HVQM4SoundContext* context = OSAllocFromHeap(__OSCurrHeap, 0x20);
  if (context == NULL) {
    return NULL;
  }
  context->info = *info;
  return context;
}

int HVQM4DecSoundClose(HVQM4SoundContext* context) {
  if (context == NULL) {
    return 0;
  }
  OSFreeToHeap(__OSCurrHeap, context);
  return 1;
}

int HVQM4DecSoundDecode(HVQM4SoundContext* context,
                       const HVQM4SoundFrameHeader* header, u32 samples,
                       const u8* code, int track, void* out) {
  u8 format;
  u16 flags;
  u32 count;
  int size;
  u32 channels;
  if (context == NULL) {
    return 0;
  }
  if (out == NULL) {
    return 0;
  }
  format = context->info.format;
  flags = header->flags;
  if (format & 0x80) {
    count = samples >> 1;
  } else {
    count = samples;
  }
  channels = context->info.channels;
  if (channels == 1) {
    size = samples * 4;
    switch (format & 0x7f) {
    case 0:
      HVQM4DecodeAdpcmCh1(context->channel, code, out, flags, count, track);
      break;
    case 1:
      HVQM4DecodePcm16Ch1(code, out, flags, count, track);
      break;
    default:
      size = 0;
      break;
    }
  } else {
    size = samples * 4;
    switch (format & 0x7f) {
    case 0:
      HVQM4DecodeAdpcmCh2(context->channel, code, out, flags, count, track);
      break;
    case 1:
      HVQM4DecodePcm16Ch2(code, out, flags, count, track);
      break;
    case 4:
      HVQM4DecodeAdp8xCh2(context->channel, code, out, flags, count, track);
      break;
    default:
      size = 0;
      break;
    }
  }
  return size;
}
