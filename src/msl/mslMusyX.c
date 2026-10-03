#include <msl/mslMusyX.h>
#include <msl/mslMusyXStream.h>
#include <dolphin/os/OSContext.h>
#include <dolphin/os.h>
#include <stdio.h>
#include <string.h>
#include <msl/mslMem.h>
#include <msl/mslMusyXUtil.h>
#include <mfl/mflFile.h>
#include <msl/mslbus.h>
#include <dolphin/ai.h>
#include <dolphin/ar.h>
#include <dolphin/arq.h>

static void mslSoundCheckPreload(mslSound* sound);
static void mslSoundPreload(mslSound* sound, mslSoundCommand* command);
static void mslSoundProcess(mslSound* sound);
static void mslPlaybackAdjustImm(mslPlayback* playback, mslSoundMix* mix);
static void mslPlaybackAdjustProcess(mslSound* sound);
static void mslMusyXDMAWrapperCallback(void);
s32 filemgr_init(void);
static void mslPlaybackStart(mslSound* sound, mslPlayback* playback);
static void mslUpdateThread(mslSoundSystem* sys);
static void mslBankRemap(mslSoundSystem* sys, mslBank* bank, void* samples, const char* name);

static mslInitConfig g_initDefault = { 12, 1, 10 };
static mslSysInitConfig g_sysinitDefault = { 16, 0, 0, 47, 0x00FFC000 };

ListPool g_listPoolBank;
ListPool g_listPoolBus;
ListPool g_listPoolSound;
u8 g_listMemBank[10 * sizeof(ListNode)];
u8 g_listMemBus[64 * (sizeof(ListNode) + 0x38)];
u8 g_listMemSound[500 * sizeof(ListNode)];
static u32 aramMemArray[2];
static mslSoundSystem* gSoundSystems[2];
s32 mslErrorFlag;

/* Defined here, between the static declarations: uninitialized statics are
   emitted in reverse order of first declaration, and `active` must sit at
   retail .sbss+0x14 (between MusyXDMAStackEnd and mslErrorFlag). */
static inline void mslMusyXSwitchFiber(void (**callback)(void), void** stack) {
  static s32 active;

  if (*callback != NULL && *stack != NULL && active == 0) {
    active = 1;
    OSSwitchFiber((u32)*callback, (u32)*stack);
    active = 0;
  }
}

static void* MusyXDMAStackEnd;
static void* MusyXDMAStack;
static void (*MusyXDMACallback)(void);
static u32 gSoundSystemCount;
static u32 mslInitialized;

static inline void mslSoundSetVolOperation(mslSound* sound, f32 volume);
static void mslPlaybackProcess(mslSound* sound);
static void mslGlobalAdjustProcess(mslSoundSystem* sys);

static inline int mslSoundInSystemOperation(mslSoundSystem* sys, mslSound* sound) {
  u32* words = (u32*)sys->sounds;
  mslSound* end = sys->sounds + sys->soundCount;
  u32* start = (u32*)sys->sounds;
  if (sound < sys->sounds || sound >= end) {
    OSReport("Bad ms pointer: %08x  pool range %08x to %08x \n", sound, start,
             end);
    while (words < (u32*)(sys->sounds + sys->soundCount)) {
      u32* row = words;
      words += 6;
      OSReport("%08x: %08x %08x %08x %08x %08x %08x\n", row,
               row[0], row[1], row[2], row[3], row[4], row[5]);
    }
    return 0;
  }
  return 1;
}

static inline mslPlayback* mslPlaybackPoolAllocOperation(mslSoundSystem* sys) {
  mslPlayback* playback;
  mslPlayback* last;
  mslPlayback* candidate;

  if (sys == NULL) {
    printf("mslPlaybackPoolAlloc: NULL msi\n");
    return NULL;
  }
  disableIRQ();
  last = sys->playbacks + (sys->playbackCount - 1);
  candidate = sys->nextFreePlayback;
  if (candidate == NULL) {
    for (candidate = sys->playbacks; candidate <= last; candidate++) {
      if (!candidate->streamFlags.unk80) {
        break;
      }
    }
    if (candidate > last) {
      playback = NULL;
      goto done;
    }
  }
  playback = candidate;
  do {
    candidate++;
    if (candidate > last) {
      candidate = sys->playbacks;
    }
  } while (candidate->streamFlags.unk80 && candidate != playback);
  if (candidate == playback) {
    candidate = NULL;
  }
  sys->nextFreePlayback = candidate;
done:
  if (playback != NULL) {
    memset(playback, 0, sizeof(*playback));
    playback->streamFlags.unk80 = 1;
  }
  enableIRQ();
  return playback;
}

static inline void mslPlaybackPoolFreeOperation(mslSoundSystem* system, mslPlayback* playback) {
  if (system == NULL || playback == NULL) {
    printf("mslPlaybackPoolFree: NULL pointer!  msi=%x mp=%x\n", system, playback);
  } else {
    disableIRQ();
    playback->streamFlags.unk80 = 0;
    enableIRQ();
  }
}

static inline f32 mslSoundGetVolOperation(mslSound* sound) {
  if (!mslSoundInSystemOperation(sound->bank->system, sound)) {
    printf("mslSoundGetVol ms pointer out of bounds %x\n", sound);
    return 0.0f;
  }
  return mkFpVolume(sound->mix.volume);
}

static inline void mslSoundUnPauseOperation(mslSound* sound) {
  if (!mslSoundInSystemOperation(sound->bank->system, sound)) {
    printf("mslSoundUnPause ms pointer out of bounds %x\n", sound);
    return;
  }
  if (sound != NULL && sound->bank != NULL && sound->bank->system != NULL) {
    mslSoundSetVolOperation(sound, sound->savedVolume);
  }
}

static inline void mslSoundSetVolOperation(mslSound* sound, f32 volume) {
  mslPlayback* playback;
  if (sound == NULL) {
    return;
  }
  if (!mslSoundInSystemOperation(sound->bank->system, sound)) {
    printf("mslSoundSetVol ms pointer out of bounds %x\n", sound);
    return;
  }
  if (sound->bank->system->flags.volumeInDB) {
    volume = dBToLinear(volume);
  }
  sound->mix.volume = mkMusyXVolume(volume);
  sound->mix.control.flags.volumeChanged = 1;
  sound->mix.control.flags.panChanged = 0;
  sound->mix.control.flags.pitchChanged = 0;
  playback = sound->playbackHead;
  while (playback != NULL) {
    mslPlaybackAdjustImm(playback, &sound->mix);
    playback = playback->next;
  }
  sound->mix.control.flags.volumeChanged = 0;
}

static inline void mslSlotRemove(mslSoundSlot* slot, mslSound* sound) {
  mslSound* next = sound->slotNext;

  if (next == NULL) {
    return;
  }
  if (slot->head != sound) {
    mslSound* previous = slot->head;
    while (previous != NULL && previous->slotNext != sound) {
      previous = previous->slotNext;
    }
    if (previous != NULL) {
      previous->slotNext = next;
      if (slot->tail == sound) {
        slot->tail = previous;
      }
    }
  } else {
    slot->head = next;
    if (slot->tail == sound) {
      slot->tail = NULL;
    }
  }
}

static inline void mslRemovePendingPlayback(mslSound* sound) {
  mslPlayback* removed;
  removed = sound->playbackHead;
  if (removed != NULL) {
    do {
      if (removed->streamFlags.activationPending) {
        if (removed->previous != NULL) {
          removed->previous->next = removed->next;
        }
        if (removed->next != NULL) {
          removed->next->previous = removed->previous;
        }
        if (removed->previous == NULL) {
          removed->sound->playbackHead = removed->next;
        }
        removed->next = NULL;
        removed->previous = NULL;
        mslPlaybackPoolFreeOperation(sound->bank->system, removed);
        removed = sound->playbackHead;
      } else {
        removed = removed->next;
      }
    } while (removed != NULL);
  }
  if (sound->preloadedPlayback != NULL &&
      sound->preloadedPlayback->streamFlags.activationPending) {
    mslPlayback* preloaded = sound->preloadedPlayback;
    sound->preloadedPlayback = NULL;
    if (preloaded->sound != NULL) {
      if (preloaded->previous != NULL) {
        preloaded->previous->next = preloaded->next;
      }
      if (preloaded->next != NULL) {
        preloaded->next->previous = preloaded->previous;
      }
      if (preloaded->previous == NULL) {
        preloaded->sound->playbackHead = preloaded->next;
      }
      preloaded->next = NULL;
      preloaded->previous = NULL;
    }
    mslPlaybackPoolFreeOperation(sound->bank->system, preloaded);
  }
}

