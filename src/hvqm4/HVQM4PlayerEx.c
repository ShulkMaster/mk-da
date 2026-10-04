#include "hvqm4.h"
#include "hvqm4player.h"
#include "hvqm4snd.h"
#include <dolphin/os.h>
#include <mwmem/mwMem.h>
#include <dolphin/vi.h>
#include <string.h>

typedef struct SeqFile SeqFile;
typedef HVQM4PlayerExFileCallback SeqFileCallback;

typedef struct MovieHeader {
  u8 version[0x18];
  u32 gops;
  u8 streamInfo[8];
  u32 frameTime;
  u32 maxRecordSize;
  u8 streamTail[4];
  u32 fileSize;
  VideoInfo video;
  HVQM4SoundInfo sound;
} MovieHeader;

typedef struct MovieFrame {
  s32 valid;
  s32 ready;
  u32 timestamp;
  void* pixels;
  HVQM4BufvNode* node;
} MovieFrame;

typedef struct MovieGop {
  u32 previousSize;
  u32 dataSize;
  u32 videoRecords;
  u32 audioRecords;
  u32 flags;
} MovieGop;

typedef struct MovieRecord {
  u16 type;
  u16 format;
  u32 size;
} MovieRecord;

struct HVQM4PlayerEx {
  SeqFile* file;                 /* 0x000 */
  MovieHeader header;             /* 0x004 */
  SeqObj decoder;                 /* 0x048 */
  s32 width;
  s32 height;
  s32 bufferCount;
  void* work;
  HVQM4Bufv* video;
  HVQM4Bufa* audio;
  HVQM4SoundContext* sound;
  s32 track;
  OSThread thread;                /* 0x078: naturally aligned to 8 */
  void* stack;                    /* 0x388 */
  u32 readSize;
  void* readBuffer;
  s32 started;
  s32 executing;
  s32 buffering;
  s32 control;
  s32 step;
  s32 skip;
  s32 wait;
  s32 drop;
  s32 exit;
  u32 interlaced;
  u32 field;
  u64 elapsed;                    /* 0x3c0 */
  u64 nextTime;
  u32 frameTime;
  void (*startCallback)(HVQM4PlayerEx*, s32);
  s32 startArgument;
  u32 frameSize;
  MovieFrame* previous;
  MovieFrame* current;
  MovieFrame* next;
  MovieFrame frames[3];           /* 0x3ec */
  s32 displayCount;
  s32 sendCount;
  s32 missedFrames;
};

extern void* gHeap;
extern const char HVQM4_FILEVERSION[];
extern SeqFile* SeqFileOpen(const char*, SeqFileCallback, HVQM4PlayerEx*, s32);
extern s32 SeqFileRead(void*, u32, SeqFile*);
extern s32 SeqFileGetDvdStatus(SeqFile*);
extern u32 SeqFileGetPos(SeqFile*);
extern s32 SeqFileSeek(SeqFile*, u32);
extern s32 SeqFileCheckBusy(SeqFile*);
extern s32 SeqFileClose(SeqFile*);

static volatile s32 lib_link_counter;
static s32 shutdown_request;
OSThread* gPlayerThread;
static u32 field_time = 16683;

static s32 _HVQM4PlayerExClose(HVQM4PlayerEx*, s32*);
static void* _HVQM4PlayerExThread(void*);
static s32 gop_decode(HVQM4PlayerEx*, const MovieGop*);
static s32 decv_decode(HVQM4PlayerEx*, u16, u32*);
static s32 decv_init(HVQM4PlayerEx*, s32*);

s32 HVQM4PlayerExControlExit(HVQM4PlayerEx* player)
{
  player->exit = 1;
  player->skip = 1;
  return 1;
}

s32 HVQM4PlayerExControlCheckNormal(HVQM4PlayerEx* player)
{
  return player->control == 0;
}

