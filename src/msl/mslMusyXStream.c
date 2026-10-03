#include <msl/mslMusyXStream.h>
#include <msl/mslMusyXUtil.h>
#include <string.h>
#include <stdio.h>
#include <dolphin/os.h>
#include <msl/mslMem.h>
#include <msl/mflFile.h>

static u32 mslStreamCallback(void* buffer1, u32 length1, void* buffer2,
                             u32 length2, u32 user);
static int mslStreamActivate(mslStream* stream);
static void mslStreamDVDExtraCallback(s32 result, void* user);
static void mslStreamDVDCallback(s32 result, void* user);

static char mslWavePath[0x100];
static mslStreamMgr streamMgr;
mslStreamFile streamFile[7];

/* Streams ignore pitch: retail is a bare blr. */
void mslStreamSetPitch(mslPlayback* playback, u16 pitch) {}

void mslStreamSetPan(mslPlayback* playback, u8 pan) {
  mslStream* stream = playback->stream;
  if (stream != NULL) {
    stream->channels[0]->pan = pan;
    if (((playback->asset->flags >> 1) & 7) == 2) {
      stream->channels[0]->pan = mkMusyXPan(-1.0f);
      stream->channels[1]->pan = mkMusyXPan(1.0f);
    }
    if (stream->state == 6) {
      mslSoundSystem* sys = playback->sound->bank->system;
      u8 volume = mkMusyXVolume(sys->volume);
      u8 mixedPan = mkMusyXPan(sys->pan);
      u8 scaledPan;
      u8 scaledVolume;

      if (stream->channels[0]->muted) {
        volume = 0;
      }
      scaledPan = stream->channels[0]->pan;
      scaledPan = mslMusyXScalePan(scaledPan, mixedPan);
      sndStreamMixParameterEx(stream->channels[0]->streamID,
          mslMusyXScale(stream->volume, volume), scaledPan, 0, 0, 0);
      if (((playback->asset->flags >> 1) & 7) == 2) {
        scaledPan = mslMusyXScalePan(stream->channels[1]->pan, mixedPan);
        scaledVolume = mslMusyXScale(stream->volume, volume);
        sndStreamMixParameterEx(stream->channels[1]->streamID, scaledVolume,
                                scaledPan, 0, 0, 0);
      }
    }
  }
}

int mslSetWavePath(mslSoundSystem* sys, const char* path) {
  strcpy(mslWavePath, path);
  return 1;
}

void mslStreamSetVol(mslPlayback* playback, u8 volume) {
  mslStream* stream = playback->stream;
  mslSoundSystem* settings = playback->sound->bank->system;

  if (stream != NULL) {
    stream->volume = volume > 17 ? volume - 17 : 0;
    if (stream->state == 6) {
      u8 mixedPan;
      u8 mixedVolume;
      int stereo = ((playback->asset->flags >> 1) & 7) == 2;
      u8 scaledPan;
      u8 scaledVolume;

      if (stream->playback->streamFlags.ducked) {
        mixedVolume = mkMusyXVolume(settings->volume * settings->duckVolume);
        mixedPan = mkMusyXPan(settings->pan + settings->duckPan);
      } else {
        mixedVolume = mkMusyXVolume(settings->volume);
        mixedPan = mkMusyXPan(settings->pan);
      }
      stream->channels[0]->muted;
      scaledPan = stream->channels[0]->pan;
      scaledPan = mslMusyXScalePan(scaledPan, mixedPan);
      scaledVolume = mslMusyXScale(stream->volume, mixedVolume);
      sndStreamMixParameterEx(stream->channels[0]->streamID, scaledVolume,
                              scaledPan, 0, 0, 0);
      if (stereo) {
        scaledPan = mslMusyXScalePan(stream->channels[1]->pan, mixedPan);
        scaledVolume = mslMusyXScale(stream->volume, mixedVolume);
        sndStreamMixParameterEx(stream->channels[1]->streamID, scaledVolume,
                                scaledPan, 0, 0, 0);
      }
    }
  }
}

