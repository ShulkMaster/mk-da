#ifndef MSL_MSLMUSYX_H
#define MSL_MSLMUSYX_H

#include <dolphin/types.h>
#include <dolphin/os.h>
#include <msl/listpool.h>
#include <musyx/musyx.h>
#include <msl/mslMusyXUtil.h>
#include <stdio.h>
#include <string.h>

typedef struct mslSoundAsset {
  u32 flags;
  u8 unk04[8];
  /* Retail activation-failure message prints the inline string at +0xC;
     0x34-byte stride (mslPlaybackStart mulli 0x34). */
  char name[0x20];
  /* Per-channel data offsets: mslStreamStart seeks channel 0 to +0x2C and
     channel 1 to +0x30. */
  u32 dataOffsets[2];
} mslSoundAsset;

struct mslStream;
struct mslPlayback;
struct mslSound;
struct mslBank;
struct mslPlaybackDefinition;
/* 4-byte control word: sound commands carry it at +8 and adjustments copy
   it whole (mslSoundProcess) to +0x2C. */
typedef struct mslControl {
  /* Adjustment pool state. */
  struct {
    u8 occupied : 1;
    u8 completed : 1;
    u8 unk3F : 6;
  } state;
  u8 unk01[2];
  struct {
    /* Play: the playback joins the duck group. Adjust: ramp the system duck
       mix (mslGlobalAdjustProcess). */
    u8 ducked : 1;
    /* Adjust: ramp the system master mix. */
    u8 master : 1;
    u8 unk30 : 2;
    u8 boundaryPan : 1;
    u8 pitchChanged : 1;
    u8 panChanged : 1;
    u8 volumeChanged : 1;
  } flags;
} mslControl;

/* 8-byte mix block: copied whole from a sound command into the sound
   (mslSoundPreload) and passed by address to mslPlaybackAdjustImm. */
typedef struct mslSoundMix {
  u8 volume;
  u8 pan;
  u16 pitch;
  mslControl control;
} mslSoundMix;

typedef struct mslSoundCommand {
  u16 opcode;
  u16 assetIndex;
  mslSoundMix mix;
  /* Delay in ms after the previous command (mslSoundProcess). */
  u32 time;
  u32 unk10;
  u32 unk14;
} mslSoundCommand;

typedef struct mslSoundCommandStream {
  u8 unk00[0x20];
  mslSoundCommand* commands;
  void* unk24;
} mslSoundCommandStream;

typedef struct mslSoundSlot {
  struct mslSound* sound;
  struct mslSound* head;
  struct mslSound* tail;
} mslSoundSlot;

typedef struct mslListenerState {
  u32 enabled;
  u32 dirty;
  SND_LISTENER listener;
  SND_FVECTOR position;
  SND_FVECTOR direction;
  SND_FVECTOR heading;
  u8 unkBC[0xC];
  SND_FVECTOR up;
  u8 unkD4[0xC];
  u8 volume;
} mslListenerState;

/* 0x40-byte volume/pan/pitch ramp (mslGlobalAdjustProcess, mslSoundProcess);
   mslInit allocates the pool (adjustmentCount nodes). */
typedef struct mslAdjustment {
  OSTime startTime;
  f32 startVolume;
  f32 startPan;
  f32 startPitch;
  u32 unk14;
  OSTime endTime;
  f32 targetVolume;
  f32 targetPan;
  f32 targetPitch;
  /* Copied from the adjust command; flags select the system triple. */
  mslControl control;
  /* Both NULL for global ramps (mslGlobalAdjustProcess). */
  struct mslPlayback* playback;
  struct mslSound* sound;
  struct mslAdjustment* previous;
  struct mslAdjustment* next;
} mslAdjustment;

/* msi: returned by mslInit; mslSetVol/mslGetVol/mslStopAll take it. */
typedef struct mslSoundSystem {
  struct {
    u8 unkC0 : 2;
    u8 volumeInDB : 1;
    u8 unk1F : 5;
  } flags;
  u8 unk01[3];
  u32 firstAutomaticSlot;
  mslSoundSlot* slots;
  /* mslSoundIsValid walks sounds[0..soundCount) (0x70 each). */
  u32 soundCount;
  struct mslSound* sounds;
  struct mslSound* nextFreeSound;
  u32 playbackCount;
  struct mslPlayback* playbacks;
  struct mslPlayback* nextFreePlayback;
  u32 adjustmentCount;
  mslAdjustment* adjustmentPool;
  mslAdjustment* nextFreeAdjustment;
  ListNode* busList;
  u32 unk34;
  f32 volume;
  f32 pan;
  f32 pitch;
  f32 duckVolume;
  f32 duckPan;
  f32 duckPitch;
  /* Global ramps (mslGlobalAdjustProcess). */
  mslAdjustment* adjustments;
  u32 sampleBytes;
  struct mslBank* banks;
  mslListenerState listenerState;
} mslSoundSystem;

typedef struct mslBankRelocNode {
  struct mslBankRelocNode* next;
  void* unk04;
} mslBankRelocNode;

