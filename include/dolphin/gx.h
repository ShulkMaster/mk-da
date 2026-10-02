#ifndef _DOLPHIN_GX
#define _DOLPHIN_GX

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef u8 GXBool;

typedef enum _GXCompare {
  GX_NEVER,
  GX_LESS,
  GX_EQUAL,
  GX_LEQUAL,
  GX_GREATER,
  GX_NEQUAL,
  GX_GEQUAL,
  GX_ALWAYS,
} GXCompare;

typedef enum _GXAlphaReadMode {
  GX_READ_00 = 0,
  GX_READ_FF = 1,
  GX_READ_NONE = 2,
} GXAlphaReadMode;

typedef enum _GXTexFmt {
  GX_TF_I4 = 0x0,
  GX_TF_I8 = 0x1,
  GX_TF_IA4 = 0x2,
  GX_TF_IA8 = 0x3,
  GX_TF_RGB565 = 0x4,
  GX_TF_RGB5A3 = 0x5,
  GX_TF_RGBA8 = 0x6,
  GX_TF_CMPR = 0xE,

  GX_CTF_R4 = 0x0 | 0x20,
  GX_CTF_RA4 = 0x2 | 0x20,
  GX_CTF_RA8 = 0x3 | 0x20,
  GX_CTF_YUVA8 = 0x6 | 0x20,
  GX_CTF_A8 = 0x7 | 0x20,
  GX_CTF_R8 = 0x8 | 0x20,
  GX_CTF_G8 = 0x9 | 0x20,
  GX_CTF_B8 = 0xA | 0x20,
  GX_CTF_RG8 = 0xB | 0x20,
  GX_CTF_GB8 = 0xC | 0x20,

  GX_TF_Z8 = 0x1 | 0x10,
  GX_TF_Z16 = 0x3 | 0x10,
  GX_TF_Z24X8 = 0x6 | 0x10,

  GX_CTF_Z4 = 0x0 | 0x10 | 0x20,
  GX_CTF_Z8M = 0x9 | 0x10 | 0x20,
  GX_CTF_Z8L = 0xA | 0x10 | 0x20,
  GX_CTF_Z16L = 0xC | 0x10 | 0x20,

  GX_TF_A8 = GX_CTF_A8,
} GXTexFmt;

typedef struct _GXTexObj {
  u32 dummy[8];
} GXTexObj;

typedef struct _GXLightObj {
  u32 dummy[16];
} GXLightObj;

typedef enum _GXAlphaOp {
  GX_AOP_AND,
  GX_AOP_OR,
  GX_AOP_XOR,
  GX_AOP_XNOR,
  GX_MAX_ALPHAOP,
} GXAlphaOp;

typedef enum _GXZFmt16 {
  GX_ZC_LINEAR,
  GX_ZC_NEAR,
  GX_ZC_MID,
  GX_ZC_FAR,
} GXZFmt16;

typedef enum _GXPixelFmt {
  GX_PF_RGB8_Z24,
  GX_PF_RGBA6_Z24,
  GX_PF_RGB565_Z16,
  GX_PF_Z24,
  GX_PF_Y8,
  GX_PF_U8,
  GX_PF_V8,
  GX_PF_YUV420,
} GXPixelFmt;

typedef enum _GXAttrType {
  GX_NONE,
  GX_DIRECT,
  GX_INDEX8,
  GX_INDEX16,
} GXAttrType;

typedef enum _GXTexMapID {
  GX_TEXMAP0,
  GX_TEXMAP1,
  GX_TEXMAP2,
  GX_TEXMAP3,
  GX_TEXMAP4,
  GX_TEXMAP5,
  GX_TEXMAP6,
  GX_TEXMAP7,
  GX_MAX_TEXMAP,
  GX_TEXMAP_NULL = 0xFF,
  GX_TEX_DISABLE = 0x100,
} GXTexMapID;

typedef enum _GXTexCoordID {
  GX_TEXCOORD0,
  GX_TEXCOORD1,
  GX_TEXCOORD2,
  GX_TEXCOORD3,
  GX_TEXCOORD4,
  GX_TEXCOORD5,
  GX_TEXCOORD6,
  GX_TEXCOORD7,
  GX_MAX_TEXCOORD,
  GX_TEXCOORD_NULL = 0xFF,
} GXTexCoordID;

typedef enum _GXTevStageID {
  GX_TEVSTAGE0,
  GX_TEVSTAGE1,
  GX_TEVSTAGE2,
  GX_TEVSTAGE3,
  GX_TEVSTAGE4,
  GX_TEVSTAGE5,
  GX_TEVSTAGE6,
  GX_TEVSTAGE7,
  GX_TEVSTAGE8,
  GX_TEVSTAGE9,
  GX_TEVSTAGE10,
  GX_TEVSTAGE11,
  GX_TEVSTAGE12,
  GX_TEVSTAGE13,
  GX_TEVSTAGE14,
  GX_TEVSTAGE15,
  GX_MAX_TEVSTAGE,
} GXTevStageID;

