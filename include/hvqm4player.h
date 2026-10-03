#ifndef _HVQM4PLAYER_H
#define _HVQM4PLAYER_H

#include "hvqm4buf.h"

typedef struct HVQM4PlayerEx HVQM4PlayerEx;
typedef void (*HVQM4PlayerExFileCallback)(HVQM4PlayerEx*, s32, s32);

s32 HVQM4PlayerExControlExit(HVQM4PlayerEx* player);
s32 HVQM4PlayerExControlCheckNormal(HVQM4PlayerEx* player);
HVQM4BufaNode* HVQM4PlayerExBufaGetSend(HVQM4PlayerEx* player);
void HVQM4PlayerExBufaSetFree(HVQM4PlayerEx* player, HVQM4BufaNode* node);
u32 HVQM4PlayerExBufaGetSize(HVQM4BufaNode* node);
void* HVQM4PlayerExBufaGetBuff(HVQM4BufaNode* node);
HVQM4BufvNode* HVQM4PlayerExBufvGetDisp(HVQM4PlayerEx* player);
void HVQM4PlayerExBufvSetBusyFlag(HVQM4BufvNode* node, s32 busy);
void* HVQM4PlayerExBufvGetBuff(HVQM4BufvNode* node);
s32 HVQM4PlayerExGetHeight(HVQM4PlayerEx* player);
s32 HVQM4PlayerExGetWidth(HVQM4PlayerEx* player);
s32 HVQM4PlayerExCheckExecute(HVQM4PlayerEx* player);
s32 HVQM4PlayerExStart(HVQM4PlayerEx* player);
s32 HVQM4PlayerExClose(HVQM4PlayerEx* player, s32* error);
HVQM4PlayerEx* HVQM4PlayerExCreate(const char* filename, s32 buffers, s32 priority,
                                 u32 stackSize, s32 field,
                                 HVQM4PlayerExFileCallback callback,
                                 s32 callbackArgument, s32* error);
s32 HVQM4PlayerExViRetrace(HVQM4PlayerEx* player);

#endif
