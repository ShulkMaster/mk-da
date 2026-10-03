#include <renderware/project_pipeline.h>

s32 _rxPipelineGlobalsOffset;

void* _rwRenderPipelineOpen(void* unk00, s32 unk04, s32 unk08) {
  _rxPipelineGlobalsOffset = unk04;
  if (_rxPipelineOpen()) {
    return unk00;
  }
  return 0;
}

void* _rwRenderPipelineClose(void* unk00, s32 unk04, s32 unk08) {
  _rxPipelineClose();
  return unk00;
}

s32 _rwPipeAttach(void) {
  return 1;
}

void _rwPipeInitForCamera(void* unk00) {}
