#ifndef _DOLPHIN_OSERROR
#define _DOLPHIN_OSERROR

#include <dolphin/types.h>
#include <dolphin/os/OSContext.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OS_ERROR_PROTECTION 15

typedef u16 OSError;
typedef void (*OSErrorHandler)(OSError error, OSContext* context, ...);

extern OSErrorHandler __OSErrorTable[16];

#ifdef __cplusplus
}
#endif

#endif
