#include <dolphin/trk.h>

#pragma dont_inline on
static void TRK_fill_mem(void* dest, int value, unsigned long length) {
#define cDest ((unsigned char*)dest)
#define lDest ((unsigned long*)dest)
  unsigned long val = (unsigned char)value;
  unsigned long i;
  lDest = (unsigned long*)dest;
  cDest = (unsigned char*)dest;

  cDest--;

  if (length >= 32) {
    i = ~(unsigned long)dest & 3;

    if (i) {
      length -= i;
      do {
        *++cDest = val;
      } while (--i);
    }

    if (val) {
      val |= val << 24 | val << 16 | val << 8;
    }

    lDest = (unsigned long*)(cDest + 1) - 1;

    i = length >> 5;
    if (i) {
      do {
        *++lDest = val;
        *++lDest = val;
        *++lDest = val;
        *++lDest = val;
        *++lDest = val;
        *++lDest = val;
        *++lDest = val;
        *++lDest = val;
      } while (--i);
    }

    i = (length & 31) >> 2;

    if (i) {
      do {
        *++lDest = val;
      } while (--i);
    }

    cDest = (unsigned char*)(lDest + 1) - 1;

    length &= 3;
  }

  if (length) {
    do {
      *++cDest = val;
    } while (--length);
  }

#undef cDest
#undef lDest
}
#pragma dont_inline reset

__declspec(section ".init") void* TRK_memcpy(void* dst, const void* src, u32 n) {
  const unsigned char* s = (const unsigned char*)src - 1;
  unsigned char* d = (unsigned char*)dst - 1;

  n++;
  while (--n != 0)
    *++d = *++s;
  return dst;
}

__declspec(section ".init") void* TRK_memset(void* dst, int val, u32 n) {
  TRK_fill_mem(dst, val, n);

  return dst;
}
