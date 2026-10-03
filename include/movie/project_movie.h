#ifndef MKDA_MOVIE_PROJECT_MOVIE_H
#define MKDA_MOVIE_PROJECT_MOVIE_H

#include <dolphin/types.h>

struct _mwMemHeap;
struct _mwMovieFileFuncs;
enum mwMemFlags {};

void *operator new(unsigned long, _mwMemHeap *, mwMemFlags, const char *);
void operator delete(void *, _mwMemHeap *, mwMemFlags, const char *);
void operator delete(void *) throw();

class mwMoviePlayerGC {
public:
  mwMoviePlayerGC(_mwMemHeap &, char *, void *, _mwMovieFileFuncs *);
  ~mwMoviePlayerGC();
  int Play(int (*)());

  _mwMemHeap *unk00;
  char *unk04;
  _mwMovieFileFuncs *unk08;
  void *unk0C[2];
};

#endif
