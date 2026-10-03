#ifndef MSL_MSLBUS_H
#define MSL_MSLBUS_H

#include <dolphin/types.h>
#include <msl/listpool.h>
#include <msl/mslMusyX.h>

/* mslBus.dirty bits, applied by mslSlaveUpdateSettings. */
enum {
  MSL_BUS_DIRTY_VOLUME = 0x8,
  MSL_BUS_DIRTY_PITCH = 0x20,
  /* Recompounds unk20 from unk1C. */
  MSL_BUS_DIRTY_40 = 0x40,
  /* Sets MSL_BUS_PAUSED. */
  MSL_BUS_DIRTY_PAUSE = 0x100,
  /* Clears MSL_BUS_PAUSED. */
  MSL_BUS_DIRTY_UNPAUSE = 0x200,
  /* Clears MSL_BUS_PAUSED and starts prepared sounds (mslSlaveUpdateSounds). */
  MSL_BUS_DIRTY_400 = 0x400
};

/* mslBus.flags bits. */
enum {
  MSL_BUS_PAUSED = 0x4000
};

/* 0x38 bytes: size of the retail g_busDefault object. */
typedef struct mslBus {
  u32 flags;
  /* Change mask: mslSlaveUpdateSettings applies each set bit, then clears it. */
  u32 dirty;
  u32 unk08;
  f32 vol;
  /* mslVolCompound(master volCompound, vol) */
  f32 volCompound;
  f32 pitch;
  /* mslPitchCompound(master pitchCompound, pitch) */
  f32 pitchCompound;
  f32 unk1C;
  /* mslVolCompound(master unk20, unk1C) */
  f32 unk20;
  ListNode* sounds;
  ListNode* slaves;
  /* "mbus->busMaster" in mslUpdateBus's error string */
  struct mslBus *busMaster;
  mslCompoundMode *compoundMode;
  u32 unk34;
} mslBus;

void mslBusInitDefault(void);
void mslUpdateBuses(mslSoundSystem* sys);
void mslUpdateBus(ListNode* node, mslBus* master);

#endif