typedef enum _GXChannelID {
  GX_COLOR0,
  GX_COLOR1,
  GX_ALPHA0,
  GX_ALPHA1,
  GX_COLOR0A0,
  GX_COLOR1A1,
  GX_COLOR_ZERO,
  GX_ALPHA_BUMP,
  GX_ALPHA_BUMPN,
  GX_COLOR_NULL = 0xFF,
} GXChannelID;

typedef enum _GXBlendMode {
  GX_BM_NONE,
  GX_BM_BLEND,
  GX_BM_LOGIC,
  GX_BM_SUBTRACT,
  GX_MAX_BLENDMODE,
} GXBlendMode;

typedef enum _GXBlendFactor {
  GX_BL_ZERO,
  GX_BL_ONE,
  GX_BL_SRCCLR,
  GX_BL_INVSRCCLR,
  GX_BL_SRCALPHA,
  GX_BL_INVSRCALPHA,
  GX_BL_DSTALPHA,
  GX_BL_INVDSTALPHA,
  GX_BL_DSTCLR = GX_BL_SRCCLR,
  GX_BL_INVDSTCLR = GX_BL_INVSRCCLR,
} GXBlendFactor;

typedef enum _GXLogicOp {
  GX_LO_CLEAR,
  GX_LO_AND,
  GX_LO_REVAND,
  GX_LO_COPY,
  GX_LO_INVAND,
  GX_LO_NOOP,
  GX_LO_XOR,
  GX_LO_OR,
  GX_LO_NOR,
  GX_LO_EQUIV,
  GX_LO_INV,
  GX_LO_REVOR,
  GX_LO_INVCOPY,
  GX_LO_INVOR,
  GX_LO_NAND,
  GX_LO_SET,
} GXLogicOp;

typedef enum _GXTevRegID {
  GX_TEVPREV,
  GX_TEVREG0,
  GX_TEVREG1,
  GX_TEVREG2,
  GX_MAX_TEVREG,
} GXTevRegID;

typedef enum _GXTexOffset {
  GX_TO_ZERO,
  GX_TO_SIXTEENTH,
  GX_TO_EIGHTH,
  GX_TO_FOURTH,
  GX_TO_HALF,
  GX_TO_ONE,
  GX_MAX_TEXOFFSET,
} GXTexOffset;

typedef enum _GXCullMode {
  GX_CULL_NONE,
  GX_CULL_FRONT,
  GX_CULL_BACK,
  GX_CULL_ALL,
} GXCullMode;

typedef enum _GXTevSwapSel {
  GX_TEV_SWAP0 = 0,
  GX_TEV_SWAP1,
  GX_TEV_SWAP2,
  GX_TEV_SWAP3,
  GX_MAX_TEVSWAP,
} GXTevSwapSel;

typedef enum _GXTevColorChan {
  GX_CH_RED = 0,
  GX_CH_GREEN,
  GX_CH_BLUE,
  GX_CH_ALPHA,
} GXTevColorChan;

typedef enum _GXTevColorArg {
  GX_CC_CPREV,
  GX_CC_APREV,
  GX_CC_C0,
  GX_CC_A0,
  GX_CC_C1,
  GX_CC_A1,
  GX_CC_C2,
  GX_CC_A2,
  GX_CC_TEXC,
  GX_CC_TEXA,
  GX_CC_RASC,
  GX_CC_RASA,
  GX_CC_ONE,
  GX_CC_HALF,
  GX_CC_KONST,
  GX_CC_ZERO,
} GXTevColorArg;

typedef enum _GXTevAlphaArg {
  GX_CA_APREV,
  GX_CA_A0,
  GX_CA_A1,
  GX_CA_A2,
  GX_CA_TEXA,
  GX_CA_RASA,
  GX_CA_KONST,
  GX_CA_ZERO,
} GXTevAlphaArg;

typedef enum _GXTevOp {
  GX_TEV_ADD = 0,
  GX_TEV_SUB = 1,
  GX_TEV_COMP_R8_GT = 8,
  GX_TEV_COMP_R8_EQ = 9,
  GX_TEV_COMP_GR16_GT = 10,
  GX_TEV_COMP_GR16_EQ = 11,
  GX_TEV_COMP_BGR24_GT = 12,
  GX_TEV_COMP_BGR24_EQ = 13,
  GX_TEV_COMP_RGB8_GT = 14,
  GX_TEV_COMP_RGB8_EQ = 15,
  GX_TEV_COMP_A8_GT = GX_TEV_COMP_RGB8_GT,
  GX_TEV_COMP_A8_EQ = GX_TEV_COMP_RGB8_EQ,
} GXTevOp;

typedef enum _GXTevBias {
  GX_TB_ZERO,
  GX_TB_ADDHALF,
  GX_TB_SUBHALF,
  GX_MAX_TEVBIAS,
} GXTevBias;

typedef enum _GXTevScale {
  GX_CS_SCALE_1,
  GX_CS_SCALE_2,
  GX_CS_SCALE_4,
  GX_CS_DIVIDE_2,
  GX_MAX_TEVSCALE,
} GXTevScale;

