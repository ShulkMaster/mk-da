#ifndef MSL_C_FILE_IO_H
#define MSL_C_FILE_IO_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Coarse view of the 0x50-byte FILE (ansi_files.c has the full layout): only
// the prefix the stdio code touches is named; the tail keeps the stride.
typedef struct {
	u32 open_mode : 2;
	u32 io_mode : 3;
	u32 buffer_mode : 2;
	u32 file_kind : 3;
	u32 file_orientation : 2;
	u32 binary_io : 1;
} __file_modes;

typedef struct {
	u32 io_state : 3;
	u32 free_buffer : 1;
	u8 eof;
	u8 error;
} __file_state;

typedef void (*__idle_proc)(void);
typedef s32 (*__pos_proc)(u32 handle, u32* position, s32 mode, __idle_proc idle_proc);
typedef s32 (*__io_proc)(u32 handle, u8* buffer, u32* count, __idle_proc idle_proc);
typedef s32 (*__close_proc)(u32 handle);

typedef struct {
	u32 handle;
	__file_modes mode;
	__file_state state;
	u8 char_buffer;
	u8 char_buffer_overflow;
	u8 ungetc_buffer[2];
	u16 ungetc_wide_buffer[2];
	u8 position_padding[4];
	u32 position;
	u8* buffer;
	u32 buffer_size;
	u8* buffer_ptr;
	u32 buffer_length;
	u32 buffer_alignment;
	u32 saved_buffer_length;
	u32 buffer_position;
	__pos_proc position_proc;
	__io_proc read_proc;
	__io_proc write_proc;
	__close_proc close_proc;
	__idle_proc idle_proc;
	u8 padding[4];
} FILE;

extern FILE __files[4];

s32 fwide(FILE* file, s32 mode);
s32 __flush_buffer(FILE* file, u32* bytes_flushed);
s32 __load_buffer(FILE* file, u32* bytes_loaded, s32 alignment);
void __prep_buffer(FILE* file);

#ifdef __cplusplus
}
#endif

#endif