static inline mslAdjustment* mslAdjustmentPoolAlloc(mslSoundSystem* sys) {
  mslAdjustment* adjustment;
  mslAdjustment* last;
  mslAdjustment* candidate;

  if (sys == NULL) {
    return NULL;
  }
  disableIRQ();
  last = sys->adjustmentPool + (sys->adjustmentCount - 1);
  candidate = sys->nextFreeAdjustment;
  if (candidate == NULL) {
    for (candidate = sys->adjustmentPool; candidate <= last; candidate++) {
      if (!candidate->control.state.occupied) {
        break;
      }
    }
    if (candidate > last) {
      adjustment = NULL;
      goto done;
    }
  }
  adjustment = candidate;
  do {
    candidate++;
    if (candidate > last) {
      candidate = sys->adjustmentPool;
    }
  } while (candidate->control.state.occupied && candidate != adjustment);
  if (candidate == adjustment) {
    candidate = NULL;
  }
  sys->nextFreeAdjustment = candidate;
done:
  if (adjustment != NULL) {
    memset(adjustment, 0, sizeof(*adjustment));
    adjustment->control.state.occupied = 1;
  }
  enableIRQ();
  return adjustment;
}

static inline mslSound* mslSoundPoolAlloc(mslSoundSystem* sys) {
  mslSound* sound;
  mslSound* last;
  mslSound* candidate;

  disableIRQ();
  last = sys->sounds + (sys->soundCount - 1);
  candidate = sys->nextFreeSound;
  if (candidate == NULL) {
    for (candidate = sys->sounds; candidate <= last; candidate++) {
      if (!candidate->activeFlags.active) {
        break;
      }
    }
    if (candidate > last) {
      sound = NULL;
      goto done;
    }
  }
  sound = candidate;
  do {
    candidate++;
    if (candidate > last) {
      candidate = sys->sounds;
    }
  } while (candidate->activeFlags.active && candidate != sound);
  if (candidate == sound) {
    candidate = NULL;
  }
  sys->nextFreeSound = candidate;
done:
  if (sound != NULL) {
    memset(sound, 0, sizeof(*sound));
    sound->activeFlags.active = 1;
  }
  enableIRQ();
  return sound;
}

static inline mslPlayback* mslSoundFindPlayback(mslSound* sound, mslPlaybackDefinition* definition) {
  mslPlayback* playback;

  for (playback = sound->playbackHead; playback != NULL; playback = playback->next) {
    if (playback->definition == definition) {
      return playback;
    }
  }
  return NULL;
}

static inline s32 mslSlotFindFree(mslSoundSystem* sys) {
  u32 i;

  for (i = sys->firstAutomaticSlot; i < 64; i++) {
    if (sys->slots[i].sound == NULL) {
      return i;
    }
  }
  return -1;
}

static inline BOOL mslSystemRegister(mslSoundSystem* sys) {
  if (gSoundSystemCount >= 2) {
    return FALSE;
  }
  gSoundSystems[gSoundSystemCount++] = sys;
  return TRUE;
}

static inline mslSound* mslSlotPop(mslSoundSlot* slot) {
  mslSound* sound = slot->head;
  if (sound != NULL) {
    slot->head = sound->slotNext;
    sound->slotNext = NULL;
    if (slot->head == NULL) {
      slot->tail = NULL;
    }
  }
  return sound;
}

static void mslMusyXDMAWrapperCallback(void) {
  mslMusyXSwitchFiber(&MusyXDMACallback, &MusyXDMAStack);
}

f32 mslVolCompound(f32 master, f32 volume, mslCompoundMode mode) {
  if (!mode.additive) {
    return master * volume;
  }
  master = (master + volume) - 1.0f;
  return master < 0.0f ? 0.0f : master;
}

f32 mslPitchCompound(f32 master, f32 pitch) {
  return master * pitch;
}

static void mslPlaybackStart(mslSound* sound, mslPlayback* playback) {
  mslSoundSystem* system = sound->bank->system;
  u8 systemVolume;
  u8 systemPan;
  u16 systemPitch;
  u8 pan;
  u16 pitch;
  u8 volume;

  if (playback->streamFlags.ducked) {
    systemVolume = mkMusyXVolume(system->volume * system->duckVolume);
    systemPan = mkMusyXPan(system->pan + system->duckPan);
    systemPitch = mkMusyXPitch(system->pitch * system->duckPitch);
  } else {
    systemVolume = mkMusyXVolume(system->volume);
    systemPan = mkMusyXPan(system->pan);
    systemPitch = mkMusyXPitch(system->pitch);
  }
  volume = sound->mix.volume;
  pan = sound->mix.pan;
  pitch = sound->mix.pitch;
  if (playback->definition->flags & 2) {
    playback->asset = sound->bank->assets + playback->definition->assetID;
    playback->stream = mslStreamStart(sound->bank, playback, volume, pan, FALSE);
    if (playback->stream == NULL) {
      playback->streamFlags.activationPending = 1;
    }
  } else {
    volume = mslMusyXScale(volume, systemVolume);
    pan = mslMusyXScalePan(pan, systemPan);
    pitch = mslMusyXScalePitch(pitch, systemPitch);
    playback->voice = sndFXStartEx(playback->definition->assetID, 127, pan, 0);
    if (playback->voice == SND_ID_ERROR) {
      playback->streamFlags.activationPending = 1;
      return;
    }
    if (playback->voice != SND_ID_ERROR) {
      sndFXCtrl(playback->voice, 7, volume);
    }
    if (sound->mix.control.flags.boundaryPan) {
      sndFXCtrl(playback->voice, 0x83, 127);
    }
    if (pitch != 0x2000) {
      sndFXCtrl14(playback->voice, 0x80, pitch);
    }
    if (playback->definition->flags & 1) {
      playback->secondVoice = sndFXStartEx(playback->definition->secondAssetID, 127, pan, 0);
      if (playback->secondVoice == SND_ID_ERROR) {
        playback->streamFlags.activationPending = 1;
        return;
      }
      if (playback->secondVoice != SND_ID_ERROR) {
        sndFXCtrl(playback->secondVoice, 7, volume);
      }
      if (sound->mix.control.flags.boundaryPan) {
        sndFXCtrl(playback->secondVoice, 0x83, 127);
      }
      if (pitch != 0x2000) {
        sndFXCtrl14(playback->secondVoice, 0x80, pitch);
      }
    }
  }
}