typedef enum _GXTevKColorSel {
  GX_TEV_KCSEL_8_8 = 0x00,
  GX_TEV_KCSEL_7_8 = 0x01,
  GX_TEV_KCSEL_6_8 = 0x02,
  GX_TEV_KCSEL_5_8 = 0x03,
  GX_TEV_KCSEL_4_8 = 0x04,
  GX_TEV_KCSEL_3_8 = 0x05,
  GX_TEV_KCSEL_2_8 = 0x06,
  GX_TEV_KCSEL_1_8 = 0x07,
  GX_TEV_KCSEL_1 = GX_TEV_KCSEL_8_8,
  GX_TEV_KCSEL_3_4 = GX_TEV_KCSEL_6_8,
  GX_TEV_KCSEL_1_2 = GX_TEV_KCSEL_4_8,
  GX_TEV_KCSEL_1_4 = GX_TEV_KCSEL_2_8,
  GX_TEV_KCSEL_K0 = 0x0C,
  GX_TEV_KCSEL_K1 = 0x0D,
  GX_TEV_KCSEL_K2 = 0x0E,
  GX_TEV_KCSEL_K3 = 0x0F,
  GX_TEV_KCSEL_K0_R = 0x10,
  GX_TEV_KCSEL_K1_R = 0x11,
  GX_TEV_KCSEL_K2_R = 0x12,
  GX_TEV_KCSEL_K3_R = 0x13,
  GX_TEV_KCSEL_K0_G = 0x14,
  GX_TEV_KCSEL_K1_G = 0x15,
  GX_TEV_KCSEL_K2_G = 0x16,
  GX_TEV_KCSEL_K3_G = 0x17,
  GX_TEV_KCSEL_K0_B = 0x18,
  GX_TEV_KCSEL_K1_B = 0x19,
  GX_TEV_KCSEL_K2_B = 0x1A,
  GX_TEV_KCSEL_K3_B = 0x1B,
  GX_TEV_KCSEL_K0_A = 0x1C,
  GX_TEV_KCSEL_K1_A = 0x1D,
  GX_TEV_KCSEL_K2_A = 0x1E,
  GX_TEV_KCSEL_K3_A = 0x1F,
} GXTevKColorSel;

typedef enum _GXTevKAlphaSel {
  GX_TEV_KASEL_8_8 = 0x00,
  GX_TEV_KASEL_7_8 = 0x01,
  GX_TEV_KASEL_6_8 = 0x02,
  GX_TEV_KASEL_5_8 = 0x03,
  GX_TEV_KASEL_4_8 = 0x04,
  GX_TEV_KASEL_3_8 = 0x05,
  GX_TEV_KASEL_2_8 = 0x06,
  GX_TEV_KASEL_1_8 = 0x07,
  GX_TEV_KASEL_1 = GX_TEV_KASEL_8_8,
  GX_TEV_KASEL_3_4 = GX_TEV_KASEL_6_8,
  GX_TEV_KASEL_1_2 = GX_TEV_KASEL_4_8,
  GX_TEV_KASEL_1_4 = GX_TEV_KASEL_2_8,
  GX_TEV_KASEL_K0_R = 0x10,
  GX_TEV_KASEL_K1_R = 0x11,
  GX_TEV_KASEL_K2_R = 0x12,
  GX_TEV_KASEL_K3_R = 0x13,
  GX_TEV_KASEL_K0_G = 0x14,
  GX_TEV_KASEL_K1_G = 0x15,
  GX_TEV_KASEL_K2_G = 0x16,
  GX_TEV_KASEL_K3_G = 0x17,
  GX_TEV_KASEL_K0_B = 0x18,
  GX_TEV_KASEL_K1_B = 0x19,
  GX_TEV_KASEL_K2_B = 0x1A,
  GX_TEV_KASEL_K3_B = 0x1B,
  GX_TEV_KASEL_K0_A = 0x1C,
  GX_TEV_KASEL_K1_A = 0x1D,
  GX_TEV_KASEL_K2_A = 0x1E,
  GX_TEV_KASEL_K3_A = 0x1F,
} GXTevKAlphaSel;

typedef enum _GXTevKColorID {
  GX_KCOLOR0 = 0,
  GX_KCOLOR1,
  GX_KCOLOR2,
  GX_KCOLOR3,
  GX_MAX_KCOLOR,
} GXTevKColorID;

typedef enum _GXZTexOp {
  GX_ZT_DISABLE,
  GX_ZT_ADD,
  GX_ZT_REPLACE,
  GX_MAX_ZTEXOP,
} GXZTexOp;

typedef enum _GXIndTexFormat {
  GX_ITF_8,
  GX_ITF_5,
  GX_ITF_4,
  GX_ITF_3,
  GX_MAX_ITFORMAT,
} GXIndTexFormat;

typedef enum _GXIndTexBiasSel {
  GX_ITB_NONE,
  GX_ITB_S,
  GX_ITB_T,
  GX_ITB_ST,
  GX_ITB_U,
  GX_ITB_SU,
  GX_ITB_TU,
  GX_ITB_STU,
  GX_MAX_ITBIAS,
} GXIndTexBiasSel;

typedef enum _GXIndTexAlphaSel {
  GX_ITBA_OFF,
  GX_ITBA_S,
  GX_ITBA_T,
  GX_ITBA_U,
  GX_MAX_ITBALPHA,
} GXIndTexAlphaSel;

