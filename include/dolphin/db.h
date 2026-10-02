#ifndef _DOLPHIN_DB
#define _DOLPHIN_DB

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DBInterface {
  u32 bPresent;
  u32 exceptionMask;
  void (*ExceptionDestination)(void);
  void* exceptionReturn;
} DBInterface;

extern DBInterface* __DBInterface;
void DBPrintf(char* format, ...);

#ifdef __cplusplus
}
#endif

#endif
