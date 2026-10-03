#include <msl/mslbus.h>
#include <msl/mslMusyX.h>
#include <stdio.h>

static void mslSlaveUpdateSettings(mslBus* bus, mslBus* master);
static void mslSlaveUpdateSounds(mslBus* bus);
static void mslBusStartPreppedSound(mslBus* bus, ListNode* node);

static mslBus g_busDefault;

void mslBusInitDefault(void) {
  g_busDefault.volCompound = 1.0f;
  g_busDefault.pitchCompound = 1.0f;
  g_busDefault.unk20 = 1.0f;
  g_busDefault.flags = 0;
}

static void mslBusStartPreppedSound(mslBus* bus, ListNode* node) {
  u32 id;

  node = ListRemove(&node);
  id = ListNodeID(NULL, node);
  mslSoundPlayAfterPrep((mslSound*)id);
  if (node == ListNodeFind(&g_listPoolSound, id)) {
    ListInsert(&bus->sounds, node);
  }
}

static void mslSlaveUpdateSettings(mslBus* bus, mslBus* master) {
  bus->dirty |= master->dirty;
  if (bus->dirty & MSL_BUS_DIRTY_VOLUME) {
    f32 volume = mslVolCompound(master->volCompound, bus->vol, *bus->compoundMode);
    if (bus->volCompound != volume) {
      bus->volCompound = volume;
    } else {
      bus->dirty &= ~MSL_BUS_DIRTY_VOLUME;
    }
  }
  if (bus->dirty & MSL_BUS_DIRTY_PITCH) {
    f32 pitch = mslPitchCompound(master->pitchCompound, bus->pitch);
    if (bus->pitchCompound != pitch) {
      bus->pitchCompound = pitch;
    } else {
      bus->dirty &= ~MSL_BUS_DIRTY_PITCH;
    }
  }
  if (bus->dirty & MSL_BUS_DIRTY_40) {
    f32 value = mslVolCompound(master->unk20, bus->unk1C, *bus->compoundMode);
    if (bus->unk20 != value) {
      bus->unk20 = value;
    } else {
      bus->dirty &= ~MSL_BUS_DIRTY_40;
    }
  }
  if (bus->dirty & MSL_BUS_DIRTY_PAUSE) {
    if (!(bus->flags & MSL_BUS_PAUSED)) {
      bus->flags |= MSL_BUS_PAUSED;
    } else {
      bus->dirty &= ~MSL_BUS_DIRTY_PAUSE;
    }
  }
  if (bus->dirty & MSL_BUS_DIRTY_UNPAUSE) {
    if (bus->flags & MSL_BUS_PAUSED) {
      bus->flags &= ~MSL_BUS_PAUSED;
    } else {
      bus->dirty &= ~MSL_BUS_DIRTY_UNPAUSE;
    }
  }
  if ((bus->dirty & MSL_BUS_DIRTY_400) && (bus->flags & MSL_BUS_PAUSED)) {
    bus->flags &= ~MSL_BUS_PAUSED;
  }
}

static void mslSlaveUpdateSounds(mslBus* bus) {
  ListNode* node = bus->sounds;
  while (node != NULL) {
    mslSound* sound;
    ListNodeID(&g_listPoolSound, node);
    sound = ListNodeData(NULL, node);
    if (!mslSoundIsPlaying(sound)) {
      ListNodeFree(&g_listPoolSound, ListRemove(&node));
      if (sound->activeFlags.active) {
        mslSoundStop(sound);
      }
    } else {
      if (bus->dirty & MSL_BUS_DIRTY_VOLUME) {
        mslSoundSetVol(sound, bus->volCompound);
      }
      if (bus->dirty & MSL_BUS_DIRTY_PITCH) {
        mslSoundSetPitch(sound, bus->pitchCompound);
      }
      if ((bus->dirty & MSL_BUS_DIRTY_40) && (bus->dirty & MSL_BUS_DIRTY_PAUSE) &&
          (bus->dirty & MSL_BUS_DIRTY_UNPAUSE)) {
        mslBusStartPreppedSound(bus, node);
      }
      if (bus->dirty & MSL_BUS_DIRTY_400) {
        mslBusStartPreppedSound(bus, node);
      }
      ListNext(&node);
    }
  }
}

void mslUpdateBus(ListNode* node, mslBus* master) {
  mslBus* bus = ListNodeData(NULL, node);
  if (master != bus->busMaster) {
    printf("mslUpdateBus ERROR: master (%08x) != mbus->busMaster (%08x)\n", master, bus->busMaster);
  }
  mslSlaveUpdateSettings(bus, master);
  mslSlaveUpdateSounds(bus);
  node = bus->slaves;
  while (node != NULL) {
    mslUpdateBus(node, bus);
    ListNext(&node);
  }
  bus->dirty = 0;
}

void mslUpdateBuses(mslSoundSystem* sys) {
  ListNode* node = sys->busList;
  while (node != NULL) {
    mslUpdateBus(node, &g_busDefault);
    ListNext(&node);
  }
}