static void mslPlaybackAdjustImm(mslPlayback* playback, mslSoundMix* mix) {
  if (playback->definition->flags & 2) {
    if (mix->control.flags.volumeChanged) {
      playback->sound->mix.volume = mix->volume;
      mslStreamSetVol(playback, playback->sound->mix.volume);
    }
    if (mix->control.flags.panChanged) {
      playback->sound->mix.pan = mix->pan;
      mslStreamSetPan(playback, playback->sound->mix.pan);
    }
    if (mix->control.flags.pitchChanged) {
      mslStreamSetPitch(playback, playback->sound->mix.pitch);
    }
  } else {
    u8 volume;
    u8 pan;
    u16 pitch;
    mslSoundSystem* system = playback->sound->bank->system;

    if (playback->streamFlags.ducked) {
      volume = mkMusyXVolume(system->volume * system->duckVolume);
      pan = mkMusyXPan(system->pan + system->duckPan);
      pitch = mkMusyXPitch(system->pitch * system->duckPitch);
    } else {
      volume = mkMusyXVolume(system->volume);
      pan = mkMusyXPan(system->pan);
      pitch = mkMusyXPitch(system->pitch);
    }
    if (mix->control.flags.volumeChanged) {
      playback->sound->mix.volume = mix->volume;
      volume = mslMusyXScale(playback->sound->mix.volume, volume);
      if (playback->voice != SND_ID_ERROR) {
        sndFXCtrl(playback->voice, 7, volume);
      }
      if (playback->definition->flags & 1) {
        if (playback->secondVoice != SND_ID_ERROR) {
          sndFXCtrl(playback->secondVoice, 7, volume);
        }
      }
    }
    if (mix->control.flags.panChanged) {
      u8 scaledPan;

      playback->sound->mix.pan = mix->pan;
      scaledPan = mslMusyXScalePan(playback->sound->mix.pan, pan);
      sndFXCtrl(playback->voice, 10, scaledPan);
      if (mix->control.flags.boundaryPan) {
        sndFXCtrl(playback->voice, 0x83, 127);
      }
      if (playback->definition->flags & 1) {
        sndFXCtrl(playback->secondVoice, 10, scaledPan);
        if (mix->control.flags.boundaryPan) {
          sndFXCtrl(playback->secondVoice, 0x83, 127);
        }
      }
    }
    if (mix->control.flags.pitchChanged) {
      u16 scaledPitch;

      playback->sound->mix.pitch = mix->pitch;
      scaledPitch = mslMusyXScalePitch(playback->sound->mix.pitch, pitch);
      sndFXCtrl14(playback->voice, 0x80, scaledPitch);
      if (playback->definition->flags & 1) {
        sndFXCtrl14(playback->secondVoice, 0x80, scaledPitch);
      }
    }
  }
}

static void mslPlaybackAdjustProcess(mslSound* sound) {
  mslAdjustment* adjustment;
  OSTime now;
  OSTime current;
  mslAdjustment* next;
  f32 volume;
  f32 pan;
  f32 pitch;

  now = OSGetTime();
  for (adjustment = sound->adjustments; adjustment != NULL; adjustment = adjustment->next) {
    current = now;
    if (adjustment->endTime < now) {
      current = adjustment->endTime;
    }
    if (current == adjustment->endTime || adjustment->endTime < OSMillisecondsToTicks(16)) {
      volume = adjustment->targetVolume;
      pan = adjustment->targetPan;
      pitch = adjustment->targetPitch;
    } else {
      volume = adjustment->startVolume +
               ((f32)(current - adjustment->startTime) * (adjustment->targetVolume - adjustment->startVolume)) /
                   (f32)(adjustment->endTime - adjustment->startTime);
      pan = adjustment->startPan +
            ((f32)(current - adjustment->startTime) * (adjustment->targetPan - adjustment->startPan)) /
                (f32)(adjustment->endTime - adjustment->startTime);
      pitch = adjustment->startPitch +
              ((f32)(current - adjustment->startTime) * (adjustment->targetPitch - adjustment->startPitch)) /
                  (f32)(adjustment->endTime - adjustment->startTime);
    }
    sound->mix.volume = volume;
    sound->mix.pan = pan;
    sound->mix.pitch = pitch;
    if (current >= adjustment->endTime) {
      adjustment->control.state.completed = 1;
    }
  }
  for (adjustment = sound->adjustments; adjustment != NULL; adjustment = next) {
    next = adjustment->next;
    if (adjustment->control.state.completed) {
      if (adjustment->previous != NULL) {
        adjustment->previous->next = adjustment->next;
      } else {
        sound->adjustments = adjustment->next;
      }
      if (adjustment->next != NULL) {
        adjustment->next->previous = adjustment->previous;
      }
      if (sound->bank->system != NULL && adjustment != NULL) {
        disableIRQ();
        adjustment->control.state.occupied = 0;
        enableIRQ();
      }
    }
  }
}

static void mslSoundCheckPreload(mslSound* sound) {
  mslSoundCommand* command = sound->commandStream->commands;

  while (command->opcode != 7) {
    if (command->opcode == 1) {
      mslSoundPreload(sound, command);
      break;
    }
    command++;
  }
}

/* TODO: [near miss] 99.48%; saved-register allocation remains. */
static void mslSoundPreload(mslSound* sound, mslSoundCommand* command) {
  mslPlayback* candidate;
  u8 volume;
  u8 scaledPan;
  mslPlaybackDefinition* definition;
  mslBank* bank;
  mslPlayback* playback;

  volume = command->mix.volume;
  if (sound->bank->system->flags.volumeInDB) {
    volume = mkMusyXVolume(dBToLinear(mkFpVolume(volume)));
  }
  {
    u8 scaledVolume;
    scaledVolume = mslMusyXScale(sound->mix.volume, volume);
    sound->mix = command->mix;
    sound->mix.volume = scaledVolume;
    scaledPan = mslMusyXScalePan(sound->mix.pan, command->mix.pan);
    bank = sound->bank;
    playback = NULL;
    definition = bank->unk38 + command->assetIndex;
    if (definition->flags & 2) {
      mslPlayback* last;
      mslPlayback* first;
      mslSoundSystem* sys;

      sys = bank->system;
      if (sys == NULL) {
        printf("mslPlaybackPoolAlloc: NULL msi\n");
        playback = NULL;
      } else {
        disableIRQ();
        first = sys->playbacks;
        last = first + (sys->playbackCount - 1);
        candidate = sys->nextFreePlayback;
        if (candidate == NULL) {
          for (candidate = first; candidate <= last; candidate++) {
            if (!candidate->streamFlags.unk80) {
              break;
            }
          }
          if (candidate > last) {
            playback = NULL;
            goto done;
          }
        }
        playback = candidate;
        do {
          candidate++;
          if (candidate > last) {
            candidate = first;
          }
        } while (candidate->streamFlags.unk80 && candidate != playback);
        if (candidate == playback) {
          candidate = NULL;
        }
        sys->nextFreePlayback = candidate;
      done:
        if (playback != NULL) {
          memset(playback, 0, sizeof(*playback));
          playback->streamFlags.unk80 = 1;
        }
        enableIRQ();
      }

      if (playback == NULL) {
        playback = NULL;
      } else {
        playback->definition = definition;
        playback->unk18 = command->unk10;
        playback->unk1C = command->unk14;
        playback->asset = bank->assets + playback->definition->assetID;
        playback->streamFlags.ducked = command->mix.control.flags.ducked;
        playback->stream = mslStreamStart(bank, playback, scaledVolume, scaledPan, TRUE);
        if (playback->stream == NULL) {
          mslPlaybackPoolFreeOperation(bank->system, playback);
          playback = NULL;
        }
      }
    }
    if (playback != NULL) {
      sound->preloadedPlayback = playback;
    }
  }
}

