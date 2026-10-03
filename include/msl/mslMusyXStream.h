#ifndef MSL_MSLMUSYXSTREAM_H
#define MSL_MSLMUSYXSTREAM_H

#include <dolphin/types.h>
#include <dolphin/os.h>
#include <musyx/musyx.h>
#include <msl/mslMusyX.h>
#include <msl/mlSysCalls.h>

struct mslStream;

/* 0x60-byte header skipped before ADPCM sample reads. */
typedef struct mslStreamDataHeader {
  u32 samples;
  u32 bytes;
  u32 frequency;
  u8 unk0C[0x10];
  SND_ADPCMSTREAM_INFO adpcmInfo;
  u8 unk3C[0x24];
} mslStreamDataHeader;

/* User data of the stream's mfl async requests. */
typedef struct mslStreamChannel {
  /* Cleared by mslStreamStop when it detaches the channel. */
  volatile u8 attached : 1;
  volatile u8 half : 1;
  /* Set while the seek with mslStreamDVDExtraCallback is queued. */
  volatile u8 seeking : 1;
  volatile u8 unk10 : 1;
  volatile u8 loopPending : 1;
  /* Forces volume 0 in mslStreamSetPan. */
  volatile u8 muted : 1;
  volatile u8 unk00_0 : 2;
  /* mkMusyXPan result */
  u8 pan;
  u16 unk02;
  struct mslStream* stream;
  /* First argument of the seek and read requests. */
  void *file;
  /* Queue entry released by mslStreamDVDExtraCallback. */
  void *pendingQ;
  /* Queue entries cancelled and freed by mslStreamStop. */
  mlAsyncRequest* requests[2];
  SND_STREAMID streamID;
  u32 frequency;
  u32 fileOffset;
  u32 readOffset;
  u32 readEnd;
  u32 unk2C;
  u32 halfSamples;
  s32 loopsRemaining;
  u32 unk38;
  u8* loopBuffer;
  u32 loopBytes;
  SND_ADPCMSTREAM_INFO adpcmInfo;
  void* buffer;
  u32 bufferSize;
  u32 sampleCount;
} mslStreamChannel;

/* Eight entries in streamMgr; retail allocation advances by 0x30. */
typedef struct mslStream {
  struct mslPlayback* playback;
  u8 state;
  u8 volume;
  u16 priority;
  OSTime lastTime;
  u32 elapsedTicks;
  u32 durationMs;
  struct {
    u8 ended : 1;
    u8 unk7F : 7;
  } playbackFlags;
  u8 unk19[3];
  u32 streamType;
  mslStreamChannel* channels[2];
  u8 unk28[8];
} mslStream;

/* 0x4A0 bytes (local streamMgr). */
typedef struct mslStreamMgr {
  mslStream streams[8];
  s32 nextStream;
  mslStreamChannel channels[7];
  s32 nextChannel;
  u32 bufferSize;
  mlSysCalls *sysCalls;
} mslStreamMgr;

typedef struct mslStreamFile {
  mflFile* file;
  u32 unk04;
} mslStreamFile;

extern mslStreamFile streamFile[7];

void mslStreamSetPan(mslPlayback* playback, u8 pan);
void mslStreamSetPitch(mslPlayback* playback, u16 pitch);
void mslStreamSetVol(mslPlayback* playback, u8 volume);
void mslStreamUnInit(void);
void mslStreamProcess(mslPlayback* playback);
void mslStreamPreloadActivate(mslPlayback* playback);
void mslStreamStop(mslPlayback* playback);
mslStream* mslStreamStart(mslBank* bank, mslPlayback* playback, u8 volume, u8 pan, BOOL preload);

int mslSetWavePath(mslSoundSystem* sys, const char* path);

#endif
