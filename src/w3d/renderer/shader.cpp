/**
 * @file
 *
 * @author OmniBlade
 * @author tomsons26
 * @author Tiberium Technologies
 *
 * @brief Base class for objects that use shaders.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "shader.h"
#include "gamedebug.h"
#include "wwstring.h"

// TODO move these to w3dfile once that file is written.
struct W3dRGBStruct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t pad;
};

struct W3dMaterial3Struct
{
    uint32_t attributes;
    W3dRGBStruct diffuse_color;
    W3dRGBStruct specular_color;
    W3dRGBStruct emissive_coeffs;
    W3dRGBStruct ambient_coeffs;
    W3dRGBStruct diffuse_coeffs;
    W3dRGBStruct specular_coeffs;
    float shininess;
    float opacity;
    float translucency;
    float fog_coeff;
};


// TODO these should be decoded into Shader_Const calls.
ShaderClass ShaderClass::s_presetOpaqueShader(0x9441B);
ShaderClass ShaderClass::s_presetAdditiveShader(0x94433);
ShaderClass ShaderClass::s_presetBumpenvmapShader(0x494C33);
ShaderClass ShaderClass::s_presetAlphaShader(0x984B3);
ShaderClass ShaderClass::s_presetMultiplicativeShader(0x90453);
ShaderClass ShaderClass::s_presetOpaque2DShader(0x94017);
ShaderClass ShaderClass::s_presetOpaqueSpriteShader(0x94013);
ShaderClass ShaderClass::s_presetAdditive2DShader(0x94037);
ShaderClass ShaderClass::s_presetAlpha2DShader(0x980B7);
ShaderClass ShaderClass::s_presetAdditiveSpriteShader(0x94033);
ShaderClass ShaderClass::s_presetAlphaSpriteShader(0x980B3);
ShaderClass ShaderClass::s_presetOpaqueSolidShader(0x8441B);
ShaderClass ShaderClass::s_presetAdditiveSolidShader(0x84433);
ShaderClass ShaderClass::s_presetAlphaSolidShader(0x884B3);
ShaderClass ShaderClass::s_presetATest2DShader(0xD4017);
ShaderClass ShaderClass::s_presetATestSpriteShader(0xD401B);
ShaderClass ShaderClass::s_presetATestBlend2DShader(0xD80B7);
ShaderClass ShaderClass::s_presetATestBlendSpriteShader(0xD80BB);
ShaderClass ShaderClass::s_presetScreen2DShader(0x94077);
ShaderClass ShaderClass::s_presetScreenSpriteShader(0x94073);
ShaderClass ShaderClass::s_presetMultiplicative2DShader(0x90057);
ShaderClass ShaderClass::s_presetMultiplicativeSpriteShader(0x90053);

#ifdef GAME_DLL
#include "hooker.h"
#else
bool ShaderClass::s_shaderDirty = false;
uint32_t ShaderClass::s_currentShader = 0;
uint32_t ShaderClass::s_polygonCullMode = 2;
#endif

/**
 * Init the shader bits based on a material struct.
 *
 * 0x00813420
 */
void ShaderClass::Init_From_Material3(const W3dMaterial3Struct &material)
{
    // TODO figure out what the attribute bits do.
    if (material.attributes & 1) {
        Set_Depth_Mask(DEPTH_WRITE_DISABLE);
        Set_Dst_Blend_Func(DSTBLEND_ONE_MINUS_SRC_ALPHA);
        Set_Src_Blend_Func(SRCBLEND_SRC_ALPHA);
    }
}

/**
 * Enable fog shading? Dunno where this is in windows binary yet.
 */
void ShaderClass::Enable_Fog(const char *source)
{
    switch (Get_Src_Blend_Func()) {
        case SRCBLEND_ZERO:
            if (Get_Dst_Blend_Func() == DSTBLEND_SRC_COLOR) {
                Set_Fog_Func(FOG_WHITE);
                return;
            }
            Report_Unable_To_Fog(source);
            break;
        case SRCBLEND_ONE:
            switch (Get_Dst_Blend_Func()) {
                case DSTBLEND_ZERO:
                    Set_Fog_Func(FOG_ENABLE);
                    return;
                case DSTBLEND_ONE:
                case DSTBLEND_ONE_MINUS_SRC_COLOR:
                    Set_Fog_Func(FOG_SCALE_FRAGMENT);
                    return;
            }
            Report_Unable_To_Fog(source);
            break;
        case SRCBLEND_SRC_ALPHA:
            if (Get_Dst_Blend_Func() == DSTBLEND_ONE_MINUS_SRC_ALPHA) {
                Set_Fog_Func(FOG_ENABLE);
                return;
            }
            Report_Unable_To_Fog(source);
            break;
        case SRCBLEND_ONE_MINUS_SRC_ALPHA:
            if (Get_Dst_Blend_Func() == DSTBLEND_SRC_ALPHA) {
                Set_Fog_Func(FOG_ENABLE);
                return;
            }
            Report_Unable_To_Fog(source);
            break;
        default:
            return;
    }
}