/* TODO: [near miss] 99.17%; register allocation remains. */
static void mslSoundProcess(mslSound* sound) {
  u32 duration;
  OSTime delta;
  mslPlayback* playback;
  u16 pitch;
  mslSoundCommand* command;
  mslAdjustment* adjustment;
  mslSound* nested;
  mslPlaybackDefinition* definition;
  mslSoundCommand* reference;
  OSTime now;
  u8 pan;
  u8 volume;
  mslSoundMix mix;
  mslSoundSystem* sys;
  mslSound* target;
  mslSoundCommandStream* stream;
  mslSoundSystem* allocationSystem;

  now = OSGetTime();
  if (sound->flags.awaitingPlay) {
    return;
  }
  sound->flags.unk08 = 1;
  if (sound->flags.unk04 || mflGetDiscErrorStatus() != 0) {
    sound->lastTime = now;
    goto finish;
  }
  if (sound->sequenceCursor == NULL) {
    sound->sequenceCursor = sound->commandStream->commands;
    sound->sequenceStart = OSGetTime();
    sound->elapsedTicks = 0;
    delta = 0;
  } else {
    delta = now - sound->lastTime;
  }
  sound->elapsedTicks += delta;
  sound->lastTime = now;
  while (OSTicksToMilliseconds(sound->elapsedTicks) + 40 >= sound->sequenceCursor->time) {
    sound->elapsedTicks = 0;
    command = sound->sequenceCursor;
    switch (command->opcode) {
    case 1: {
      mslPlayback* preloaded;
      preloaded = sound->preloadedPlayback;
      if (preloaded == NULL) {
        preloaded = mslPlaybackPoolAllocOperation(sound->bank->system);
        if (preloaded == NULL) {
          sound->flags.unk10 = 1;
          goto finish;
        }
        preloaded->definition = sound->bank->unk38 + command->assetIndex;
        preloaded->unk18 = command->unk10;
        preloaded->unk1C = command->unk14;
        preloaded->streamFlags.ducked = command->mix.control.flags.ducked;
        preloaded->sound = sound;
        volume = command->mix.volume;
        if (sound->bank->system->flags.volumeInDB) {
          volume = mkMusyXVolume(dBToLinear(mkFpVolume(volume)));
        }
        pan = command->mix.pan;
        pitch = command->mix.pitch;
        sound->mix.volume = mslMusyXScale(sound->mix.volume, volume);
        sound->mix.pan = mslMusyXScalePan(sound->mix.pan, pan);
        sound->mix.pitch = mslMusyXScalePitch(sound->mix.pitch, pitch);
        mslPlaybackStart(sound, preloaded);
      } else {
        sound->preloadedPlayback = NULL;
        preloaded->sound = sound;
        mslStreamSetVol(preloaded, sound->mix.volume);
        mslStreamSetPan(preloaded, sound->mix.pan);
        mslStreamPreloadActivate(preloaded);
      }
      preloaded->previous = NULL;
      preloaded->next = sound->playbackHead;
      sound->playbackHead = preloaded;
      if (preloaded->next != NULL) {
        preloaded->next->previous = preloaded;
      }
      preloaded->sound = sound;
      break;
    }
    case 8:
      playback = mslSoundFindPlayback(sound, sound->bank->unk38 + command->assetIndex);
      if (playback != NULL) {
        if (playback->definition->flags & 2) {
          mslStreamStop(playback);
        } else {
          if (playback->voice != SND_ID_ERROR) {
            sndFXKeyOff(playback->voice);
          }
          if ((playback->definition->flags & 1) && playback->secondVoice != SND_ID_ERROR) {
            sndFXKeyOff(playback->secondVoice);
          }
        }
        playback->streamFlags.activationPending = 1;
      }
      break;
    case 2:
      if (command->mix.control.flags.master || command->mix.control.flags.ducked) {
        f32 startPitch;
        f32 startPan;
        f32 startVolume;

        sys = sound->bank->system;
        duration = command->unk14;
        adjustment = mslAdjustmentPoolAlloc(sys);
        if (adjustment == NULL) {
          break;
        }
        adjustment->control = command->mix.control;
        if (command->mix.control.flags.ducked) {
          startVolume = sys->duckVolume;
          startPan = sys->duckPan;
          startPitch = sys->duckPitch;
        } else if (command->mix.control.flags.master) {
          startVolume = sys->volume;
          startPan = sys->pan;
          startPitch = sys->pitch;
        }
        adjustment->startTime = now;
        adjustment->startVolume = startVolume;
        adjustment->startPan = startPan;
        adjustment->startPitch = startPitch;
        adjustment->endTime = now + duration * OSMillisecondsToTicks(1);
        adjustment->targetVolume = mkFpVolume(command->mix.volume);
        adjustment->targetPan = mkFpPan(command->mix.pan);
        adjustment->targetPitch = mkFpPitch(command->mix.pitch);
        adjustment->playback = NULL;
        adjustment->sound = NULL;
        adjustment->previous = NULL;
        adjustment->next = sys->adjustments;
        if (adjustment->next != NULL) {
          adjustment->next->previous = adjustment;
        }
        sys->adjustments = adjustment;
      } else {
        f32 startVolume;
        f32 startPitch;
        f32 rawVolume;
        f32 startPan;

        reference = sound->commandStream->commands + command->assetIndex;
        if (reference->opcode != 1) {
          break;
        }
        definition = sound->bank->unk38 + reference->assetIndex;
        if (definition == NULL) {
          break;
        }
        playback = mslSoundFindPlayback(sound, definition);
        if (playback == NULL) {
          break;
        }
        mix = command->mix;
        if (sound->bank->system->flags.volumeInDB) {
          mix.volume = mkMusyXVolume(dBToLinear(mkFpVolume(mix.volume)));
        }
        target = playback->sound;
        duration = command->unk14;
        rawVolume = target->rawVolume;
        allocationSystem = target->bank->system;
        adjustment = mslAdjustmentPoolAlloc(allocationSystem);
        if (adjustment == NULL) {
          break;
        }
        adjustment->control = mix.control;
        startVolume = target->mix.volume;
        startPan = target->mix.pan;
        startPitch = target->mix.pitch;
        adjustment->startTime = now;
        adjustment->startVolume = startVolume;
        adjustment->startPan = startPan;
        adjustment->startPitch = startPitch;
        adjustment->endTime = now + duration * OSMillisecondsToTicks(1);
        adjustment->targetVolume = mslMusyXScale(mix.volume, mkMusyXVolume(rawVolume));
        adjustment->targetPan = mslMusyXScalePan(mix.pan, startPan);
        adjustment->targetPitch = mslMusyXScalePitch(mix.pitch, startPitch);
        adjustment->playback = playback;
        adjustment->sound = target;
        adjustment->previous = NULL;
        adjustment->next = target->adjustments;
        if (adjustment->next != NULL) {
          adjustment->next->previous = adjustment;
        }
        playback->sound->adjustments = adjustment;
      }
      break;
    case 4:
      stream = sound->bank->commands + command->assetIndex;
      allocationSystem = sound->bank->system;
      nested = mslSoundPoolAlloc(allocationSystem);
      if (nested == NULL) {
        sound->flags.unk10 = 1;
      }
      nested->commandStream = stream;
      nested->sequenceCursor = NULL;
      nested->loopStart = NULL;
      nested->sequenceStart = 0;
      nested->flags.unk80 = 1;
      break;
    case 5:
      sound->loopStart = command + 1;
      sound->loopCount = -2;
      break;
    case 6:
      if (sound->loopCount == -2) {
        sound->loopCount = command->unk10 - 1;
      } else {
        if (sound->loopCount == 0) {
          break;
        }
        if (sound->loopCount != -1) {
          sound->loopCount--;
        }
      }
      sound->sequenceCursor = sound->loopStart;
      continue;
    case 7:
      sound->sequenceCursor = NULL;
      sound->flags.unk10 = 1;
      goto finish;
    }
    sound->sequenceCursor++;
  }
  if (sound->preloadedPlayback == NULL && sound->sequenceCursor != NULL && sound->sequenceCursor->opcode == 1) {
    mslSoundPreload(sound, sound->sequenceCursor);
  }
finish:
  mslPlaybackProcess(sound);
  mslRemovePendingPlayback(sound);
  sound->flags.unk08 = 0;
}

static void mslPlaybackProcess(mslSound* sound) {
  mslPlayback* playback;
  BOOL streaming;
  mslSoundSystem* system;
  u8 volume;
  u8 pan;
  u16 pitch;
  u8 systemVolume;
  u8 systemPan;
  u16 systemPitch;

  streaming = FALSE;
  system = sound->bank->system;
  mslPlaybackAdjustProcess(sound);
  for (playback = sound->playbackHead; playback != NULL; playback = playback->next) {
    if (playback->definition->flags & 2) {
      mslStreamProcess(playback);
      if (playback->streamFlags.unk20) {
        streaming = TRUE;
      }
    } else {
      if (playback->streamFlags.ducked) {
        systemVolume = mkMusyXVolume(system->volume * system->duckVolume);
        systemPan = mkMusyXPan(system->pan + system->duckPan);
        systemPitch = mkMusyXPitch(system->pitch * system->duckPitch);
      } else {
        systemVolume = mkMusyXVolume(system->volume);
        systemPan = mkMusyXPan(system->pan);
        systemPitch = mkMusyXPitch(system->pitch);
      }
      volume = mslMusyXScale(playback->sound->mix.volume, systemVolume);
      pan = mslMusyXScalePan(playback->sound->mix.pan, systemPan);
      pitch = mslMusyXScalePitch(playback->sound->mix.pitch, systemPitch);
      if (playback->definition->flags & 1) {
        if (playback->voice != SND_ID_ERROR && sndFXCheck(playback->voice) != SND_ID_ERROR) {
          if (playback->voice != SND_ID_ERROR) {
            sndFXCtrl(playback->voice, 7, volume);
          }
          sndFXCtrl(playback->voice, 10, pan);
          if (playback->sound->mix.control.flags.boundaryPan) {
            sndFXCtrl(playback->voice, 0x83, 127);
          }
          sndFXCtrl14(playback->voice, 0x80, pitch);
        }
        if (playback->secondVoice != SND_ID_ERROR && sndFXCheck(playback->secondVoice) != SND_ID_ERROR) {
          if (playback->secondVoice != SND_ID_ERROR) {
            sndFXCtrl(playback->secondVoice, 7, volume);
          }
          sndFXCtrl(playback->secondVoice, 10, pan);
          if (playback->sound->mix.control.flags.boundaryPan) {
            sndFXCtrl(playback->secondVoice, 0x83, 127);
          }
          sndFXCtrl14(playback->secondVoice, 0x80, pitch);
        }
      } else {
        if (playback->voice != SND_ID_ERROR && sndFXCheck(playback->voice) != SND_ID_ERROR) {
          if (playback->voice != SND_ID_ERROR) {
            sndFXCtrl(playback->voice, 7, volume);
          }
          sndFXCtrl(playback->voice, 10, pan);
          if (playback->sound->mix.control.flags.boundaryPan) {
            sndFXCtrl(playback->voice, 0x83, 127);
          }
          sndFXCtrl14(playback->voice, 0x80, pitch);
        }
      }
    }
  }
  sound->flags.unk04 = streaming;
}

