#include <renderware/project_resources.h>
#include <renderware/project_state.h>
#include <renderware/project_error.h>

static s32 resourcesModule[2];

void* _rwResourcesOpen(void* unk00, s32 unk04, s32 unk08) {
  u8* unk10;
  u32 unk0C;
  s32 unk24;
  ResmemHeap8* unk28;

  resourcesModule[0] = unk04;
  unk0C = *(u32*)(RwEngineInstance + 0x150);
  unk10 = RwEngineInstance + unk04;
  do {
    if (unk0C != 0) {
      BaerrState unk1C;
      BaerrState unk14;
      *(ResmemHeap8**)(unk10 + 0x0C) =
          (*(ResmemHeap8* (**)(u32))(RwEngineInstance + 0x130))(unk0C);
      if (*(ResmemHeap8**)(unk10 + 0x0C) == 0) {
        unk14.unk00 = 1;
        unk14.unk04 = _rwerror(0x80000013, unk0C);
        RwErrorSet(&unk14);
        unk10 = 0;
        break;
      }
      unk24 = _rwResHeapInit(*(ResmemHeap8**)(unk10 + 0x0C), unk0C);
      if (unk24 == 0) {
        unk28 = *(ResmemHeap8**)(unk10 + 0x0C);
        (*(void (**)(ResmemHeap8*))(RwEngineInstance + 0x134))(unk28);
        unk1C.unk00 = 1;
        unk1C.unk04 = _rwerror(0x0C, 0);
        RwErrorSet(&unk1C);
        unk10 = 0;
        break;
      }
    } else {
      *(ResmemHeap8**)(unk10 + 0x0C) = 0;
    }
    *(u8**)(unk10 + 0x10) = unk10 + 0x10;
    *(u8**)(unk10 + 0x14) = unk10 + 0x10;
    *(u8**)(unk10 + 0x18) = unk10 + 0x18;
    *(u8**)(unk10 + 0x1C) = unk10 + 0x18;
    *(u8**)(unk10 + 0x24) = unk10 + 0x10;
    *(u8**)(unk10 + 0x20) = unk10 + 0x18;
    *(u32*)(unk10 + 0x00) = unk0C;
    *(u32*)(unk10 + 0x04) = 0;
    *(u32*)(unk10 + 0x08) = 0;
  } while (0);
  if (unk10 == 0) {
    return 0;
  }
  ++resourcesModule[1];
  return unk00;
}

void* _rwResourcesClose(void* unk00, s32 unk04, s32 unk08) {
  RwResourcesEmptyArena();
  _rwResHeapClose(*(ResmemHeap8**)(RwEngineInstance + resourcesModule[0] + 0x0C));
  (*(void (**)(ResmemHeap8*))(RwEngineInstance + 0x134))(
      *(ResmemHeap8**)(RwEngineInstance + resourcesModule[0] + 0x0C));
  *(ResmemHeap8**)(RwEngineInstance + resourcesModule[0] + 0x0C) = NULL;
  --resourcesModule[1];
  return unk00;
}

s32 RwResourcesFreeResEntry(u8* unk00) {
  void (*unk04)(u8*) = *(void (**)(u8*))(unk00 + 0x14);
  u8** unk08;
  if (unk04 != 0) {
    unk04(unk00);
  }
  unk08 = *(u8***)(unk00 + 0x10);
  if (unk08 != 0) {
    *unk08 = 0;
  }
  if (*(u8**)unk00 != 0) {
    u32 unk0C;
    *(u8**)*(u8**)(unk00 + 4) = *(u8**)unk00;
    *(u8**)(*(u8**)unk00 + 4) = *(u8**)(unk00 + 4);
    unk0C = *(u32*)(unk00 + 8);
    *(u32*)(RwEngineInstance + resourcesModule[0] + 4) -= unk0C;
    _rwResHeapFree(unk00);
  } else {
    (*(void (**)(void*))(RwEngineInstance + 0x134))(unk00);
  }
  return 1;
}

void _rwResourcesPurge(void) {
  u8* unk00 = RwEngineInstance + resourcesModule[0];
  void** unk04 = *(void***)(unk00 + 0x24);
  void** unk08 = *(void***)(unk00 + 0x20);
  void** unk0C = (void**)unk08[0];
  if (unk0C != unk08) {
    if (unk04[0] == unk04) {
      unk04[0] = unk0C;
      ((void**)unk04[0])[1] = unk04;
      unk04[1] = unk08[1];
      ((void**)unk04[1])[0] = unk04;
      unk08[0] = unk08;
      unk08[1] = unk08;
    } else {
      void** unk10 = (void**)unk04[1];
      unk10[0] = unk0C;
      unk0C[1] = unk10;
      unk10 = (void**)unk08[1];
      unk10[0] = unk04;
      unk04[1] = unk10;
      unk08[0] = unk08;
      unk08[1] = unk08;
    }
  }
  *(void***)(RwEngineInstance + resourcesModule[0] + 0x24) = unk08;
  *(void***)(RwEngineInstance + resourcesModule[0] + 0x20) = unk04;
  *(u32*)(RwEngineInstance + resourcesModule[0] + 8) = 0;
}

