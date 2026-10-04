/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/asm_sequences.inc>

void _savefpr_14(void);
void _savefpr_15(void);
void _savefpr_16(void);
void _savefpr_17(void);
void _savefpr_18(void);
void _savefpr_19(void);
void _savefpr_20(void);
void _savefpr_21(void);
void _savefpr_22(void);
void _savefpr_23(void);
void _savefpr_24(void);
void _savefpr_25(void);
void _savefpr_26(void);
void _savefpr_27(void);
void _savefpr_28(void);
void _savefpr_29(void);
void _savefpr_30(void);
void _savefpr_31(void);
void _restfpr_14(void);
void _restfpr_15(void);
void _restfpr_16(void);
void _restfpr_17(void);
void _restfpr_18(void);
void _restfpr_19(void);
void _restfpr_20(void);
void _restfpr_21(void);
void _restfpr_22(void);
void _restfpr_23(void);
void _restfpr_24(void);
void _restfpr_25(void);
void _restfpr_26(void);
void _restfpr_27(void);
void _restfpr_28(void);
void _restfpr_29(void);
void _restfpr_30(void);
void _restfpr_31(void);
void _savegpr_14(void);
void _savegpr_15(void);
void _savegpr_16(void);
void _savegpr_17(void);
void _savegpr_18(void);
void _savegpr_19(void);
void _savegpr_20(void);
void _savegpr_21(void);
void _savegpr_22(void);
void _savegpr_23(void);
void _savegpr_24(void);
void _savegpr_25(void);
void _savegpr_26(void);
void _savegpr_27(void);
void _savegpr_28(void);
void _savegpr_29(void);
void _savegpr_30(void);
void _savegpr_31(void);
void _restgpr_14(void);
void _restgpr_15(void);
void _restgpr_16(void);
void _restgpr_17(void);
void _restgpr_18(void);
void _restgpr_19(void);
void _restgpr_20(void);
void _restgpr_21(void);
void _restgpr_22(void);
void _restgpr_23(void);
void _restgpr_24(void);
void _restgpr_25(void);
void _restgpr_26(void);
void _restgpr_27(void);
void _restgpr_28(void);
void _restgpr_29(void);
void _restgpr_30(void);
void _restgpr_31(void);

static const unsigned long long __constants[] = {
  0x0000000000000000, // 0.0
  0x41F0000000000000, // 2**32
  0x41E0000000000000, // 2**31
};

asm unsigned long __cvt_fp2unsigned(register double d) { SEQ___cvt_fp2unsigned(); }

asm void __save_fpr(void) { SEQ___save_fpr(); }

asm void __restore_fpr(void) { SEQ___restore_fpr(); }

asm void __save_gpr(void) { SEQ___save_gpr(); }

asm void __restore_gpr(void) { SEQ___restore_gpr(); }

asm void __div2u(void) { SEQ___div2u(); }

asm void __div2i(void) { SEQ___div2i(); }

asm void __mod2u(void) { SEQ___mod2u(); }

asm void __shl2i(void) { SEQ___shl2i(); }

asm void __shr2u(void) { SEQ___shr2u(); }

asm void __shr2i(void) { SEQ___shr2i(); }

asm void __cvt_sll_flt(void) { SEQ___cvt_sll_flt(); }

asm unsigned long __cvt_dbl_usll(register double d) { SEQ___cvt_dbl_usll(); }
