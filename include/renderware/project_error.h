#ifndef MKDA_RENDERWARE_PROJECT_ERROR_H
#define MKDA_RENDERWARE_PROJECT_ERROR_H

#include <dolphin/types.h>

typedef struct {
  s32 unk00;
  s32 unk04;
} BaerrState;

BaerrState* RwErrorSet(BaerrState* unk00);
s32 _rwerror(s32 arg0, ...);

#endif