/**
 * Work out what sort category the current shader belongs to.
 *
 * 0x00813F80
 */
ShaderClass::StaticSortCategoryType ShaderClass::Get_Static_Sort_Category() const
{
    if (Get_Alpha_Test() == ALPHATEST_DISABLE && Get_Dst_Blend_Func() == DSTBLEND_ZERO) {
        return SSCAT_OPAQUE;
    }
    
    if (Get_Alpha_Test() == MASK_ALPHATEST) {
        if (Get_Dst_Blend_Func() == DSTBLEND_ZERO
            || (Get_Dst_Blend_Func() == DSTBLEND_ONE_MINUS_SRC_ALPHA) && Get_Src_Blend_Func() == SRCBLEND_SRC_ALPHA) {
            return SSCAT_ALPHA_TEST;
        }
    }

    if (Get_Src_Blend_Func() == SRCBLEND_ONE) {
        return SSCAT_OTHER2;
    }

    if (Get_Dst_Blend_Func() == DSTBLEND_ONE) {
        return SSCAT_ADDITIVE;
    }

    return Get_Dst_Blend_Func() == DSTBLEND_ONE_MINUS_SRC_COLOR ? SSCAT_OTHER : SSCAT_OTHER2;
}

/**
 * Work out what sort level the current shader belongs to.
 *
 * 0x00814010
 */
int ShaderClass::Guess_Sort_Level() const
{
    switch (Get_Static_Sort_Category()) {
        case SSCAT_OPAQUE: // Fallthrough.
        case SSCAT_ALPHA_TEST:
            return SORT_LEVEL_NONE;
        case SSCAT_ADDITIVE:
            return SORT_LEVEL_BIN3;
        case SSCAT_OTHER:
            return SORT_LEVEL_BIN2;
        default:
            return SORT_LEVEL_BIN1;
    }
}

/**
 * Debug function to get readout of what options are set in the current shader.
 */
