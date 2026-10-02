/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/gx.h>
#include <dolphin/gx/GXPriv.h>

typedef struct GXLightObjInt {
  u32 padding[3];
  u32 color;
  float a0;
  float a1;
  float a2;
  float k0;
  float k1;
  float k2;
  float px;
  float py;
  float pz;
  float nx;
  float ny;
  float nz;
} GXLightObjInt;

extern float cosf(float x);
#define PI 3.14159265358979323846F



void GXInitLightAttn(GXLightObj *lt_obj, f32 a0, f32 a1, f32 a2, f32 k0, f32 k1, f32 k2) {
  GXLightObjInt *obj = (GXLightObjInt *)lt_obj;
  obj->a0 = a0;
  obj->a1 = a1;
  obj->a2 = a2;
  obj->k0 = k0;
  obj->k1 = k1;
  obj->k2 = k2;
}

void GXInitLightAttnA(GXLightObj *lt_obj, f32 a0, f32 a1, f32 a2) {
  GXLightObjInt *obj = (GXLightObjInt *)lt_obj;
  obj->a0 = a0;
  obj->a1 = a1;
  obj->a2 = a2;
}

void GXInitLightSpot(GXLightObj *lt_obj, f32 cutoff, GXSpotFn spot_func)
{
    float a0, a1, a2;
    float r;
    float d;
    float cr;
    GXLightObjInt *obj;


    obj = (GXLightObjInt *)lt_obj;


    if (cutoff <= 0.0f || cutoff > 90.0f)
        spot_func = GX_SP_OFF;

    r = (3.1415927f * cutoff) / 180.0f;
    cr = cosf(r);
    switch (spot_func) {
    case GX_SP_FLAT:
        a0 = -1000.0f * cr;
        a1 = 1000.0f;
        a2 = 0.0f;
        break;
    case GX_SP_COS:
        a0 = -cr / (1.0f - cr);
        a1 = 1.0f / (1.0f - cr);
        a2 = 0.0f;
        break;
    case GX_SP_COS2:
        a0 = 0.0f;
        a1 = -cr / (1.0f - cr);
        a2 = 1.0f / (1.0f - cr);
        break;
    case GX_SP_SHARP:
        d = (1.0f - cr) * (1.0f - cr);
        a0 = (cr * (cr - 2.0f)) / d;
        a1 = 2.0f / d;
        a2 = -1.0f / d;
        break;
    case GX_SP_RING1:
        d = (1.0f - cr) * (1.0f - cr);
        a0 = (-4.0f * cr) / d;
        a1 = (4.0f * (1.0f + cr)) / d;
        a2 = -4.0f / d;
        break;
    case GX_SP_RING2:
        d = (1.0f - cr) * (1.0f - cr);
        a0 = 1.0f - ((2.0f * cr * cr) / d);
        a1 = (4.0f * cr) / d;
        a2 = -2.0f / d;
        break;
    case GX_SP_OFF:
    default:
        a0 = 1.0f;
        a1 = 0.0f;
        a2 = 0.0f;
        break;
    }
    obj->a0 = a0;
    obj->a1 = a1;
    obj->a2 = a2;
}


void GXInitLightDistAttn(GXLightObj *lt_obj, f32 ref_dist, f32 ref_br, GXDistAttnFn dist_func) {
  f32 k0, k1, k2;
  GXLightObjInt *obj = (GXLightObjInt *)lt_obj;

  if (ref_dist < 0.0F) {
    dist_func = GX_DA_OFF;
  }

  if (ref_br <= 0.0F || ref_br >= 1.0F) {
    dist_func = GX_DA_OFF;
  }

  switch (dist_func) {
  case GX_DA_GENTLE:
    k0 = 1.0F;
    k1 = (1.0F - ref_br) / (ref_br * ref_dist);
    k2 = 0.0F;
    break;
  case GX_DA_MEDIUM:
    k0 = 1.0f;
    k1 = 0.5f * (1.0f - ref_br) / (ref_br * ref_dist);
    k2 = 0.5f * (1.0f - ref_br) / (ref_br * ref_dist * ref_dist);
    break;
  case GX_DA_STEEP:
    k0 = 1.0f;
    k1 = 0.0f;
    k2 = (1.0f - ref_br) / (ref_br * ref_dist * ref_dist);
    break;
  case GX_DA_OFF:
  default:
    k0 = 1.0f;
    k1 = 0.0f;
    k2 = 0.0f;
    break;
  }

  obj->k0 = k0;
  obj->k1 = k1;
  obj->k2 = k2;
}

