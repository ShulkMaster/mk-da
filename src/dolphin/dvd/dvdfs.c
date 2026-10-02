/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/dvd.h>

BOOL DVDClose(DVDFileInfo* fileInfo) {
  DVDCancel(&(fileInfo->cb));
  return TRUE;
}
