#include <dolphin/types.h>
#include <dolphin/gx.h>

u16 _RwDlTokenCurrent = 1;
static GXDrawSyncCallback _RwDlDrawSyncCallbackPrev;

u16 _RwDlTokenLastSeen;

static void _rwDlDrawSyncCallback(u16 arg0) {
  _RwDlTokenLastSeen = arg0;
}

void _rwDlTokenOpen(void) {
  _RwDlDrawSyncCallbackPrev = GXSetDrawSyncCallback(_rwDlDrawSyncCallback);
}

void _rwDlTokenClose(void) {
  GXSetDrawSyncCallback(_RwDlDrawSyncCallbackPrev);
}

void _rwDlTokenWaitDone(u16 arg0) {
  s32 complete;
  if (arg0 == _RwDlTokenCurrent) {
    GXSetDrawSync(_RwDlTokenCurrent);
    _RwDlTokenCurrent = (_RwDlTokenCurrent + 1) % 0x8000;
  }
  for (;;) {
    s32 pending = 0;
    if (arg0 > _RwDlTokenLastSeen && arg0 <= _RwDlTokenCurrent) {
      pending = 1;
    }
    complete = !pending ? 1 : 0;
    if (complete) {
      break;
    }
  }
}
