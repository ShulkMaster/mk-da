/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/os/OSException.h>

extern __OSExceptionHandler* OSExceptionTable_8041CD50;

__OSExceptionHandler __OSGetExceptionHandler(__OSException exception) {
  return OSExceptionTable_8041CD50[exception];
}
