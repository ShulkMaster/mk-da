#include <msl/mslMusyX.h>
#include <msl/mslMusyXStream.h>
#include <dolphin/os/OSContext.h>
#include <dolphin/os.h>
#include <stdio.h>
#include <string.h>
#include <msl/mslMem.h>
#include <msl/mslMusyXUtil.h>
#include <msl/mflFile.h>

static void mslSoundCheckPreload(mslSound* sound);
static void mslSoundPreload(mslSound* sound, mslSoundCommand* command);
static void mslSoundProcess(mslSound* sound);
static void mslPlaybackAdjustImm(mslPlayback* playback, mslSoundMix* mix);
static void mslPlaybackAdjustProcess(mslSound* sound);
static void mslPlaybackStart(mslSound* sound, mslPlayback* playback);
static void mslUpdateThread(mslSoundSystem* sys);
static void mslBankRemap(mslSoundSystem* sys, mslBank* bank, void* samples, const char* name);

static u32 g_initDefault[3] = { 12, 1, 10 };
static struct {
  u32 unk00;
  u32 unk04;
  u16 unk08;
  u16 unk0A;
  u32 unk0C;
} g_sysinitDefault = { 16, 0, 0, 47, 0x00FFC000 };

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

static inline int mslSoundInSystem(mslSoundSystem* sys, mslSound* sound) {
  mslSound* end = sys->sounds + sys->soundCount;
  u32* words = (u32*)sys->sounds;
  if (sound < sys->sounds || sound >= end) {
    OSReport("Bad ms pointer: %08x  pool range %08x to %08x \n", sound, words,
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

void mslSoundPlayAfterPrep(mslSound* sound) {
  sound->flags.awaitingPlay = 0;
  mslSoundProcess(sound);
}

/* TODO: [near miss] 97.74%; diagnostic argument staging and TU literal layout remain. */
void mslSoundSetPitch(mslSound* sound, f32 pitch) {
  mslPlayback* playback;
  if (!mslSoundInSystem(sound->bank->system, sound)) {
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

/* TODO: [near miss] 97.95%; shared diagnostic argument staging and TU literal layout remain. */
void mslSoundSetVol(mslSound* sound, f32 volume) {
  mslPlayback* playback;
  if (sound == NULL) {
    return;
  }
  if (!mslSoundInSystem(sound->bank->system, sound)) {
    printf("mslSoundSetVol: ms pointer out of bounds %x\n", sound);
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

static inline f32 mslSoundGetVol(mslSound* sound) {
  if (!mslSoundInSystem(sound->bank->system, sound)) {
    printf("mslSoundGetVol ms pointer out of bounds %x\n", sound);
    return 0.0f;
  }
  return mkFpVolume(sound->mix.volume);
}

/* TODO: [near miss] 96.92%; shared diagnostic argument staging and TU literal layout remain. */
void mslSoundPause(mslSound* sound) {
  if (!mslSoundInSystem(sound->bank->system, sound)) {
    printf("mslSoundPause ms pointer out of bounds %x\n", sound);
    return;
  }
  if (sound != NULL && sound->bank != NULL && sound->bank->system != NULL) {
    sound->savedVolume = mslSoundGetVol(sound);
    mslSoundSetVol(sound, 0.0f);
  }
}

/* TODO: [near miss] 97.09%; register allocation, slot guard and literal layout remain. */
void mslSoundStop(mslSound* sound) {
  mslPlayback* playback;
  mslPlayback* removed;
  mslSoundSlot* slot;
  if (sound == NULL) {
    return;
  }
  if (!mslSoundInSystem(sound->bank->system, sound)) {
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
        mslPlaybackPoolFree(sound->bank->system, removed);
        removed = sound->playbackHead;
      } else {
        removed = removed->next;
      }
    } while (removed != NULL);
  }
  {
    mslPlayback* preloaded;
    preloaded = sound->preloadedPlayback;
    if (preloaded != NULL && preloaded->streamFlags.activationPending) {
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
      mslPlaybackPoolFree(sound->bank->system, preloaded);
    }
  }
  slot = sound->slot;
  if (sound->slot->sound == sound) {
    slot->sound = NULL;
  } else if (sound->slotNext != NULL) {
    mslSound* previous = slot->head;
    if (previous != sound) {
      while (previous != NULL && previous->slotNext != sound) {
        previous = previous->slotNext;
      }
      if (previous != NULL) {
        previous->slotNext = sound->slotNext;
        if (slot->tail == sound) {
          slot->tail = previous;
        }
      }
    } else {
      slot->head = sound->slotNext;
      if (slot->tail == sound) {
        slot->tail = NULL;
      }
    }
  }
  sound->sequenceCursor = NULL;
  if (!mslSoundInSystem(sound->bank->system, sound)) {
    printf("mslBankRemoveSound ms pointer out of bounds %x\n", sound);
  } else {
    if (sound->previous != NULL) {
      if (!mslSoundInSystem(sound->bank->system, sound->previous)) {
        printf("mslBankRemoveSound ms pointer out of bounds %x\n", sound);
      }
      sound->previous->next = sound->next;
    }
    if (sound->next != NULL) {
      if (!mslSoundInSystem(sound->bank->system, sound->next)) {
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

/* TODO: [near miss] 97.33%; diagnostic argument staging and TU literal layout remain. */
int mslSoundIsValid(mslSound* sound) {
  if (sound == NULL || !sound->activeFlags.active) {
    return 0;
  }
  if (!mslSoundInSystem(sound->bank->system, sound)) {
    printf("mslSoundisValid: ms pointer out of bounds %x\n", sound);
    return 0;
  }
  return sound->sequenceCursor != NULL || sound->flags.awaitingPlay;
}

int mslSoundIsPlaying(mslSound* sound) {
  if (sound == NULL || !sound->activeFlags.active) {
    return 0;
  }
  return sound->sequenceCursor != NULL;
}

static inline mslPlayback* mslPlaybackPoolAlloc(mslSoundSystem* sys) {
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

/* TODO: [near miss] 99.23%; mslSoundInSystem argument staging and literal layout remain. */
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
  if (!mslSoundInSystem(bank->system, sound)) {
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

mslSound* mslBankPlayVol(mslBank* bank, s32 index, s32 slot, s32 priority, u32 flags, f32 volume) {
  return mslBankPlayVolPanPitch(bank, index, slot, priority, flags, volume, 0.0f, 1.0f);
}

void mslSetDuckVol(mslSoundSystem* sys, f32 volume) {
  sys->duckVolume = volume < 0.0f ? 0.0f : volume > 1.0f ? 1.0f : volume;
}

f32 mslGetVol(mslSoundSystem* sys) {
  return sys->volume;
}

void mslSetVol(mslSoundSystem* sys, f32 volume) {
  sys->volume = volume < 0.0f ? 0.0f : volume > 1.0f ? 1.0f : volume;
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

void mslStopAll(mslSoundSystem* sys) {
  mslEndAll(sys);
}

/* TODO: [near miss] 92.18%; empty Stop stub, diagnostic staging and literal layout remain. */
void mslEndAll(mslSoundSystem* sys) {
  mslBank* bank = sys->banks;
  while (bank != NULL) {
    mslSound* sound = bank->sounds;
    while (sound != NULL) {
      mslSound* next;
      if (!mslSoundInSystem(sys, sound)) {
        printf("mslEndAll ms pointer out of bounds %x\n", sound);
      }
      next = sound->next;
      mslSoundStop(sound);
      sound = next;
    }
    bank = bank->next;
  }
}

static inline void mslSoundUnPause(mslSound* sound) {
  if (!mslSoundInSystem(sound->bank->system, sound)) {
    printf("mslSoundUnPause ms pointer out of bounds %x\n", sound);
    return;
  }
  if (sound != NULL && sound->bank != NULL && sound->bank->system != NULL) {
    mslSoundSetVol(sound, sound->savedVolume);
  }
}

/* TODO: [near miss] 97.04%; shared diagnostic argument staging and TU literal layout remain. */
void mslUnPauseAll(void) {
  if (mslInitialized != 0) {
    u32 i;
    for (i = 0; i < gSoundSystemCount; i++) {
      mslSoundSystem* sys = gSoundSystems[i];
      mslBank* bank;
      for (bank = sys->banks; bank != NULL; bank = bank->next) {
        mslSound* sound;
        for (sound = bank->sounds; sound != NULL; sound = sound->next) {
          if (!mslSoundInSystem(sys, sound)) {
            printf("mslUnPauseAll ms pointer out of bounds %x\n", sound);
          }
          mslSoundUnPause(sound);
        }
      }
    }
  }
}

/* TODO: [near miss] 94.63%; pause callee, diagnostic staging and literal layout remain. */
void mslPauseAll(void) {
  if (mslInitialized != 0) {
    u32 i;
    for (i = 0; i < gSoundSystemCount; i++) {
      mslSoundSystem* sys = gSoundSystems[i];
      mslBank* bank;
      for (bank = sys->banks; bank != NULL; bank = bank->next) {
        mslSound* sound;
        for (sound = bank->sounds; sound != NULL; sound = sound->next) {
          if (!mslSoundInSystem(sys, sound)) {
            printf("mslPauseAll ms pointer out of bounds %x\n", sound);
          }
          mslSoundPause(sound);
        }
      }
    }
  }
}

/* TODO: [near miss] 97.86%; volatile register assignment remains. */
s32 mslUnInit(mslSoundSystem* sys) {
  u32 i;

  mslStreamUnInit();
  sndQuit();
  for (i = 0; i < gSoundSystemCount; i++) {
    if (gSoundSystems[i] == sys) {
      break;
    }
  }
  if (i < gSoundSystemCount) {
    if (gSoundSystemCount - 1 > i) {
      memmove(&gSoundSystems[i] + 1, &gSoundSystems[i], (gSoundSystemCount - 1 - i) * sizeof(gSoundSystems[0]));
    }
    gSoundSystemCount--;
  }
  mlHeapFree(sys);
  return 0;
}

/* TODO: [borked] 0.00%; placeholder stub, body not started. */
void mslInit(void) {}

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

#include <msl/mslbus.h>

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

/* TODO: [near miss] 91.20%; register allocation (sys below bank) and literal layout remain. */
static void mslUpdateThread(mslSoundSystem* sys) {
  mslListenerState* mic = &sys->listenerState;
  mslBank* bank;
  u32 i;
  mslSoundSlot* slot;
  if (mic->enabled != 0 && (mic->dirty & 0x7F)) {
    if (!sndUpdateListener(&mic->listener, &mic->position, &mic->direction,
                           &mic->heading, &mic->up, mic->volume, NULL)) {
      printf("mslUpdateThread: sndUpdateListener failed for Mic %x\n", mic);
    }
  }
  mslUpdateBuses(sys);
  mslGlobalAdjustProcess(sys);
  bank = sys->banks;
  while (bank != NULL) {
    mslSound* sound = bank->sounds;
    while (sound != NULL) {
      mslSound* next;
      if (!mslSoundInSystem(sys, sound)) {
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
  slot = sys->slots;
  for (i = 0; i < 64; i++) {
    if (slot->sound == NULL && slot->head != NULL) {
      mslSound* sound = mslSlotPop(slot);
      slot->sound = sound;
      sound->flags.awaitingPlay = 0;
      mslSoundProcess(sound);
    }
    slot++;
  }
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

/* TODO: [near miss] 98.85%; register coloring remains. */
static void mslSoundProcess(mslSound* sound) {
  u32 duration;
  OSTime delta;
  mslSoundCommand* command;
  mslPlayback* playback;
  mslSoundSystem* sys;
  mslAdjustment* adjustment;
  mslSound* target;
  mslSoundCommandStream* stream;
  mslSound* nested;
  mslPlaybackDefinition* definition;
  mslSoundCommand* reference;
  mslPlayback* preloaded;
  OSTime now;
  mslSoundMix mix;
  u8 pan;
  u8 volume;
  u16 pitch;

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
    case 1:
      preloaded = sound->preloadedPlayback;
      if (preloaded == NULL) {
        playback = mslPlaybackPoolAlloc(sound->bank->system);
        preloaded = playback;
        if (playback == NULL) {
          sound->flags.unk10 = 1;
          goto finish;
        }
        playback->definition = sound->bank->unk38 + command->assetIndex;
        playback->unk18 = command->unk10;
        playback->unk1C = command->unk14;
        playback->streamFlags.ducked = command->mix.control.flags.ducked;
        playback->sound = sound;
        volume = command->mix.volume;
        if (sound->bank->system->flags.volumeInDB) {
          volume = mkMusyXVolume(dBToLinear(mkFpVolume(volume)));
        }
        pan = command->mix.pan;
        pitch = command->mix.pitch;
        sound->mix.volume = mslMusyXScale(sound->mix.volume, volume);
        sound->mix.pan = mslMusyXScalePan(sound->mix.pan, pan);
        sound->mix.pitch = mslMusyXScalePitch(sound->mix.pitch, pitch);
        mslPlaybackStart(sound, playback);
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
        f32 startVolume;
        f32 startPan;
        f32 startPitch;

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
        f32 startPan;
        f32 startPitch;
        f32 rawVolume;

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
        adjustment = mslAdjustmentPoolAlloc(target->bank->system);
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
      nested = mslSoundPoolAlloc(sound->bank->system);
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
  playback = sound->playbackHead;
  if (playback != NULL) {
    do {
      if (playback->streamFlags.activationPending) {
        if (playback->previous != NULL) {
          playback->previous->next = playback->next;
        }
        if (playback->next != NULL) {
          playback->next->previous = playback->previous;
        }
        if (playback->previous == NULL) {
          playback->sound->playbackHead = playback->next;
        }
        playback->next = NULL;
        playback->previous = NULL;
        mslPlaybackPoolFree(sound->bank->system, playback);
        playback = sound->playbackHead;
      } else {
        playback = playback->next;
      }
    } while (playback != NULL);
  }
  preloaded = sound->preloadedPlayback;
  if (preloaded != NULL && preloaded->streamFlags.activationPending) {
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
    mslPlaybackPoolFree(sound->bank->system, preloaded);
  }
  sound->flags.unk08 = 0;
}

/* TODO: [near miss] 97.32%; register coloring remains. */
static void mslSoundPreload(mslSound* sound, mslSoundCommand* command) {
  u8 volume;
  u8 scaledPan;
  mslPlaybackDefinition* definition;
  mslBank* bank;
  mslPlayback* playback;
  u8 scaledVolume;

  volume = command->mix.volume;
  if (sound->bank->system->flags.volumeInDB) {
    volume = mkMusyXVolume(dBToLinear(mkFpVolume(volume)));
  }
  scaledVolume = mslMusyXScale(sound->mix.volume, volume);
  sound->mix = command->mix;
  sound->mix.volume = scaledVolume;
  scaledPan = mslMusyXScalePan(sound->mix.pan, command->mix.pan);
  bank = sound->bank;
  playback = NULL;
  definition = bank->unk38 + command->assetIndex;
  if (definition->flags & 2) {
    mslPlayback* last;
    mslPlayback* candidate;
    mslSoundSystem* sys;

    sys = bank->system;
    if (sys == NULL) {
      printf("mslPlaybackPoolAlloc: NULL msi\n");
      playback = NULL;
    } else {
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
        mslPlaybackPoolFree(bank->system, playback);
        playback = NULL;
      }
    }
  }
  if (playback != NULL) {
    sound->preloadedPlayback = playback;
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

f32 mslPitchCompound(f32 master, f32 pitch) {
  return master * pitch;
}

f32 mslVolCompound(f32 master, f32 volume, mslCompoundMode mode) {
  if (!mode.additive) {
    return master * volume;
  }
  master = (master + volume) - 1.0f;
  return master < 0.0f ? 0.0f : master;
}

static void mslMusyXDMAWrapperCallback(void) {
  mslMusyXSwitchFiber(&MusyXDMACallback, &MusyXDMAStack);
}
