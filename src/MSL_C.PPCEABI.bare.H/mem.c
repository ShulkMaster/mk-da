#include "types.h"
#include <stddef.h>

void __copy_longs_rev_unaligned(void* dst, const void* src, size_t n);
void __copy_longs_unaligned(void* dst, const void* src, size_t n);
void __copy_longs_rev_aligned(void* dst, const void* src, size_t n);
void __copy_longs_aligned(void* dst, const void* src, size_t n);

int memcmp(const void* s1, const void* s2, size_t n)
{
  const unsigned char* p1;
  const unsigned char* p2;
  for (p1 = (const unsigned char*)(s1)-1, p2 = (const unsigned char*)(s2)-1, n++; --n;)
    if (*++p1 != *++p2)
      return *p1 < *p2 ? -1 : 1;
  return 0;
}

void* __memrchr(const void* ptr, int value, size_t num)
{
  const unsigned char* str;
  unsigned int v = value & 0xff;
  for (str = (const unsigned char*)(ptr) + num, num++; --num;)
    if (*--str == v)
      return (void*)str;
  return 0;
}

void* memchr(const void* ptr, int value, size_t num)
{
  const unsigned char* str;
  unsigned int v = value & 0xff;
  for (str = (const unsigned char*)ptr - 1, num++; --num;)
    if ((*++str & 0xff) == v)
      return (void*)str;
  return 0;
}

void *memmove(void *dst, const void *src, size_t len)
{
  const char *csrc;
  char *cdst;

  int reverse = (u32)src < (u32)dst;

  if (len >= 32)
  {
    if (((int)dst ^ (int)src) & 3)
    {
      if (!reverse)
      {
        __copy_longs_unaligned(dst, src, len);
      }
      else
      {
        __copy_longs_rev_unaligned(dst, src, len);
      }
    }
    else
    {
      if (!reverse)
      {
        __copy_longs_aligned(dst, src, len);
      }
      else
      {
        __copy_longs_rev_aligned(dst, src, len);
      }
    }

    return dst;
  }
  else
  {
    if (!reverse)
    {
      for (csrc = (const char *)src - 1, cdst = (char *)dst - 1, len++; --len;)
      {
        *++cdst = *++csrc;
      }
    }
    else
    {
      for (csrc = (const char *)src + len, cdst = (char *)dst + len, len++; --len;)
      {
        *--cdst = *--csrc;
      }
    }
  }

  return dst;
}
