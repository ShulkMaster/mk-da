#ifndef DOLPHIN_TRK_H
#define DOLPHIN_TRK_H

#include <dolphin/types.h>

typedef int DSError;
typedef int MessageBufferID;
typedef u8 MessageCommandID;

enum {
  DS_NoError = 0x0,
  DS_NoMessageBufferAvailable = 0x300,
  DS_MessageBufferOverflow = 0x301,
  DS_MessageBufferReadError = 0x302,
};

enum {
  DSMSG_ReplyACK = 0x80,
  DSMSG_ReplyNAK = 0xFF,
};

enum {
  DSREPLY_PacketSizeError = 0x2,
  DSREPLY_EscapeError = 0x4,
};

enum {
  DSRECV_Wait = 0,
  DSRECV_Found = 1,
  DSRECV_InFrame = 2,
  DSRECV_FrameOverflow = 3,
};

enum {
  NUBEVENT_Request = 2,
};

typedef struct MessageBuffer {
  u32 mutex;
  BOOL is_in_use;
  u32 length;
  u32 position;
  u8 data[0x880];
} MessageBuffer;

typedef struct TRKEvent {
  u8 event_type;
  u32 event_id;
  MessageBufferID message_buffer_id;
} TRKEvent;

typedef struct TRKFramingState {
  MessageBufferID msg_buffer_id;
  MessageBuffer* buffer;
  u8 receive_state;
  BOOL is_escape;
  u8 fcs_type;
} TRKFramingState;

extern BOOL gTRKBigEndian;
DSError TRKInitializeMutex(void* mutex);
DSError TRKAcquireMutex(void* mutex);
DSError TRKReleaseMutex(void* mutex);
void* TRK_memset(void* destination, int value, u32 size);
void* TRK_memcpy(void* destination, const void* source, u32 size);
DSError TRKMessageSend(MessageBuffer* message);
DSError TRKGetFreeBuffer(MessageBufferID* id, MessageBuffer** buffer);
MessageBuffer* TRKGetBuffer(MessageBufferID id);
void TRKReleaseBuffer(MessageBufferID id);
void TRKResetBuffer(MessageBuffer* message, u8 keep_data);
DSError TRKSetBufferPosition(MessageBuffer* message, u32 position);
DSError TRKAppendBuffer(MessageBuffer* message, const void* data, u32 length);
DSError TRKReadBuffer(MessageBuffer* message, void* data, u32 length);
DSError TRKAppendBuffer1_ui16(MessageBuffer* message, const u16 data);
DSError TRKAppendBuffer1_ui32(MessageBuffer* message, const u32 data);
DSError TRKAppendBuffer1_ui64(MessageBuffer* message, const u64 data);
DSError TRKAppendBuffer_ui8(MessageBuffer* message, const u8* data, int count);
DSError TRKAppendBuffer_ui32(MessageBuffer* message, const u32* data, int count);
DSError TRKReadBuffer1_ui8(MessageBuffer* message, u8* data);
DSError TRKReadBuffer1_ui16(MessageBuffer* message, u16* data);
DSError TRKReadBuffer1_ui32(MessageBuffer* message, u32* data);
DSError TRKReadBuffer1_ui64(MessageBuffer* message, u64* data);
DSError TRKReadBuffer_ui8(MessageBuffer* message, u8* data, int count);
DSError TRKReadBuffer_ui32(MessageBuffer* message, u32* data, int count);
DSError TRKInitializeMessageBuffers(void);
DSError TRKStandardACK(MessageBuffer* message, MessageCommandID command, int reply_error);
BOOL usr_puts_serial(const char* message);
int TRKReadUARTPoll(s8* byte);
int WriteUART1(s8 byte);
int WriteUARTFlush(void);
BOOL TRKGetNextEvent(TRKEvent* event);
DSError TRKInitializeEventQueue(void);
DSError TRKPostEvent(TRKEvent* event);
void TRKConstructEvent(TRKEvent* event, int event_type);
void TRKDestructEvent(TRKEvent* event);
DSError TRKDispatchMessage(MessageBuffer* message);
void TRKGetInput(void);
void TRKProcessInput(MessageBufferID id);
DSError TRKInitializeSerialHandler(void);
DSError TRKTerminateSerialHandler(void);
extern volatile u8* gTRKInputPendingPtr;

