#ifndef _DOLPHIN_PAD
#define _DOLPHIN_PAD
#include <dolphin/types.h>
#define PAD_SPEC_0 0
#define PAD_SPEC_1 1
#define PAD_SPEC_2 2
#define PAD_SPEC_3 3
#define PAD_SPEC_4 4
#define PAD_SPEC_5 5
#define PAD_MOTOR_STOP 0
#define PAD_MOTOR_STOP_HARD 2
#define PAD_ERR_NONE 0
#define PAD_ERR_NO_CONTROLLER -1
#define PAD_ERR_NOT_READY -2
#define PAD_ERR_TRANSFER -3
#define PAD_BUTTON_LEFT 0x0001
#define PAD_BUTTON_RIGHT 0x0002
#define PAD_BUTTON_DOWN 0x0004
#define PAD_BUTTON_UP 0x0008
#define PAD_TRIGGER_Z 0x0010
#define PAD_TRIGGER_R 0x0020
#define PAD_TRIGGER_L 0x0040
#define PAD_BUTTON_A 0x0100
#define PAD_BUTTON_B 0x0200
#define PAD_BUTTON_X 0x0400
#define PAD_BUTTON_Y 0x0800
#define PAD_BUTTON_MENU 0x1000
#define PAD_BUTTON_START 0x1000
#define PAD_CHAN0_BIT 0x80000000
#define PAD_CHAN1_BIT 0x40000000
#define PAD_CHAN2_BIT 0x20000000
#define PAD_CHAN3_BIT 0x10000000
#ifdef __cplusplus
extern "C" {
#endif
typedef void (*PADSamplingCallback)(void);
typedef struct PADStatus {
  u16 button;
  s8 stickX;
  s8 stickY;
  s8 substickX;
  s8 substickY;
  u8 triggerLeft;
  u8 triggerRight;
  u8 analogA;
  u8 analogB;
  s8 err;
} PADStatus;
BOOL PADReset(u32 mask);
BOOL PADInit();
u32 PADRead(PADStatus *status);
void PADControlMotor(s32 chan, u32 command);
void PADSetSpec(u32 spec);
PADSamplingCallback PADSetSamplingCallback(PADSamplingCallback callback);
BOOL __PADDisableRecalibration(BOOL disable);
#ifdef __cplusplus
}
#endif
#endif
