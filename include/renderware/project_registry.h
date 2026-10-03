#ifndef MKDA_RENDERWARE_PROJECT_REGISTRY_H
#define MKDA_RENDERWARE_PROJECT_REGISTRY_H

#include <dolphin/types.h>

typedef void* (*ProjectRegistryCall)(void*, s32, s32);
typedef void* (*ProjectRegistryCopy)(void*, const void*, s32, s32);

s32 _rwPluginRegistryAddPlugin(u8* arg0, s32 arg1, u32 arg2,
    ProjectRegistryCall arg3, ProjectRegistryCall arg4, ProjectRegistryCopy arg5);

typedef void* (*ProjectRegistryRead)(void*, s32, void*, s32, s32);
typedef void* (*ProjectRegistryWrite)(void*, s32, const void*, s32, s32);
typedef s32 (*ProjectRegistrySize)(const void*, s32, s32);

s32 _rwPluginRegistryAddPluginStream(u8* arg0, u32 arg1,
    ProjectRegistryRead arg2, ProjectRegistryWrite arg3, ProjectRegistrySize arg4);

#endif
