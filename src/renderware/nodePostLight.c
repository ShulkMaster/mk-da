#include <renderware/project_pipeline.h>
#include <renderware/project_node_defs.h>

static inline u8* project_read_cluster(u8* packet, u32 slot) {
  s32 index = (*(s32**)(packet + 8))[slot];
  if (index == -1) {
    return NULL;
  }
  *(void**)(packet + index * 0x1C + 0x1C) =
      *(void**)(packet + index * 0x1C + 0x18);
  return packet + (*(s32**)(packet + 8))[slot] * 0x1C + 0x14;
}

static inline void project_limit_word(f32* word, const f32* limit) {
  if (*(u32*)word > *(const u32*)limit) {
    *(u32*)word = *(const u32*)limit & ~(*(s32*)word >> 31);
  }
}

/* TODO: [near miss] 96.76%; packet and unscaled-loop registers plus clamp reloads remain. */
static s32 _PostLightFn(u8* node, u8* context) {
  static const f32 maxlum = 255.0f;
  u8* packet;
  u8* mesh;
  u8* cluster0;
  u8* cluster1;
  u8* root;
  {
    if (*(s32*)(*(u8**)_rxExecCtxGlobal + 0x10) == 3) {
      *(s32*)(*(u8**)_rxExecCtxGlobal + 0x10) = 2;
      packet = *(u8**)(*(u8**)_rxExecCtxGlobal + 0x14);
    } else {
      packet = NULL;
    }
  }
  mesh = *(u8**)(project_read_cluster(packet, 0) + 8);
  if (*(s32*)(mesh + 0xB0) == 0) {
    u32 packed;
    u8 one = 1;
    f32* values;
    u8* output;
    u8* steps;
    s32 count;
    s32 stepAdvance;
    u32 skip;
    packed = *(u32*)(mesh + 0x98);
    cluster0 = RxClusterLockWrite(packet, 1, node);
    cluster1 = RxClusterLockWrite(packet, 2, node);
    {
      u8* stepCluster = project_read_cluster(packet, 3);
      u32 elementCount;
      if (stepCluster != NULL && (elementCount = *(u32*)(stepCluster + 0x10)) != 0) {
        steps = *(u8**)(stepCluster + 4);
        stepAdvance = 1;
        count = elementCount;
        skip = *steps;
        while (skip--) {
          *(u8**)(cluster0 + 8) += *(u16*)(cluster0 + 2);
          *(u8**)(cluster1 + 8) += *(u16*)(cluster1 + 2);
        }
        ++steps;
      } else {
        steps = &one;
        stepAdvance = 0;
        count = *(u32*)(mesh + 0xA8);
      }
    }
    if ((*(u32*)mesh & 0x40) == 0 || packed == 0xFFFFFFFFU) {
      while (count--) {
        f32* unscaled = (f32*)(*(u8**)(cluster0 + 8) + 0x10);
        output = *(u8**)(cluster1 + 8);
        project_limit_word(&unscaled[0], &maxlum);
        project_limit_word(&unscaled[1], &maxlum);
        project_limit_word(&unscaled[2], &maxlum);
        output[0xC] = (u8)unscaled[0];
        output[0xD] = (u8)unscaled[1];
        output[0xE] = (u8)unscaled[2];
        output[0xF] = (u8)unscaled[3];
        skip = *steps;
        steps += stepAdvance;
        while (skip--) {
          *(u8**)(cluster0 + 8) += *(u16*)(cluster0 + 2);
          *(u8**)(cluster1 + 8) += *(u16*)(cluster1 + 2);
        }
      }
    } else {
      f32 factor0 = (1.0f / 255.0f) * ((u8*)&packed)[0];
      f32 factor1 = (1.0f / 255.0f) * ((u8*)&packed)[1];
      f32 factor2 = (1.0f / 255.0f) * ((u8*)&packed)[2];
      f32 factor3 = (1.0f / 255.0f) * ((u8*)&packed)[3];
      while (count--) {
        values = (f32*)(*(u8**)(cluster0 + 8) + 0x10);
        output = *(u8**)(cluster1 + 8);
        values[0] *= factor0;
        values[1] *= factor1;
        values[2] *= factor2;
        values[3] *= factor3;
        project_limit_word(&values[0], &maxlum);
        project_limit_word(&values[1], &maxlum);
        project_limit_word(&values[2], &maxlum);
        project_limit_word(&values[3], &maxlum);
        output[0xC] = (u8)values[0];
        output[0xD] = (u8)values[1];
        output[0xE] = (u8)values[2];
        output[0xF] = (u8)values[3];
        skip = *steps;
        steps += stepAdvance;
        while (skip--) {
          *(u8**)(cluster0 + 8) += *(u16*)(cluster0 + 2);
          *(u8**)(cluster1 + 8) += *(u16*)(cluster1 + 2);
        }
      }
    }
  }
  root = *(u8**)_rxExecCtxGlobal;
  if (*(s32*)(_rxExecCtxGlobal + 8) != 0) {
    u8* next = _rxEmbeddedPacketBetweenNodes(root, node, 0);
    if (next != NULL) {
      u32 result = (*(ProjectNodeDef**)next)->unk04(next, _rxExecCtxGlobal + 0x10);
      if (result == 0) {
        *(u32*)(_rxExecCtxGlobal + 8) = result;
      }
    }
  }
  if (*(s32*)(root + 0x10) > 1) {
    *(s32*)(root + 0x10) = 2;
    _rxPacketDestroy(*(u8**)(root + 0x14));
  }
  return 1;
}

ProjectNodeDef* RxNodeDefinitionGetPostLight(void) {
  static ProjectNodeCluster PostLightCLI[4] = {
    {&RxClMeshState, 0, 0},
    {&RxClCamSpace3DVertices, 0, 0},
    {&RxClScrSpace2DVertices, 0, 0},
    {&RxClVSteps, 0, 0}
  };
  static u32 PostLightInputVR[4] = {1, 1, 1, 2};
  static u32 PostLightOutputV[4] = {0, 1, 1, 0};
  static char _Output[] = "Output";
  static ProjectNodeOutput PostLightOutput[1] = {
    {_Output, PostLightOutputV, 0}
  };
  static char _PostLight_csl[] = "PostLight.csl";
  static ProjectNodeDef nodePostLightCSL = {
    _PostLight_csl, _PostLightFn,
    0, 0, 0, 0, 0, 0,
    4, PostLightCLI, PostLightInputVR, 1, PostLightOutput,
    0, 0, 0
  };
  return &nodePostLightCSL;
}
