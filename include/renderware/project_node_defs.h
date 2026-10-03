#ifndef MKDA_RENDERWARE_PROJECT_NODE_DEFS_H
#define MKDA_RENDERWARE_PROJECT_NODE_DEFS_H

#include <dolphin/types.h>

typedef struct {
  const char* unk00;
  u32 unk04;
  u32 unk08;
  u32 unk0C;
} ProjectClusterDef;

typedef struct {
  ProjectClusterDef* unk00;
  u32 unk04;
  u32 unk08;
} ProjectNodeCluster;

typedef struct {
  char* unk00;
  u32* unk04;
  u32 unk08;
} ProjectNodeOutput;

typedef struct {
  char* unk00;
  s32 (*unk04)(u8*, u8*);
  u32 unk08;
  u32 unk0C;
  u32 unk10;
  u32 unk14;
  u32 unk18;
  u32 unk1C;
  u32 unk20;
  ProjectNodeCluster* unk24;
  u32* unk28;
  u32 unk2C;
  ProjectNodeOutput* unk30;
  u32 unk34;
  u32 unk38;
  u32 unk3C;
} ProjectNodeDef;

extern ProjectClusterDef RxClObjSpace3DVertices;
extern ProjectClusterDef RxClCamSpace3DVertices;
extern ProjectClusterDef RxClScrSpace2DVertices;
extern ProjectClusterDef RxClMeshState;
extern ProjectClusterDef RxClRenderState;
extern ProjectClusterDef RxClIndices;
extern ProjectClusterDef RxClLights;
extern ProjectClusterDef RxClVSteps;

#endif
