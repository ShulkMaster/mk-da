#ifndef MKDA_RENDERWARE_PROJECT_REGISTRY_H
#define MKDA_RENDERWARE_PROJECT_REGISTRY_H

#include <dolphin/types.h>

typedef void* (*ProjectRegistryCall)(void*, s32, s32);
typedef void* (*ProjectRegistryCopy)(void*, const void*, s32, s32);

s32 _rwPluginRegistryAddPlugin(u8* arg0, s32 arg1, u32 arg2,
    ProjectRegistryCall arg3, ProjectRegistryCall arg4, ProjectRegistryCopy arg5);

#endif
