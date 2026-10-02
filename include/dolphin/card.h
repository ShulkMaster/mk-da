#ifndef _DOLPHIN_CARD
#define _DOLPHIN_CARD
#include <dolphin/types.h>
typedef void (*CARDCallback)(s32 chan, s32 result);
#define CARD_FILENAME_MAX 32
#define CARD_MAX_FILE 127
#define CARD_RESULT_NOCARD -3
#define CARD_RESULT_BROKEN -6
#define CARD_RESULT_INSSPACE -9
#define CARD_RESULT_READY 0
#define CARD_RESULT_NOFILE -4
#define CARD_RESULT_NOPERM -10
#define CARD_RESULT_ENCODING -13
#define CARD_RESULT_FATAL_ERROR -128
#define CARD_ATTR_PUBLIC 0x04u
#define CARD_ICON_MAX 8
#define CARD_ICON_WIDTH 32
#define CARD_ICON_HEIGHT 32
#define CARD_BANNER_WIDTH 96
#define CARD_BANNER_HEIGHT 32
#define CARD_READ_SIZE 512
#define CARD_COMMENT_SIZE 64
#define CARD_STAT_BANNER_C8 1
#define CARD_STAT_BANNER_RGB5A3 2
#define CARD_STAT_ICON_C8 1
#define CARD_STAT_ICON_RGB5A3 2
#define CARD_STAT_SPEED_FAST 1
#define CARDGetBannerFormat(stat) ((stat)->bannerFormat & 3)
#define CARDGetIconFormat(stat, n) (((stat)->iconFormat >> (2 * (n))) & 3)
#define CARDSetIconSpeed(stat, n, f) \
  ((stat)->iconSpeed = (u16)(((stat)->iconSpeed & ~(3 << (2 * (n)))) | ((f) << (2 * (n)))))

typedef struct CARDFileInfo {
  s32 chan;
  s32 fileNo;

  s32 offset;
  s32 length;
  u16 iBlock;
  u16 __padding;
} CARDFileInfo;

typedef struct CARDStat {
  char fileName[CARD_FILENAME_MAX];
  u32 length;
  u32 time; // seconds since 01/01/2000 midnight
  u8 gameName[4];
  u8 company[2];

  u8 bannerFormat;
  u8 __padding;
  u32 iconAddr; // offset to the banner, bannerTlut, icon, iconTlut data set.
  u16 iconFormat;
  u16 iconSpeed;
  u32 commentAddr; // offset to the pair of 32 byte character strings.

  u32 offsetBanner;
  u32 offsetBannerTlut;
  u32 offsetIcon[CARD_ICON_MAX];
  u32 offsetIconTlut;
  u32 offsetData;
} CARDStat;

#define CARD_RESULT_BUSY -1
#define CARD_RESULT_IOERROR -5
#define CARD_RESULT_WRONGDEVICE -2
#define CARD_RESULT_UNLOCKED 1
#define CARD_WORKAREA_SIZE (5 * 8192)
s32 CARDUnmount(s32 chan);
s32 CARDMountAsync(s32 chan, void* workArea, CARDCallback detachCallback, CARDCallback attachCallback);
#endif
