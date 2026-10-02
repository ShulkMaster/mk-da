/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/os.h>

OSThread* __OSCurrentThread AT_ADDRESS(0x800000E4);

void OSInitThreadQueue(OSThreadQueue* queue) { queue->head = queue->tail = NULL; }

OSThread* OSGetCurrentThread(void) { return __OSCurrentThread; }
