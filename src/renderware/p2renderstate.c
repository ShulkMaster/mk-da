#include <renderware/project_renderstate.h>
#include <renderware/project_state.h>
#include <renderware/project_error.h>

ProjectRenderState* RxRenderStateVectorSetDefaultRenderStateVector(ProjectRenderState* arg0) {
  if (arg0 != 0) {
    if (*(s32*)(RwEngineInstance + 0x14C) == 3) {
      *arg0 = ((ProjectRenderModule*)(RwEngineInstance + _rxPipelineGlobalsOffset))->unk04;
    } else {
      if (arg0 != &((ProjectRenderModule*)(RwEngineInstance + _rxPipelineGlobalsOffset))->unk04) {
        BaerrState unk00;
        unk00.unk00 = 1;
        unk00.unk04 = _rwerror(0x80000018);
        RwErrorSet(&unk00);
        return 0;
      }
      {
        struct {
          u32 unk00;
        } unk00 = {0xFFFFFFFF};
        arg0->unk00[0] = 7;
        arg0->unk00[1] = 2;
        arg0->unk00[2] = 5;
        arg0->unk00[3] = 6;
        arg0->unk00[4] = 0;
        arg0->unk00[5] = 1;
        arg0->unk00[6] = 1;
        arg0->unk00[7] = 2;
        arg0->unk00[8] = unk00.unk00;
        arg0->unk00[9] = 0;
        arg0->unk00[10] = unk00.unk00;
        arg0->unk00[11] = 0;
      }
    }
    return arg0;
  } else {
    BaerrState unk04;
    unk04.unk00 = 1;
    unk04.unk04 = _rwerror(0x80000016);
    RwErrorSet(&unk04);
    return 0;
  }
}

ProjectRenderState* RxRenderStateVectorCreate(s32 arg0) {
  if (*(s32*)(RwEngineInstance + 0x14C) == 3) {
    ProjectRenderState* unk00 = (*(ProjectRenderState* (**)(u32))(RwEngineInstance + 0x130))(0x30);
    if (unk00 != 0) {
      if (arg0 != 0) {
        if (RxRenderStateVectorLoadDriverState(unk00) == 0) {
          RxRenderStateVectorDestroy(unk00);
        }
      } else {
        *unk00 = ((ProjectRenderModule*)(RwEngineInstance + _rxPipelineGlobalsOffset))->unk04;
      }
      return unk00;
    } else {
      BaerrState unk04;
      unk04.unk00 = 1;
      unk04.unk04 = _rwerror(0x80000013, 0x30);
      RwErrorSet(&unk04);
      return 0;
    }
  } else {
    BaerrState unk08;
    unk08.unk00 = 1;
    unk08.unk04 = _rwerror(0x80000018);
    RwErrorSet(&unk08);
    return 0;
  }
}

void RxRenderStateVectorDestroy(ProjectRenderState* arg0) {
  if (arg0 != 0) {
    (*(void (**)(void*))(RwEngineInstance + 0x134))(arg0);
  } else {
    BaerrState unk00;
    unk00.unk00 = 1;
    unk00.unk04 = _rwerror(0x80000016);
    RwErrorSet(&unk00);
  }
}

ProjectRenderState* RxRenderStateVectorLoadDriverState(ProjectRenderState* arg0) {
  if (arg0 != 0) {
    s32 unk00;
    arg0->unk00[0] = 0;
    RwRenderStateGet(5, &unk00);
    if (unk00 != 0) arg0->unk00[0] |= 1;
    RwRenderStateGet(6, &unk00);
    if (unk00 != 0) arg0->unk00[0] |= 2;
    RwRenderStateGet(8, &unk00);
    if (unk00 != 0) arg0->unk00[0] |= 4;
    RwRenderStateGet(0x13, &unk00);
    if (unk00 != 0) arg0->unk00[0] |= 0x20;
    RwRenderStateGet(0xC, &unk00);
    if (unk00 != 0) arg0->unk00[0] |= 8;
    RwRenderStateGet(7, &arg0->unk00[1]);
    RwRenderStateGet(0xA, &arg0->unk00[2]);
    RwRenderStateGet(0xB, &arg0->unk00[3]);
    RwRenderStateGet(1, &arg0->unk00[4]);
    if (RwRenderStateGet(2, &arg0->unk00[5]) != 0) {
      arg0->unk00[6] = arg0->unk00[5];
    } else {
      RwRenderStateGet(3, &arg0->unk00[5]);
      RwRenderStateGet(4, &arg0->unk00[6]);
    }
    RwRenderStateGet(9, &arg0->unk00[7]);
    RwRenderStateGet(0xD, &arg0->unk00[8]);
    RwRenderStateGet(0x10, &arg0->unk00[9]);
    RwRenderStateGet(0xF, &arg0->unk00[10]);
    RwRenderStateGet(0x12, &arg0->unk00[11]);
    return arg0;
  } else {
    BaerrState unk04;
    unk04.unk00 = 1;
    unk04.unk04 = _rwerror(0x80000016);
    RwErrorSet(&unk04);
    return 0;
  }
}