HVQM4BufaNode* HVQM4PlayerExBufaGetSend(HVQM4PlayerEx* player)
{
  HVQM4BufaNode* node;
  HVQM4BufaNode* next;
  if (player->buffering || !player->started) {
    return NULL;
  }
  if (player->audio == NULL) {
    return NULL;
  }
  if (shutdown_request) {
    while ((node = HVQM4BufaGetSend(player->audio)) != NULL) {
      HVQM4BufaSetFree(player->audio, node);
    }
  } else {
    node = HVQM4BufaGetSend(player->audio);
    if (node != NULL && player->drop) {
      while ((next = HVQM4BufaGetSend(player->audio)) != NULL) {
        HVQM4BufaSetFree(player->audio, node);
        node = next;
      }
    }
  }
  return node;
}

void HVQM4PlayerExBufaSetFree(HVQM4PlayerEx* player, HVQM4BufaNode* node)
{
  HVQM4BufaSetFree(player->audio, node);
}

u32 HVQM4PlayerExBufaGetSize(HVQM4BufaNode* node)
{
  return node->size;
}

void* HVQM4PlayerExBufaGetBuff(HVQM4BufaNode* node)
{
  return node->data;
}

HVQM4BufvNode* HVQM4PlayerExBufvGetDisp(HVQM4PlayerEx* player)
{
  HVQM4BufvNode* node;
  HVQM4BufvNode* next;
  if (player->buffering) {
    return NULL;
  }
  if (shutdown_request) {
    while ((node = HVQM4BufvGetDisp(player->video)) != NULL) {
      node->refCount = 0;
    }
  } else {
    node = HVQM4BufvGetDisp(player->video);
    if (node != NULL && player->drop) {
      while ((next = HVQM4BufvGetDisp(player->video)) != NULL) {
        node->refCount = 0;
        node = next;
      }
    }
    if (node == NULL) {
      player->missedFrames++;
    }
  }
  return node;
}

void HVQM4PlayerExBufvSetBusyFlag(HVQM4BufvNode* node, s32 busy)
{
  node->refCount = busy;
}

void* HVQM4PlayerExBufvGetBuff(HVQM4BufvNode* node)
{
  return node->data;
}

s32 HVQM4PlayerExGetHeight(HVQM4PlayerEx* player)
{
  return player->height;
}

s32 HVQM4PlayerExGetWidth(HVQM4PlayerEx* player)
{
  return player->width;
}

s32 HVQM4PlayerExCheckExecute(HVQM4PlayerEx* player)
{
  return player->executing;
}

s32 HVQM4PlayerExStart(HVQM4PlayerEx* player)
{
  shutdown_request = 0;
  OSResumeThread(&player->thread);
  return 1;
}

s32 HVQM4PlayerExClose(HVQM4PlayerEx* player, s32* error)
{
  void* result;
  s32 success = 1;
  OSJoinThread(&player->thread, &result);
  if (!_HVQM4PlayerExClose(player, error)) {
    success = 0;
  }
  if (shutdown_request) {
    if (error != NULL) {
      *error = 13;
    }
    success = 0;
  }
  return success;
}

