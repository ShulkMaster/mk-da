#include <mfl/mlSysCalls.h>

mlSysCalls* FileSystemItf;

s32 filemgr_init(void) {
  FileSystemItf = &g_SysCalls;
  return 1;
}
