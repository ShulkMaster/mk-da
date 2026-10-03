#include <dolphin/dvd.h>
#include <dolphin/os.h>
#include "movie/project_alloc.h"
#include <seqfile.h>

static s32 shutdown_request;
static void callback(s32 result, DVDFileInfo* fileInfo);

static inline s32 seqfile_window_full(const SeqFile* file) {
  u32 windowEnd = file->unk54;
  windowEnd /= 0x18000;
  windowEnd *= 0x18000;
  windowEnd += 0x210000;
  return file->unk60 >= windowEnd;
}

static inline u32 seqfile_next_read_length(u32 length, u32 end) {
  length -= end;
  if (length > 0x18000) {
    length = 0x18000;
  } else {
    length += 0x1f;
    length &= ~0x1f;
  }
  return length;
}

static inline s32 seqfile_prefetch(SeqFile* file) {
  s32 result = 0;
  if (file->unk68 != 0) {
    file->unk64 = 1;
  } else if (file->unk60 >= file->unk50) {
    file->unk64 = 2;
  } else if (seqfile_window_full(file)) {
    file->unk64 = 1;
  } else {
    u32 length = seqfile_next_read_length(file->unk50, file->unk60);
    {
      u32 offset = file->unk60;
      offset %= 0x210000;
      file->unk5C = file->unk40 + offset;
    }
    file->unk58 = file->unk60 + length;
    file->unk64 = 0;
    if (!DVDReadAsyncPrio(&file->unk00, file->unk5C, length, file->unk60, callback, 2)) {
      file->unk64 = -1;
      result = 0;
    } else {
      result = 1;
    }
  }
  return result;
}

s32 SeqFileGetDvdStatus(SeqFile* file) {
  return DVDGetCommandBlockStatus((DVDCommandBlock*)file);
}

u32 SeqFileGetPos(SeqFile* file) {
  return *(u32*)((u8*)file + 0x54);
}

s32 SeqFileSeek(SeqFile* file, u32 pos) {
  u32 start;
  if (file->unk64 == -1) {
    return 0;
  }
  if (pos == file->unk54) {
    return 1;
  }
  if (file->unk60 > pos && pos >= file->unk54) {
    s32 count = pos - file->unk54;
    return SeqFileRead(0, count, file) == count;
  }
  file->unk68 = 1;
  while (file->unk64 == 0) {
    s32 status = DVDGetCommandBlockStatus(&file->unk00.cb);
    if (status != file->unk4C) {
      file->unk4C = status;
      if (file->unk6C != 0) {
        file->unk6C(file->unk70, file->unk74, status);
      }
    }
  }
  file->unk68 = 0;
  start = (pos / 0x18000) * 0x18000;
  file->unk44 = file->unk40 + start % 0x210000;
  file->unk54 = start;
  file->unk58 = 0;
  file->unk60 = start;
  if (file->unk64 == 2) {
    file->unk64 = 1;
  }
  pos -= start;
  if ((s32)pos != SeqFileRead(0, pos, file)) {
    return 0;
  }
  return 1;
}

s32 SeqFileRead(void* arg0, u32 count, SeqFile* file) {
  u8* destination = arg0;
  s32 total = 0;
  s32 amount;
  while (total < (s32)count) {
    amount = file->unk60 - file->unk54;
    if (amount > (s32)count - total) {
      amount = (s32)count - total;
    }
    {
      s32 status = DVDGetCommandBlockStatus(&file->unk00.cb);
      if (status != file->unk4C) {
        file->unk4C = status;
        if (file->unk6C != 0) {
          file->unk6C(file->unk70, file->unk74, status);
        }
      }
    }
    if (file->unk64 == 1) {
      seqfile_prefetch(file);
    }
    if (file->unk64 == -1) {
      return -1;
    }
    if (shutdown_request) {
      DVDCancel(&file->unk00.cb);
      amount = 0;
    }
    if (amount > 0) {
      if (destination == 0) {
        u8* cursor = file->unk44;
        u8* base;
        u8* limit = file->unk48;
        cursor += amount;
        base = file->unk40;
        if (cursor >= limit) {
          cursor = base + (cursor - limit);
        }
        file->unk44 = cursor;
      } else {
        u8* cursor;
        u8* base;
        u8* limit;
        u8* end = destination + amount;
        cursor = file->unk44;
        base = file->unk40;
        limit = file->unk48;
        while (destination < end) {
          u8 value = *cursor++;
          *destination++ = value;
          if (cursor == limit) {
            cursor = base;
          }
        }
        file->unk44 = cursor;
      }
      total += amount;
      file->unk54 += amount;
    }
  }
  if (file->unk64 == 1) {
    seqfile_prefetch(file);
  }
  if (file->unk64 == -1) {
    return -1;
  }
  {
    s32 status = DVDGetCommandBlockStatus(&file->unk00.cb);
    if (status != file->unk4C) {
      file->unk4C = status;
      if (file->unk6C != 0) {
        file->unk6C(file->unk70, file->unk74, status);
      }
    }
  }
  return total;
}