static inline void mslStreamChannelStop(mslStreamChannel *channel) {
  void *file;
  u32 i;

  disableIRQ();
  if (channel->pendingQ != NULL) {
    streamMgr.sysCalls->cancelQ(channel->pendingQ);
    streamMgr.sysCalls->freeQ(channel->pendingQ);
    channel->pendingQ = NULL;
  }
  for (i = 0; i < 2; i++) {
    if (channel->requests[i] != NULL) {
      streamMgr.sysCalls->cancelQ(channel->requests[i]);
      streamMgr.sysCalls->freeQ(channel->requests[i]);
      channel->requests[i] = NULL;
    }
  }
  channel->seeking = 0;
  enableIRQ();
  file = channel->file;
  for (i = 0; i < 7; i++) {
    if (file == streamFile[i].file) {
      streamFile[i].unk04 = 1;
    }
  }
  channel->file = NULL;
}

static inline void mslStreamHalt(mslStream* stream) {
  disableIRQ();
  if (stream->channels[0] != NULL) {
    sndStreamDeactivate(stream->channels[0]->streamID);
    mslStreamChannelStop(stream->channels[0]);
  }
  if (((stream->playback->asset->flags >> 1) & 7) == 2 && stream->channels[1] != NULL) {
    sndStreamDeactivate(stream->channels[1]->streamID);
    mslStreamChannelStop(stream->channels[1]);
  }
  if (stream->channels[0] != NULL) {
    stream->channels[0]->attached = 0;
    stream->channels[0] = NULL;
  }
  if (stream->channels[1] != NULL) {
    stream->channels[1]->attached = 0;
    stream->channels[1] = NULL;
  }
  stream->state = 0;
  enableIRQ();
}

/* TODO: [near miss] 98.32%; stream/stereo register swap and entry branch shape remain. */
void mslStreamProcess(mslPlayback* playback) {
  OSTime time = OSGetTime();
  if (playback != NULL && playback->stream != NULL) {
    mslStream* stream = playback->stream;
    int stereo = ((playback->asset->flags >> 1) & 7) == 2;
    if (stream != NULL) {
      if (stream->channels[0] == NULL || (stereo && stream->channels[1] == NULL)) {
        mslStreamHalt(stream);
        playback->streamFlags.activationPending = 1;
      } else if (!stream->channels[0]->seeking &&
                 (!stereo || !stream->channels[1]->seeking)) {
        stream->elapsedTicks += time - stream->lastTime;
        stream->lastTime = time;
        if (stream->playbackFlags.ended &&
            stream->elapsedTicks >= OSMillisecondsToTicks(stream->durationMs)) {
          mslStreamHalt(stream);
          playback->streamFlags.activationPending = 1;
        } else {
          mslSoundSystem* system = playback->sound->bank->system;
          u8 volume;
          u8 pan;
          u8 scaledPan;
          u8 scaledVolume;
          if (stream->playback->streamFlags.ducked) {
            volume = mkMusyXVolume(system->volume * system->duckVolume);
            pan = mkMusyXPan(system->pan + system->duckPan);
          } else {
            volume = mkMusyXVolume(system->volume);
            pan = mkMusyXPan(system->pan);
          }
          if (mflGetDiscErrorStatus()) {
            volume = 0;
          }
          scaledPan = mslMusyXScalePan(stream->channels[0]->pan, pan);
          scaledVolume = mslMusyXScale(stream->volume, volume);
          sndStreamMixParameterEx(stream->channels[0]->streamID, scaledVolume,
                                  scaledPan, 0, 0, 0);
          if (stereo) {
            scaledPan = mslMusyXScalePan(stream->channels[1]->pan, pan);
            scaledVolume = mslMusyXScale(stream->volume, volume);
            sndStreamMixParameterEx(stream->channels[1]->streamID, scaledVolume,
                                    scaledPan, 0, 0, 0);
          }
        }
      }
    }
  }
}

void mslStreamPreloadActivate(mslPlayback* playback) {
  mslStream* stream = playback->stream;

  if (stream != NULL) {
    if (stream->channels[0] == NULL ||
        (((playback->asset->flags >> 1) & 7) == 2 && stream->channels[1] == NULL)) {
      playback->streamFlags.activationPending = 1;
      return;
    }
    if (stream->streamType != 0) {
      switch (stream->state) {
      case 3:
        stream->state = 2;
        break;
      case 5:
        stream->state = 4;
        break;
      }
    } else {
      mslStreamActivate(stream);
    }
  }
}

void mslStreamStop(mslPlayback* playback) {
  mslStream* stream = playback->stream;

  if (stream != NULL) {
    mslStreamHalt(stream);
  }
}

