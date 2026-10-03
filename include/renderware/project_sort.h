#ifndef MKDA_RENDERWARE_PROJECT_SORT_H
#define MKDA_RENDERWARE_PROJECT_SORT_H

#include <dolphin/types.h>
#include <stddef.h>

void qsort(void* arg0, size_t arg1, size_t arg2,
    int (*arg3)(const void*, const void*));

void _rx_rxRadixExchangeSort(u8* unk00, u32 unk04, u32 unk08, u32 unk0C, u32 unk10, u32 unk14);

#endif
