/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

void TRKTargetSetStopped(int stopped);
void UnreserveEXI2Port(void);
void TRKSwapAndGo(void);
void ReserveEXI2Port(void);

int TRKTargetContinue(void) {
  TRKTargetSetStopped(0);
  UnreserveEXI2Port();
  TRKSwapAndGo();
  ReserveEXI2Port();
  return 0;
}