static inline void mslStreamFindNext(void) {
  s32 start = streamMgr.nextStream;

  if (start < 0) {
    start = streamMgr.nextStream = 0;
  }
  do {
    if (streamMgr.streams[streamMgr.nextStream].state == 0) {
      return;
    }
    if (++streamMgr.nextStream >= 8) {
      streamMgr.nextStream = 0;
    }
  } while (streamMgr.nextStream != start);
  streamMgr.nextStream = -1;
}

static inline mslStream* mslStreamAlloc(void) {
  mslStream* stream;

  disableIRQ();
  if (streamMgr.nextStream < 0) {
    mslStreamFindNext();
    if (streamMgr.nextStream < 0) {
      enableIRQ();
      return NULL;
    }
  }
  stream = &streamMgr.streams[streamMgr.nextStream];
  stream->state = 1;
  stream->volume = mkMusyXVolume(0.0f);
  stream->playbackFlags.ended = 0;
  stream->channels[0] = NULL;
  stream->channels[1] = NULL;
  if (++streamMgr.nextStream >= 8) {
    streamMgr.nextStream = 0;
  }
  mslStreamFindNext();
  enableIRQ();
  return stream;
}

static inline void mslStreamChannelFindNext(void) {
  u32 start = streamMgr.nextChannel;

  if (streamMgr.nextChannel < 0) {
    start = streamMgr.nextChannel = 0;
  }
  do {
    if (!streamMgr.channels[streamMgr.nextChannel].attached) {
      return;
    }
    if (++streamMgr.nextChannel >= 7) {
      streamMgr.nextChannel = 0;
    }
  } while (streamMgr.nextChannel != start);
  streamMgr.nextChannel = -1;
}

static inline mslStreamChannel* mslStreamChannelAlloc(void) {
  mslStreamChannel* channel;

  disableIRQ();
  if (streamMgr.nextChannel < 0) {
    mslStreamChannelFindNext();
    if (streamMgr.nextChannel < 0) {
      enableIRQ();
      return NULL;
    }
  }
  channel = &streamMgr.channels[streamMgr.nextChannel];
  channel->attached = 1;
  channel->bufferSize = streamMgr.bufferSize;
  channel->sampleCount = (streamMgr.bufferSize / 8) * 14;
  channel->halfSamples = channel->sampleCount / 2;
  channel->fileOffset = 0;
  channel->readOffset = 0;
  channel->frequency = 48000;
  channel->half = 0;
  channel->seeking = 0;
  channel->unk10 = 0;
  channel->pan = mkMusyXPan(0.0f);
  channel->loopsRemaining = 0;
  channel->unk38 = 0;
  channel->pendingQ = NULL;
  if (channel->buffer == NULL) {
    channel->buffer = mslHeapAlignedAlloc(MSLMFL_HEAP, channel->bufferSize, "stream channel buffer");
    channel->streamID = sndStreamAllocEx(0xFF, channel->buffer, channel->sampleCount, 48000,
                                         mkMusyXVolume(1.0f), mkMusyXPan(0.0f), mkMusyXPan(0.0f),
                                         mkMusyXVolume(1.0f), mkMusyXVolume(0.0f), 0, 0x30001,
                                         mslStreamCallback, (u32)channel, NULL);
  }
  if (++streamMgr.nextChannel >= 7) {
    streamMgr.nextChannel = 0;
  }
  mslStreamChannelFindNext();
  enableIRQ();
  return channel;
}

static inline void mslStreamRelease(mslStream* stream) {
  if (stream->channels[0] != NULL) {
    stream->channels[0]->attached = 0;
    stream->channels[0] = NULL;
  }
  if (stream->channels[1] != NULL) {
    stream->channels[1]->attached = 0;
    stream->channels[1] = NULL;
  }
  stream->state = 0;
}

static inline mflFile* mslStreamFileGet(void) {
  u32 i;

  for (i = 0; i < 7; i++) {
    if (streamFile[i].unk04 != 0) {
      streamFile[i].unk04 = 0;
      streamFile[i].file->flags.allocated = 1;
      streamFile[i].file->flags.open = 1;
      streamFile[i].file->flags.unk08 = 0;
      streamFile[i].file->flags.modeU = 1;
      streamFile[i].file->position = 0;
      streamFile[i].file->bufferOffset = -1;
      streamFile[i].file->bufferBytes = 0;
      streamFile[i].file->lastResult = 0;
      return streamFile[i].file;
    }
  }
  OSReport("MslStream: mflOpenStream() overflow too many open files %d\n", i);
  return NULL;
}

