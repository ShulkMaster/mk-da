/* #audit 2026-10-03T04:06Z clean-room FIXED (audit) */
#include "renderware/project_state.h"
#include <stdio.h>

void *RwOsGetFileInterface(void)
{
  return RwEngineInstance + 0xC4;
}

static s32 rwfexist(const char *unk0)
{
  FILE *unk4;
  s32 unk8;

  unk4 = (*(FILE *(**)(const char *, const char *))(RwEngineInstance + 0xC8))(unk0, "rb");
  unk8 = unk4 != NULL;
  if (unk4 != NULL) {
    (*(int (**)(FILE *))(RwEngineInstance + 0xCC))(unk4);
  }
  return unk8;
}

s32 _rwFileSystemOpen(void) {
  *(s32 (**)(const char*))(RwEngineInstance + 0xC4) = rwfexist;
  *(FILE* (**)(const char*, const char*))(RwEngineInstance + 0xC8) = fopen;
  *(int (**)(FILE*))(RwEngineInstance + 0xCC) = fclose;
  *(size_t (**)(const void*, size_t, size_t, FILE*))(RwEngineInstance + 0xD0) = fread;
  *(size_t (**)(const void*, size_t, size_t, FILE*))(RwEngineInstance + 0xD4) = fwrite;
  *(char* (**)(char*, int, FILE*))(RwEngineInstance + 0xD8) = fgets;
  *(int (**)(const char*, FILE*))(RwEngineInstance + 0xDC) = fputs;
  *(int (**)(FILE*))(RwEngineInstance + 0xE0) = feof;
  *(int (**)(FILE*, long, int))(RwEngineInstance + 0xE4) = fseek;
  *(int (**)(FILE*))(RwEngineInstance + 0xE8) = fflush;
  *(long (**)(FILE*))(RwEngineInstance + 0xEC) = ftell;
  return 1;
}

void _rwFileSystemClose(void) {}
