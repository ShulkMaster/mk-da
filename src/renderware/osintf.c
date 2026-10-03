/* #audit 2026-10-03T04:06Z clean-room PASS (audit) */
#include <dolphin/types.h>

s32 _rwpathisabsolute(const char* unk00) {
  if (unk00[1] == ':' &&
      ((unk00[0] >= 'A' && unk00[0] <= 'Z') ||
       (unk00[0] >= 'a' && unk00[0] <= 'z'))) {
    return 1;
  }
  return unk00[0] == '\\';
}
