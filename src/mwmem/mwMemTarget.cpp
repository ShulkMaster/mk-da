#include <mwmem/mwMem.h>
#include <stdarg.h>
#include <stdio.h>

static char printBuffer[512];

void MEMPRINT(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vsprintf(printBuffer, format, args);
  va_end(args);
  printf(printBuffer);
}
