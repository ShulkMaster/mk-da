/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

int TRKInitializeNub(void);
void TRKNubWelcome(void);
void TRKNubMainLoop(void);
int TRKTerminateNub(void);

static int TRK_mainError;

int TRK_main(void) {
  TRK_mainError = TRKInitializeNub();
  if (TRK_mainError == 0) {
    TRKNubWelcome();
    TRKNubMainLoop();
  }
  TRK_mainError = TRKTerminateNub();
  return TRK_mainError;
}