/* trk4: PowerPC target state (targimpl.c, dolphin_trk.c) */
typedef struct Default_PPC {
  u32 GPR[32];
  u32 PC;
  u32 LR;
  u32 CR;
  u32 CTR;
  u32 XER;
} Default_PPC;

typedef struct Float_PPC {
  u64 FPR[32];
  u64 FPSCR;
  u64 FPECR;
} Float_PPC;

typedef struct Extended1_PPC_6xx_7xx {
  u32 SR[16];
  u32 TBL;
  u32 TBU;
  u32 HID0;
  u32 HID1;
  u32 MSR;
  u32 PVR;
  u32 IBAT0U;
  u32 IBAT0L;
  u32 IBAT1U;
  u32 IBAT1L;
  u32 IBAT2U;
  u32 IBAT2L;
  u32 IBAT3U;
  u32 IBAT3L;
  u32 DBAT0U;
  u32 DBAT0L;
  u32 DBAT1U;
  u32 DBAT1L;
  u32 DBAT2U;
  u32 DBAT2L;
  u32 DBAT3U;
  u32 DBAT3L;
  u32 DMISS;
  u32 DCMP;
  u32 HASH1;
  u32 HASH2;
  u32 IMISS;
  u32 ICMP;
  u32 RPA;
  u32 SDR1;
  u32 DAR;
  u32 DSISR;
  u32 SPRG0;
  u32 SPRG1;
  u32 SPRG2;
  u32 SPRG3;
  u32 DEC;
  u32 IABR;
  u32 EAR;
  u32 DABR;
  u32 PMC1;
  u32 PMC2;
  u32 PMC3;
  u32 PMC4;
  u32 SIA;
  u32 MMCR0;
  u32 MMCR1;
  u32 THRM1;
  u32 THRM2;
  u32 THRM3;
  u32 ICTC;
  u32 L2CR;
  u32 UMMCR2;
  u32 UBAMR;
  u32 UMMCR0;
  u32 UPMC1;
  u32 UPMC2;
  u32 USIA;
  u32 UMMCR1;
  u32 UPMC3;
  u32 UPMC4;
  u32 USDA;
  u32 MMCR2;
  u32 BAMR;
  u32 SDA;
  u32 MSSCR0;
  u32 MSSCR1;
  u32 PIR;
  u32 exceptionID;
  u32 GQR[8];
  u32 HID_G;
  u32 WPAR;
  u32 DMA_U;
  u32 DMA_L;
} Extended1_PPC_6xx_7xx;

typedef struct Extended2_PPC_6xx_7xx {
  u32 PSR[32][2];
} Extended2_PPC_6xx_7xx;

typedef struct ProcessorState_PPC_6xx_7xx {
  Default_PPC Default;
  Float_PPC Float;
  Extended1_PPC_6xx_7xx Extended1;
  Extended2_PPC_6xx_7xx Extended2;
  u32 transport_handler_saved_ra;
} ProcessorState_PPC_6xx_7xx;

typedef ProcessorState_PPC_6xx_7xx ProcessorState_PPC;

typedef struct TRKState {
  u32 gpr[32];
  u32 lr;
  u32 ctr;
  u32 xer;
  u32 msr;
  u32 dar;
  u32 dsisr;
  BOOL isStopped;
  BOOL inputActivated;
  void* inputPendingPtr;
} TRKState;

extern ProcessorState_PPC gTRKCPUState;
extern TRKState gTRKState;
u32 __TRK_get_MSR(void);
void __TRK_set_MSR(u32 msr);

#endif