static inline void mslStreamFileRelease(mflFile* file) {
  u32 i;

  for (i = 0; i < 7; i++) {
    if (file == streamFile[i].file) {
      streamFile[i].unk04 = 1;
    }
  }
}

/* TODO: [near miss] 99.90%; state and loop-word register staging remain. */
mslStream* mslStreamStart(mslBank* bank, mslPlayback* playback, u8 volume, u8 pan, BOOL preload) {
  mslStream* stream = playback->stream;
  s32 state;
  mlAsyncRequest* request;

  if (stream == NULL) {
    stream = mslStreamAlloc();
    if (stream == NULL) {
      return NULL;
    }
    stream->playback = playback;
    stream->volume = volume > 17 ? volume - 17 : 0;
    stream->priority = ((playback->definition->flags >> 2) & 1) + 10;
    stream->channels[0] = mslStreamChannelAlloc();
    if (stream->channels[0] == NULL) {
      mslStreamRelease(stream);
      return NULL;
    }
    stream->channels[0]->stream = stream;
    if (((playback->asset->flags >> 1) & 7) == 2) {
      stream->channels[1] = mslStreamChannelAlloc();
      if (stream->channels[1] == NULL) {
        mslStreamRelease(stream);
        return NULL;
      }
      stream->channels[1]->stream = stream;
    }
  }
  stream->streamType = (playback->asset->flags >> 1) & 7;
  playback->streamFlags.unk20 = 1;
  switch (stream->streamType) {
  case 1:
    stream->channels[0]->file = mslStreamFileGet();
    if (stream->channels[0]->file == NULL) {
      mslStreamRelease(stream);
      return NULL;
    }
    stream->channels[0]->pan = mkMusyXPan(0.0f);
    stream->channels[0]->loopsRemaining = playback->unk18;
    stream->channels[0]->unk38 = playback->unk1C;
    state = 2;
    if (preload) {
      state = 3;
    }
    stream->state = state;
    stream->channels[0]->seeking = 1;
    request = streamMgr.sysCalls->seekAsync(stream->channels[0]->file, playback->asset->dataOffsets[0], 0,
                                           stream->priority, mslStreamDVDExtraCallback, stream->channels[0]);
    streamMgr.sysCalls->freeQ(request);
    stream->channels[0]->fileOffset = playback->asset->dataOffsets[0];
    stream->channels[0]->pendingQ = streamMgr.sysCalls->readAsync(
        stream->channels[0]->buffer, 0x60, 1, stream->channels[0]->file, stream->priority,
        mslStreamDVDCallback, stream->channels[0]);
    break;
  case 2:
    stream->channels[0]->file = mslStreamFileGet();
    stream->channels[1]->file = mslStreamFileGet();
    if (stream->channels[0]->file == NULL || stream->channels[1]->file == NULL) {
      if (stream->channels[0]->file != NULL) {
        mslStreamFileRelease(stream->channels[0]->file);
      }
      if (stream->channels[1]->file != NULL) {
        mslStreamFileRelease(stream->channels[1]->file);
      }
      mslStreamRelease(stream);
      return NULL;
    }
    stream->channels[0]->pan = mkMusyXPan(-1.0f);
    stream->channels[1]->pan = mkMusyXPan(1.0f);
    stream->channels[0]->loopsRemaining = playback->unk18;
    stream->channels[0]->unk38 = playback->unk1C;
    stream->channels[1]->loopsRemaining = playback->unk18;
    stream->channels[1]->unk38 = playback->unk1C;
    state = 2;
    if (preload) {
      state = 3;
    }
    stream->state = state;
    stream->channels[0]->seeking = 1;
    stream->channels[1]->seeking = 1;
    request = streamMgr.sysCalls->seekAsync(stream->channels[0]->file, playback->asset->dataOffsets[0], 0,
                                           stream->priority, mslStreamDVDExtraCallback, stream->channels[0]);
    streamMgr.sysCalls->freeQ(request);
    stream->channels[0]->fileOffset = playback->asset->dataOffsets[0];
    stream->channels[0]->pendingQ = streamMgr.sysCalls->readAsync(
        stream->channels[0]->buffer, 0x60, 1, stream->channels[0]->file, stream->priority,
        mslStreamDVDCallback, stream->channels[0]);
    streamMgr.sysCalls->freeQ(streamMgr.sysCalls->seekAsync(
        stream->channels[1]->file, playback->asset->dataOffsets[1], 0, stream->priority,
        mslStreamDVDExtraCallback, stream->channels[1]));
    stream->channels[1]->fileOffset = playback->asset->dataOffsets[1];
    stream->channels[1]->pendingQ = streamMgr.sysCalls->readAsync(
        stream->channels[1]->buffer, 0x60, 1, stream->channels[1]->file, stream->priority,
        mslStreamDVDCallback, stream->channels[1]);
    break;
  default:
    return NULL;
  }
  return stream;
}

