#include <exception>

extern "C" void abort(void);

namespace std {

static void dthandler();
static void duhandler();

static terminate_handler thandler = dthandler;
static unexpected_handler uhandler = duhandler;

} // namespace std

extern "C" char __throw_catch_compare(const char* throwtype, const char* catchtype, long* offset_result)
{
  const char *cptr1, *cptr2;

  *offset_result = 0;

  if ((cptr2 = catchtype) == 0) {
    return true;
  }

  cptr1 = throwtype;

  if (*cptr2 == 'P') {
    cptr2++;
    if (*cptr2 == 'C')
      cptr2++;
    if (*cptr2 == 'V')
      cptr2++;
    if (*cptr2 == 'v') {
      if (*cptr1 == 'P' || *cptr1 == '*') {
        return true;
      }
    }
    cptr2 = catchtype;
  }

  switch (*cptr1) {
  case '*':
  case '!':
    if (*cptr1++ != *cptr2++)
      return false;
    for (;;) {
      if (*cptr1 == *cptr2++) {
        if (*cptr1++ == '!') {
          long offset;

          for (offset = 0; *cptr1 != '!';) {
            offset = offset * 10 + *cptr1++ - '0';
          }
          *offset_result = offset;
          return true;
        }
      } else {
        while (*cptr1++ != '!') {
        }
        while (*cptr1++ != '!') {
        }
        if (*cptr1 == 0)
          return false;

        cptr2 = catchtype + 1;
      }
    }
    return false;
  }

  while ((*cptr1 == 'P' || *cptr1 == 'R') && *cptr1 == *cptr2) {
    cptr1++;
    cptr2++;

    if (*cptr2 == 'C') {
      if (*cptr1 == 'C')
        cptr1++;
      cptr2++;
    }
    if (*cptr1 == 'C')
      return false;

    if (*cptr2 == 'V') {
      if (*cptr1 == 'V')
        cptr1++;
      cptr2++;
    }
    if (*cptr1 == 'V')
      return false;
  }

  for (; *cptr1 == *cptr2; cptr1++, cptr2++) {
    if (*cptr1 == 0)
      return true;
  }

  return false;
}

namespace std {

extern void unexpected() { uhandler(); }

extern void terminate() { thandler(); }

static void duhandler() { terminate(); }

static void dthandler() { abort(); }

} // namespace std