void ShaderClass::Get_Description(StringClass &desc)
{
    desc = "";
    switch (Get_Depth_Compare()) {
        case PASS_NEVER:
            desc += "DEPTH_COMPARE:PASS_NEVER | ";
            break;
        case PASS_LESS:
            desc += "DEPTH_COMPARE:PASS_LESS | ";
            break;
        case PASS_EQUAL:
            desc += "DEPTH_COMPARE:PASS_EQUAL | ";
            break;
        case PASS_LEQUAL:
            desc += "DEPTH_COMPARE:PASS_LEQUAL | ";
            break;
        case PASS_GREATER:
            desc += "DEPTH_COMPARE:PASS_GREATER | ";
            break;
        case PASS_NOTEQUAL:
            desc += "DEPTH_COMPARE:PASS_NOTEQUAL | ";
            break;
        case PASS_GEQUAL:
            desc += "DEPTH_COMPARE:PASS_GEQUAL | ";
            break;
        case PASS_ALWAYS:
            desc += "DEPTH_COMPARE:PASS_ALWAYS | ";
            break;
        default:
            break;
    }

    switch (Get_Depth_Mask()) {
        case DEPTH_WRITE_DISABLE:
            desc += "DEPTH_WRITE_DISABLE | ";
            break;
        case DEPTH_WRITE_ENABLE:
            desc += "DEPTH_WRITE_ENABLE | ";
            break;
        default:
            break;
    }

    switch (Get_Color_Mask()) {
        case DEPTH_WRITE_DISABLE:
            desc += "COLOR_WRITE_DISABLE | ";
            break;
        case COLOR_WRITE_ENABLE:
            desc += "COLOR_WRITE_ENABLE | ";
            break;
        default:
            break;
    }

    switch (Get_Dst_Blend_Func()) {
        case DSTBLEND_ZERO:
            desc += "DSTBLEND_ZERO | ";
            break;
        case DSTBLEND_ONE:
            desc += "DSTBLEND_ONE | ";
            break;
        case DSTBLEND_SRC_COLOR:
            desc += "DSTBLEND_SRC_COLOR | ";
            break;
        case DSTBLEND_ONE_MINUS_SRC_COLOR:
            desc += "DSTBLEND_ONE_MINUS_SRC_COLOR | ";
            break;
        case DSTBLEND_SRC_ALPHA:
            desc += "DSTBLEND_SRC_ALPHA | ";
            break;
        case DSTBLEND_ONE_MINUS_SRC_ALPHA:
            desc += "DSTBLEND_ONE_MINUS_SRC_ALPHA | ";
            break;
        default:
            break;
    }

    switch (Get_Fog_Func()) {
        case FOG_DISABLE:
            desc += "FOG_DISABLE | ";
            break;
        case FOG_ENABLE:
            desc += "FOG_ENABLE | ";
            break;
        case FOG_SCALE_FRAGMENT:
            desc += "FOG_SCALE_FRAGMENT | ";
            break;
        case FOG_WHITE:
            desc += "FOG_WHITE | ";
            break;
        default:
            break;
    }

    switch (Get_Primary_Gradient()) {
        case GRADIENT_DISABLE:
            desc += "GRADIENT_DISABLE | ";
            break;
        case GRADIENT_MODULATE:
            desc += "GRADIENT_MODULATE | ";
            break;
        case GRADIENT_ADD:
            desc += "GRADIENT_ADD | ";
            break;
        case GRADIENT_BUMPENVMAP:
            desc += "GRADIENT_BUMPENVMAP | ";
            break;
        case GRADIENT_BUMPENVMAPLUMINANCE:
            desc += "GRADIENT_BUMPENVMAPLUMINANCE | ";
            break;
        case GRADIENT_MODULATE2X:
            desc += "GRADIENT_MODULATE2X | ";
            break;
        default:
            break;
    }

    switch (Get_Secondary_Gradient()) {
        case SECONDARY_GRADIENT_DISABLE:
            desc += "SECONDARY_GRADIENT_DISABLE | ";
            break;
        case SECONDARY_GRADIENT_ENABLE:
            desc += "SECONDARY_GRADIENT_ENABLE | ";
            break;
        default:
            break;
    }

    switch (Get_Src_Blend_Func()) {
        case SRCBLEND_ZERO:
            desc += "SRCBLEND_ZERO | ";
            break;
        case SRCBLEND_ONE:
            desc += "SRCBLEND_ONE | ";
            break;
        case SRCBLEND_SRC_ALPHA:
            desc += "SRCBLEND_SRC_ALPHA | ";
            break;
        case SRCBLEND_ONE_MINUS_SRC_ALPHA:
            desc += "SRCBLEND_ONE_MINUS_SRC_ALPHA | ";
            break;
        default:
            break;
    }

    switch (Get_Texturing()) {
        case TEXTURING_DISABLE:
            desc += "TEXTURING_DISABLE | ";
            break;
        case TEXTURING_ENABLE:
            desc += "TEXTURING_ENABLE | ";
            break;
        default:
            break;
    }

    switch (Get_NPatch_Enable()) {
        case NPATCH_DISABLE:
            desc += "NPATCH_DISABLE | ";
            break;
        case NPATCH_ENABLE:
            desc += "NPATCH_ENABLE | ";
            break;
        default:
            break;
    }

    switch (Get_Alpha_Test()) {
        case ALPHATEST_DISABLE:
            desc += "ALPHATEST_DISABLE | ";
            break;
        case ALPHATEST_ENABLE:
            desc += "ALPHATEST_ENABLE | ";
            break;
        default:
            break;
    }

    switch (Get_Cull_Mode()) {
        case CULL_MODE_DISABLE:
            desc += "CULL_MODE_DISABLE | ";
            break;
        case CULL_MODE_ENABLE:
            desc += "CULL_MODE_ENABLE | ";
            break;
        default:
            break;
    }

    switch (Get_Post_Detail_Color_Func()) {
        case DETAILCOLOR_DISABLE:
            desc += "DETAILCOLOR_DISABLE";
            break;
        case DETAILCOLOR_DETAIL:
            desc += "DETAILCOLOR_DETAIL";
            break;
        case DETAILCOLOR_SCALE:
            desc += "DETAILCOLOR_SCALE";
            break;
        case DETAILCOLOR_INVSCALE:
            desc += "DETAILCOLOR_INVSCALE";
            break;
        case DETAILCOLOR_ADD:
            desc += "DETAILCOLOR_ADD";
            break;
        case DETAILCOLOR_SUB:
            desc += "DETAILCOLOR_SUB";
            break;
        case DETAILCOLOR_SUBR:
            desc += "DETAILCOLOR_SUBR";
            break;
        case DETAILCOLOR_BLEND:
            desc += "DETAILCOLOR_BLEND";
            break;
        case DETAILCOLOR_DETAILBLEND:
            desc += "DETAILCOLOR_DETAILBLEND";
            break;
        case DETAILCOLOR_ADDSIGNED:
            desc += "DETAILCOLOR_ADDSIGNED";
            break;
        case DETAILCOLOR_ADDSIGNED2X:
            desc += "DETAILCOLOR_ADDSIGNED2X";
            break;
        case DETAILCOLOR_SCALE2X:
            desc += "DETAILCOLOR_SCALE2X";
            break;
        case DETAILCOLOR_MODALPHAADDCOLOR:
            desc += "DETAILCOLOR_MODALPHAADDCOLOR";
            break;
        default:
            return;
    }
}

/**
 * Sets the culling mode.
 *
 * 0x00813F60
 */
void ShaderClass::Invert_Backface_Culling(bool onoff)
{
    Invalidate();
    s_polygonCullMode = onoff ? 3 : 2;
}

/**
 * Logs inability to enable fog.
 */
void ShaderClass::Report_Unable_To_Fog(const char *source)
{
    DEBUG_LOG("WARNING: Unable to fog shader in %s with given blending mode.\n");
}

/**
 * Applies shader.
 *
 * 0x00813590
 */
void ShaderClass::Apply()
{
    // TODO Needs DX8Wrapper, DX8Caps.
#ifdef GAME_DLL
    Call_Method<void, ShaderClass>(PICK_ADDRESS(0x00813590, 0x004E22D0), this);
#endif
}