/* TODO: [near miss] 95.82%; register allocation and file-loop scheduling remain. */
void mslStreamUnInit(void) {
  int i;
  mslStream* stream = streamMgr.streams;
  mslStreamChannel* channel;

  for (i = 0; i < 8; i++, stream++) {
    if (stream->state != 0) {
      stream->state = 0;
    }
  }

  channel = streamMgr.channels;
  for (i = 0; i < 7U; i++, channel++) {
    if (channel->streamID != (SND_STREAMID)-1) {
      sndStreamFree(channel->streamID);
      channel->streamID = (SND_STREAMID)-1;
    }
    if (channel->buffer != NULL) {
      mlHeapFree(channel->buffer);
      channel->buffer = NULL;
    }
  }

  for (i = 0; i < 7; i++) {
    mflClose(streamFile[i].file);
    streamFile[i].file = NULL;
    streamFile[i].unk04 = 0;
  }
}

/* TODO: [near miss] 99.58%; register allocation and a literal dependency remain. */
void mslStreamInit(void) {
  mslStreamChannel* channel;
  u8 span;
  mslStream* stream;
  f32 requestedSize = 41148.0f;
  u8 auxa;
  u32 i;
  u8 pan;

  memset(&streamMgr, 0, sizeof(streamMgr));
  streamMgr.bufferSize = ((u32)requestedSize + 63) & ~63;
  streamMgr.sysCalls = &g_SysCalls;
  stream = streamMgr.streams;
  for (i = 0; i < 8; ++i, ++stream) {
    stream->state = 0;
    stream->volume = mkMusyXVolume(0.0f);
    stream->channels[0] = NULL;
    stream->channels[1] = NULL;
  }
  channel = streamMgr.channels;
  for (i = 0; i < 7; ++i, ++channel) {
    channel->bufferSize = streamMgr.bufferSize;
    channel->sampleCount = (streamMgr.bufferSize >> 3) * 14;
    channel->halfSamples = channel->sampleCount >> 1;
    channel->fileOffset = 0;
    channel->readOffset = 0;
    channel->frequency = 48000;
    channel->half = 0;
    channel->seeking = 0;
    channel->unk10 = 0;
    channel->pan = mkMusyXPan(0.0f);
    channel->loopsRemaining = 0;
    channel->unk38 = 0;
    channel->pendingQ = NULL;
    if (channel->buffer == NULL) {
      u8 auxb;

      channel->buffer = mslHeapAlignedAlloc(MSLMFL_HEAP, channel->bufferSize, "stream channel buffer");
      auxb = mkMusyXVolume(0.0f);
      auxa = mkMusyXVolume(1.0f);
      span = mkMusyXPan(0.0f);
      pan = mkMusyXPan(0.0f);
      channel->streamID = sndStreamAllocEx(255, channel->buffer, channel->sampleCount,
          48000, mkMusyXVolume(1.0f), pan, span, auxa, auxb, 0, 0x30001,
          mslStreamCallback, (u32)channel, NULL);
    }
    channel->attached = 0;
  }
  streamMgr.nextStream = -1;
  streamMgr.nextChannel = -1;
  {
    s32 fileIndex;
    char path[128] = {0};

    strcpy(path, "/sndsgc/CombinedADP.cmb");
    for (fileIndex = 0; fileIndex < 7; ++fileIndex) {
      streamFile[fileIndex].file = mflOpen(path, "rbu");
      streamFile[fileIndex].unk04 = 1;
    }
  }
}