HVQM4PlayerEx* HVQM4PlayerExCreate(const char* filename, s32 buffers, s32 priority,
                                 u32 stackSize, s32 field, SeqFileCallback callback,
                                 s32 callbackArgument, s32* error)
{
  s32 localError;
  BOOL enabled;
  HVQM4PlayerEx* player;
  u32 size;
  u32 workSize;
  s32 success;
  if (error == NULL) {
    error = &localError;
  }
  if (shutdown_request) {
    *error = 13;
    return NULL;
  }
  enabled = OSDisableInterrupts();
  lib_link_counter++;
  OSRestoreInterrupts(enabled);
  switch (VIGetTvFormat()) {
  case 0: field_time = 16683; break;
  case 2: field_time = 16683; break;
  case 1: field_time = 20000; break;
  }
  player = _mwMemMalloc(gHeap, sizeof(HVQM4PlayerEx), 5, "hHVQM4PlayerEx movie", "HVQM4PlayerEx.c", 786);
  if (player == NULL) {
    *error = 1;
    enabled = OSDisableInterrupts();
    if (lib_link_counter > 0) {
      lib_link_counter--;
    }
    OSRestoreInterrupts(enabled);
    return NULL;
  }
  player->file = NULL;
  player->work = NULL;
  player->audio = NULL;
  player->video = NULL;
  player->sound = NULL;
  player->track = 0;
  player->stack = NULL;
  player->readBuffer = NULL;
  player->readSize = 0;
  player->bufferCount = buffers;
  player->startCallback = NULL;
  player->startArgument = 0;
  player->control = 0;
  player->step = 0;
  player->skip = 0;
  player->wait = 0;
  player->drop = 0;
  player->missedFrames = 0;
  if ((player->file = SeqFileOpen(filename, callback, player, callbackArgument)) == NULL) {
    _HVQM4PlayerExClose(player, NULL);
    *error = 2;
    return NULL;
  }
  if (SeqFileRead(&player->header, 0x44, player->file) <= 0) {
    _HVQM4PlayerExClose(player, NULL);
    *error = 3;
    return NULL;
  }
  if (strncmp(HVQM4_FILEVERSION, (char*)&player->header, strlen(HVQM4_FILEVERSION))) {
    _HVQM4PlayerExClose(player, NULL);
    *error = 4;
    return NULL;
  }
  player->interlaced = 0;
  player->field = 1;
  if (((u8*)&player->header.video)[6] & 3) {
    if ((VIGetTvFormat() == 0 || VIGetTvFormat() == 2) &&
        (player->header.frameTime == 33366 || player->header.frameTime == 33367)) {
      player->interlaced = 1;
    }
    if (VIGetTvFormat() == 1 && player->header.frameTime == 40000) {
      player->interlaced = 1;
    }
  }
  if (player->interlaced == 1) {
    if (field == -1) {
      player->field = ((u8*)&player->header.video)[6] & 1;
    } else {
      player->field = field;
    }
  }
  HVQM4InitDecoder();
  HVQM4InitSeqObj(&player->decoder, &player->header.video);
  player->width = player->decoder.frame_width;
  player->height = player->decoder.frame_height;
  workSize = HVQM4BuffSize(&player->decoder);
  player->work = _mwMemMalloc(gHeap, workSize, 5, "work buffer movie", "HVQM4PlayerEx.c", 891);
  if (player->work == NULL) {
    _HVQM4PlayerExClose(player, NULL);
    *error = 5;
    return NULL;
  }
  HVQM4SetBuffer(&player->decoder, player->work);
  if (!decv_init(player, error)) {
    _HVQM4PlayerExClose(player, NULL);
    return NULL;
  }
  if (player->header.sound.sampleRate == 0 && player->header.sound.channels == 0) {
    success = 1;
  } else {
    u32 samples = player->header.sound.sampleRate * player->header.frameTime;
    samples = (samples / 1000000 + 64) * 4;
    samples &= ~127;
    player->audio = HVQM4BufaCreate(samples, player->bufferCount * 2);
    if (player->audio == NULL) {
      *error = 10;
      success = 0;
    } else {
      if ((player->sound = HVQM4DecSoundCreate(&player->header.sound)) == NULL) {
        *error = 11;
        success = 0;
      } else {
        success = 1;
      }
    }
  }
  if (!success) {
    _HVQM4PlayerExClose(player, NULL);
    return NULL;
  }
  stackSize = (stackSize + 31) & ~31;
  player->stack = _mwMemMalloc(gHeap, stackSize, 5, "hHVQM4PlayerEx Stack movie", "HVQM4PlayerEx.c", 915);
  if (player->stack == NULL) {
    _HVQM4PlayerExClose(player, NULL);
    *error = 6;
    return NULL;
  }
  if (player->header.maxRecordSize <= player->readSize) {
    success = 1;
  } else {
    size = (player->header.maxRecordSize + 32767) & ~32767;
    if (player->readBuffer != NULL) {
      _mwMemFree(player->readBuffer, "HVQM4PlayerEx.c", 511);
      player->readBuffer = NULL;
      player->readSize = 0;
    }
    player->readBuffer = _mwMemMalloc(gHeap, size, 5, "read buffer movie", "HVQM4PlayerEx.c", 515);
    if (player->readBuffer == NULL) {
      success = 0;
    } else {
      player->readSize = size;
      success = 1;
    }
  }
  if (!success) {
    _HVQM4PlayerExClose(player, NULL);
    *error = 7;
    return NULL;
  }
  if (!OSCreateThread(&player->thread, _HVQM4PlayerExThread, player,
                      (u8*)player->stack + stackSize, stackSize, priority, 0)) {
    _HVQM4PlayerExClose(player, NULL);
    *error = 8;
    return NULL;
  }
  player->started = 0;
  player->executing = 1;
  player->buffering = 1;
  gPlayerThread = &player->thread;
  return player;
}