/* TODO: [near miss] 95.50%; sys/sound allocation and four extra Boolean instructions remain. */
static void mslUpdateThread(mslSoundSystem* sys) {
  mslListenerState* mic = &sys->listenerState;
  mslSound* sound;
  mslBank* bank;
  u32 i;
  mslSoundSlot* slots;
  {
    int needsListenerUpdate = mic->enabled != 0 && (mic->dirty & 0x7F);
    if (needsListenerUpdate) {
      if (!sndUpdateListener(&mic->listener, &mic->position, &mic->direction,
                             &mic->heading, &mic->up, mic->volume, NULL)) {
        printf("mslUpdateThread: sndUpdateListener failed for Mic %x\n", mic);
      }
    }
  }
  mslUpdateBuses(sys);
  mslGlobalAdjustProcess(sys);
  bank = sys->banks;
  while (bank != NULL) {
    sound = bank->sounds;
    while (sound != NULL) {
      mslSound* next;
      if (!mslSoundInSystemOperation(sys, sound)) {
        printf("mslUpdateThread ms pointer out of bounds %x\n", sound);
        return;
      }
      next = sound->next;
      if (!sound->flags.unk10 && !sound->flags.unk08) {
        mslSoundProcess(sound);
      }
      if (sound->flags.unk80 && sound->flags.unk10) {
        mslSoundStop(sound);
      }
      sound = next;
    }
    bank = bank->next;
  }
  slots = sys->slots;
  for (i = 0; i < 64; i++) {
    if (slots[i].sound == NULL && slots[i].head != NULL) {
      mslSound* sound = mslSlotPop(&slots[i]);
      slots[i].sound = sound;
      sound->flags.awaitingPlay = 0;
      mslSoundProcess(sound);
    }
  }
}

static void mslGlobalAdjustProcess(mslSoundSystem* sys) {
  mslAdjustment* adjustment;
  OSTime now;
  OSTime current;
  mslAdjustment* next;
  f32 volume;
  f32 pan;
  f32 pitch;

  now = OSGetTime();
  for (adjustment = sys->adjustments; adjustment != NULL; adjustment = adjustment->next) {
    current = now;
    if (adjustment->endTime < now) {
      current = adjustment->endTime;
    }
    if (current == adjustment->endTime || adjustment->endTime < OSMillisecondsToTicks(16)) {
      volume = adjustment->targetVolume;
      pan = adjustment->targetPan;
      pitch = adjustment->targetPitch;
    } else {
      volume = adjustment->startVolume +
               ((f32)(current - adjustment->startTime) * (adjustment->targetVolume - adjustment->startVolume)) /
                   (f32)(adjustment->endTime - adjustment->startTime);
      pan = adjustment->startPan +
            ((f32)(current - adjustment->startTime) * (adjustment->targetPan - adjustment->startPan)) /
                (f32)(adjustment->endTime - adjustment->startTime);
      pitch = adjustment->startPitch +
              ((f32)(current - adjustment->startTime) * (adjustment->targetPitch - adjustment->startPitch)) /
                  (f32)(adjustment->endTime - adjustment->startTime);
    }
    if (adjustment->control.flags.ducked) {
      sys->duckVolume = volume;
      sys->duckPan = pan;
      sys->duckPitch = pitch;
    } else if (adjustment->control.flags.master) {
      if (sys->flags.volumeInDB) {
        volume = dBToLinear(volume);
      }
      sys->volume = volume;
      sys->pan = pan;
      sys->pitch = pitch;
    }
    if (current >= adjustment->endTime) {
      adjustment->control.state.completed = 1;
    }
  }
  for (adjustment = sys->adjustments; adjustment != NULL; adjustment = next) {
    next = adjustment->next;
    if (adjustment->control.state.completed) {
      if (adjustment->previous != NULL) {
        adjustment->previous->next = adjustment->next;
      } else {
        sys->adjustments = adjustment->next;
      }
      if (adjustment->next != NULL) {
        adjustment->next->previous = adjustment->previous;
      }
      if (sys != NULL && adjustment != NULL) {
        disableIRQ();
        adjustment->control.state.occupied = 0;
        enableIRQ();
      }
    }
  }
}

static void mslBankRemap(mslSoundSystem* sys, mslBank* bank, void* samples, const char* name) {
  u32 i;
  bank->groupData = (void*)((u32)bank->groupData + (u32)bank);
  bank->groupPool = (void*)((u32)bank->groupPool + (u32)bank);
  bank->sampleDirectory = (void*)((u32)bank->sampleDirectory + (u32)bank);
  bank->relocTable = (mslBankRelocNode**)((u32)bank->relocTable + (u32)bank);
  for (i = 0; i < bank->relocCount; i++) {
    if (bank->relocTable[i] != NULL) {
      mslBankRelocNode* node;
      bank->relocTable[i] = (mslBankRelocNode*)((u32)bank->relocTable[i] + (u32)bank->relocTable);
      node = bank->relocTable[i];
      while (node != NULL) {
        node->unk04 = (void*)((u32)node->unk04 + (u32)bank->relocTable);
        if (node->next != NULL) {
          node->next = (mslBankRelocNode*)((u32)node->next + (u32)bank->relocTable);
        }
        node = node->next;
      }
    }
  }
  bank->unk38 = (void*)((u32)bank->unk38 + (u32)bank);
  bank->assets = bank->assets == NULL ? NULL :
      (mslSoundAsset*)((u32)bank->assets + (u32)bank);
  if (samples != NULL) {
    sndPushGroup(bank->groupData, bank->groupID, samples,
                 bank->sampleDirectory, bank->groupPool);
    bank->flags |= 1;
    mlHeapFree(samples);
  }
  bank->commandIndices = (s16*)((u32)bank->commandIndices + (u32)bank);
  if (bank->commandCount != 0) {
    mslSoundCommandStream* command;
    bank->commands = (mslSoundCommandStream*)((u32)bank->commands + (u32)bank);
    command = bank->commands;
    for (i = 0; i < bank->commandCount; i++) {
      command->commands = (mslSoundCommand*)((u32)command->commands + (u32)bank->commands);
      command->unk24 = (void*)((u32)command->unk24 + (u32)bank->commands);
      command++;
    }
  }
  bank->system = sys;
  bank->next = bank->system->banks;
  bank->system->banks = bank;
  bank->sounds = NULL;
}