/* First argument of mslBankPlayVol* and mslStreamStart. */
typedef struct mslBank {
  u8 unk00[0x20];
  u32 flags;
  struct mslBank* next;
  void* groupData;
  void* groupPool;
  void* sampleDirectory;
  u32 groupID;
  /* 0xC-byte entries indexed by sound command asset indices. */
  struct mslPlaybackDefinition* unk38;
  /* 0x34-byte entries indexed by playback def->assetIndex. */
  mslSoundAsset* assets;
  u32 commandIndexCount;
  s16* commandIndices;
  u32 commandCount;
  mslSoundCommandStream* commands;
  u32 relocCount;
  mslBankRelocNode** relocTable;
  struct mslSound* sounds;
  mslSoundSystem* system;
  u32 sampleBytes;
} mslBank;

/* 0x70-byte elements of mslSoundSystem.sounds: returned by
   mslBankPlayVolPanPitch and held in the bus sound lists; every mslSound* API
   takes it. Fields past 0x54 wait for the large sound functions. */
typedef struct mslSound {
  struct {
    u8 unk80 : 1;
    /* Cleared by mslSoundPlayAfterPrep; mslSoundProcess waits while set. */
    u8 awaitingPlay : 1;
    u8 unk20 : 1;
    u8 unk10 : 1;
    u8 unk08 : 1;
    /* Any stream playback has streamFlags.unk20 set (mslPlaybackProcess). */
    u8 unk04 : 1;
    u8 unk03 : 2;
  } flags;
  u8 unk01[3];
  struct {
    /* mslSoundIsPlaying, mslSoundIsValid. */
    u8 active : 1;
    u8 unk7F : 7;
  } activeFlags;
  u8 unk05[3];
  struct mslSound* next;
  struct mslSound* previous;
  struct mslSound* slotNext;
  mslBank* bank;
  mslSoundSlot* slot;
  mslSoundMix mix;
  s32 commandIndex;
  s32 priority;
  mslSoundCommandStream* commandStream;
  OSTime sequenceStart;
  OSTime lastTime;
  OSTime elapsedTicks;
  s32 loopCount;
  u32 unk4C;
  /* Non-NULL while playing (mslSoundIsPlaying). */
  mslSoundCommand* sequenceCursor;
  /* First command after the loop-start opcode. */
  mslSoundCommand* loopStart;
  struct mslPlayback* playbackHead;
  /* SoundProcess consumes this with mslStreamPreloadActivate. */
  struct mslPlayback* preloadedPlayback;
  /* Per-sound ramps (mslPlaybackAdjustProcess). */
  mslAdjustment* adjustments;
  f32 rawVolume;
  f32 savedVolume;
  u32 unk6C;
} mslSound;

/* Stream/playback item: mslPlaybackStart/Process and the mslStream* functions
   take it; mslStream.playback points back to it. */
/* +0 flags select stream (bit1) or paired voices (bit0); +4 is
   the stream asset index or MusyX effect ID in mslPlaybackStart. */
typedef struct mslPlaybackDefinition {
  u32 flags;
  u32 assetID;
  u32 secondAssetID;
} mslPlaybackDefinition;

typedef struct mslPlayback {
  struct mslPlayback* next;
  struct mslPlayback* previous;
  struct {
    u8 unk80 : 1;
    u8 activationPending : 1;
    u8 unk20 : 1;
    u8 ducked : 1;
    u8 unk0F : 4;
  } streamFlags;
  u8 unk09[3];
  mslSound* sound;
  mslPlaybackDefinition* definition;
  mslSoundAsset* asset;
  u32 unk18;
  u32 unk1C;
  union {
    struct mslStream* stream;
    SND_VOICEID voice;
  };
  SND_VOICEID secondVoice;
} mslPlayback;

int mslSoundIsValid(mslSound* sound);
int mslSoundIsPlaying(mslSound* sound);
void mslSoundStop(mslSound* sound);
void mslSoundPause(mslSound* sound);
void mslSoundSetVol(mslSound* sound, f32 volume);
void mslSoundSetPitch(mslSound* sound, f32 pitch);
void mslSoundPlayAfterPrep(mslSound* sound);
void mslStopAll(mslSoundSystem* sys);
void mslEndAll(mslSoundSystem* sys);
s32 mslBankUnLoad(mslBank* bank);
void mslPauseAll(void);
void mslUnPauseAll(void);
s32 mslTick(void);
s32 mslUnInit(mslSoundSystem* sys);
mslSound* mslBankPlayVolPanPitch(mslBank* bank, s32 index, s32 slot, s32 priority, u32 flags, f32 volume, f32 pan, f32 pitch);
mslSound* mslBankPlayVol(mslBank* bank, s32 index, s32 slot, s32 priority, u32 flags, f32 volume);
f32 mslGetVol(mslSoundSystem* sys);
void mslSetVol(mslSoundSystem* sys, f32 volume);
void mslSetDuckVol(mslSoundSystem* sys, f32 volume);

/* Passed by value (callers copy the word). additive set: volumes add and 1.0
   is subtracted (floored at 0); clear: volumes multiply. */
typedef struct mslCompoundMode {
  u32 unk80 : 1;
  u32 unk40 : 1;
  u32 additive : 1;
  u32 unk1F : 29;
} mslCompoundMode;

f32 mslVolCompound(f32 master, f32 volume, mslCompoundMode mode);
f32 mslPitchCompound(f32 master, f32 pitch);

static inline void mslPlaybackPoolFree(mslSoundSystem* system, mslPlayback* playback) {
  if (system == NULL || playback == NULL) {
    printf("mslPlaybackPoolFree: NULL pointer!  msi=%x mp=%x\n", system, playback);
  } else {
    disableIRQ();
    playback->streamFlags.unk80 = 0;
    enableIRQ();
  }
}

#endif
