#include <mfl/mflFile.h>
#include <mfl/mflPlatform.h>
#include <mfl/mflPools.h>
#include <mfl/mlSysCalls.h>
#include <stdio.h>
#include <string.h>
#include <msl/mslMusyXUtil.h>

static inline mflPoolConfig mflDefaultPoolConfig(void);
static inline void mflCompleteCommand(mflFileCommand* command);
static inline mflFileCommand* mflQueueFileCommand(void* file, s32 operation, s32 priority);

static void (*spTickErrorCallback)(void);
static mflQueue* mflIOQueue;
s32 mflFileLogging;

s32 mflFileSystemInit(void) {
  mflInitializePools(mflDefaultPoolConfig());
  mflIOQueue = mflQueueNew();
  mflQueueSetCurrent(mflIOQueue);
  mlSetSysCalls(1);
  mflPlatformStatisticsReset();
  return 0;
}

mflFileCommand* mflOpenAsync(const char* filename, const char* mode,
                             mflAsyncCallback callback, void* user) {
  mflFileCommand* request;
  if (mflQueueGetCurrent() == NULL) {
    return NULL;
  }
  request = mflFileCommandAlloc();
  if (request == NULL) {
    return NULL;
  }
  strncpy(request->filename, filename, sizeof(request->filename));
  request->filename[sizeof(request->filename) - 1] = 0;
  strncpy(request->mode, mode, sizeof(request->mode));
  request->mode[sizeof(request->mode) - 1] = 0;
  request->argument0.filename = request->filename;
  request->argument1.mode = request->mode;
  request->callback = callback;
  request->user = user;
  request->operation = 2;
  request->state = 0;
  mflQueueAdd(&request->queueEntry, 10);
  if (mflQueueSize() == 1) {
    mflTick();
  }
  return request;
}

mflFileCommand* mflReadAsync(void* destination, u32 elementSize,
                             u32 elementCount, void* file, s32 priority,
                             mflAsyncCallback callback, void* user) {
  mflFileCommand* request;
  if (mflQueueGetCurrent() == NULL) {
    return NULL;
  }
  request = mflFileCommandAlloc();
  if (request == NULL) {
    return NULL;
  }
  request->file = file;
  request->argument0.buffer = destination;
  request->elementSize = elementSize;
  request->elementCount = elementCount;
  request->callback = callback;
  request->user = user;
  request->operation = 4;
  request->state = 0;
  mflQueueAdd(&request->queueEntry, priority);
  if (mflQueueSize() == 1) {
    mflTick();
  }
  return request;
}

mflFileCommand* mflSeekAsync(void* file, u32 offset, s32 origin,
                             s32 priority, mflAsyncCallback callback,
                             void* user) {
  mflFileCommand* request;
  if (mflQueueGetCurrent() == NULL) {
    return NULL;
  }
  request = mflFileCommandAlloc();
  if (request == NULL) {
    return NULL;
  }
  request->file = file;
  request->argument0.value = offset;
  request->argument1.value = origin;
  request->callback = callback;
  request->user = user;
  request->operation = 6;
  request->state = 0;
  mflQueueAdd(&request->queueEntry, priority);
  if (mflQueueSize() == 1) {
    mflTick();
  }
  return request;
}

void mflCancelQ(mflFileCommand* request) {
  if (request != NULL) {
    if (request->operation == 4 && &request->queueEntry == mflQueueGet()) {
      mflGcnDvdReadCancel(request->file);
    }
    mflQueueRemove(&request->queueEntry);
    if (request->state != 3) {
      if (request->operation == 4) {
        request->readProgress = request->file->readProgress;
      }
      request->state = 4;
      request->flags.done = 1;
    }
  }
}

s32 mflCheckQ(mlAsyncRequest* request) {
  if (request == NULL) {
    return 1;
  }
  return request->flags.done;
}

u32 mflCheckQRead(mflFileCommand* request) {
  if (request == NULL) {
    return 0;
  }
  return request->readProgress;
}

void mflFreeQ(mflFileCommand* request) {
  if (request != NULL) {
    if (request->state == 2) {
      mflFileCommand* head = (mflFileCommand*)mflQueueGet();
      if (head != NULL && head->state == 2) {
        mflCompleteCommand(head);
      }
    }
    if (request->flags.done) {
      mflFileCommandFree(request);
    } else {
      request->flags.pendingFree = 1;
    }
  }
}

