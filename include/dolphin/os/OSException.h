#ifndef _DOLPHIN_OSEXCEPTION
#define _DOLPHIN_OSEXCEPTION

#include <dolphin/types.h>
#include <dolphin/os/OSContext.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef u8 __OSException;
typedef void (*__OSExceptionHandler)(__OSException exception, OSContext* context);

__OSExceptionHandler __OSGetExceptionHandler(__OSException exception);

void __OSUnhandledException(__OSException exception, OSContext* context, u32 dsisr, u32 dar);

#ifdef __cplusplus
}
#endif

#endif
