/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/os/OSInterrupt.h>

extern __OSInterruptHandler* InterruptHandlerTable_8041CD90;

__OSInterruptHandler
    __OSSetInterruptHandler(__OSInterrupt interrupt, __OSInterruptHandler handler) {
  __OSInterruptHandler oldHandler;

  oldHandler = InterruptHandlerTable_8041CD90[interrupt];
  InterruptHandlerTable_8041CD90[interrupt] = handler;
  return oldHandler;
}

__OSInterruptHandler __OSGetInterruptHandler(__OSInterrupt interrupt) {
  return InterruptHandlerTable_8041CD90[interrupt];
}