s32 mflTick(void) {
  mflFileCommand* command;
  if (mflGcnDvdDriveErrorCheck(0) == 0) {
    if (spTickErrorCallback != NULL) {
      spTickErrorCallback();
    }
    return 1;
  }
  for (;;) {
    command = (mflFileCommand*)mflQueueGet();
    if (command == NULL) {
      break;
    }
    if (command->state == 2) {
      command = (mflFileCommand*)mflQueueGet();
      if (command != NULL && command->state == 2) {
        mflCompleteCommand(command);
      }
    } else {
      if (command->state != 0) {
        if (command->operation == 4) {
          command->readProgress = mflGcnDvdReadProgress(command->file);
        }
      } else {
        mflFile* file;
        disableIRQ();
        file = command->file;
        switch (command->operation) {
        case 2:
          command->file = mflGcnDvdOpen(command->argument0.filename,
                                       command->argument1.mode);
          command->state = 2;
          break;
        case 3:
          mflGcnDvdClose(file);
          command->file = NULL;
          command->state = 2;
          break;
        case 1:
          command->argument0.value = mflGcnDvdExists(command->argument0.filename);
          command->state = 2;
          break;
        case 8:
          command->argument0.value = mflGcnDvdEof(file);
          command->state = 2;
          break;
        case 6:
          command->argument0.value = mflGcnDvdSeek(file,
              command->argument0.value, command->argument1.value);
          command->state = 2;
          break;
        case 7:
          command->argument0.value = mflGcnDvdTell(file);
          command->argument1.value = file->size;
          command->state = 2;
          break;
        case 4:
          command->flags.done = 0;
          if (mflGcnDvdReadStart(command->argument0.buffer,
              command->elementSize, command->elementCount, file) == 0) {
            command->readProgress = file->readProgress;
            command->state = 2;
          }
          break;
        }
        if (command->state != 2) {
          command->state = 1;
        }
        command = (mflFileCommand*)mflQueueGet();
        if (command != NULL && command->state == 2) {
          mflCompleteCommand(command);
        }
      }
      break;
    }
  }
  mflFileCommandFreePending();
  enableIRQ();
  return 0;
}

static inline mflFileCommand* mflQueueExistCommand(const char* filename) {
  mflFileCommand* command;
  if (mflQueueGetCurrent() == NULL) {
    return NULL;
  }
  command = mflFileCommandAlloc();
  if (command == NULL) {
    return NULL;
  }
  strncpy(command->filename, filename, sizeof(command->filename));
  command->filename[sizeof(command->filename) - 1] = 0;
  command->argument0.filename = command->filename;
  command->callback = NULL;
  command->user = NULL;
  command->operation = 1;
  command->state = 0;
  mflQueueAdd(&command->queueEntry, 10);
  if (mflQueueSize() == 1) {
    mflTick();
  }
  return command;
}

s32 mflExist(const char* filename) {
  mflFileCommand* command = mflQueueExistCommand(filename);
  s32 result;
  if (command == NULL) {
    return -1;
  }
  while (!command->flags.done) {
    mflTick();
  }
  result = command->argument0.value;
  mflFreeQ(command);
  return result;
}

void* mflOpen(const char* filename, const char* mode) {
  mflFileCommand* command = mflOpenAsync(filename, mode, NULL, NULL);
  void* file;
  if (command == NULL) {
    return NULL;
  }
  while (!command->flags.done) {
    mflTick();
  }
  file = command->file;
  mflFreeQ(command);
  return file;
}

s32 mflClose(void* file) {
  mflFileCommand* command = mflQueueFileCommand(file, 3, 10);
  if (command == NULL) {
    return 0;
  }
  while (!command->flags.done) {
    mflTick();
  }
  mflFreeQ(command);
  return 1;
}

s32 mflReadEx(void* buffer, s32 elementSize, s32 elementCount,
                void* file, s32 priority) {
  mflFileCommand* command = mflReadAsync(buffer, elementSize, elementCount,
                                       file, priority, NULL, NULL);
  s32 result;
  if (command == NULL) {
    return -1;
  }
  while (!command->flags.done) {
    mflTick();
  }
  result = command->file->lastResult;
  mflFreeQ(command);
  if (result > 0) {
    return result / elementSize;
  }
  return result;
}