typedef enum _GXIndTexMtxID {
  GX_ITM_OFF,
  GX_ITM_0,
  GX_ITM_1,
  GX_ITM_2,
  GX_ITM_S0 = 5,
  GX_ITM_S1,
  GX_ITM_S2,
  GX_ITM_T0 = 9,
  GX_ITM_T1,
  GX_ITM_T2,
} GXIndTexMtxID;

typedef enum _GXIndTexWrap {
  GX_ITW_OFF,
  GX_ITW_256,
  GX_ITW_128,
  GX_ITW_64,
  GX_ITW_32,
  GX_ITW_16,
  GX_ITW_0,
  GX_MAX_ITWRAP,
} GXIndTexWrap;

typedef enum _GXIndTexStageID {
  GX_INDTEXSTAGE0,
  GX_INDTEXSTAGE1,
  GX_INDTEXSTAGE2,
  GX_INDTEXSTAGE3,
  GX_MAX_INDTEXSTAGE,
} GXIndTexStageID;

typedef enum _GXIndTexScale {
  GX_ITS_1,
  GX_ITS_2,
  GX_ITS_4,
  GX_ITS_8,
  GX_ITS_16,
  GX_ITS_32,
  GX_ITS_64,
  GX_ITS_128,
  GX_ITS_256,
  GX_MAX_ITSCALE,
} GXIndTexScale;

typedef enum _GXPerf0 {
  GX_PERF0_VERTICES,
  GX_PERF0_CLIP_VTX,
  GX_PERF0_CLIP_CLKS,
  GX_PERF0_XF_WAIT_IN,
  GX_PERF0_XF_WAIT_OUT,
  GX_PERF0_XF_XFRM_CLKS,
  GX_PERF0_XF_LIT_CLKS,
  GX_PERF0_XF_BOT_CLKS,
  GX_PERF0_XF_REGLD_CLKS,
  GX_PERF0_XF_REGRD_CLKS,
  GX_PERF0_CLIP_RATIO,

  GX_PERF0_TRIANGLES,
  GX_PERF0_TRIANGLES_CULLED,
  GX_PERF0_TRIANGLES_PASSED,
  GX_PERF0_TRIANGLES_SCISSORED,
  GX_PERF0_TRIANGLES_0TEX,
  GX_PERF0_TRIANGLES_1TEX,
  GX_PERF0_TRIANGLES_2TEX,
  GX_PERF0_TRIANGLES_3TEX,
  GX_PERF0_TRIANGLES_4TEX,
  GX_PERF0_TRIANGLES_5TEX,
  GX_PERF0_TRIANGLES_6TEX,
  GX_PERF0_TRIANGLES_7TEX,
  GX_PERF0_TRIANGLES_8TEX,
  GX_PERF0_TRIANGLES_0CLR,
  GX_PERF0_TRIANGLES_1CLR,
  GX_PERF0_TRIANGLES_2CLR,

  GX_PERF0_QUAD_0CVG,
  GX_PERF0_QUAD_NON0CVG,
  GX_PERF0_QUAD_1CVG,
  GX_PERF0_QUAD_2CVG,
  GX_PERF0_QUAD_3CVG,
  GX_PERF0_QUAD_4CVG,
  GX_PERF0_AVG_QUAD_CNT,

  GX_PERF0_CLOCKS,
  GX_PERF0_NONE

} GXPerf0;

typedef enum _GXPerf1 {
  GX_PERF1_TEXELS,
  GX_PERF1_TX_IDLE,
  GX_PERF1_TX_REGS,
  GX_PERF1_TX_MEMSTALL,
  GX_PERF1_TC_CHECK1_2,
  GX_PERF1_TC_CHECK3_4,
  GX_PERF1_TC_CHECK5_6,
  GX_PERF1_TC_CHECK7_8,
  GX_PERF1_TC_MISS,

  GX_PERF1_VC_ELEMQ_FULL,
  GX_PERF1_VC_MISSQ_FULL,
  GX_PERF1_VC_MEMREQ_FULL,
  GX_PERF1_VC_STATUS7,
  GX_PERF1_VC_MISSREP_FULL,
  GX_PERF1_VC_STREAMBUF_LOW,
  GX_PERF1_VC_ALL_STALLS,
  GX_PERF1_VERTICES,

  GX_PERF1_FIFO_REQ,
  GX_PERF1_CALL_REQ,
  GX_PERF1_VC_MISS_REQ,
  GX_PERF1_CP_ALL_REQ,

  GX_PERF1_CLOCKS,
  GX_PERF1_NONE

} GXPerf1;

typedef struct _GXColor {
  u8 r;
  u8 g;
  u8 b;
  u8 a;
} GXColor;

typedef struct _GXColorS10 {
  s16 r;
  s16 g;
  s16 b;
  s16 a;
} GXColorS10;

typedef struct _GXFogAdjTable {
  u16 r[10];
} GXFogAdjTable;

typedef struct _GXTexRegion {
  u32 dummy[4];
} GXTexRegion;

typedef struct _GXTlutRegion {
  u32 dummy[4];
} GXTlutRegion;

