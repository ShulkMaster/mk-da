#ifndef MKDA_SEQFILE_H
#define MKDA_SEQFILE_H

#include <dolphin/dvd.h>
#include <hvqm4player.h>

typedef struct SeqFile {
  DVDFileInfo unk00;
  void* unk3C;
  u8* unk40;
  u8* unk44;
  u8* unk48;
  volatile s32 unk4C;
  u32 unk50;
  volatile u32 unk54;
  volatile u32 unk58;
  u8* unk5C;
  volatile u32 unk60;
  volatile s32 unk64;
  s32 unk68;
  HVQM4PlayerExFileCallback unk6C;
  HVQM4PlayerEx* unk70;
  s32 unk74;
} SeqFile;

#ifdef __cplusplus
extern "C" {
#endif

s32 SeqFileGetDvdStatus(SeqFile* file);
s32 SeqFileSeek(SeqFile* file, u32 pos);
s32 SeqFileRead(void* arg0, u32 count, SeqFile* file);

#ifdef __cplusplus
}
#endif

#endif
