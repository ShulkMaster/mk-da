#ifndef _DOLPHIN_DSP
#define _DOLPHIN_DSP

#include <dolphin/types.h>
#include <dolphin/os.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DSP_TASK_FLAG_CLEARALL 0x00000000
#define DSP_TASK_FLAG_CANCEL 0x00000002

#define DSP_TASK_STATE_INIT 0
#define DSP_TASK_STATE_RUN 1
#define DSP_TASK_STATE_YIELD 2
#define DSP_TASK_STATE_DONE 3

typedef void (*DSPCallback)(void* task);

typedef struct STRUCT_DSP_TASK {
  vu32 state;
  vu32 priority;
  vu32 flags;
  u16* iram_mmem_addr;
  u32 iram_length;
  u32 iram_addr;

  u16* dram_mmem_addr;
  u32 dram_length;
  u32 dram_addr;

  u16 dsp_init_vector;
  u16 dsp_resume_vector;

  DSPCallback init_cb;
  DSPCallback res_cb;
  DSPCallback done_cb;
  DSPCallback req_cb;

  struct STRUCT_DSP_TASK* next;
  struct STRUCT_DSP_TASK* prev;

  OSTime t_context;
  OSTime t_task;

} DSPTaskInfo;

u32 DSPCheckMailToDSP(void);
u32 DSPCheckMailFromDSP(void);
u32 DSPReadMailFromDSP(void);
void DSPSendMailToDSP(u32 mail);

void __DSP_exec_task(DSPTaskInfo* curr, DSPTaskInfo* next);
void __DSP_boot_task(DSPTaskInfo* task);
void __DSP_insert_task(DSPTaskInfo* task);
void __DSP_remove_task(DSPTaskInfo* task);
void __DSP_debug_printf(const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_DSP
