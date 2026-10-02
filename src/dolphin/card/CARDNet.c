#include <dolphin/CARDPriv.h>

u16 __CARDVendorID = 0xffff;

s32 __CARDGetStatusEx(s32 chan, s32 fileNo, CARDDir* dirent);
s32 __CARDSetStatusExAsync(s32 chan, s32 fileNo, CARDDir* dirent,
                          CARDCallback callback);

s32 CARDSetAttributesAsync(s32 chan, s32 fileNo, u8 attr, CARDCallback callback) {
  CARDDir dirent;
  s32 result;

  result = __CARDGetStatusEx(chan, fileNo, &dirent);
  if (result < 0) {
    return result;
  }
  dirent.permission = attr;
  return __CARDSetStatusExAsync(chan, fileNo, &dirent, callback);
}

s32 CARDSetAttributes(s32 chan, s32 fileNo, u8 attr) {
  s32 result;

  result = CARDSetAttributesAsync(chan, fileNo, attr, __CARDSyncCallback);
  if (result < 0) {
    return result;
  }
  return __CARDSync(chan);
}
