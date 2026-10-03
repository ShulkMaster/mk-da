/* #audit 2026-10-03T04:06Z clean-room FIXED (audit) */
#include <dolphin/types.h>
#include <renderware/project_state.h>
#include <stdio.h>
#include <string.h>

static s32 StrICmp(const s8* unk00, const s8* unk04) {
  s8 unk08;
  s8 unk0c;
  if (unk00 != 0 && unk04 != 0) {
    do {
      unk08 = *unk00;
      unk0c = *unk04;
      if (unk08 >= 0x41 && unk08 <= 0x5a) {
        unk08 += 0x20;
      }
      if (unk0c >= 0x41 && unk0c <= 0x5a) {
        unk0c += 0x20;
      }
      if (unk08 != unk0c) {
        return unk08 - unk0c;
      }
      unk00++;
      unk04++;
    } while (unk08 != 0 && unk0c != 0);
    if (unk08 != unk0c) {
      return unk08 - unk0c;
    }
  }
  return 0;
}

static char* StrUpr(char* unk00) {
  if (unk00 != 0) {
    char unk08;
    char* unk04 = unk00;
    while ((unk08 = *unk04) != 0) {
      if (unk08 >= 'a' && unk08 <= 'z') {
        *unk04 = unk08 - 0x20;
      }
      ++unk04;
    }
  }
  return unk00;
}

static char* StrLwr(char* arg0) {
  char unk08;
  char* unk04;
  if (arg0 != 0) {
    unk04 = arg0;
    while ((unk08 = *unk04) != 0) {
      if (unk08 >= 'A' && unk08 <= 'Z') {
        *unk04 = unk08 + 0x20;
      }
      unk04++;
    }
  }
  return arg0;
}

static char* StrChr(const char* unk00, int unk04) {
  char* unk0C;
  char unk08 = unk04;
  char unk10;
  unk0C = 0;
  do {
    unk10 = *unk00;
    if (unk10 == unk08) {
      unk0C = (char*)unk00;
      break;
    }
    ++unk00;
  } while (unk10 != 0);
  return unk0C;
}

static char* StrRChr(const char* unk00, s32 unk04) {
  char* unk0C;
  char unk08 = unk04;
  char unk10;
  unk0C = 0;
  do {
    unk10 = *unk00;
    if (unk10 == unk08) {
      unk0C = (char*)unk00;
    }
    ++unk00;
  } while (unk10 != 0);
  return unk0C;
}

s32 _rwStringOpen(void) {
  *(u32*)(RwEngineInstance + 0xF0) = (u32)sprintf;
  *(u32*)(RwEngineInstance + 0xF4) = (u32)vsprintf;
  *(u32*)(RwEngineInstance + 0xF8) = (u32)strcpy;
  *(u32*)(RwEngineInstance + 0xFC) = (u32)strncpy;
  *(u32*)(RwEngineInstance + 0x100) = (u32)strcat;
  *(u32*)(RwEngineInstance + 0x104) = (u32)strncat;
  *(u32*)(RwEngineInstance + 0x108) = (u32)StrRChr;
  *(u32*)(RwEngineInstance + 0x10C) = (u32)StrChr;
  *(u32*)(RwEngineInstance + 0x110) = (u32)strstr;
  *(u32*)(RwEngineInstance + 0x114) = (u32)strcmp;
  *(u32*)(RwEngineInstance + 0x118) = (u32)StrICmp;
  *(u32*)(RwEngineInstance + 0x11C) = (u32)strlen;
  *(u32*)(RwEngineInstance + 0x120) = (u32)StrUpr;
  *(u32*)(RwEngineInstance + 0x124) = (u32)StrLwr;
  *(u32*)(RwEngineInstance + 0x128) = (u32)strtok;
  *(u32*)(RwEngineInstance + 0x12C) = (u32)sscanf;
  return 1;
}

void _rwStringClose(void) {}