static s32 _HVQM4PlayerExClose(HVQM4PlayerEx* player, s32* error)
{
  BOOL enabled;
  s32 success = 1;
  if (player->readBuffer != NULL) {
    _mwMemFree(player->readBuffer, "HVQM4PlayerEx.c", 715);
    player->readBuffer = NULL;
    player->readSize = 0;
  }
  if (player->file != NULL) {
    if (SeqFileGetDvdStatus(player->file) == -1) {
      if (error != NULL) {
        *error = 12;
      }
      success = 0;
    }
    SeqFileClose(player->file);
    player->file = NULL;
  }
  if (player->work != NULL) {
    _mwMemFree(player->work, "HVQM4PlayerEx.c", 732);
    player->work = NULL;
  }
  if (player->video != NULL) {
    HVQM4BufvClose(player->video);
    player->video = NULL;
  }
  if (player->audio != NULL) {
    HVQM4BufaClose(player->audio);
    player->audio = NULL;
  }
  if (player->sound != NULL) {
    HVQM4DecSoundClose(player->sound);
    player->sound = NULL;
  }
  if (player->stack != NULL) {
    _mwMemFree(player->stack, "HVQM4PlayerEx.c", 748);
    player->stack = NULL;
  }
  _mwMemFree(player, "HVQM4PlayerEx.c", 751);
  enabled = OSDisableInterrupts();
  if (lib_link_counter > 0) {
    lib_link_counter--;
  }
  OSRestoreInterrupts(enabled);
  return success;
}

static void* _HVQM4PlayerExThread(void* argument)
{
  HVQM4PlayerEx* player = argument;
  u32 count = player->header.gops;
  u32 index = 0;
  MovieGop gop;
  player->frameTime = player->header.frameTime;
  player->elapsed = 0;
  player->nextTime = 0;
  player->exit = 0;
  while (index < count && !player->exit && !shutdown_request) {
    u32 pos = SeqFileGetPos(player->file);
    u32 end;
    u32 previous;
    s32 control;
    if (SeqFileRead(&gop, sizeof(gop), player->file) <= 0) {
      break;
    }
    control = player->control;
    end = sizeof(gop);
    previous = pos - gop.previousSize;
    end += pos + gop.dataSize;
    if (!gop_decode(player, &gop)) {
      break;
    }
    switch (control) {
    case 4:
      if (index == 0) {
        player->control = 2;
        SeqFileSeek(player->file, pos);
      } else {
        SeqFileSeek(player->file, previous);
        index--;
      }
      break;
    case 3:
      if (index + 1 == count) {
        player->control = 2;
        SeqFileSeek(player->file, pos);
      } else {
        SeqFileSeek(player->file, end);
        index++;
      }
      break;
    case 2:
      {
        BOOL enabled;
        SeqFileSeek(player->file, pos);
        enabled = OSDisableInterrupts();
        player->wait++;
        OSRestoreInterrupts(enabled);
        while (1) {
          if (player->wait <= 0) {
            break;
          }
          if (shutdown_request || player->exit) {
            player->wait = 0;
            break;
          }
          VIWaitForRetrace();
        }
      }
      break;
    default:
      SeqFileSeek(player->file, end);
      index++;
      break;
    }
  }
  if (player->current->ready == 1) {
    player->current->ready = 0;
    player->current->node->refCount = 1;
    HVQM4BufvSetDisp(player->video, player->current->node);
  } else if (player->previous->ready == 1) {
    player->previous->ready = 0;
    player->previous->node->refCount = 1;
    HVQM4BufvSetDisp(player->video, player->previous->node);
  } else if (player->previous->ready == 0 && player->current->ready == 0 && player->next->ready == 1) {
    player->next->ready = 0;
    player->next->node->refCount = 1;
    HVQM4BufvSetDisp(player->video, player->next->node);
  }
  player->displayCount = HVQM4BufvGetDispNums(player->video);
  if (player->audio == NULL) {
    player->sendCount = 0;
  } else {
    player->sendCount = HVQM4BufaGetSendNums(player->audio);
  }
  player->buffering = 0;
  if (player->started) {
    while (HVQM4BufvChkDisp(player->video) != NULL) {}
    if (player->audio != NULL) {
      while (HVQM4BufaChkSend(player->audio) != NULL) {}
    }
  }
  player->started = 0;
  player->executing = 0;
  return NULL;
}

