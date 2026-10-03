#ifndef _HVQM4BUF_H
#define _HVQM4BUF_H

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HVQM4BufaNode {
  struct HVQM4BufaNode* nextFree;
  struct HVQM4BufaNode* nextSend;
  u32 size;
  void* data;
} HVQM4BufaNode;

typedef struct HVQM4Bufa {
  HVQM4BufaNode* freeHead;
  HVQM4BufaNode* freeTail;
  HVQM4BufaNode* sendHead;
  HVQM4BufaNode* sendTail;
} HVQM4Bufa;

typedef struct HVQM4BufvNode {
  struct HVQM4BufvNode* nextFree;
  struct HVQM4BufvNode* prevFree;
  struct HVQM4BufvNode* nextDisp;
  u32 size;
  void* data;
  s32 refCount;
} HVQM4BufvNode;

typedef struct HVQM4Bufv {
  HVQM4BufvNode* freeHead;
  HVQM4BufvNode* dispHead;
  HVQM4BufvNode* dispTail;
} HVQM4Bufv;

s32 HVQM4BufaGetSendNums(HVQM4Bufa* buf);
HVQM4BufaNode* HVQM4BufaChkSend(HVQM4Bufa* buf);
HVQM4BufaNode* HVQM4BufaGetSend(HVQM4Bufa* buf);
void HVQM4BufaSetSend(HVQM4Bufa* buf, HVQM4BufaNode* node);
HVQM4BufaNode* HVQM4BufaGetFree(HVQM4Bufa* buf);
void HVQM4BufaSetFree(HVQM4Bufa* buf, HVQM4BufaNode* node);
s32 HVQM4BufaClose(HVQM4Bufa* buf);
HVQM4Bufa* HVQM4BufaCreate(int size, unsigned int count);

s32 HVQM4BufvGetDispNums(HVQM4Bufv* buf);
HVQM4BufvNode* HVQM4BufvChkDisp(HVQM4Bufv* buf);
HVQM4BufvNode* HVQM4BufvGetDisp(HVQM4Bufv* buf);
void HVQM4BufvSetDisp(HVQM4Bufv* buf, HVQM4BufvNode* node);
HVQM4BufvNode* HVQM4BufvGetFree(HVQM4Bufv* buf);
void HVQM4BufvSetFree(HVQM4Bufv* buf, HVQM4BufvNode* node);
s32 HVQM4BufvClose(HVQM4Bufv* buf);
HVQM4Bufv* HVQM4BufvCreate(int size, unsigned int count);

#ifdef __cplusplus
}
#endif

#endif