void GXSetLineWidth(u8 width, GXTexOffset texOffsets);
void GXSetPointSize(u8 pointSize, GXTexOffset texOffsets);
void GXEnableTexOffsets(GXTexCoordID coord, u8 line_enable, u8 point_enable);
void GXSetCullMode(GXCullMode mode);
void GXSetCoPlanar(GXBool enable);
void GXSetTevIndirect(GXTevStageID tev_stage, GXIndTexStageID ind_stage, GXIndTexFormat format,
                      GXIndTexBiasSel bias_sel, GXIndTexMtxID matrix_sel, GXIndTexWrap wrap_s,
                      GXIndTexWrap wrap_t, GXBool add_prev, GXBool utc_lod,
                      GXIndTexAlphaSel alpha_sel);
void GXSetIndTexCoordScale(GXIndTexStageID ind_state, GXIndTexScale scale_s,
                           GXIndTexScale scale_t);
void GXSetNumIndStages(u8 nIndStages);
void GXSetTevDirect(GXTevStageID tev_stage);
void GXSetTevColorIn(GXTevStageID stage, GXTevColorArg a, GXTevColorArg b, GXTevColorArg c,
                     GXTevColorArg d);
void GXSetTevAlphaIn(GXTevStageID stage, GXTevAlphaArg a, GXTevAlphaArg b, GXTevAlphaArg c,
                     GXTevAlphaArg d);
void GXSetTevColorOp(GXTevStageID stage, GXTevOp op, GXTevBias bias, GXTevScale scale, GXBool clamp,
                     GXTevRegID out_reg);
void GXSetTevAlphaOp(GXTevStageID stage, GXTevOp op, GXTevBias bias, GXTevScale scale, GXBool clamp,
                     GXTevRegID out_reg);
void GXSetTevColor(GXTevRegID id, GXColor color);
void GXSetTevColorS10(GXTevRegID id, GXColorS10 color);
void GXSetTevKColor(GXTevKColorID id, GXColor color);
void GXSetTevKColorSel(GXTevStageID stage, GXTevKColorSel sel);
void GXSetTevKAlphaSel(GXTevStageID stage, GXTevKAlphaSel sel);
void GXSetTevSwapMode(GXTevStageID stage, GXTevSwapSel ras_sel, GXTevSwapSel tex_sel);
void GXSetTevSwapModeTable(GXTevSwapSel table, GXTevColorChan red, GXTevColorChan green,
                           GXTevColorChan blue, GXTevColorChan alpha);
void GXSetAlphaCompare(GXCompare comp0, u8 ref0, GXAlphaOp op, GXCompare comp1, u8 ref1);
void GXSetZTexture(GXZTexOp op, GXTexFmt fmt, u32 bias);
void GXSetTevOrder(GXTevStageID stage, GXTexCoordID coord, GXTexMapID map, GXChannelID color);
void GXSetNumTevStages(u8 nStages);
void GXSetFogRangeAdj(GXBool enable, u16 center, const GXFogAdjTable *table);
void GXSetBlendMode(GXBlendMode type, GXBlendFactor src_factor,
                    GXBlendFactor dst_factor, GXLogicOp op);
void GXSetColorUpdate(GXBool update_enable);
void GXSetAlphaUpdate(GXBool update_enable);
void GXSetZMode(GXBool compare_enable, GXCompare func, GXBool update_enable);
void GXSetZCompLoc(GXBool before_tex);
void GXSetPixelFmt(GXPixelFmt pix_fmt, GXZFmt16 z_fmt);
void GXSetDither(GXBool dither);
void GXSetDstAlpha(GXBool enable, u8 alpha);
void GXSetFieldMask(GXBool odd_mask, GXBool even_mask);
void GXSetFieldMode(GXBool field_mode, GXBool half_aspect_ratio);

typedef enum _GXGamma {
  GX_GM_1_0,
  GX_GM_1_7,
  GX_GM_2_2,
} GXGamma;

typedef enum _GXFBClamp {
  GX_CLAMP_NONE = 0,
  GX_CLAMP_TOP = 1,
  GX_CLAMP_BOTTOM = 2,
} GXFBClamp;

typedef enum _GXCopyMode {
  GX_COPY_PROGRESSIVE = 0,
  GX_COPY_INTLC_EVEN = 2,
  GX_COPY_INTLC_ODD = 3,
} GXCopyMode;

typedef enum _VIXFBMode {
  VI_XFBMODE_SF = 0,
  VI_XFBMODE_DF = 1,
} VIXFBMode;

typedef struct _GXRenderModeObj {
  u32 viTVmode;
  u16 fbWidth;
  u16 efbHeight;
  u16 xfbHeight;
  u16 viXOrigin;
  u16 viYOrigin;
  u16 viWidth;
  u16 viHeight;
  VIXFBMode xFBmode;
  u8 field_rendering;
  u8 aa;
  u8 sample_pattern[12][2];
  u8 vfilter[7];
} GXRenderModeObj;

void GXAdjustForOverscan(const GXRenderModeObj *rIn, GXRenderModeObj *rOut, u16 horiz, u16 vert);

void GXSetDispCopySrc(u16 left, u16 top, u16 wd, u16 ht);

void GXSetTexCopySrc(u16 left, u16 top, u16 wd, u16 ht);

void GXSetDispCopyDst(u16 wd, u16 ht);

