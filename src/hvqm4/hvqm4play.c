#include "hvqm4player.h"
#include <dolphin/dvd.h>
#include <dolphin/os.h>
#include <dolphin/vi.h>

#include <musyx/musyx.h>

extern void ReportErrcode(s32 error);
extern s32 mflTick(void);
extern void* _mwMemMalloc(void* heap, u32 size, u32 alignment,
    const char* name, const char* file, s32 line);
extern void* _mwMemCalloc(void* heap, u32 count, u32 size, u32 alignment,
    const char* name, const char* file, s32 line);
extern void _mwMemFree(void* ptr, const char* file, s32 line);
extern GXRenderModeObj GXNtsc480Int, GXPal528Int, GXMpal480Int;
extern OSThread* gPlayerThread;

OSThread vDispThread;
static SND_STREAMID StreamL = SND_ID_ERROR;
static SND_STREAMID StreamR = SND_ID_ERROR;
static s32 err;
static u8* FrameBuffer[2];
static BOOL FrameBufferClearFlag[2];
static u8* vDispThreadStack;
static u16* s_buff[2];
static u32 s_buff_r_pos_R;
static u32 s_buff_r_pos_L;
static u32 s_buff_w_pos;
static void* pStreamBufR;
static void* pStreamBufL;
static u8 sInMainLoop;
static u32 FrameBufferPage;
static u32 FrameBufferTextOffset;
static u32 FrameBufferHeight;
static u32 FrameBufferSize;
static void* FrameBufferDisp;
static void* FrameBufferAlloc;
static BOOL MOVIE_DISP_THREAD;
OSThread* gDispThread;
HVQM4PlayerEx* PlayerHandle;
void* gHeap;

static u32 MusyXCallbackR(void*, u32, void*, u32, u32);
static u32 MusyXCallbackL(void*, u32, void*, u32, u32);
static void get_sound_data(void);
static void vdisp_init(u32 width, u32 height, u32 size);
static void* vDispThread_Entry(void* arg);
static void dvd_status_callback(HVQM4PlayerEx* player, s32 user, s32 status);
static void vdisp_copy_frame(u8* dst, u8* src, u32 width, u32 height);

static void close_sound(void)
{
  if (StreamL != SND_ID_ERROR) {
    sndStreamDeactivate(StreamL);
    sndStreamFree(StreamL);
    StreamL = SND_ID_ERROR;
  }
  if (StreamR != SND_ID_ERROR) {
    sndStreamDeactivate(StreamR);
    sndStreamFree(StreamR);
    StreamR = SND_ID_ERROR;
  }
  if (pStreamBufL != NULL) {
    _mwMemFree(pStreamBufL, "hvqm4play.c", 0x29A);
    pStreamBufL = NULL;
  }
  if (pStreamBufR != NULL) {
    _mwMemFree(pStreamBufR, "hvqm4play.c", 0x29F);
    pStreamBufR = NULL;
  }
  if (s_buff[0] != NULL) {
    _mwMemFree(s_buff[0], "hvqm4play.c", 0x2A4);
    s_buff[0] = NULL;
  }
  if (s_buff[1] != NULL) {
    _mwMemFree(s_buff[1], "hvqm4play.c", 0x2A9);
    s_buff[1] = NULL;
  }
}

static inline void close_movie(void)
{
  MOVIE_DISP_THREAD = 0;
  while (!OSIsThreadTerminated(&vDispThread)) {
    OSYieldThread();
  }
  gPlayerThread = NULL;
  gDispThread = NULL;
  if (PlayerHandle != NULL) {
    HVQM4PlayerExControlExit(PlayerHandle);
    if (!HVQM4PlayerExClose(PlayerHandle, &err)) {
      ReportErrcode(err);
      return;
    }
    PlayerHandle = NULL;
  }
  if (vDispThreadStack != NULL) {
    _mwMemFree(vDispThreadStack, "hvqm4play.c", 0x3BA);
    vDispThreadStack = NULL;
  }
  if (FrameBufferAlloc != NULL) {
    _mwMemFree(FrameBufferAlloc, "hvqm4play.c", 0x3BF);
    FrameBufferAlloc = NULL;
  }
  close_sound();
}

