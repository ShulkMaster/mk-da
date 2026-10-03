/* #audit 2026-10-03T06:20Z clean-room FIXED (audit) */
#include <renderware/project_state.h>
#include <renderware/project_pipeline.h>
#include <renderware/project_renderstate.h>
#include <renderware/project_heap.h>
#include <renderware/project_error.h>
#include <renderware/project_memory.h>
#include <string.h>


s32 _rxHeapInitialSize = 0x1000;
s32 _rxPipelineMaxNodes = 0x40;

ProjectHeapState* _rxHeapGlobal;
s32 RxPipelineInstanced;

s32 _rxPipelineClose(void) {
  if (RxPipelineInstanced != 0) {
    RwFreeListDestroy(*(void**)(RwEngineInstance + _rxPipelineGlobalsOffset));
    *(void**)(RwEngineInstance + _rxPipelineGlobalsOffset) = 0;
    RxHeapDestroy(_rxHeapGlobal);
    _rxHeapGlobal = 0;
    RxPipelineInstanced = 0;
  }
  return 1;
}

s32 _rxPipelineOpen(void) {
  if (RxPipelineInstanced == 0) {
    _rxHeapGlobal = RxHeapCreate(_rxHeapInitialSize);
    if (_rxHeapGlobal == 0) {
      return 0;
    }
    *(void**)(RwEngineInstance + _rxPipelineGlobalsOffset) = RwFreeListCreate(0x34, 0x40, 4);
    if (*(void**)(RwEngineInstance + _rxPipelineGlobalsOffset) == 0) {
      RxHeapDestroy(_rxHeapGlobal);
      _rxHeapGlobal = 0;
      return 0;
    }
    *(s32*)(RwEngineInstance + _rxPipelineGlobalsOffset + 0x3C) = _rxPipelineMaxNodes;
    RxRenderStateVectorSetDefaultRenderStateVector(
        &((ProjectRenderModule*)(RwEngineInstance + _rxPipelineGlobalsOffset))->unk04);
    *(s32*)(RwEngineInstance + _rxPipelineGlobalsOffset + 0x38) = 0;
    *(s32*)(RwEngineInstance + _rxPipelineGlobalsOffset + 0x34) = 0;
    RxPipelineInstanced = 1;
    return 1;
  }
  return 0;
}