void GXSetTexCopyDst(u16 wd, u16 ht, GXTexFmt fmt, GXBool mipmap);

void GXSetDispCopyFrame2Field(GXCopyMode mode);

void GXSetCopyClamp(GXFBClamp clamp);

u32 GXSetDispCopyYScale(f32 vscale);

void GXSetCopyClear(GXColor clear_clr, u32 clear_z);

void GXSetCopyFilter(GXBool aa, const u8 sample_pattern[12][2], GXBool vf, const u8 vfilter[7]);

void GXSetDispCopyGamma(GXGamma gamma);

void GXCopyDisp(void *dest, GXBool clear);

void GXCopyTex(void *dest, GXBool clear);

void GXClearBoundingBox(void);

typedef struct _GXTlutObj {
  u32 dummy[3];
} GXTlutObj;

typedef GXTexRegion* (*GXTexRegionCallback)(const GXTexObj* texObj, GXTexMapID id);
typedef GXTlutRegion* (*GXTlutRegionCallback)(u32 tlut);

void GXSetGPMetric(GXPerf0 perf0, GXPerf1 perf1);

void GXPokeBlendMode(GXBlendMode type, GXBlendFactor src_factor, GXBlendFactor dst_factor,
                     GXLogicOp op);

void GXPokeColorUpdate(GXBool update_enable);

void GXPokeDstAlpha(GXBool enable, u8 alpha);

void GXPokeDither(GXBool dither);

void GXPokeZMode(GXBool compare_enable, GXCompare func, GXBool update_enable);

typedef enum _GXVtxFmt {
  GX_VTXFMT0,
  GX_VTXFMT1,
  GX_VTXFMT2,
  GX_VTXFMT3,
  GX_VTXFMT4,
  GX_VTXFMT5,
  GX_VTXFMT6,
  GX_VTXFMT7,
  GX_MAX_VTXFMT,
} GXVtxFmt;

typedef enum _GXAttr {
  GX_VA_PNMTXIDX,
  GX_VA_TEX0MTXIDX,
  GX_VA_TEX1MTXIDX,
  GX_VA_TEX2MTXIDX,
  GX_VA_TEX3MTXIDX,
  GX_VA_TEX4MTXIDX,
  GX_VA_TEX5MTXIDX,
  GX_VA_TEX6MTXIDX,
  GX_VA_TEX7MTXIDX,
  GX_VA_POS,
  GX_VA_NRM,
  GX_VA_CLR0,
  GX_VA_CLR1,
  GX_VA_TEX0,
  GX_VA_TEX1,
  GX_VA_TEX2,
  GX_VA_TEX3,
  GX_VA_TEX4,
  GX_VA_TEX5,
  GX_VA_TEX6,
  GX_VA_TEX7,
  GX_POS_MTX_ARRAY,
  GX_NRM_MTX_ARRAY,
  GX_TEX_MTX_ARRAY,
  GX_LIGHT_ARRAY,
  GX_VA_NBT,
  GX_VA_MAX_ATTR,
  GX_VA_NULL = 0xFF,
} GXAttr;

typedef enum _GXCITexFmt {
  GX_TF_C4 = 0x8,
  GX_TF_C8 = 0x9,
  GX_TF_C14X2 = 0xa,
} GXCITexFmt;

typedef enum _GXTexWrapMode {
  GX_CLAMP,
  GX_REPEAT,
  GX_MIRROR,
  GX_MAX_TEXWRAPMODE,
} GXTexWrapMode;

typedef enum _GXTexFilter {
  GX_NEAR,
  GX_LINEAR,
  GX_NEAR_MIP_NEAR,
  GX_LIN_MIP_NEAR,
  GX_NEAR_MIP_LIN,
  GX_LIN_MIP_LIN,
} GXTexFilter;

typedef enum _GXAnisotropy {
  GX_ANISO_1,
  GX_ANISO_2,
  GX_ANISO_4,
  GX_MAX_ANISOTROPY,
} GXAnisotropy;

typedef enum _GXTexGenType {
  GX_TG_MTX3x4,
  GX_TG_MTX2x4,
  GX_TG_BUMP0,
  GX_TG_BUMP1,
  GX_TG_BUMP2,
  GX_TG_BUMP3,
  GX_TG_BUMP4,
  GX_TG_BUMP5,
  GX_TG_BUMP6,
  GX_TG_BUMP7,
  GX_TG_SRTG,
} GXTexGenType;

typedef enum _GXTexGenSrc {
  GX_TG_POS,
  GX_TG_NRM,
  GX_TG_BINRM,
  GX_TG_TANGENT,
  GX_TG_TEX0,
  GX_TG_TEX1,
  GX_TG_TEX2,
  GX_TG_TEX3,
  GX_TG_TEX4,
  GX_TG_TEX5,
  GX_TG_TEX6,
  GX_TG_TEX7,
  GX_TG_TEXCOORD0,
  GX_TG_TEXCOORD1,
  GX_TG_TEXCOORD2,
  GX_TG_TEXCOORD3,
  GX_TG_TEXCOORD4,
  GX_TG_TEXCOORD5,
  GX_TG_TEXCOORD6,
  GX_TG_COLOR0,
  GX_TG_COLOR1,
  GX_MAX_TEXGENSRC,
} GXTexGenSrc;

