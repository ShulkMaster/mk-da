#include "dolphin/os.h"

void ReportErrcode(s32 code)
{
  switch (code) {
  case 0:
    OSReport("HVQM4PLAYEREX_ERROR_NOERROR\n");
    break;
  case 1:
    OSReport("HVQM4PLAYEREX_ERROR_ALLOCHANDLE\n");
    break;
  case 2:
    OSReport("HVQM4PLAYEREX_ERROR_OPENFILE\n");
    break;
  case 3:
    OSReport("HVQM4PLAYEREX_ERROR_READHEADER\n");
    break;
  case 4:
    OSReport("HVQM4PLAYEREX_ERROR_VERSION\n");
    break;
  case 5:
    OSReport("HVQM4PLAYEREX_ERROR_ALLOCWORK\n");
    break;
  case 6:
    OSReport("HVQM4PLAYEREX_ERROR_ALLOCSTACK\n");
    break;
  case 7:
    OSReport("HVQM4PLAYEREX_ERROR_ALLOCRDBUF\n");
    break;
  case 8:
    OSReport("HVQM4PLAYEREX_ERROR_CREATETHREAD\n");
    break;
  case 9:
    OSReport("HVQM4PLAYEREX_ERROR_INITVBUF\n");
    break;
  case 10:
    OSReport("HVQM4PLAYEREX_ERROR_INITABUF\n");
    break;
  case 11:
    OSReport("HVQM4PLAYEREX_ERROR_INITADEC\n");
    break;
  case 12:
    OSReport("HVQM4PLAYEREX_ERROR_READERROR\n");
    break;
  case 13:
    OSReport("HVQM4PLAYEREX_ERROR_SHUTDOWN\n");
    break;
  default:
    OSReport("HVQM4PLAYEREX_ERROR_%d\n", code);
    break;
  }
}