/* TODO: [near miss] 99.72%; state register allocation and one zero argument load remain. */
static void mslStreamDVDCallback(s32 result, void* user) {
  void* buffer;
  mslStreamChannel* channel = user;
  mslStream* stream;
  int preloaded = 0;
  u32 bytes;

  if (channel != NULL) {
    stream = channel->stream;
  } else {
    stream = NULL;
  }
  if (!channel->attached || result <= 0) {
    return;
  }
  channel->seeking = 0;
  switch (stream->state) {
  case 3:
    preloaded = 1;
  case 2: {
    mslStreamDataHeader* header = channel->buffer;

    channel->readEnd = (header->bytes >> 1) & ~31U;
    channel->fileOffset += 0x60;
    channel->frequency = header->frequency;
    channel->unk2C = (u32)(1000.0f * ((f32)header->samples / (f32)header->frequency));
    memcpy(&channel->adpcmInfo, &header->adpcmInfo, 0x20);
    if (channel->pendingQ != NULL) {
      streamMgr.sysCalls->freeQ(channel->pendingQ);
      channel->pendingQ = NULL;
    }
    if (channel->frequency > 48000U) {
      disableIRQ();
      if (stream->channels[0] != NULL) {
        sndStreamDeactivate(stream->channels[0]->streamID);
        mslStreamChannelStop(stream->channels[0]);
      }
      if (((stream->playback->asset->flags >> 1) & 7) == 2 && stream->channels[1] != NULL) {
        sndStreamDeactivate(stream->channels[1]->streamID);
        mslStreamChannelStop(stream->channels[1]);
      }
      if (stream->channels[0] != NULL) {
        stream->channels[0]->attached = 0;
        stream->channels[0] = NULL;
      }
      if (stream->channels[1] != NULL) {
        stream->channels[1]->attached = 0;
        stream->channels[1] = NULL;
      }
      stream->state = 0;
      enableIRQ();
      return;
    }
    --stream->streamType;
    if (stream->streamType == 0) {
      s32 state;
      u32 firstBytes;
      u32 secondBytes;
      u32 duration;

      stream->streamType = (stream->playback->asset->flags >> 1) & 7;
      firstBytes = stream->channels[0]->bufferSize >> 1;
      if (stream->channels[0]->readEnd < firstBytes) {
        memset(stream->channels[0]->buffer, 0, stream->channels[0]->bufferSize);
      }
      stream->channels[0]->readOffset = firstBytes;
      if (stream->streamType > 1U) {
        secondBytes = stream->channels[1]->bufferSize >> 1;
        if (stream->channels[1]->readEnd < secondBytes) {
          memset(stream->channels[1]->buffer, 0, stream->channels[1]->bufferSize);
        }
        stream->channels[1]->readOffset = secondBytes;
      }
      state = 4;
      duration = stream->channels[0]->unk2C;
      stream->durationMs = duration;
      stream->elapsedTicks = 0;
      if (preloaded) {
        state = 5;
      }
      stream->state = state;
      stream->channels[0]->seeking = 1;
      stream->channels[0]->pendingQ = streamMgr.sysCalls->seekAsync(
          stream->channels[0]->file, stream->channels[0]->fileOffset, 0,
          stream->priority, mslStreamDVDExtraCallback, stream->channels[0]);
      stream->channels[0]->requests[stream->channels[0]->half] =
          streamMgr.sysCalls->readAsync(stream->channels[0]->buffer, firstBytes, 1,
              stream->channels[0]->file, stream->priority,
              mslStreamDVDCallback, stream->channels[0]);
      if (((stream->playback->asset->flags >> 1) & 7) == 2) {
        stream->channels[1]->seeking = 1;
        stream->channels[1]->pendingQ = streamMgr.sysCalls->seekAsync(
            stream->channels[1]->file, stream->channels[1]->fileOffset, 0,
            stream->priority, mslStreamDVDExtraCallback, stream->channels[1]);
        stream->channels[1]->requests[stream->channels[1]->half] =
            streamMgr.sysCalls->readAsync(stream->channels[1]->buffer, secondBytes, 1,
                stream->channels[1]->file, stream->priority,
                mslStreamDVDCallback, stream->channels[1]);
      }
    }
    break;
  }
  case 4:
    --stream->streamType;
    if (stream->streamType == 0) {
      mslStreamActivate(stream);
    }
    break;
  case 5:
    --stream->streamType;
    break;
  case 6:
    if (channel->loopPending) {
      channel->loopPending = 0;
      channel->readOffset = 0;
      bytes = channel->loopBytes;
      buffer = channel->loopBuffer;
      channel->seeking = 1;
      if (channel->requests[channel->half] != NULL) {
        if (!channel->requests[channel->half]->flags.done) {
          if (mflGetDiscErrorStatus()) {
            sndStreamMixParameterEx(channel->streamID, 0, 63, 0, 0, 0);
          }
          channel->muted = 1;
          return;
        }
        channel->muted = 0;
        streamMgr.sysCalls->freeQ(channel->requests[channel->half]);
      }
      streamMgr.sysCalls->freeQ(streamMgr.sysCalls->seekAsync(
          channel->file, channel->fileOffset + channel->readOffset, 0,
          channel->stream->priority, mslStreamDVDExtraCallback, channel));
      channel->requests[channel->half] = streamMgr.sysCalls->readAsync(
          buffer, bytes, 1, channel->file, channel->stream->priority,
          mslStreamDVDCallback, channel);
      channel->readOffset += bytes;
    } else {
      u32 offset;

      if (channel->half) {
        offset = 0;
      } else {
        offset = channel->sampleCount >> 1;
      }
      sndStreamARAMUpdate(channel->streamID, offset, channel->halfSamples, 0, 0);
    }
    break;
  }
}