typedef enum _GXCompCnt {
  GX_POS_XY = 0,
  GX_POS_XYZ = 1,
  GX_NRM_XYZ = 0,
  GX_NRM_NBT = 1,
  GX_NRM_NBT3 = 2,
  GX_CLR_RGB = 0,
  GX_CLR_RGBA = 1,
  GX_TEX_S = 0,
  GX_TEX_ST = 1,
} GXCompCnt;

typedef enum _GXCompType {
  GX_U8 = 0,
  GX_S8 = 1,
  GX_U16 = 2,
  GX_S16 = 3,
  GX_F32 = 4,
  GX_RGB565 = 0,
  GX_RGB8 = 1,
  GX_RGBX8 = 2,
  GX_RGBA4 = 3,
  GX_RGBA6 = 4,
  GX_RGBA8 = 5,
} GXCompType;

typedef enum _GXDiffuseFn {
  GX_DF_NONE,
  GX_DF_SIGN,
  GX_DF_CLAMP,
} GXDiffuseFn;

typedef enum _GXColorSrc {
  GX_SRC_REG,
  GX_SRC_VTX,
} GXColorSrc;

typedef enum _GXAttnFn {
  GX_AF_SPEC,
  GX_AF_SPOT,
  GX_AF_NONE,
} GXAttnFn;

typedef enum _GXLightID {
  GX_LIGHT0 = 0x001,
  GX_LIGHT1 = 0x002,
  GX_LIGHT2 = 0x004,
  GX_LIGHT3 = 0x008,
  GX_LIGHT4 = 0x010,
  GX_LIGHT5 = 0x020,
  GX_LIGHT6 = 0x040,
  GX_LIGHT7 = 0x080,
  GX_MAX_LIGHT = 0x100,
  GX_LIGHT_NULL = 0,
} GXLightID;

typedef enum _GXSpotFn {
  GX_SP_OFF,
  GX_SP_FLAT,
  GX_SP_COS,
  GX_SP_COS2,
  GX_SP_SHARP,
  GX_SP_RING1,
  GX_SP_RING2,
} GXSpotFn;

typedef enum _GXDistAttnFn {
  GX_DA_OFF,
  GX_DA_GENTLE,
  GX_DA_MEDIUM,
  GX_DA_STEEP,
} GXDistAttnFn;

typedef enum _GXTlut {
  GX_TLUT0 = 0,
  GX_TLUT1 = 1,
  GX_TLUT2 = 2,
  GX_TLUT3 = 3,
  GX_TLUT4 = 4,
  GX_TLUT5 = 5,
  GX_TLUT6 = 6,
  GX_TLUT7 = 7,
  GX_TLUT8 = 8,
  GX_TLUT9 = 9,
  GX_TLUT10 = 10,
  GX_TLUT11 = 11,
  GX_TLUT12 = 12,
  GX_TLUT13 = 13,
  GX_TLUT14 = 14,
  GX_TLUT15 = 15,
  GX_BIGTLUT0 = 16,
  GX_BIGTLUT1 = 17,
  GX_BIGTLUT2 = 18,
  GX_BIGTLUT3 = 19,
} GXTlut;

typedef enum _GXTlutSize {
  GX_TLUT_16 = 1,
  GX_TLUT_32 = 2,
  GX_TLUT_64 = 4,
  GX_TLUT_128 = 8,
  GX_TLUT_256 = 16,
  GX_TLUT_512 = 32,
  GX_TLUT_1K = 64,
  GX_TLUT_2K = 128,
  GX_TLUT_4K = 256,
  GX_TLUT_8K = 512,
  GX_TLUT_16K = 1024,
} GXTlutSize;

typedef enum _GXTlutFmt {
  GX_TL_IA8,
  GX_TL_RGB565,
  GX_TL_RGB5A3,
  GX_MAX_TLUTFMT,
} GXTlutFmt;

typedef enum _GXTexCacheSize {
  GX_TEXCACHE_32K,
  GX_TEXCACHE_128K,
  GX_TEXCACHE_512K,
  GX_TEXCACHE_NONE
} GXTexCacheSize;

void GXSetVtxDesc(GXAttr attr, GXAttrType type);

void __GXSetVCD(void);

void GXClearVtxDesc(void);

void GXSetVtxAttrFmt(GXVtxFmt vtxfmt, GXAttr attr, GXCompCnt cnt, GXCompType type, u8 frac);

void __GXSetVAT(void);

void GXSetArray(GXAttr attr, const void *base_ptr, u8 stride);

void GXSetTexCoordGen2(GXTexCoordID dst_coord, GXTexGenType func, GXTexGenSrc src_param, u32 mtx,
                       GXBool normalize, u32 pt_texmtx);

void GXSetNumTexGens(u8 nTexGens);

void __GetImageTileCount(GXTexFmt fmt, u16 wd, u16 ht, u32 *rowTiles, u32 *colTiles,
                         u32 *cmpTiles);

void GXInitTexObj(GXTexObj *obj, const void *image_ptr, u16 width, u16 height, u32 format,
                  GXTexWrapMode wrap_s, GXTexWrapMode wrap_t, GXBool mipmap);