mslSoundSystem* mslInit(mslInitConfig* init, mslSysInitConfig* sysinit) {
  mslSoundSystem* sys;

  if (init == NULL) {
    printf("DEFAULT ");
    init = &g_initDefault;
  }
  printf("Init: FLAGS=%d, Tracks=%d\n", init->flags, init->tracks);
  if (sysinit == NULL) {
    printf("DEFAULT ");
    sysinit = &g_sysinitDefault;
  } else {
    if (sysinit->voices == 0) {
      sysinit->voices = g_sysinitDefault.voices;
    }
    if (sysinit->aramSize == 0) {
      sysinit->aramSize = g_sysinitDefault.aramSize;
    }
  }
  printf("SysInit: FLAGS=%d, Voices %d-%d\n", sysinit->flags, sysinit->reservedVoices, sysinit->voices);
  if (sysinit->reservedVoices != 0) {
    sysinit->voices -= sysinit->reservedVoices;
    sysinit->reservedVoices = 0;
    printf("SysInit: UNSUPPORTED 'reserved' voices, using %d=%d\n", sysinit->reservedVoices, sysinit->voices);
  }
  mflFileSystemInit();
  filemgr_init();
  if (!mslInitialized) {
    u8 voices = sysinit->voices + 1;
    u32 aramSize = sysinit->aramSize;
    SND_HOOKS hooks = { mslAlignedAlloc, mlHeapFree };

    ARInit(aramMemArray, 2);
    ARQInit();
    AIInit(NULL);
    sndSetHooks(&hooks);
    sndInit(voices, 0, voices, 1, 1, aramSize);
    MusyXDMACallback = AIRegisterDMACallback(mslMusyXDMAWrapperCallback);
    MusyXDMAStackEnd = mslHeapAlignedAlloc(MSLMFL_HEAP, 0x10000, "MusyX DMA Stack");
    MusyXDMAStack = (u8*)MusyXDMAStackEnd + 0x10000 - 4;
    *(u32*)MusyXDMAStackEnd = 0xDEADBABE;
    switch (OSGetSoundMode()) {
    case OS_SOUND_MODE_MONO:
      sndOutputMode(SND_OUTPUTMODE_MONO);
      break;
    default:
      sndOutputMode(SND_OUTPUTMODE_SURROUND);
      break;
    }
    sndVolume(mkMusyXVolume(0.95f), 0, 0xFC);
    sndVolume(mkMusyXVolume(0.95f), 0, 0xFF);
    sndMasterVolume(mkMusyXVolume(0.95f), 0, 1, 1);
    mslStreamInit();
    ListPoolAttach(&g_listPoolBank, g_listMemBank, 10, 0);
    ListPoolAttach(&g_listPoolBus, g_listMemBus, 64, 0x38);
    ListPoolAttach(&g_listPoolSound, g_listMemSound, 500, 0);
    mslInitialized = 1;
  }
  sys = mslHeapAlloc(MSLMFL_HEAP, sizeof(mslSoundSystem), "mslSystem");
  memset(sys, 0, sizeof(mslSoundSystem));
  if (!mslSystemRegister(sys)) {
    mlHeapFree(sys);
    return NULL;
  }
  mslBusInitDefault();
  sys->firstAutomaticSlot = init->tracks;
  sys->volume = 1.0f;
  sys->pan = 0.0f;
  sys->pitch = 1.0f;
  sys->duckVolume = 1.0f;
  sys->duckPan = 0.0f;
  sys->duckPitch = 1.0f;
  sys->flags.volumeInDB = init->flags == 0;
  sys->soundCount = 32;
  sys->sounds = mslHeapAlloc(MSLMFL_HEAP, sys->soundCount * sizeof(mslSound), "Sound Pool");
  sys->nextFreeSound = sys->sounds;
  memset(sys->sounds, 0, sys->soundCount * sizeof(mslSound));
  sys->playbackCount = 64;
  sys->playbacks = mslHeapAlloc(MSLMFL_HEAP, sys->playbackCount * sizeof(mslPlayback), "Playback Pool");
  sys->nextFreePlayback = sys->playbacks;
  memset(sys->playbacks, 0, sys->playbackCount * sizeof(mslPlayback));
  sys->adjustmentCount = 8;
  sys->adjustmentPool = mslHeapAlloc(MSLMFL_HEAP, sys->adjustmentCount * sizeof(mslAdjustment), "Adjust Pool");
  sys->nextFreeAdjustment = sys->adjustmentPool;
  memset(sys->adjustmentPool, 0, sys->adjustmentCount * sizeof(mslAdjustment));
  sys->slots = mslHeapAlloc(MSLMFL_HEAP, 64 * sizeof(mslSoundSlot), "Tracks");
  memset(sys->slots, 0, 64 * sizeof(mslSoundSlot));
  return sys;
}

s32 mslUnInit(mslSoundSystem* sys) {
  u32 i;
  mslSoundSystem** system;
  u32 count;

  mslStreamUnInit();
  sndQuit();
  count = gSoundSystemCount;
  for (i = 0, system = gSoundSystems; count > i; system++, i++) {
    if (*system == sys) {
      break;
    }
  }
  if (i < count) {
    u32 remaining = count - 1;
    if (i < remaining) {
      memmove(&gSoundSystems[i] + 1, &gSoundSystems[i], (remaining - i) * sizeof(gSoundSystems[0]));
    }
    gSoundSystemCount--;
  }
  mlHeapFree(sys);
  return 0;
}

void mslPauseAll(void) {
  if (mslInitialized != 0) {
    u32 i;
    for (i = 0; i < gSoundSystemCount; i++) {
      mslSoundSystem* sys = gSoundSystems[i];
      mslBank* bank;
      for (bank = sys->banks; bank != NULL; bank = bank->next) {
        mslSound* sound;
        for (sound = bank->sounds; sound != NULL; sound = sound->next) {
          if (!mslSoundInSystemOperation(sys, sound)) {
            printf("mslPauseAll ms pointer out of bounds %x\n", sound);
          }
          mslSoundPause(sound);
        }
      }
    }
  }
}

void mslUnPauseAll(void) {
  if (mslInitialized != 0) {
    u32 i;
    for (i = 0; i < gSoundSystemCount; i++) {
      mslSoundSystem* sys = gSoundSystems[i];
      mslBank* bank;
      for (bank = sys->banks; bank != NULL; bank = bank->next) {
        mslSound* sound;
        for (sound = bank->sounds; sound != NULL; sound = sound->next) {
          if (!mslSoundInSystemOperation(sys, sound)) {
            printf("mslUnPauseAll ms pointer out of bounds %x\n", sound);
          }
          mslSoundUnPauseOperation(sound);
        }
      }
    }
  }
}

void mslEndAll(mslSoundSystem* sys) {
  mslBank* bank = sys->banks;
  while (bank != NULL) {
    mslSound* sound = bank->sounds;
    while (sound != NULL) {
      mslSound* next;
      if (!mslSoundInSystemOperation(sys, sound)) {
        printf("mslEndAll ms pointer out of bounds %x\n", sound);
      }
      next = sound->next;
      mslSoundStop(sound);
      sound = next;
    }
    bank = bank->next;
  }
}

void mslBankFindName(const char* name) {
  printf("mslBankFindName NULL name\n");
  printf("mslBankFindName NULL bank for \"%s\"\n", name);
  printf("mslBankFindName \"%s\" Not Found\n", name);
}

void mslBankGetNames(void) {
  printf("mslBankGetNames NULL bank\n");
}

void mslBankGetIDs(s32 count) {
  printf("mslBankGetIDs NULL bank\n");
  printf("mslBankGetIDs found more than %d, punting rest\n", count);
}

void mslBankFindID(s32 id, s32 count, s32 index) {
  printf("mslBankFindID NULL bank\n");
  printf("mslBankFindID ID %d out of range (%d).\n", id, count);
  printf("mslBankFindID ID %d yielded invalid index %d\n", id, index);
}

void mslBankSoundGetIDNullDiagnostic(void) {
  printf("mslBankSoundGetID NULL sound\n");
}

void mslBankSoundHasStream(void* sound) {
  printf("mslBankSoundHasStream NULL sound\n");
  printf("mslBankSoundHasStream invalid mbs %x\n", sound);
}

void mslBankPlayNamed(const char* name) {
  printf("mslBankPlayNamed (%s): no strings in bank file\n", name);
}

void mslBankPlayNamedPrep(const char* name) {
  printf("mslBankPlayNamedPrep (%s): no strings in bank file\n", name);
}