static void mslStreamDVDExtraCallback(s32 result, void *user) {
  mslStreamChannel *channel = user;

  if (channel != NULL && channel->pendingQ != NULL) {
    disableIRQ();
    streamMgr.sysCalls->freeQ(channel->pendingQ);
    channel->pendingQ = NULL;
    enableIRQ();
  }
}

/* Queues the next half-buffer read unless the previous request on this half
   is still in flight. */
static inline void mslStreamChannelRefill(mslStreamChannel* channel, u8* destination, u32 readBytes) {
  if (channel->requests[channel->half] != NULL) {
    if (!channel->requests[channel->half]->flags.done) {
      if (mflGetDiscErrorStatus() != 0) {
        sndStreamMixParameterEx(channel->streamID, 0, 63, 0, 0, 0);
      }
      channel->muted = 1;
      return;
    }
    channel->muted = 0;
    streamMgr.sysCalls->freeQ(channel->requests[channel->half]);
  }
  streamMgr.sysCalls->freeQ(streamMgr.sysCalls->seekAsync(
      channel->file, channel->fileOffset + channel->readOffset, 0,
      channel->stream->priority, mslStreamDVDExtraCallback, channel));
  channel->requests[channel->half] =
      streamMgr.sysCalls->readAsync(destination, readBytes, 1, channel->file,
          channel->stream->priority, mslStreamDVDCallback, channel);
  channel->readOffset += readBytes;
}

static inline void mslStreamChannelFill(mslStreamChannel* channel) {
  u32 readBytes;
  u8* destination;
  u32 halfBytes;

  readBytes = halfBytes = channel->bufferSize / 2;
  if (channel->half) {
    destination = (u8*)channel->buffer + halfBytes;
  } else {
    destination = channel->buffer;
  }
  memset(destination, 0, halfBytes);
  if (channel->readOffset + readBytes > channel->readEnd) {
    if (channel->readEnd > channel->readOffset) {
      readBytes = channel->readEnd - channel->readOffset;
      if (channel->loopsRemaining != 0) {
        if (channel->loopsRemaining != -1) {
          channel->loopsRemaining--;
        }
        channel->loopBuffer = destination + readBytes;
        channel->loopBytes = halfBytes - readBytes;
        channel->loopPending = 1;
      }
    } else {
      readBytes = 0;
    }
  }
  if (readBytes != 0) {
    channel->seeking = 1;
    mslStreamChannelRefill(channel, destination, readBytes);
  } else {
    channel->stream->playbackFlags.ended = 1;
  }
  channel->half ^= 1;
}

