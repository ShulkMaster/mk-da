/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/trk.h>

typedef unsigned long size_t;

typedef struct {
  unsigned int open_mode : 2;
  unsigned int io_mode : 3;
  unsigned int buffer_mode : 2;
  unsigned int file_kind : 3;
  unsigned int file_orientation : 2;
  unsigned int binary_io : 1;
} __file_modes;

typedef enum {
  DS_IONoError = 0,
  DS_IOError = 1,
  DS_IOEOF = 2,
} DSIOResult;

enum {
  DS_Stdin = 0,
  DS_Stdout = 1,
};

enum {
  DSMSG_WriteFile = 0xD0,
  DSMSG_ReadFile = 0xD1,
};

#define DS_NoError 0

DSError GetTRKConnected(void);
u8 GetUseSerialIO(void);

int TRKAccessFile(u32 command, u32 file_handle, size_t* length, u8* buffer);
int TRKOpenFile(u32 command, const char* name, u8 mode, u32* handle);
int TRKCloseFile(u32 command, u32 file_handle);
int TRKPositionFile(u32 command, u32 file_handle, u32* position, u8 mode);

DSIOResult __read_file(u32 handle, u8* buffer, size_t* count, void* ref_con);
DSIOResult __write_file(u32 handle, u8* buffer, size_t* count, void* ref_con);

static inline DSIOResult __access_file(u32 handle, u8* buffer, size_t* count, void* ref_con,
                                       MessageCommandID cmd) {
  size_t countTemp;
  u32 r0;

  if (GetTRKConnected() == DS_NoError) {
    return DS_IOError;
  }

  countTemp = *count;
  r0 = TRKAccessFile(cmd, handle, &countTemp, buffer);
  *count = countTemp;

  switch ((u8)r0) {
  case DS_IONoError:
    return DS_IONoError;
  case DS_IOEOF:
    return DS_IOEOF;
  }

  return DS_IOError;
}

DSIOResult __read_console(u32 handle, u8* buffer, size_t* count, void* ref_con) {
  if (GetUseSerialIO() == 0) {
    return DS_IOError;
  }
  return __read_file(DS_Stdin, buffer, count, ref_con);
}

DSIOResult __TRK_write_console(u32 handle, u8* buffer, size_t* count, void* ref_con) {
  if (GetUseSerialIO() == 0) {
    return DS_IOError;
  }
  return __write_file(DS_Stdout, buffer, count, ref_con);
}

DSIOResult __read_file(u32 handle, u8* buffer, size_t* count, void* ref_con) {
  return __access_file(handle, buffer, count, ref_con, DSMSG_ReadFile);
}

DSIOResult __write_file(u32 handle, u8* buffer, size_t* count, void* ref_con) {
  return __access_file(handle, buffer, count, ref_con, DSMSG_WriteFile);
}

static int convertFileMode(__file_modes* mode);

DSIOResult __open_file(const char* name, __file_modes* mode, u32* handle) {
  u32 r0;
  int m;

  if (GetTRKConnected() == DS_NoError) {
    return DS_IOError;
  }

  m = convertFileMode(mode);
  r0 = TRKOpenFile(0xD2, name, m, handle);

  switch ((u8)r0) {
  case DS_IONoError:
    return DS_IONoError;
  case DS_IOEOF:
    return DS_IOEOF;
  }

  return DS_IOError;
}

DSIOResult __close_file(u32 handle) {
  u32 r0;

  if (GetTRKConnected() == DS_NoError) {
    return DS_IOError;
  }

  r0 = TRKCloseFile(0xD3, handle);

  switch ((u8)r0) {
  case DS_IONoError:
    return DS_IONoError;
  case DS_IOEOF:
    return DS_IOEOF;
  }

  return DS_IOError;
}

DSIOResult __position_file(u32 handle, u32* position, int mode, void* ref_con) {
  u32 r0;
  int m = 0;

  if (GetTRKConnected() == DS_NoError) {
    return DS_IOError;
  }

  if (mode == 0) {
    m = 0;
  } else if (mode == 1) {
    m = 1;
  } else if (mode == 2) {
    m = 2;
  }

  r0 = TRKPositionFile(0xD4, handle, position, m);

  switch ((u8)r0) {
  case DS_IONoError:
    return DS_IONoError;
  case DS_IOEOF:
    return DS_IOEOF;
  }

  return DS_IOError;
}

static int convertFileMode(__file_modes* mode) {
  int result = 0;
  unsigned int open_mode = mode->open_mode;
  unsigned int io_mode = mode->io_mode;
  unsigned int binary_io = mode->binary_io;

  switch (open_mode) {
  case 0:
    result |= 1;
    break;
  case 2:
    result |= 2;
    break;
  case 1:
    result |= 4;
    break;
  }

  switch (io_mode) {
  case 1:
    result |= 1;
    break;
  case 2:
    result |= 2;
    break;
  case 6:
    result |= 4;
    break;
  case 3:
    result |= 0x12;
    break;
  case 7:
    result |= 7;
    break;
  }

  if (binary_io == 1) {
    result = (result | 8) & 0xFF;
  }

  return result;
}
