#ifndef _DOLPHIN_PPCARCH
#define _DOLPHIN_PPCARCH

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

u32 PPCMfmsr(void);
void PPCMtmsr(u32 newMSR);
u32 PPCMfhid0(void);
u32 PPCMfl2cr(void);
void PPCMtl2cr(u32 newL2cr);
void PPCMtdec(u32 newDec);
void PPCSync(void);
void PPCHalt(void);
u32 PPCMfhid2(void);
void PPCMthid2(u32 newhid2);
u32 PPCMfwpar(void);
void PPCMtwpar(u32 newwpar);

#ifdef __cplusplus
}
#endif

#endif
