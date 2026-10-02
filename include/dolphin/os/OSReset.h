#ifndef _DOLPHIN_OSRESET
#define _DOLPHIN_OSRESET

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef BOOL (*OSResetFunction)(BOOL final);
typedef struct OSResetFunctionInfo OSResetFunctionInfo;

struct OSResetFunctionInfo {
  OSResetFunction func;
  u32 priority;
  OSResetFunctionInfo* next;
  OSResetFunctionInfo* prev;
};

void OSRegisterResetFunction(OSResetFunctionInfo* func);

#ifdef __cplusplus
}
#endif

#endif