u8* PipelineNodeDestroy(u8* arg0, u8* arg1) {
  typedef struct {
    u8* unk00;
    u32 unk04;
    u32* unk08;
    u8 unk0c[0x10];
    u8* unk1c;
    void* unk20;
    u32 unk24;
  } Node28;
  if (*(s32*)arg1 == 0) {
    void (*unk00)(u8*) = *(void (**)(u8*))(*(u8**)arg0 + 0x14);
    if (unk00 != 0) {
      unk00(arg0);
    }
    --*(s32*)(*(u8**)arg0 + 0x3c);
    if (*(s32*)(*(u8**)arg0 + 0x3c) == 0) {
      void (*unk04)(u8*) = *(void (**)(u8*))(*(u8**)arg0 + 0xc);
      if (unk04 != 0) {
        unk04(*(u8**)arg0);
      }
      if (*(s32*)(*(u8**)arg0 + 0x38) != 0) {
        (*(void (**)(void*))(RwEngineInstance + 0x134))(*(u8**)arg0);
        *(u8**)arg0 = 0;
      }
    }
    if (*(void**)(arg0 + 0x20) != 0) {
      (*(void (**)(void*))(RwEngineInstance + 0x134))(*(void**)(arg0 + 0x20));
      *(void**)(arg0 + 0x20) = 0;
      *(u32*)(arg0 + 0x24) = 0;
    }
    memset(arg0, 0, 0x28);
  } else {
    s32 unk08;
    if (*(void**)(arg0 + 0x20) != 0) {
      (*(void (**)(void*))(RwEngineInstance + 0x134))(*(void**)(arg0 + 0x20));
      *(void**)(arg0 + 0x20) = 0;
      *(u32*)(arg0 + 0x24) = 0;
    }
    if (*(s32*)(*(u8**)arg0 + 0x3c) == 0 &&
        *(s32*)(*(u8**)arg0 + 0x38) != 0) {
      (*(void (**)(void*))(RwEngineInstance + 0x134))(*(u8**)arg0);
      *(u8**)arg0 = 0;
    }
    unk08 = (arg0 - *(u8**)(arg1 + 8)) / 0x28;
    if (unk08 < *(u32*)(arg1 + 4) - 1) {
      u32 unk0c;
      u8* unk10 = (u8*)(*(Node28**)(arg1 + 8) +
          *(s32*)(RwEngineInstance + _rxPipelineGlobalsOffset + 0x3c));
      u8* unk14;
      unk10 += unk08 * 0x80;
      unk14 = unk10 + 0x80;
      for (unk0c = unk08; unk0c < *(u32*)(arg1 + 4) - 1; ++unk0c) {
        memcpy(unk10, unk14, 0x80);
        unk10 = unk14;
        unk14 += 0x80;
      }
      unk10 = (u8*)(*(Node28**)(arg1 + 8) +
          *(s32*)(RwEngineInstance + _rxPipelineGlobalsOffset + 0x3c));
      unk10 += *(s32*)(RwEngineInstance + _rxPipelineGlobalsOffset + 0x3c) * 0x80;
      unk14 = unk10 + 0xc;
      for (unk0c = unk08; unk0c < *(u32*)(arg1 + 4) - 1; ++unk0c) {
        memcpy(unk10, unk14, 0xc);
        unk10 = unk14;
        unk14 += 0xc;
      }
      for (unk0c = unk08; unk0c < *(u32*)(arg1 + 4) - 1; ++unk0c) {
        u8* unk24 = *(u8**)(arg1 + 8);
        u32 unk18 = unk0c * 0x28;
        memcpy(unk24 + unk18, unk24 + (unk0c + 1) * 0x28, 0x28);
        (*(Node28**)(arg1 + 8))[unk0c].unk08 -= 0x20;
        (*(Node28**)(arg1 + 8))[unk0c].unk1c -= 0xc;
      }
      for (unk0c = 0; unk0c < *(u32*)(arg1 + 4) - 1; ++unk0c) {
        u32 unk1c;
        for (unk1c = 0; unk1c < (*(Node28**)(arg1 + 8))[unk0c].unk04; ++unk1c) {
          u32 unk20 = (*(Node28**)(arg1 + 8))[unk0c].unk08[unk1c];
          if (unk20 >= unk08) {
            if (unk08 == unk20) {
              (*(Node28**)(arg1 + 8))[unk0c].unk08[unk1c] = -1;
            } else {
              (*(Node28**)(arg1 + 8))[unk0c].unk08[unk1c] = unk20 - 1;
            }
          }
        }
      }
    }
  }
  --*(u32*)(arg1 + 4);
  return arg0;
}

ProjectHeapState* RxHeapGetGlobalHeap(void) {
  return _rxHeapGlobal;
}

u8* RxPacketCreate(const u8* unk00) {
  u8* unk04;
  *(s32*)(*(u8**)_rxExecCtxGlobal + 0x10) = 2;
  unk04 = *(u8**)(*(u8**)_rxExecCtxGlobal + 0x14);
  *(u32*)(unk04 + 8) = *(const u32*)(unk00 + 0x18);
  *(u32*)(unk04 + 0x0C) = *(const u32*)(unk00 + 0x10);
  *(u32*)(unk04 + 0x10) = *(const u32*)(unk00 + 0x0C);
  return unk04;
}