static s32 gop_decode(HVQM4PlayerEx* player, const MovieGop* gop)
{
  MovieRecord record;
  u32 size;
  u32 recordSize;
  u16 format;
  u32* payload;
  u32 count;
  u32 index;
  BOOL success;
  HVQM4BufvNode* video;
  HVQM4BufaNode* audio;
  if (player->exit) {
    player->skip = 1;
  }
  if (player->control == 4U) {
    player->skip = 1;
  } else if (player->control == 3U) {
    player->skip = 1;
  } else if (player->control == 2U) {
    player->skip = 1;
  } else {
    player->skip = 0;
  }
  index = 0;
  count = gop->videoRecords + gop->audioRecords;
  while (index < count) {
    index++;
    if (shutdown_request) {
      return 0;
    }
    if (SeqFileRead(&record, sizeof(record), player->file) <= 0) {
      return 0;
    }
    recordSize = record.size;
    if (recordSize <= player->readSize) {
      success = 1;
    } else {
      size = (recordSize + 32767) & ~32767;
      if (player->readBuffer != NULL) {
        _mwMemFree(player->readBuffer, "HVQM4PlayerEx.c", 511);
        player->readBuffer = NULL;
        player->readSize = 0;
      }
      player->readBuffer = _mwMemMalloc(gHeap, size, 5, "read buffer movie", "HVQM4PlayerEx.c", 515);
      if (player->readBuffer == NULL) {
        success = 0;
      } else {
        player->readSize = size;
        success = 1;
      }
    }
    if (!success) {
      return 0;
    }
    payload = player->readBuffer;
    if (SeqFileRead(payload, recordSize, player->file) <= 0) {
      return 0;
    }
    format = record.format;
    if (record.type == 1) {
      while ((video = HVQM4BufvGetFree(player->video)) == NULL) {
        player->buffering = 0;
        if (player->executing && player->elapsed == 0 && !SeqFileCheckBusy(player->file)) {
          player->elapsed = 1;
          player->started = 1;
          if (player->startCallback != NULL) {
            player->startCallback(player, player->startArgument);
          }
        }
      }
      HVQM4BufvSetFree(player->video, player->current->node);
      player->current->node = video;
      player->current->pixels = video->data;
      if (!decv_decode(player, format, payload)) {
        return 0;
      }
      if (player->current->ready == 1) {
        player->current->ready = 0;
        player->current->node->refCount = 1;
        HVQM4BufvSetDisp(player->video, player->current->node);
      } else if (player->previous->ready == 1) {
        player->previous->ready = 0;
        player->previous->node->refCount = 1;
        HVQM4BufvSetDisp(player->video, player->previous->node);
      } else if (player->previous->ready == 0 && player->current->ready == 0 && player->next->ready == 1) {
        player->next->ready = 0;
        player->next->node->refCount = 1;
        HVQM4BufvSetDisp(player->video, player->next->node);
      }
    } else if (record.type == 0) {
      while (1) {
        audio = HVQM4BufaGetFree(player->audio);
        if (audio != NULL) {
          break;
        }
        if (shutdown_request) {
          goto record_done;
        }
        VIWaitForRetrace();
      }
      {
        u32 samples = payload[0];
        void* output = audio->data;
        audio->size = samples * 4;
        HVQM4DecSoundDecode(player->sound, (HVQM4SoundFrameHeader*)&record, samples, (u8*)(payload + 1), player->track, output);
        HVQM4BufaSetSend(player->audio, audio);
      }
    }
record_done:
    if (index >= 4 && !(index & 1) && player->skip) {
      break;
    }
    if (!(index & 1)) {
      if (!player->step && player->control == 1U) {
        BOOL enabled = OSDisableInterrupts();
        player->wait++;
        OSRestoreInterrupts(enabled);
        while (1) {
          if (player->wait <= 0) {
            break;
          }
          if (shutdown_request || player->exit) {
            player->wait = 0;
            break;
          }
          VIWaitForRetrace();
        }
      }
      player->step = 0;
    }
  }
  return 1;
}

