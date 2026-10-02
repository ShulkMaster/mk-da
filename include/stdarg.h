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

void* __va_arg(va_list v_list, unsigned char type);

#define va_start(ap, fmt) ((void)fmt, __builtin_va_info(&ap))
#define va_arg(ap, t) (*((t*)__va_arg(ap, _var_arg_typeof(t))))
#define va_end(ap) (void)0
#else
typedef __builtin_va_list va_list;
#define va_start(ap, fmt) __builtin_va_start(ap, fmt)
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(v, l) __builtin_va_arg(v, l)
#endif

#endif