s32 SeqFileCheckBusy(SeqFile* file) {
  return *(s32*)((u8*)file + 0x64) == 0;
}

s32 SeqFileClose(SeqFile *file)
{
  s32 status = DVDGetCommandBlockStatus(&file->unk00.cb);
  if (status != file->unk4C) {
    file->unk4C = status;
    if (file->unk6C != NULL) {
      file->unk6C(file->unk70, file->unk74, status);
    }
  }
  while (file->unk64 == 0) {
    status = DVDGetCommandBlockStatus(&file->unk00.cb);
    if (status != file->unk4C) {
      file->unk4C = status;
      if (file->unk6C != NULL) {
        file->unk6C(file->unk70, file->unk74, status);
      }
    }
  }
  DVDClose(&file->unk00);
  _mwMemFree(file->unk3C, "seqfile.c", 0x152);
  _mwMemFree(file, "seqfile.c", 0x153);
  return !shutdown_request;
}

SeqFile* SeqFileOpen(const char* filename, HVQM4PlayerExFileCallback callbackArg,
                     HVQM4PlayerEx* player, s32 argument) {
  SeqFile* file;
  u32* p;
  u32* end;
  s32 status;
  if (shutdown_request != 0) {
    return 0;
  }
  file = _mwMemMalloc(gHeap, 0x78, 5, "seqfile movie", "seqfile.c", 0x109);
  if (file == 0) {
    return 0;
  }
  file->unk3C = _mwMemMalloc(gHeap, 0x210080, 6, "readbuffSys movie", "seqfile.c", 0x10f);
  if (file->unk3C == 0) {
    _mwMemFree(file, "seqfile.c", 0x112);
    return 0;
  }
  file->unk40 = (u8*)(((u32)file->unk3C + 0x40) & ~0x3f);
  end = (u32*)(file->unk40 + 0x210000);
  for (p = (u32*)file->unk40; p < end; p++) {
    *p = 0;
  }
  DCFlushRange(file->unk40, 0x210000);
  if (!DVDOpen((char*)filename, &file->unk00)) {
    _mwMemFree(file->unk3C, "seqfile.c", 0x125);
    _mwMemFree(file, "seqfile.c", 0x126);
    return 0;
  }
  file->unk44 = file->unk40;
  file->unk48 = file->unk40 + 0x210000;
  file->unk4C = 0;
  file->unk50 = file->unk00.length;
  file->unk54 = 0;
  file->unk58 = 0;
  file->unk60 = 0;
  file->unk64 = 1;
  file->unk68 = 0;
  file->unk50 = file->unk00.length;
  file->unk6C = callbackArg;
  file->unk70 = player;
  file->unk74 = argument;
  if (!seqfile_prefetch(file)) {
    _mwMemFree(file->unk3C, "seqfile.c", 0x13d);
    _mwMemFree(file, "seqfile.c", 0x13e);
    return 0;
  }
  status = DVDGetCommandBlockStatus(&file->unk00.cb);
  if (status != file->unk4C) {
    file->unk4C = status;
    if (file->unk6C != 0) {
      file->unk6C(file->unk70, file->unk74, status);
    }
  }
  return file;
}

/* TODO: [breakthrough needed] 98.01%; completion-field reloads and prefetch scheduling remain. */
static void callback(s32 result, DVDFileInfo* fileInfo) {
  SeqFile* file = (SeqFile*)fileInfo;
  {
    s32 status = SeqFileGetDvdStatus(file);
    if (status != file->unk4C) {
      file->unk4C = status;
      if (file->unk6C != 0) {
        file->unk6C(file->unk70, file->unk74, status);
      }
    }
  }
  if (file->unk64 != 0) {
    return;
  }
  if (result < 0) {
    file->unk64 = -1;
    return;
  }
  if ((u32)(file->unk58 - file->unk60) != (u32)result) {
    return;
  }
  if (shutdown_request != 0) {
    file->unk64 = -1;
    return;
  }
  file->unk60 = file->unk58;
  if (file->unk68 != 0) {
    file->unk64 = 1;
  } else if (file->unk60 >= file->unk50) {
    file->unk64 = 2;
  } else if (file->unk60 >= (file->unk54 / 0x18000) * 0x18000 + 0x210000) {
    file->unk64 = 1;
  } else {
    u32 length = file->unk50;
    length -= file->unk60;
    if (length > 0x18000) {
      length = 0x18000;
    } else {
      length += 0x1F;
      length &= ~0x1FU;
    }
    {
      u32 offset = file->unk60;
      offset %= 0x210000;
      file->unk5C = file->unk40 + offset;
    }
    file->unk58 = file->unk60 + length;
    file->unk64 = 0;
    if (!DVDReadAsyncPrio(&file->unk00, file->unk5C, length, file->unk60, callback, 2)) {
      file->unk64 = -1;
    }
  }
}
