#include "movie/project_movie.h"

extern "C" int mpeg_fullscreen(void*, const char*, void**, int (*)(void));

/* TODO: [borked] 0.00%; placeholder stub, body not started. */
int mwMoviePlayerGC::Play(int (*checkExit)()) {
  mpeg_fullscreen(unk00, unk04, unk0C, checkExit);
  return 0;
}

mwMoviePlayerGC::~mwMoviePlayerGC() {}

mwMoviePlayerGC::mwMoviePlayerGC(_mwMemHeap& arg0, char* arg1, void* arg2, _mwMovieFileFuncs* arg3) {
  unk00 = &arg0;
  unk04 = arg1;
  unk08 = arg3;
  unk0C[0] = ((void**)arg2)[0];
  unk0C[1] = ((void**)arg2)[1];
}