static inline GXRenderModeObj* get_render_mode(void)
{
  switch (VIGetTvFormat()) {
  case VI_NTSC: return &GXNtsc480Int;
  case VI_PAL: return &GXPal528Int;
  case VI_MPAL: return &GXMpal480Int;
  default: return &GXNtsc480Int;
  }
}

int mpeg_fullscreen(void* heap, const char* filename, void** framebuffer,
    int (*checkExit)(void))
{
  GXRenderModeObj mode;
  GXRenderModeObj* renderMode;
  void* oldFramebuffer = *framebuffer;
  BOOL suspended = FALSE;
  u32 size;
  int i;
  BOOL started;

  gHeap = heap;
  sInMainLoop = 0;
  renderMode = get_render_mode();
  size = ((renderMode->fbWidth + 15) & 0xFFF0) * renderMode->xfbHeight * 2;
  GXAdjustForOverscan(renderMode, &mode, 0, 0);
  FrameBufferTextOffset = mode.fbWidth * (((u32)mode.xfbHeight >> 1) - 24) * 2 + 400;
  VIFlush();
  VIWaitForRetrace();
  vdisp_init(mode.fbWidth, mode.xfbHeight, size);
  s_buff[0] = _mwMemMalloc(gHeap, 0x4000, 3, "s_buff movie", "hvqm4play.c", 0x23B);
  s_buff[1] = _mwMemMalloc(gHeap, 0x4000, 3, "s_buff movie", "hvqm4play.c", 0x23C);
  pStreamBufL = _mwMemCalloc(gHeap, 1, 0x3C00, 5, "pStreamBufL movie", "hvqm4play.c", 0x23F);
  pStreamBufR = _mwMemCalloc(gHeap, 1, 0x3C00, 5, "pStreamBufR movie", "hvqm4play.c", 0x240);
  StreamL = sndStreamAllocEx(255, pStreamBufL, 0x1E00, 48000, 0, 0, 0, 0,
      0, 0, SND_STREAM_MANUALARAMUPD, MusyXCallbackL, 0, NULL);
  StreamR = sndStreamAllocEx(255, pStreamBufR, 0x1E00, 48000, 0, 127, 0, 0,
      0, 0, SND_STREAM_MANUALARAMUPD, MusyXCallbackR, 1, NULL);
  for (i = 0; i < 8; i++) {
    VIWaitForRetrace();
  }
  sndStreamMixParameterEx(StreamL, 127, 0, 0, 0, 0);
  sndStreamMixParameterEx(StreamR, 127, 127, 0, 0, 0);
  PlayerHandle = HVQM4PlayerExCreate(filename, 15, 24, 0x2000, -1,
      dvd_status_callback, 0, &err);
  if (PlayerHandle == NULL) {
    ReportErrcode(err);
    started = FALSE;
  } else {
    HVQM4PlayerExStart(PlayerHandle);
    started = TRUE;
  }
  if (!started) {
    close_movie();
    return 0;
  }
  sInMainLoop = 1;
  for (;;) {
    s32 status = DVDGetDriveStatus();
    BOOL executing;
    if (status == 11 || (u32)(status - 4) <= 2 || status == -1) {
      OSReport("Movie player error - dvd_status_code - %d\n", status);
      if (!suspended) {
        OSSuspendThread(&vDispThread);
        suspended = TRUE;
      }
      mflTick();
      continue;
    }
    if (suspended) {
      OSResumeThread(&vDispThread);
      suspended = FALSE;
    }
    if (HVQM4PlayerExCheckExecute(PlayerHandle)) {
      VIWaitForRetrace();
      executing = TRUE;
    } else {
      executing = FALSE;
    }
    if (!executing) {
      break;
    }
    if (checkExit != NULL && checkExit()) {
      HVQM4PlayerExControlExit(PlayerHandle);
    }
  }
  sInMainLoop = 0;
  VISetNextFrameBuffer(oldFramebuffer);
  VISetBlack(FALSE);
  VIFlush();
  close_movie();
  VISetNextFrameBuffer(oldFramebuffer);
  VISetBlack(FALSE);
  VIFlush();
  return 1;
}