/* TODO: [near miss] 97.56%; two entry-save moves precede the sentinel branches. */
u8* RwResourcesAllocateResEntry(void* arg0, u8** arg1, u32 arg2, void (*arg3)(u8*)) {
  u8* unk00;
  s32 unk04 = 0;
  while (unk04 == 0) {
    unk00 = _rwResHeapAlloc(*(ResmemHeap8**)(RwEngineInstance + resourcesModule[0] + 0xC), arg2 + 0x18);
    if (unk00 != 0) {
      *(u8**)unk00 = **(u8***)(RwEngineInstance + resourcesModule[0] + 0x24);
      *(u8**)(unk00 + 4) = *(u8**)(RwEngineInstance + resourcesModule[0] + 0x24);
      *(u8**)(**(u8***)(RwEngineInstance + resourcesModule[0] + 0x24) + 4) = unk00;
      **(u8***)(RwEngineInstance + resourcesModule[0] + 0x24) = unk00;
      *(void**)(unk00 + 0xC) = arg0;
      *(u32*)(unk00 + 8) = arg2;
      *(u8***)(unk00 + 0x10) = arg1;
      *(void (**)(u8*))(unk00 + 0x14) = arg3;
      *(u32*)(RwEngineInstance + resourcesModule[0] + 4) += arg2;
      if (arg1 != 0) {
        *arg1 = unk00;
      }
      return unk00;
    }
    {
      u8* unk08 = RwEngineInstance + resourcesModule[0];
      u8* unk0C = *(u8**)(unk08 + 0x20);
      unk00 = *(u8**)(unk0C + 4);
      if (*(u8**)(unk0C + 4) != unk0C) {
        RwResourcesFreeResEntry(unk00);
      } else {
        unk0C = *(u8**)(unk08 + 0x24);
        unk00 = *(u8**)(unk0C + 4);
        if (*(u8**)(unk0C + 4) != unk0C) {
          *(u32*)(unk08 + 8) += *(u32*)(*(u8**)(unk0C + 4) + 8);
          RwResourcesFreeResEntry(unk00);
        } else {
          unk04 = 1;
        }
      }
    }
  }
  if (arg1 != 0) {
    *arg1 = 0;
  }
  {
    BaerrState unk14;
    unk14.unk00 = 1;
    unk14.unk04 = _rwerror(0xC, arg2);
    RwErrorSet(&unk14);
  }
  return 0;
}

void RwResourcesUseResEntry(u8* arg0) {
  if (*(u8**)arg0 != 0) {
    *(u8**)*(u8**)(arg0 + 4) = *(u8**)arg0;
    *(u8**)(*(u8**)arg0 + 4) = *(u8**)(arg0 + 4);
    *(u8**)arg0 = **(u8***)(RwEngineInstance + resourcesModule[0] + 0x24);
    *(u8**)(arg0 + 4) = *(u8**)(RwEngineInstance + resourcesModule[0] + 0x24);
    *(u8**)(**(u8***)(RwEngineInstance + resourcesModule[0] + 0x24) + 4) = arg0;
    **(u8***)(RwEngineInstance + resourcesModule[0] + 0x24) = arg0;
  }
}

s32 RwResourcesEmptyArena(void) {
  u8* unk00;
  u8* unk04;
  u8* unk08;
  unk00 = RwEngineInstance + resourcesModule[0];
  *(u8**)*(u8**)(unk00 + 0x14) = *(u8**)(unk00 + 0x18);
  unk00 = RwEngineInstance + resourcesModule[0];
  unk04 = *(u8**)(unk00 + 0x10);
  unk08 = unk00 + 0x18;
  while (unk04 != unk08) {
    u8* unk0C = unk04;
    unk04 = *(u8**)unk04;
    RwResourcesFreeResEntry(unk0C);
  }
  *(u8**)(RwEngineInstance + resourcesModule[0] + 0x10) =
      RwEngineInstance + resourcesModule[0] + 0x10;
  unk00 = RwEngineInstance + resourcesModule[0];
  *(u8**)(unk00 + 0x14) = unk00 + 0x10;
  *(u8**)(RwEngineInstance + resourcesModule[0] + 0x18) =
      RwEngineInstance + resourcesModule[0] + 0x18;
  unk00 = RwEngineInstance + resourcesModule[0];
  *(u8**)(unk00 + 0x1C) = unk00 + 0x18;
  *(u32*)(RwEngineInstance + resourcesModule[0] + 8) = 0;
  return 1;
}
