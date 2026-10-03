#include "movie/project_movie.h"

extern "C" int movieplayer_fullscreen(_mwMemHeap *heap, char *name, void *arg2,
                                    _mwMovieFileFuncs *funcs, int (*checkExit)())
{
  mwMoviePlayerGC *player = new (heap, (mwMemFlags)0, "movie player instance")
      mwMoviePlayerGC(*heap, name, arg2, funcs);
  player->Play(checkExit);
  delete player;
  return 1;
}
