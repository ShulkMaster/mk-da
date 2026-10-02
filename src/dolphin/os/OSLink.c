/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/os.h>

typedef struct OSModuleInfo OSModuleInfo;
typedef struct OSModuleQueue {
  OSModuleInfo* head;
  OSModuleInfo* tail;
} OSModuleQueue;

OSModuleQueue __OSModuleInfoList : (OS_BASE_CACHED | 0x30C8);
const void* __OSStringTable : (OS_BASE_CACHED | 0x30D0);

void __OSModuleInit(void) {
  __OSModuleInfoList.head = __OSModuleInfoList.tail = 0;
  __OSStringTable = 0;
}