static u32 MusyXCallbackR(void* buffer, u32 position, void* buffer2, u32 length2, u32 user)
{
  u32 offset = ((position + 480) / 960) * 960;
  u16* start;
  u16* dst;
  u16* end;
  u32 read;
  u32 write;
  if (offset >= 0x1E00) {
    offset -= 0x1E00;
  }
  start = (u16*)buffer + offset;
  dst = start;
  end = start + 960;
  read = s_buff_r_pos_R;
  write = s_buff_w_pos;
  while (dst < end && read < write) {
    *dst++ = s_buff[1][read++ & 0x1FFF];
  }
  s_buff_r_pos_R = read;
  while (dst < end) {
    *dst++ = 0;
  }
  DCStoreRange(start, 0x780);
  sndStreamARAMUpdate(StreamR, offset, 960, 0, 0);
  return 0;
}

static u32 MusyXCallbackL(void* buffer, u32 position, void* buffer2, u32 length2, u32 user)
{
  u32 offset;
  u16* start;
  u16* end;
  u16* dst;
  u32 read;
  u32 write;
  get_sound_data();
  offset = ((position + 480) / 960) * 960;
  if (offset >= 0x1E00) {
    offset -= 0x1E00;
  }
  start = (u16*)buffer + offset;
  dst = start;
  end = start + 960;
  read = s_buff_r_pos_L;
  write = s_buff_w_pos;
  while (dst < end && read < write) {
    *dst++ = s_buff[0][read++ & 0x1FFF];
  }
  s_buff_r_pos_L = read;
  while (dst < end) {
    *dst++ = 0;
  }
  DCStoreRange(start, 0x780);
  sndStreamARAMUpdate(StreamL, offset, 960, 0, 0);
  return 0;
}

static void get_sound_data(void)
{
  BOOL more = TRUE;
  while (more) {
    BOOL enabled;
    HVQM4BufaNode* node;
    if (s_buff_w_pos >= s_buff_r_pos_L + 0x1000 ||
        s_buff_w_pos >= s_buff_r_pos_R + 0x1000) {
      break;
    }
    enabled = OSDisableInterrupts();
    if (PlayerHandle == NULL) {
      more = FALSE;
    } else {
      node = HVQM4PlayerExBufaGetSend(PlayerHandle);
      if (node == NULL) {
        more = FALSE;
      } else {
        if (HVQM4PlayerExControlCheckNormal(PlayerHandle)) {
          u32 size = HVQM4PlayerExBufaGetSize(node);
          u16* src = HVQM4PlayerExBufaGetBuff(node);
          while (size != 0) {
            u32 offset = s_buff_w_pos & 0x1FFF;
            size -= 4;
            s_buff[1][offset] = *src++;
            s_buff[0][offset] = *src++;
            s_buff_w_pos++;
          }
        } else {
          more = FALSE;
        }
        HVQM4PlayerExBufaSetFree(PlayerHandle, node);
      }
    }
    OSRestoreInterrupts(enabled);
  }
}

#include "hvqm4alloc.h"

static void vdisp_init(u32 width, u32 height, u32 size)
{
  u8* next;
  u32* framebuffer;
  u32* p;
  u32* end;
  OSReport("vdisp_init(%d,%d,%d)\n", width, height, size);
  if (width != 640) {
    OSPanic("hvqm4play.c", 0x105, "fbWidth error !\n");
  }
  FrameBufferSize = size;
  FrameBufferHeight = height;
  FrameBufferAlloc = _mwMemMalloc(gHeap, ((size + 63) & ~63) * 2 + 64, 6,
      "Frame Buffer Movie", "hvqm4play.c", 0x110);
  if (FrameBufferAlloc == NULL) {
    OSPanic("hvqm4play.c", 0x112, "Alloc Frame Buffer Error !!\n");
  }
  next = FrameBufferAlloc;
  FrameBuffer[0] = next;
  next += size;
  FrameBufferClearFlag[0] = 1;
  FrameBuffer[1] = next;
  FrameBufferClearFlag[1] = 1;
  FrameBufferPage = 1;
  framebuffer = (u32*)FrameBuffer[0];
  end = (u32*)(FrameBuffer[0] + FrameBufferSize);
  for (p = (u32*)FrameBuffer[0]; p < end; p++) {
    *p = 0x00800080;
  }
  DCFlushRangeNoSync(framebuffer, FrameBufferSize);
  FrameBufferDisp = framebuffer;
  VISetNextFrameBuffer(framebuffer);
  VISetBlack(FALSE);
  VIFlush();
  MOVIE_DISP_THREAD = 1;
  vDispThreadStack = _mwMemMalloc(gHeap, 0x2000, 3, "s_buff movie", "hvqm4play.c", 0x12F);
  if (!OSCreateThread(&vDispThread, vDispThread_Entry, NULL,
      vDispThreadStack + 0x2000, 0x2000, 8, OS_THREAD_ATTR_DETACH)) {
    OSPanic("hvqm4play.c", 0x13A, "Create Disp Thread Error !!\n");
  }
  gDispThread = &vDispThread;
  OSResumeThread(&vDispThread);
}