u8* RxClusterSetExternalData(u8* unk00, void* unk04, u32 unk08, u32 unk0C) {
  void* unk10 = *(void**)(unk00 + 0x04);
  if (unk10 != NULL && (*(u16*)unk00 & 2) == 0 && unk10 != unk04) {
    RxHeapFree(_rxHeapGlobal, unk10);
    *(void**)(unk00 + 0x04) = NULL;
  }
  *(void**)(unk00 + 0x04) = unk04;
  *(void**)(unk00 + 0x08) = unk04;
  *(u16*)(unk00 + 0x02) = unk08;
  *(u16*)unk00 |= 3;
  *(u32*)(unk00 + 0x0C) = unk0C;
  *(u32*)(unk00 + 0x10) = unk0C;
  return unk00;
}

u8* RxClusterSetData(u8* arg0, void* arg1, u16 arg2, u32 arg3) {
  void* unk00 = *(void**)(arg0 + 4);
  if (unk00 != 0 && (*(u16*)arg0 & 2) == 0 && unk00 != arg1) {
    RxHeapFree(_rxHeapGlobal, unk00);
    *(void**)(arg0 + 4) = 0;
  }
  *(void**)(arg0 + 4) = arg1;
  *(void**)(arg0 + 8) = arg1;
  *(u16*)(arg0 + 2) = arg2;
  *(u16*)arg0 |= 1;
  *(u32*)(arg0 + 0xC) = arg3;
  *(u32*)(arg0 + 0x10) = arg3;
  return arg0;
}

u8* RxClusterInitializeData(u8* arg0, u32 arg1, u16 arg2) {
  u32 unk00 = arg1 * arg2;
  void* unk04 = *(void**)(arg0 + 4);
  if (unk04 != 0 && (*(u16*)arg0 & 2) == 0) {
    RxHeapFree(_rxHeapGlobal, unk04);
  }
  unk04 = RxHeapAlloc(_rxHeapGlobal, unk00);
  *(void**)(arg0 + 4) = unk04;
  *(void**)(arg0 + 8) = unk04;
  *(u32*)(arg0 + 0xC) = arg1;
  *(u32*)(arg0 + 0x10) = 0;
  *(u16*)arg0 |= 1;
  *(u16*)(arg0 + 2) = arg2;
  return arg0;
}

u8* RxClusterResizeData(u8* unk00, u32 unk04) {
  u32 unk0C = unk04 * *(u16*)(unk00 + 0x02);
  void* unk08 = RxHeapRealloc(_rxHeapGlobal, *(void**)(unk00 + 0x04), unk0C, 1);
  *(void**)(unk00 + 0x04) = unk08;
  *(void**)(unk00 + 0x08) = unk08;
  *(u16*)unk00 |= 1;
  if (*(u32*)(unk00 + 0x10) > unk04) {
    *(u32*)(unk00 + 0x10) = unk04;
  }
  *(u32*)(unk00 + 0x0C) = unk04;
  return unk00;
}

u8* RxClusterLockWrite(u8* arg0, u32 arg1, u8* arg2) {
  u32* unk00 = *(u32**)(arg0 + 8);
  u32 unk04 = unk00[arg1];
  if (unk04 != 0xFFFFFFFF) {
    typedef struct {
      u8 unk00[0x1C];
    } Cluster1C;
    typedef struct {
      u8 unk00[0x14];
      Cluster1C unk14[];
    } Packet14;
    u8* unk08 = (u8*)&((Packet14*)arg0)->unk14[unk04];
    if ((*(u8**)(unk08 + 0x14)) == 0) {
      u8* unk0C = (*(u8***)(arg0 + 0x10))[unk04];
      (*(u8**)(unk08 + 0x14)) = unk0C;
      (*(u32*)(unk08 + 0x18)) = *(u32*)(unk0C + 4);
    }
    (*(u16*)(unk08 + 0x0)) |= 8;
    if (((*(u16*)(unk08 + 0x0)) & 6) == 2U) {
      void* unk10 = (*(void**)(unk08 + 0x4));
      u32 unk14 = (*(u32*)(unk08 + 0xC));
      u32 unk18 = (*(u32*)(unk08 + 0x10));
      (*(u16*)(unk08 + 0x0)) &= ~2;
      (*(void**)(unk08 + 0x4)) = 0;
      if (RxClusterInitializeData(unk08, unk14, (*(u16*)(unk08 + 0x2))) == 0) {
        (*(u16*)(unk08 + 0x0)) |= 2;
        (*(void**)(unk08 + 0x4)) = unk10;
        return 0;
      }
      (*(u32*)(unk08 + 0x10)) = unk18;
      memcpy((*(void**)(unk08 + 0x4)), unk10, unk18 * (*(u16*)(unk08 + 0x2)));
    }
    (*(void**)(unk08 + 0x8)) = (*(void**)(unk08 + 0x4));
    return unk08;
  }
  return 0;
}