s32 mflSeek(void* file, s32 offset, s32 origin) {
  mflFileCommand* command = mflSeekAsync(file, offset, origin, 10, NULL, NULL);
  s32 result;
  if (command == NULL) {
    return -1;
  }
  while (!command->flags.done) {
    mflTick();
  }
  result = command->argument0.value;
  mflFreeQ(command);
  return result;
}

s32 mflSeekEx(void* file, s32 offset, s32 origin, s32 priority) {
  mflFileCommand* command = mflSeekAsync(file, offset, origin, priority, NULL, NULL);
  s32 result;
  if (command == NULL) {
    return -1;
  }
  while (!command->flags.done) {
    mflTick();
  }
  result = command->argument0.value;
  mflFreeQ(command);
  return result;
}

s32 mflTell(void* file) {
  s32 result;
  mflFileCommand* command = mflQueueFileCommand(file, 7, 10);
  while (!command->flags.done) {
    mflTick();
  }
  result = command->argument0.value;
  mflFreeQ(command);
  return result;
}

static inline s32 mflReadBytes(char* buffer, s32 bytes, mflFile* file) {
  s32 result;
  mflFileCommand* command = mflReadAsync(buffer, 1, bytes, file, 10, NULL, NULL);
  if (command == NULL) {
    return -1;
  }
  while (!command->flags.done) {
    mflTick();
  }
  result = command->file->lastResult;
  mflFreeQ(command);
  return result;
}

char* mflGetS(char* buffer, s32 bytes, void* file) {
  bytes = mflReadBytes(buffer, bytes, file);

  if (bytes == 0) {
    return NULL;
  }
  if (bytes != 0) {
    s32 index;
    for (index = 0; index < bytes; index++) {
      if (buffer[index] == '\n') {
        mflSeek(file, index - bytes + 1, 1);
      }
    }
    buffer[index] = 0;
    return buffer;
  }
  return NULL;
}


s32 mflEof(void* file) {
  mflFileCommand* command = mflQueueFileCommand(file, 7, 10);
  s32 result;
  if (command == NULL) {
    return -1;
  }
  while (!command->flags.done) {
    mflTick();
  }
  result = (u32)command->argument0.value >= command->argument1.value;
  mflFreeQ(command);
  return result;
}

s32 mflSize(const char* filename) {
  mflFile* file = mflOpen(filename, "rb");
  s32 size = 0;

  if (file != NULL) {
    size = file->size;
    mflClose(file);
  }
  return size;
}


void mflFileLog(const char* message) {
  if (mflFileLogging != 0) {
    printf(">>> MFL: %s\n", message);
  }
}

s32 mflGetDiscErrorStatus(void) {
  return mflGcnDvdGetDriveErrorStatus();
}

void mflSetTickErrorCallback(void (*callback)(void)) {
  spTickErrorCallback = callback;
}

static inline mflFileCommand* mflQueueFileCommand(void* file, s32 operation,
                                                 s32 priority) {
  mflFileCommand* command;
  if (mflQueueGetCurrent() == NULL) {
    return NULL;
  }
  command = mflFileCommandAlloc();
  if (command == NULL) {
    return NULL;
  }
  command->file = file;
  command->callback = NULL;
  command->user = NULL;
  command->operation = operation;
  command->state = 0;
  mflQueueAdd(&command->queueEntry, priority);
  if (mflQueueSize() == 1) {
    mflTick();
  }
  return command;
}


static inline void mflCompleteCommand(mflFileCommand* command) {
  if (command->operation == 4) {
    command->readProgress = command->file->readProgress;
  }
  mflQueueRemoveFirst();
  command->state = 3;
  command->flags.done = 1;
  if (command != NULL && command->callback != NULL) {
    command->callback(command->file->lastResult, command->user);
  }
}

static inline mflPoolConfig mflDefaultPoolConfig(void) {
  mflPoolConfig config;
  config.value = 0;
  config.flags.files = 1;
  config.flags.commands = 1;
  config.flags.queues = 1;
  return config;
}
