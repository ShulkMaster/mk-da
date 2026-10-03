#ifndef MKDA_RENDERWARE_PROJECT_RENDERSTATE_H
#define MKDA_RENDERWARE_PROJECT_RENDERSTATE_H

#include <dolphin/types.h>

typedef struct {
  u32 unk00[12];
} ProjectRenderState;

typedef struct {
  void* unk00;
  ProjectRenderState unk04;
} ProjectRenderModule;

extern s32 _rxPipelineGlobalsOffset;

ProjectRenderState* RxRenderStateVectorSetDefaultRenderStateVector(ProjectRenderState* arg0);
void RxRenderStateVectorDestroy(ProjectRenderState* arg0);
ProjectRenderState* RxRenderStateVectorLoadDriverState(ProjectRenderState* arg0);

#endif
