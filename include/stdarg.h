#ifndef _STDARG_H_
#define _STDARG_H_

#ifdef __MWERKS__
typedef struct {
  char gpr;
  char fpr;
  char reserved[2];
  char* input_arg_area;
  char* reg_save_area;
} va_list[1];

#define va_start(ap, fmt) ((void)fmt, __builtin_va_info(&ap))
#define va_end(ap) (void)0
#else
typedef __builtin_va_list va_list;
#define va_start(ap, fmt) __builtin_va_start(ap, fmt)
#define va_end(ap) __builtin_va_end(ap)
#endif

#endif