void GXInitTexObjCI(GXTexObj *obj, const void *image_ptr, u16 width, u16 height, GXCITexFmt format,
                    GXTexWrapMode wrap_s, GXTexWrapMode wrap_t, GXBool mipmap, u32 tlut_name);

void GXInitTexObjLOD(GXTexObj *obj, GXTexFilter min_filt, GXTexFilter mag_filt, f32 min_lod,
                     f32 max_lod, f32 lod_bias, u8 bias_clamp, u8 do_edge_lod,
                     GXAnisotropy max_aniso);

void GXLoadTexObjPreLoaded(GXTexObj *obj, GXTexRegion *region, GXTexMapID id);

void GXLoadTexObj(GXTexObj *obj, GXTexMapID id);

void GXInitTlutObj(GXTlutObj *tlut_obj, void *lut, GXTlutFmt fmt, u16 n_entries);

void GXLoadTlut(const GXTlutObj *tlut_obj, GXTlut tlut_name);

void GXInitTexCacheRegion(GXTexRegion *region, u8 is_32b_mipmap, u32 tmem_even,
                          GXTexCacheSize size_even, u32 tmem_odd, GXTexCacheSize size_odd);

void GXInitTlutRegion(GXTlutRegion *region, u32 tmem_addr, GXTlutSize tlut_size);

void GXInvalidateTexAll(void);

GXTexRegionCallback GXSetTexRegionCallback(GXTexRegionCallback f);

GXTlutRegionCallback GXSetTlutRegionCallback(GXTlutRegionCallback f);

void __GXSetSUTexRegs(void);

void __GXSetTmemConfig(u32 config);

void GXInitLightSpot(GXLightObj *lt_obj, f32 cutoff, GXSpotFn spot_func);

void GXInitLightDistAttn(GXLightObj *lt_obj, f32 ref_dist, f32 ref_br, GXDistAttnFn dist_func);

void GXInitLightPos(GXLightObj *lt_obj, f32 x, f32 y, f32 z);

void GXInitLightDir(GXLightObj *lt_obj, f32 nx, f32 ny, f32 nz);

void GXInitLightColor(GXLightObj *lt_obj, GXColor color);

void GXLoadLightObjImm(GXLightObj *lt_obj, GXLightID light);

void GXSetChanAmbColor(GXChannelID chan, GXColor amb_color);

void GXSetChanMatColor(GXChannelID chan, GXColor mat_color);

void GXSetNumChans(u8 nChans);

void GXSetChanCtrl(GXChannelID chan, GXBool enable, GXColorSrc amb_src,
    GXColorSrc mat_src, u32 light_mask, GXDiffuseFn diff_fn, GXAttnFn attn_fn);

typedef enum _GXProjectionType {
  GX_PERSPECTIVE,
  GX_ORTHOGRAPHIC,
} GXProjectionType;

typedef enum _GXPrimitive {
  GX_QUADS = 0x80,
  GX_TRIANGLES = 0x90,
  GX_TRIANGLESTRIP = 0x98,
  GX_TRIANGLEFAN = 0xA0,
  GX_LINES = 0xA8,
  GX_LINESTRIP = 0xB0,
  GX_POINTS = 0xB8,
} GXPrimitive;

typedef enum _GXClipMode {
  GX_CLIP_ENABLE = 0,
  GX_CLIP_DISABLE = 1,
} GXClipMode;

typedef f32 Mtx44[4][4];

void GXSetProjection(const Mtx44 proj, GXProjectionType type);

void GXSetProjectionv(const f32 *proj);

void GXGetProjectionv(f32 *ptr);

void GXSetCurrentMtx(u32 id);

void GXSetViewportJitter(f32 left, f32 top, f32 wd, f32 ht, f32 nearz, f32 farz, u32 field);

void GXSetViewport(f32 left, f32 top, f32 width, f32 height, f32 nearZ, f32 farZ);

void GXGetViewportv(f32 *vp);

void GXSetScissor(u32 left, u32 top, u32 wd, u32 ht);

void GXSetScissorBoxOffset(s32 x_off, s32 y_off);

void GXSetClipMode(GXClipMode mode);

void __GXSetMatrixIndex(GXAttr matIdxAttr);

void __GXSetDirtyState(void);

void GXBegin(GXPrimitive type, GXVtxFmt vtxfmt, u16 nverts);

void __GXSendFlushPrim(void);

void __GXSetGenMode(void);

void __GXUpdateBPMask(void);

void __GXFlushTextureState(void);

void GXPokeAlphaMode(GXCompare func, u8 threshold);
void GXPokeAlphaRead(GXAlphaReadMode mode);
void GXPokeAlphaUpdate(GXBool update_enable);
void GXInitLightAttn(GXLightObj* lt_obj, f32 a0, f32 a1, f32 a2, f32 k0, f32 k1, f32 k2);
void GXInitLightAttnA(GXLightObj* lt_obj, f32 a0, f32 a1, f32 a2);
void GXInvalidateVtxCache(void);
GXTexFmt GXGetTexObjFmt(const GXTexObj* to);
void GXClearGPMetric(void);

#ifdef __cplusplus
}
#endif

#endif
