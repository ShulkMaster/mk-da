#include <msl/mslMusyXUtil.h>
#include <math.h>
#include <dolphin/os.h>

static BOOL irqState = TRUE;
static u32 irqCounter;

f32 dBToLinear(f32 db) {
  f32 result = 0.0f;
  if (db != 0.0f) {
    result = pow(10.0, -(db / 10.0f));
  }
  return result;
}

f32 mkFpPitch(u16 pitch) {
  return pitch / 8192.0f;
}

f32 mkFpPan(u8 pan) {
  return 2.0f * ((f32)pan / 127.0f) - 1.0f;
}

f32 mkFpVolume(u8 volume) {
  return volume / 127.0f;
}

u16 mslMusyXScalePitch(u16 pitch, u16 scale) {
  if (pitch == 0 || scale == 0) {
    return 0;
  }
  return pitch + scale - 0x2000;
}

u8 mslMusyXScalePan(u8 pan, u8 scale) {
  s8 signedPan = pan;
  s8 signedScale = scale;
  f32 center = 0.0f;
  u8 result;
  center = center < -1.0 ? -1.0 : center > 1.0 ? 1.0 : center;
  center = 0.5f * (1.0f + center);
  signedScale -= (s32)(127.0f * center);
  signedPan += signedScale;
  result = signedPan;
  return result > 127 ? 127 : result;
}

u8 mslMusyXScale(u8 volume, u8 scale) {
  s32 scaled = (((volume + 1) * (scale + 1)) >> 7) - 1;
  /* Clears negative results (sign mask). */
  return scaled & ~(scaled >> 31);
}

u16 mkMusyXPitch(f32 pitch) {
  f32 logTwo = log(2.0);
  f32 logPitch = log(pitch);
  return (s32)(128.0f * (12.0f * (logPitch / logTwo))) + 0x2000;
}

u8 mkMusyXPan(f32 pan) {
  f64 bounded = pan;
  if (bounded < -1.0) {
    bounded = -1.0;
  } else if (bounded > 1.0) {
    bounded = 1.0;
  }
  pan = bounded;
  pan = 0.5f * (1.0f + pan);
  return 127.0f * pan;
}

u8 mkMusyXVolume(f32 volume) {
  return (127.0f * (volume < 0.0f ? 0.0f : volume > 1.0f ? 1.0f : volume));
}

void enableIRQ(void) {
  if (irqCounter != 0) {
    if (--irqCounter == 0) {
      OSRestoreInterrupts(irqState);
    }
  }
}

void disableIRQ(void) {
  if (irqCounter == 0) {
    irqState = OSDisableInterrupts();
  }
  ++irqCounter;
}
