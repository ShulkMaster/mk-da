#ifndef MKDA_EXCEPTION_FRAGMENT_H
#define MKDA_EXCEPTION_FRAGMENT_H

struct __eti_init_info {
  void* eti_start;
  void* eti_end;
  void* code_start;
  unsigned long code_size;
};

#ifdef __cplusplus
extern "C" {
#endif

int __register_fragment(struct __eti_init_info* info, char* TOC);
void __unregister_fragment(int fragmentID);

#ifdef __cplusplus
}
#endif

#endif