u8* RxPipelineExecute(u8* unk00, void* unk04, s32 unk08) {
  u8* unk0C;
  void** unk10;
  u8* unk14;
  u32 unk18;
  u8* unk1C;
  if (unk08 != 0) {
    _rxHeapGlobal->unk18 == 0 ? 1 : _rxHeapReset(_rxHeapGlobal);
  }
  *(s32*)(_rxExecCtxGlobal + 8) = 1;
  unk0C = _rxExecCtxGlobal;
  *(u8**)unk0C = unk00;
  unk10 = (void**)(unk0C + 0x10);
  *unk10 = unk04;
  *(ProjectHeapState**)(unk0C + 0x14) = _rxHeapGlobal;
  *(s32*)(unk00 + 0x10) = 0;
  unk14 = *(u8**)(unk00 + 8);
  unk1C = *(u8**)unk14;
  unk18 = (*(u32 (**)(u8*, void**))(unk1C + 4))(unk14, unk10);
  if (unk18 == 0) {
    *(u32*)(unk0C + 8) = unk18;
  }
  if (*(s32*)(unk00 + 0x10) > 1) {
    *(s32*)(unk00 + 0x10) = 2;
    _rxPacketDestroy(*(u8**)(unk00 + 0x14));
  }
  *(u8**)_rxExecCtxGlobal = 0;
  *unk10 = 0;
  *(ProjectHeapState**)(unk0C + 0x14) = 0;
  if (*(s32*)(unk0C + 8) != 0) {
    return unk00;
  }
  return 0;
}

u8* RxPipelineCreate(void) {
  u8* unk00 = (*(u8* (**)(void*))(RwEngineInstance + 0x140))(
      *(void**)(RwEngineInstance + _rxPipelineGlobalsOffset));
  if (unk00 != 0) {
    memset(unk00, 0, 0x34);
    *(s32*)unk00 = 0;
    return unk00;
  }
  {
    BaerrState unk04;
    unk04.unk00 = 1;
    unk04.unk04 = _rwerror(0x80000013, 0x34);
    RwErrorSet(&unk04);
  }
  return 0;
}

void _rxPipelineDestroy(u8* arg0) {
  if (arg0 != NULL) {
    u32 limit;
    u32 index;
    u8* node;
    node = *(u8**)(arg0 + 8);
    limit = *(u32*)(arg0 + 4);
    index = 0;
    while (index < limit) {
      PipelineNodeDestroy(node, arg0);
      node += 0x28;
      ++index;
    }
    *(void**)(arg0 + 8) = NULL;
    if (*(void**)(arg0 + 0x20) != NULL) {
      (*(void (**)(void*))(RwEngineInstance + 0x134))(*(void**)(arg0 + 0x20));
      *(void**)(arg0 + 0x20) = NULL;
      *(u32*)(arg0 + 0x24) = 0;
    }
    (*(void* (**)(void*, void*))(RwEngineInstance + 0x144))(
        *(void**)(RwEngineInstance + _rxPipelineGlobalsOffset), arg0);
  }
}