void GXInitLightPos(GXLightObj *lt_obj, f32 x, f32 y, f32 z) {
  GXLightObjInt *obj = (GXLightObjInt *)lt_obj;

  obj->px = x;
  obj->py = y;
  obj->pz = z;
}

void GXInitLightDir(GXLightObj *lt_obj, f32 nx, f32 ny, f32 nz) {
  GXLightObjInt *obj = (GXLightObjInt *)lt_obj;

  obj->nx = -nx;
  obj->ny = -ny;
  obj->nz = -nz;
}

void GXInitLightColor(GXLightObj *lt_obj, GXColor color)
{
    GXLightObjInt *obj;


    obj = (GXLightObjInt *)lt_obj;


    obj->color = (color.r << 24) | (color.g << 16) | (color.b << 8) | color.a;
}


void GXLoadLightObjImm(GXLightObj *lt_obj, GXLightID light)
{
    unsigned long addr;
    unsigned long idx;
    GXLightObjInt * obj;


    obj = (GXLightObjInt *)lt_obj;


    switch (light) {
    case GX_LIGHT0: idx = 0; break;
    case GX_LIGHT1: idx = 1; break;
    case GX_LIGHT2: idx = 2; break;
    case GX_LIGHT3: idx = 3; break;
    case GX_LIGHT4: idx = 4; break;
    case GX_LIGHT5: idx = 5; break;
    case GX_LIGHT6: idx = 6; break;
    case GX_LIGHT7: idx = 7; break;
    default:
        idx = 0;

        break;
    }

    addr = idx * 0x10 + 0x600;
    GX_WRITE_U8(0x10);
    GX_WRITE_U32(addr | 0xF0000);

    GX_WRITE_U32(0);
    GX_WRITE_U32(0);
    GX_WRITE_U32(0);
    GX_WRITE_U32(obj->color);
    GX_WRITE_F32(obj->a0);
    GX_WRITE_F32(obj->a1);
    GX_WRITE_F32(obj->a2);
    GX_WRITE_F32(obj->k0);
    GX_WRITE_F32(obj->k1);
    GX_WRITE_F32(obj->k2);
    GX_WRITE_F32(obj->px);
    GX_WRITE_F32(obj->py);
    GX_WRITE_F32(obj->pz);
    GX_WRITE_F32(obj->nx);
    GX_WRITE_F32(obj->ny);
    GX_WRITE_F32(obj->nz);

    __GXData->bpSentNot = 1;
}

void GXSetChanAmbColor(GXChannelID chan, GXColor amb_color)
{
    u32 reg = 0;
    u32 colIdx;
    u32 alpha;



    switch (chan) {
    case GX_COLOR0:
        alpha = __GXData->ambColor[0] & 0xFF;
        SET_REG_FIELD(reg, 8, 0, alpha);
        SET_REG_FIELD(reg, 8, 8, amb_color.b);
        SET_REG_FIELD(reg, 8, 16, amb_color.g);
        SET_REG_FIELD(reg, 8, 24, amb_color.r);
        colIdx = 0;
        break;
    case GX_COLOR1:
        alpha = __GXData->ambColor[1] & 0xFF;
        SET_REG_FIELD(reg, 8, 0, alpha);
        SET_REG_FIELD(reg, 8, 8, amb_color.b);
        SET_REG_FIELD(reg, 8, 16, amb_color.g);
        SET_REG_FIELD(reg, 8, 24, amb_color.r);
        colIdx = 1;
        break;
    case GX_ALPHA0:
        reg = __GXData->ambColor[0];
        SET_REG_FIELD(reg, 8, 0, amb_color.a);
        colIdx = 0;
        break;
    case GX_ALPHA1:
        reg = __GXData->ambColor[1];
        SET_REG_FIELD(reg, 8, 0, amb_color.a);
        colIdx = 1;
        break;
    case GX_COLOR0A0:
        SET_REG_FIELD(reg, 8, 0, amb_color.a);
        SET_REG_FIELD(reg, 8, 8, amb_color.b);
        SET_REG_FIELD(reg, 8, 16, amb_color.g);
        SET_REG_FIELD(reg, 8, 24, amb_color.r);
        colIdx = 0;
        break;
    case GX_COLOR1A1:
        SET_REG_FIELD(reg, 8, 0, amb_color.a);
        SET_REG_FIELD(reg, 8, 8, amb_color.b);
        SET_REG_FIELD(reg, 8, 16, amb_color.g);
        SET_REG_FIELD(reg, 8, 24, amb_color.r);
        colIdx = 1;
        break;
    default:

        return;
    }

    GX_WRITE_XF_REG(colIdx + 10, reg);
    __GXData->bpSentNot = 1;
    __GXData->ambColor[colIdx] = reg;
}