void mslBankPlayQNamed(const char* name) {
  printf("mslBankPlayQNamed (%s): no strings in bank file\n", name);
}

void mslStopAll(mslSoundSystem* sys) {
  mslEndAll(sys);
}

mslBank* mslBankLoad(mslSoundSystem* sys, const char* filename) {
  char bankName[256];
  char sampleName[256];
  mlSysCalls* calls = &g_SysCalls;
  void* file;
  u32 bankBytes;
  u32 sampleBytes;
  mslBank* bank = NULL;
  void* samples = NULL;
  char* extension;
  strcpy(bankName, filename);
  extension = strrchr(bankName, '.');
  if (extension != NULL) {
    *extension = 0;
  }
  strcpy(sampleName, bankName);
  strcat(bankName, ".gsb");
  strcat(sampleName, ".dsp");
  file = calls->open(bankName, "rbu");
  do {
    if (file == NULL) break;
    calls->seek(file, 0, 2);
    bankBytes = calls->tell(file);
    calls->seek(file, 0, 0);
    bank = mslHeapAlignedAlloc(MSLMFL_HEAP, bankBytes, "mslBank");
    if (calls->read(bank, bankBytes, 1, file) != 1) break;
    calls->close(file);
    if (bank->flags & 2) {
      file = calls->open(sampleName, "rbu");
      if (file == NULL) break;
      calls->seek(file, 0, 2);
      sampleBytes = calls->tell(file);
      calls->seek(file, 0, 0);
      sys->sampleBytes += sampleBytes;
      bank->sampleBytes = sampleBytes;
      samples = mslHeapAlignedAlloc(wave_heap, sampleBytes, "samples");
      if (calls->read(samples, sampleBytes, 1, file) != 1) break;
      calls->close(file);
    } else {
      samples = NULL;
      bank->sampleBytes = 0;
    }
    mslBankRemap(sys, bank, samples, bankName);
    return bank;
  } while (0);
  if (file != NULL) {
    calls->close(file);
  }
  if (bank != NULL) {
    mlHeapFree(bank);
  }
  if (samples != NULL) {
    mlHeapFree(samples);
  }
  return NULL;
}

s32 mslBankUnLoad(mslBank* bank) {
  if (bank == NULL) {
    return 0;
  }
  bank->system->banks = bank->next;
  while (bank->sounds != NULL) {
    mslSoundStop(bank->sounds);
  }
  if (bank->flags & 1) {
    sndPopGroup();
  }
  bank->system->sampleBytes -= bank->sampleBytes;
  mlHeapFree(bank);
  return 0;
}

void mslSetVol(mslSoundSystem* sys, f32 volume) {
  sys->volume = volume < 0.0f ? 0.0f : volume > 1.0f ? 1.0f : volume;
}

f32 mslGetVol(mslSoundSystem* sys) {
  return sys->volume;
}

void mslSetDuckVol(mslSoundSystem* sys, f32 volume) {
  sys->duckVolume = volume < 0.0f ? 0.0f : volume > 1.0f ? 1.0f : volume;
}

mslSound* mslBankPlayVol(mslBank* bank, s32 index, s32 slot, s32 priority, u32 flags, f32 volume) {
  return mslBankPlayVolPanPitch(bank, index, slot, priority, flags, volume, 0.0f, 1.0f);
}

mslSound* mslBankPlayVolPanPitch(mslBank* bank, s32 index, s32 slot, s32 priority, u32 flags, f32 volume, f32 pan, f32 pitch) {
  s32 keepEqualPriority = flags & 8;
  mslSoundSlot* slotInfo;
  mslSound* sound;
  s32 awaitingPlay = (flags >> 3) & 1;

  if ((u32)bank < (u32)MSLMFL_HEAP ||
      (u32)bank >= (u32)MSLMFL_HEAP + mslGetHeapSize()) {
    return NULL;
  }
  if ((u32)index >= bank->commandIndexCount || index < 0) {
    return NULL;
  }
  if (bank->commandIndices[index] == -1) {
    return NULL;
  }
  if (mflGetDiscErrorStatus() != 0) {
    return NULL;
  }
  if (slot == -1) {
    slot = mslSlotFindFree(bank->system);
    if (slot < 0) {
      return NULL;
    }
  }
  slotInfo = bank->system->slots + slot;
  if (!awaitingPlay) {
    mslSound* previousSound = slotInfo->sound;
    if (previousSound != NULL) {
      if (previousSound->priority > priority ||
          (keepEqualPriority && previousSound->priority >= priority)) {
        return NULL;
      }
      mslSoundStop(previousSound);
    }
  }

  sound = mslSoundPoolAlloc(bank->system);
  if (sound == NULL) {
    return NULL;
  }
  if (bank->system->flags.volumeInDB) {
    volume = dBToLinear(volume);
  }
  sound->commandIndex = bank->commandIndices[index];
  sound->commandStream = bank->commands + sound->commandIndex;
  sound->sequenceCursor = NULL;
  sound->loopStart = NULL;
  sound->sequenceStart = 0;
  sound->slot = slotInfo;
  sound->mix.volume = mkMusyXVolume(volume);
  sound->mix.pan = mkMusyXPan(pan);
  sound->mix.pitch = mkMusyXPitch(pitch);
  sound->rawVolume = volume;
  sound->flags.unk80 = 1;
  sound->flags.awaitingPlay = awaitingPlay;
  if (pan <= -1.0 || pan >= 1.0) {
    sound->mix.control.flags.boundaryPan = 1;
  }
  sound->previous = NULL;
  sound->next = bank->sounds;
  bank->sounds = sound;
  if (!mslSoundInSystemOperation(bank->system, sound)) {
    printf("mslBankAddSound ms pointer out of bounds %x\n", sound);
  }
  if (sound->next != NULL) {
    sound->next->previous = sound;
  }
  sound->bank = bank;
  slotInfo->sound = sound;
  if (sound->flags.awaitingPlay) {
    mslSoundCheckPreload(sound);
  } else {
    mslSoundProcess(sound);
  }
  return sound;
}

int mslSoundIsPlaying(mslSound* sound) {
  if (sound == NULL || !sound->activeFlags.active) {
    return 0;
  }
  return sound->sequenceCursor != NULL;
}

int mslSoundIsValid(mslSound* sound) {
  if (sound == NULL || !sound->activeFlags.active) {
    return 0;
  }
  if (!mslSoundInSystemOperation(sound->bank->system, sound)) {
    printf("mslSoundisValid: ms pointer out of bounds %x\n", sound);
    return 0;
  }
  return sound->sequenceCursor != NULL || sound->flags.awaitingPlay;
}

void mslSoundStop(mslSound* sound) {
  mslPlayback* playback;
  if (sound == NULL) {
    return;
  }
  if (!mslSoundInSystemOperation(sound->bank->system, sound)) {
    printf("mslSoundStop ms pointer out of bounds %x\n", sound);
    return;
  }
  playback = sound->playbackHead;
  while (playback != NULL) {
    if (playback->definition->flags & 2) {
      mslStreamStop(playback);
    } else {
      if (playback->voice != 0xFFFFFFFF) {
        sndFXKeyOff(playback->voice);
      }
      if ((playback->definition->flags & 1) &&
          playback->secondVoice != 0xFFFFFFFF) {
        sndFXKeyOff(playback->secondVoice);
      }
    }
    playback->streamFlags.activationPending = 1;
    playback = playback->next;
  }
  playback = sound->preloadedPlayback;
  if (playback != NULL) {
    if (playback->definition->flags & 2) {
      mslStreamStop(playback);
    } else {
      if (playback->voice != 0xFFFFFFFF) {
        sndFXKeyOff(playback->voice);
      }
      if ((playback->definition->flags & 1) &&
          playback->secondVoice != 0xFFFFFFFF) {
        sndFXKeyOff(playback->secondVoice);
      }
    }
    playback->streamFlags.activationPending = 1;
  }
  mslRemovePendingPlayback(sound);
  if (sound->slot->sound == sound) {
    sound->slot->sound = NULL;
  } else if (sound->slotNext != NULL) {
    mslSlotRemove(sound->slot, sound);
  }
  sound->sequenceCursor = NULL;
  if (!mslSoundInSystemOperation(sound->bank->system, sound)) {
    printf("mslBankRemoveSound ms pointer out of bounds %x\n", sound);
  } else {
    if (sound->previous != NULL) {
      if (!mslSoundInSystemOperation(sound->bank->system, sound->previous)) {
        printf("mslBankRemoveSound ms pointer out of bounds %x\n", sound);
      }
      sound->previous->next = sound->next;
    }
    if (sound->next != NULL) {
      if (!mslSoundInSystemOperation(sound->bank->system, sound->next)) {
        printf("mslBankRemoveSound ms pointer out of bounds %x\n", sound);
      }
      sound->next->previous = sound->previous;
    }
    if (sound->previous == NULL) {
      sound->bank->sounds = sound->next;
    }
    sound->next = NULL;
    sound->previous = NULL;
  }
  disableIRQ();
  sound->activeFlags.active = 0;
  enableIRQ();
}