static s32 decv_decode(HVQM4PlayerEx* player, u16 type, u32* payload)
{
  u32 timestamp = payload[0];
  MovieFrame* frame;
  if (type != 0x30) {
    frame = player->previous;
    player->previous = player->next;
    player->next = frame;
    player->next->valid = 0;
  }
  if ((type == 0x20 && !player->previous->valid) ||
      (type == 0x30 && !player->previous->valid && !player->next->valid)) {
    return 0;
  }
  switch (type) {
  case 0x10:
    HVQM4DecodeIpic(&player->decoder, payload + 1, player->current->pixels);
    player->current->valid = 1;
    player->current->ready = 1;
    player->current->timestamp = timestamp;
    break;
  case 0x20:
    HVQM4DecodePpic(&player->decoder, payload + 1, player->current->pixels, player->previous->pixels);
    player->current->valid = 1;
    player->current->ready = 1;
    player->current->timestamp = timestamp;
    break;
  case 0x30:
    HVQM4DecodeBpic(&player->decoder, payload + 1, player->current->pixels, player->previous->pixels, player->next->pixels);
    player->current->valid = 1;
    player->current->ready = 1;
    player->current->timestamp = timestamp;
    break;
  default:
    return 0;
  }
  if (type != 0x30) {
    frame = player->current;
    player->current = player->next;
    player->next = frame;
  }
  return 1;
}

static s32 decv_init(HVQM4PlayerEx* player, s32* error)
{
  u32 sampling = player->decoder.h_samp * player->decoder.v_samp;
  u32 pixels = player->decoder.frame_width * player->decoder.frame_height;
  player->frameSize = pixels * (sampling + 2) / sampling;
  player->video = HVQM4BufvCreate(player->frameSize, player->bufferCount);
  if (player->video == NULL) {
    *error = 9;
    return 0;
  }
  if ((player->frames[0].node = HVQM4BufvGetFree(player->video)) == NULL) {
    OSPanic("HVQM4PlayerEx.c", 242, "decv_open(), buff[0].bufv set error !\n");
    *error = 9;
    return 0;
  }
  if ((player->frames[1].node = HVQM4BufvGetFree(player->video)) == NULL) {
    OSPanic("HVQM4PlayerEx.c", 247, "decv_open(), buff[1].bufv set error !\n");
    *error = 9;
    return 0;
  }
  if ((player->frames[2].node = HVQM4BufvGetFree(player->video)) == NULL) {
    OSPanic("HVQM4PlayerEx.c", 252, "decv_open(), buff[2].bufv set error !\n");
    *error = 9;
    return 0;
  }
  player->frames[0].valid = 0;
  player->frames[0].ready = -1;
  player->frames[1].valid = 0;
  player->frames[1].ready = -1;
  player->frames[2].valid = 0;
  player->frames[2].ready = -1;
  player->frames[0].pixels = player->frames[0].node->data;
  player->frames[1].pixels = player->frames[1].node->data;
  player->frames[2].pixels = player->frames[2].node->data;
  player->previous = &player->frames[0];
  player->current = &player->frames[1];
  player->next = &player->frames[2];
  return 1;
}

s32 HVQM4PlayerExViRetrace(HVQM4PlayerEx* player)
{
  if (!player->started) {
    return 0;
  }
  if (player->buffering) {
    return 0;
  }
  if (player->interlaced == 1) {
    if (player->field == VIGetNextField()) {
      player->elapsed += player->frameTime;
    }
  } else {
    player->elapsed += field_time;
  }
  if (player->elapsed >= player->nextTime) {
    player->nextTime += player->frameTime;
    return 1;
  }
  return 0;
}
