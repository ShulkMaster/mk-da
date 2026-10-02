/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/types.h>

void OSReport(const char* msg);
u32 GetTRKConnected(void);
void SetTRKConnected(u32 connected);

void usr_put_initialize(void)
{
}

BOOL usr_puts_serial(const char* message)
{
  BOOL error = 0;
  char character;
  char buffer[2];

  while (!error && (character = *message++) != '\0') {
    BOOL connected = GetTRKConnected();

    buffer[0] = character;
    buffer[1] = '\0';
    SetTRKConnected(0);
    OSReport(buffer);
    SetTRKConnected(connected);
    error = 0;
  }
  return error;
}