void mslSoundPause(mslSound* sound) {
  if (!mslSoundInSystemOperation(sound->bank->system, sound)) {
    printf("mslSoundPause ms pointer out of bounds %x\n", sound);
    return;
  }
  if (sound != NULL && sound->bank != NULL && sound->bank->system != NULL) {
    sound->savedVolume = mslSoundGetVolOperation(sound);
    mslSoundSetVolOperation(sound, 0.0f);
  }
}

void mslSoundUnPause(mslSound* sound) {
  mslSoundUnPauseOperation(sound);
}

void mslSoundSetVol(mslSound* sound, f32 volume) {
  mslPlayback* playback;
  if (sound == NULL) {
    return;
  }
  if (!mslSoundInSystemOperation(sound->bank->system, sound)) {
    printf("mslSoundSetVol ms pointer out of bounds %x\n", sound);
    return;
  }
  if (sound->bank->system->flags.volumeInDB) {
    volume = dBToLinear(volume);
  }
  sound->mix.volume = mkMusyXVolume(volume);
  sound->mix.control.flags.volumeChanged = 1;
  sound->mix.control.flags.panChanged = 0;
  sound->mix.control.flags.pitchChanged = 0;
  playback = sound->playbackHead;
  while (playback != NULL) {
    mslPlaybackAdjustImm(playback, &sound->mix);
    playback = playback->next;
  }
  sound->mix.control.flags.volumeChanged = 0;
}

static f32 msl3DEmitterUnsignedValueScaffold(u32 value) {
  return value;
}

static f32 mslMicDefaultVolumeScaffold(void) {
  return 1.0f;
}

f32 mslSoundGetVol(mslSound* sound) {
  return mslSoundGetVolOperation(sound);
}

void mslSoundSetPan(mslSound* sound) {
  printf("mslSoundSetPan ms pointer out of bounds %x\n", sound);
}

void mslSoundSetPitch(mslSound* sound, f32 pitch) {
  mslPlayback* playback;
  if (!mslSoundInSystemOperation(sound->bank->system, sound)) {
    printf("mslSoundSetPitch ms pointer out of bounds %x\n", sound);
    return;
  }
  sound->mix.pitch = mkMusyXPitch(pitch);
  sound->mix.control.flags.volumeChanged = 0;
  sound->mix.control.flags.panChanged = 0;
  sound->mix.control.flags.pitchChanged = 1;
  playback = sound->playbackHead;
  while (playback != NULL) {
    mslPlaybackAdjustImm(playback, &sound->mix);
    playback = playback->next;
  }
  sound->mix.control.flags.pitchChanged = 0;
}

void mslSoundPlayAfterPrep(mslSound* sound) {
  sound->flags.awaitingPlay = 0;
  mslSoundProcess(sound);
}

s32 mslTick(void) {
  u32 i;

  mflTick();
  if (mflGetDiscErrorStatus() != 0) {
    if (mslErrorFlag == 0) {
      mslPauseAll();
    }
    mslErrorFlag = 1;
    return 10;
  }
  if (mslErrorFlag == 1) {
    mslErrorFlag = 0;
    mslUnPauseAll();
  }
  if (mslInitialized != 0) {
    for (i = 0; i < gSoundSystemCount; i++) {
      mslUpdateThread(gSoundSystems[i]);
    }
  }
  return 0;
}

void mslSoundNameDiagnostic(void) {
  printf("<unknown>");
}

void msl3DEmitterDiagnostics(void) {
  printf("No emitter for this sound!\n");
  printf("Couldn't update emitter\n");
}

void msl3DListenerDiagnostics(void) {
  printf("Couldn't add listener.\n");
  printf("Couldn't update listener\n");
}

void msl3DDynamicSoundPlay(void) {
  printf("msl3DDynamicSoundPlay");
  printf("Couldn't add 3D emmitter.\n");
}

void mslMicNew(void) {
  printf("mslMicNew: NULL mslSystem*\n");
  printf("mslMicNew: no more free Mics\n");
  printf("non-positional");
  printf("position-only");
  printf(" mics not yet supported; using POS and VOL\n");
  printf("mslMicNew: sndAddListener failed\n");
}

void mslMicFree(void* mic) {
  printf("mslMicFree %x already freed!\n", mic);
  printf("mslMicFree: sndRemoveListener failed\n");
}

void mslMicSetPosition(void) {
  printf("mslMicSetPosition: NULL mic\n");
}

void mslMicSetOrientation1(void* mic) {
  printf("mslMicSetOrientation1: NULL mic\n");
  printf("mslMicSetOrientation1: NOP; mic was not created with MSL_MIC_POSITION\n");
}

void mslMicSetOrientation2(void* mic) {
  printf("mslMicSetOrientation2: NULL mic\n");
  printf("mslMicSetOrientation2: NOP; mic was not created with MSL_MIC_POSITION\n");
}

void mslMicSetOrientation3(void* mic) {
  printf("mslMicSetOrientation3: NULL mic\n");
  printf("mslMicSetOrientation3: NOP; mic was not created with MSL_MIC_POSITION\n");
}

void mslPlaybackPoolFree(mslSoundSystem* system, mslPlayback* playback) {
  mslPlaybackPoolFreeOperation(system, playback);
}

mslPlayback* mslPlaybackPoolAlloc(mslSoundSystem* sys) {
  return mslPlaybackPoolAllocOperation(sys);
}

void mslBankAddSound(mslSound* sound) {
  printf("mslBankAddSound ms pointer out of bounds %x\n", sound);
}

int mslSoundInSystem(mslSoundSystem* sys, mslSound* sound) {
  return mslSoundInSystemOperation(sys, sound);
}

void mslBusPrepFailure(const char* name) {
  printf("mslBusPrepRespond: Unable to create sound instance for \"%s\".\n", name);
}

void mslBankSoundGetName(void) {
  printf("mslBankSoundGetName NULL sound\n");
}

void mslBankSoundGetID(void* sound) {
  printf("mslBankSoundGetID invalid mbs %x\n", sound);
}

void mslBusAddSoundAt(s32 time) {
  printf("mslBusAddSoundAt UNSUPPORTED time %d, using MSL_TIME_NOW instead\n", time);
}

void mslBusPrepRespond(s32 error) {
  printf("mslBusPrepRespond: ERR code %d\n", error);
}

void mslPicker(void) {
  printf("mslPicker");
}

void mslUnknownVectorInitializer(mslListenerState* mic) {
  SND_FVECTOR up = { 0.0f, 1.0f, 0.0f };
  sndUpdateListener(&mic->listener, &mic->position, &mic->direction,
                    &mic->heading, &up, mic->volume, NULL);
}

void mslUnknownReverbStorage(void) {
  static SND_AUX_REVERBHI revH;
  revH.tempDisableFX = 0;
}

void mslUnknownChorusStorage(void) {
  static SND_AUX_CHORUS cho;
  cho.baseDelay = 0;
}

void mslUnknownPrintStorageSmall(void) {
  static char buf[0x401];
  buf[0] = 0;
}

void mslUnknownPrintStorageLarge(void) {
  static char buf[0x1004];
  buf[0] = 0;
}

SND_LISTENER sl;

void mslUnknownListenerStorage(void) {
  sl.next = NULL;
}