void GXSetChanMatColor(GXChannelID chan, GXColor mat_color)
{
    u32 reg = 0;
    u32 alpha;
    u32 colIdx;



    switch (chan) {
    case GX_COLOR0:
        alpha = __GXData->matColor[0] & 0xFF;
        SET_REG_FIELD(reg, 8, 0, alpha);
        SET_REG_FIELD(reg, 8, 8, mat_color.b);
        SET_REG_FIELD(reg, 8, 16, mat_color.g);
        SET_REG_FIELD(reg, 8, 24, mat_color.r);
        colIdx = 0;
        break;
    case GX_COLOR1:
        alpha = __GXData->matColor[1] & 0xFF;
        SET_REG_FIELD(reg, 8, 0, alpha);
        SET_REG_FIELD(reg, 8, 8, mat_color.b);
        SET_REG_FIELD(reg, 8, 16, mat_color.g);
        SET_REG_FIELD(reg, 8, 24, mat_color.r);
        colIdx = 1;
        break;
    case GX_ALPHA0:
        reg = __GXData->matColor[0];
        SET_REG_FIELD(reg, 8, 0, mat_color.a);
        colIdx = 0;
        break;
    case GX_ALPHA1:
        reg = __GXData->matColor[1];
        SET_REG_FIELD(reg, 8, 0, mat_color.a);
        colIdx = 1;
        break;
    case GX_COLOR0A0:
        SET_REG_FIELD(reg, 8, 0, mat_color.a);
        SET_REG_FIELD(reg, 8, 8, mat_color.b);
        SET_REG_FIELD(reg, 8, 16, mat_color.g);
        SET_REG_FIELD(reg, 8, 24, mat_color.r);
        colIdx = 0;
        break;
    case GX_COLOR1A1:
        SET_REG_FIELD(reg, 8, 0, mat_color.a);
        SET_REG_FIELD(reg, 8, 8, mat_color.b);
        SET_REG_FIELD(reg, 8, 16, mat_color.g);
        SET_REG_FIELD(reg, 8, 24, mat_color.r);
        colIdx = 1;
        break;
    default:

        return;
    }

    GX_WRITE_XF_REG(colIdx + 12, reg);
    __GXData->bpSentNot = 1;
    __GXData->matColor[colIdx] = reg;
}

void GXSetNumChans(u8 nChans) {
  SET_REG_FIELD(__GXData->genMode, 3, 4, nChans);
  GX_WRITE_XF_REG(9, nChans);
  __GXData->dirtyState |= 4;
}

void GXSetChanCtrl(GXChannelID chan, GXBool enable, GXColorSrc amb_src,
    GXColorSrc mat_src, u32 light_mask, GXDiffuseFn diff_fn, GXAttnFn attn_fn)
{
    u32 reg; // r31
    u32 idx; // r26





    if (chan == 4)
        idx = 0;
    else if (chan == 5)
        idx = 1;
    else
        idx = chan;

    reg = 0;
    SET_REG_FIELD(reg, 1, 1, enable);
    SET_REG_FIELD(reg, 1, 0, mat_src);
    SET_REG_FIELD(reg, 1, 6, amb_src);
    SET_REG_FIELD(reg, 1, 2, (light_mask & GX_LIGHT0) != 0);
    SET_REG_FIELD(reg, 1, 3, (light_mask & GX_LIGHT1) != 0);
    SET_REG_FIELD(reg, 1, 4, (light_mask & GX_LIGHT2) != 0);
    SET_REG_FIELD(reg, 1, 5, (light_mask & GX_LIGHT3) != 0);
    SET_REG_FIELD(reg, 1, 11, (light_mask & GX_LIGHT4) != 0);
    SET_REG_FIELD(reg, 1, 12, (light_mask & GX_LIGHT5) != 0);
    SET_REG_FIELD(reg, 1, 13, (light_mask & GX_LIGHT6) != 0);
    SET_REG_FIELD(reg, 1, 14, (light_mask & GX_LIGHT7) != 0);
    SET_REG_FIELD(reg, 2, 7, (attn_fn == 0) ? 0 : diff_fn);
    SET_REG_FIELD(reg, 1, 9, (attn_fn != 2));
    SET_REG_FIELD(reg, 1, 10, (attn_fn != 0));

    GX_WRITE_XF_REG(idx + 14, reg);
    __GXData->bpSentNot = 1;
    if (chan == GX_COLOR0A0) {
        GX_WRITE_XF_REG(16, reg);
    } else if (chan == GX_COLOR1A1) {
        GX_WRITE_XF_REG(17, reg);
    }
}