static void* vDispThread_Entry(void* arg)
{
  u8* destination;
  OSReport("vDispTread_Entry()\n");
  while (MOVIE_DISP_THREAD) {
    if (PlayerHandle != NULL && HVQM4PlayerExViRetrace(PlayerHandle)) {
      HVQM4BufvNode* node = HVQM4PlayerExBufvGetDisp(PlayerHandle);
      if (node != NULL) {
        u32 height;
        u32 width;
        u8* src;
        u8* framebuffer;
        u32 margin;
        framebuffer = FrameBuffer[FrameBufferPage];
        src = HVQM4PlayerExBufvGetBuff(node);
        width = margin = HVQM4PlayerExGetWidth(PlayerHandle);
        height = HVQM4PlayerExGetHeight(PlayerHandle);
        if (FrameBufferClearFlag[FrameBufferPage]) {
          u32* p;
          u32* end = (u32*)(framebuffer + FrameBufferSize);
          for (p = (u32*)framebuffer; p < end; p++) {
            *p = 0x00800080;
          }
          DCFlushRangeNoSync(framebuffer, FrameBufferSize);
          FrameBufferClearFlag[FrameBufferPage] = 0;
        }
        margin = (640 - margin) & ~1;
        destination = framebuffer + (((FrameBufferHeight - height) >> 1) * 1280 + margin);
        vdisp_copy_frame(destination, src, width, height);
        HVQM4PlayerExBufvSetBusyFlag(node, 0);
        FrameBufferDisp = framebuffer;
        VISetNextFrameBuffer(framebuffer);
        VIFlush();
        FrameBufferPage++;
        if (FrameBufferPage == 2) {
          FrameBufferPage = 0;
        }
      }
    }
    VIWaitForRetrace();
  }
  return NULL;
}

static void dvd_status_callback(HVQM4PlayerEx* player, s32 user, s32 status)
{
  if (sInMainLoop == 0 && (status == 11 || (u32)(status - 4) <= 2 || status == -1)) {
    mflTick();
  }
}

static void vdisp_copy_frame(u8* dst, u8* src, u32 width, u32 height)
{
  int rows = height >> 1;
  u8* y = src;
  u8* u = src + width * height;
  u8* v = u + (width >> 1) * (height >> 1);
  u8* rowDst = dst;
  while (rows > 0) {
    u32* out = (u32*)rowDst;
    u8* y0 = y;
    u32* end = out + (width >> 1);
    y += width;
    while (out < end) {
      u32 a;
      u32 b;
      u32 c;
      u32 d;
      b = ((y0[2] << 24) | (u[1] << 16) | (y0[3] << 8)) | v[1];
      c = ((y0[4] << 24) | (u[2] << 16) | (y0[5] << 8)) | v[2];
      d = ((y0[6] << 24) | (u[3] << 16) | (y0[7] << 8)) | v[3];
      out[0] = ((y0[0] << 24) | (u[0] << 16) | (y0[1] << 8)) | v[0];
      y0 += 8;

      out[1] = b;
      out[2] = c;
      out[3] = d;
      a = ((y[0] << 24) | (u[0] << 16) | (y[1] << 8)) | v[0];
      b = ((y[2] << 24) | (u[1] << 16) | (y[3] << 8)) | v[1];
      c = ((y[4] << 24) | (u[2] << 16) | (y[5] << 8)) | v[2];
      d = ((y[6] << 24) | (u[3] << 16) | (y[7] << 8)) | v[3];

      out[320] = a;
      u += 4;
      v += 4;

      out[321] = b;
      out[322] = c;

      y += 8;
      out[323] = d;

      out += 4;
    }
    DCFlushRangeNoSync(rowDst, width * 2);
    DCFlushRangeNoSync(rowDst + 1280, width * 2);
    rowDst += 2560;
    rows--;
  }
}