static u32 mslStreamCallback(void* buffer1, u32 length1, void* buffer2,
                             u32 length2, u32 user) {
  u32 readBytes;
  u8* destination;
  mslStreamChannel* channel = (mslStreamChannel*)user;
  u32 halfBytes;
  if (channel->stream->playbackFlags.ended) {
    return 0;
  }
  if (length1 + length2 < channel->sampleCount / 2) {
    return 0;
  }
  readBytes = halfBytes = channel->bufferSize / 2;
  if (channel->half) {
    destination = (u8*)channel->buffer + halfBytes;
  } else {
    destination = channel->buffer;
  }
  memset(destination, 0, halfBytes);
  if (channel->readOffset + readBytes > channel->readEnd) {
    if (channel->readEnd > channel->readOffset) {
      readBytes = channel->readEnd - channel->readOffset;
      if (channel->loopsRemaining != 0) {
        if (channel->loopsRemaining != -1) {
          channel->loopsRemaining--;
        }
        channel->loopBuffer = destination + readBytes;
        channel->loopBytes = halfBytes - readBytes;
        channel->loopPending = 1;
      }
    } else {
      readBytes = 0;
    }
  }
  if (readBytes != 0) {
    channel->seeking = 1;
    mslStreamChannelRefill(channel, destination, readBytes);
  } else {
    channel->stream->playbackFlags.ended = 1;
  }
  channel->half ^= 1;
  return channel->sampleCount / 2;
}

/* TODO: [near miss] 98.86%; entry zero sharing, pan narrowing and register allocation remain. */
static int mslStreamActivate(mslStream* stream) {
  int rightActive = 1;
  int leftActive;
  stream->playback->streamFlags.unk20 = 0;
  {
    mslStreamChannel* channel = stream->channels[0];
    u32 offset = 0;
    if (channel->half) {
      offset = channel->sampleCount >> 1;
    }
    sndStreamARAMUpdate(channel->streamID, offset, channel->sampleCount >> 1, 0, 0);
    channel->half ^= 1;
  }
  {
    u8 volume;
    mslStreamChannel* channel = stream->channels[0];
    u8 streamVolume = stream->volume;
    mslPlayback* playback = channel->stream->playback;
    mslSoundSystem* settings = playback->sound->bank->system;
    u8 pan;
    u8 channelPan;
    if (playback->streamFlags.ducked) {
      volume = mkMusyXVolume(settings->volume * settings->duckVolume);
      pan = mkMusyXPan(settings->pan + settings->duckPan);
    } else {
      volume = mkMusyXVolume(settings->volume);
      pan = mkMusyXPan(settings->pan);
    }
    channelPan = channel->pan;
    channelPan = mslMusyXScalePan(channelPan, pan);
    sndStreamMixParameterEx(channel->streamID, mslMusyXScale(streamVolume, volume), channelPan, 0, 0, 0);
    sndStreamFrq(channel->streamID, channel->frequency);
    sndStreamADPCMParameter(channel->streamID, &channel->adpcmInfo);
    leftActive = sndStreamActivate(channel->streamID) != 0;
  }
  if (((stream->playback->asset->flags >> 1) & 7) == 2) {
    {
      mslStreamChannel* channel = stream->channels[1];
      u32 offset;

      if (channel->half) {
        offset = channel->sampleCount >> 1;
      } else {
        offset = 0;
      }
      sndStreamARAMUpdate(channel->streamID, offset, channel->sampleCount >> 1, 0, 0);
      channel->half ^= 1;
    }
    {
      u8 volume;
      mslStreamChannel* channel = stream->channels[1];
      u8 streamVolume = stream->volume;
      mslPlayback* playback = channel->stream->playback;
      mslSoundSystem* settings = playback->sound->bank->system;
      u8 pan;
      u8 channelPan;
      if (playback->streamFlags.ducked) {
        volume = mkMusyXVolume(settings->volume * settings->duckVolume);
        pan = mkMusyXPan(settings->pan + settings->duckPan);
      } else {
        volume = mkMusyXVolume(settings->volume);
        pan = mkMusyXPan(settings->pan);
      }
      channelPan = channel->pan;
      channelPan = mslMusyXScalePan(channelPan, pan);
      sndStreamMixParameterEx(channel->streamID, mslMusyXScale(streamVolume, volume), channelPan, 0, 0, 0);
      sndStreamFrq(channel->streamID, channel->frequency);
      sndStreamADPCMParameter(channel->streamID, &channel->adpcmInfo);
      rightActive = sndStreamActivate(channel->streamID) != 0;
    }
  }
  mslStreamChannelFill(stream->channels[0]);
  if (((stream->playback->asset->flags >> 1) & 7) == 2) {
    mslStreamChannelFill(stream->channels[1]);
  }
  if (leftActive && rightActive) {
    stream->state = 6;
    stream->lastTime = OSGetTime();
  } else {
    printf("mslStreamActivate: activate failed, killing stream %s.\n", stream->playback->asset->name);
    mslStreamHalt(stream);
    return 0;
  }
  return 1;
}

