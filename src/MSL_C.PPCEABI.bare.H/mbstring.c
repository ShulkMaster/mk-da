#include "types.h"

extern char* strncpy(char* dst, const char* src, unsigned long count);

static s32 is_utf8_complete(const char* string, u32 count);

inline s32 wctomb(char* string, u16 wide_char)
{
  s32 number_of_bytes;
  char* target;
  unsigned char first_byte_mark[4] = { 0x00, 0x00, 0xC0, 0xE0 };

  if (!string)
    return (0);

  if (wide_char < 0x80)
    number_of_bytes = 1;
  else if (wide_char < 0x800)
    number_of_bytes = 2;
  else
    number_of_bytes = 3;

  target = string + number_of_bytes;

  switch (number_of_bytes) {
    case 3:
      *--target = (wide_char & 0x3F) | 0x80;
      wide_char >>= 6;
    case 2:
      *--target = (wide_char & 0x3F) | 0x80;
      wide_char >>= 6;
    case 1:
      *--target = wide_char | first_byte_mark[number_of_bytes];
  }

  return number_of_bytes;
}

unsigned long wcstombs(char* s, const u16* pwcs, unsigned long n)
{
  int chars_written = 0;
  int result;
  char temp[3];
  u16* source;

  if (!s || !pwcs)
    return (0);

  source = (u16*)pwcs;
  while (chars_written <= n) {
    if (!*source) {
      *(s + chars_written) = '\0';
      break;
    } else {
      result = wctomb(temp, *source++);
      if ((chars_written + result) <= n) {
        strncpy(s + chars_written, temp, result);
        chars_written += result;
      } else
        break;
    }
  }

  return chars_written;
}

inline static s32 utf8_to_unicode(u16* wide_char, const char* string, u32 count)
{
  s32 number_of_bytes;
  s32 is_utf8;
  char* source;
  u16 result = 0;

  if (!string)
    return 0;

  if (count <= 0)
    return -1;

  number_of_bytes = is_utf8_complete(string, count);
  if (number_of_bytes < 0)
    return -1;

  source = (char*)string;
  switch (number_of_bytes) {
    case 3:
      result |= *source++ & 0x0F;
      result <<= 6;
    case 2:
      result |= *source++ & 0x3F;
      result <<= 6;
    case 1:
      result |= *source++ & 0x7F;
  }

  if (result == 0)
    is_utf8 = 0;
  else if (result < 0x80)
    is_utf8 = 1;
  else if (result < 0x800)
    is_utf8 = 2;
  else
    is_utf8 = 3;

  if (is_utf8 != number_of_bytes)
    return -1;

  if (wide_char)
    *wide_char = result;

  return number_of_bytes;
}

s32 mbtowc(u16* wide_char, const char* string, u32 count)
{
  return utf8_to_unicode(wide_char, string, count);
}

static s32 is_utf8_complete(const char* string, u32 count)
{
  if (count == 0)
    return -1;

  if (string[0] == 0)
    return 0;

  if ((string[0] & 0x80) == 0) {
    return 1;
  } else if ((string[0] & 0xE0) == 0xC0) {
    if (count >= 2) {
      if ((string[1] & 0x80) == 0x80)
        return 2;
      return -1;
    }
    return -2;
  } else if ((string[0] & 0xF0) == 0xE0) {
    if (count >= 3) {
      if ((string[1] & 0x80) == 0x80) {
        if ((string[2] & 0x80) == 0x80)
          return 3;
      }
      return -1;
    } else if ((count == 2 && (string[1] & 0x80) == 0x80) || count == 1) {
      return -2;
    }
    return -1;
  }

  return -1;
}
