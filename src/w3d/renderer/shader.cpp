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
#include "dx8wrapper.h"
#include "w3d.h"
#include "w3d_file.h"
#include "wwstring.h"
#include <captainslog.h>

class DX8Caps;
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
StringClass &ShaderClass::Get_Description(StringClass &desc) const
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
            break;
    }

    return desc;
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
    captainslog_warn("Unable to fog shader in %s with given blending mode.");
}

#if BUILD_WITH_D3D8
struct Blend
{
    D3DBLEND func;
    bool useAlpha;
};
static Blend _srcBlendLUT[] = {
    { D3DBLEND_ZERO, false },
    { D3DBLEND_ONE, false },
    { D3DBLEND_SRCALPHA, true },
    { D3DBLEND_DESTCOLOR, true },
};

static Blend _dstBlendLUT[] = {
    { D3DBLEND_ZERO, false },
    { D3DBLEND_ONE, false },
    { D3DBLEND_SRCCOLOR, false },
    { D3DBLEND_INVSRCCOLOR, false },
    { D3DBLEND_SRCALPHA, true },
    { D3DBLEND_INVSRCALPHA, true },
};
#endif

inline unsigned long F2DW(float f)
{
    return *((unsigned *)&f);
}

// temp
bool log_shader_warnings = true;

/**
 * Applies shader.
 *
 * 0x00813590
 */
void ShaderClass::Apply()
{
    // TODO this needs a lot of very careful incrimental cleanup
#if BUILD_WITH_D3D8
    // to simplify usage of as MS used defines for these..
    // enum class D3DTEXARG
    enum D3DTEXARG
    {
        D3DTEXARG_SELECTMASK = 15,
        D3DTEXARG_DIFFUSE = 0,
        D3DTEXARG_CURRENT = 1,
        D3DTEXARG_TEXTURE = 2,
        D3DTEXARG_TFACTOR = 3,
        D3DTEXARG_SPECULAR = 4,
        D3DTEXARG_TEMP = 5,
        D3DTEXARG_COMPLEMENT = 16,
        D3DTEXARG_ALPHAREPLICATE = 32,
    };
    // to simplify usage of as MS used defines for these..
    // enum class D3DTEXTUREOPCAPS
    enum D3DTEXTUREOPCAPS
    {
        D3DTEXTUREOPCAPS_DISABLE = 0x00000001L,
        D3DTEXTUREOPCAPS_SELECTARG1 = 0x00000002L,
        D3DTEXTUREOPCAPS_SELECTARG2 = 0x00000004L,
        D3DTEXTUREOPCAPS_MODULATE = 0x00000008L,
        D3DTEXTUREOPCAPS_MODULATE2X = 0x00000010L,
        D3DTEXTUREOPCAPS_MODULATE4X = 0x00000020L,
        D3DTEXTUREOPCAPS_ADD = 0x00000040L,
        D3DTEXTUREOPCAPS_ADDSIGNED = 0x00000080L,
        D3DTEXTUREOPCAPS_ADDSIGNED2X = 0x00000100L,
        D3DTEXTUREOPCAPS_SUBTRACT = 0x00000200L,
        D3DTEXTUREOPCAPS_ADDSMOOTH = 0x00000400L,
        D3DTEXTUREOPCAPS_BLENDDIFFUSEALPHA = 0x00000800L,
        D3DTEXTUREOPCAPS_BLENDTEXTUREALPHA = 0x00001000L,
        D3DTEXTUREOPCAPS_BLENDFACTORALPHA = 0x00002000L,
        D3DTEXTUREOPCAPS_BLENDTEXTUREALPHAPM = 0x00004000L,
        D3DTEXTUREOPCAPS_BLENDCURRENTALPHA = 0x00008000L,
        D3DTEXTUREOPCAPS_PREMODULATE = 0x00010000L,
        D3DTEXTUREOPCAPS_MODULATEALPHA_ADDCOLOR = 0x00020000L,
        D3DTEXTUREOPCAPS_MODULATECOLOR_ADDALPHA = 0x00040000L,
        D3DTEXTUREOPCAPS_MODULATEINVALPHA_ADDCOLOR = 0x00080000L,
        D3DTEXTUREOPCAPS_MODULATEINVCOLOR_ADDALPHA = 0x00100000L,
        D3DTEXTUREOPCAPS_BUMPENVMAP = 0x00200000L,
        D3DTEXTUREOPCAPS_BUMPENVMAPLUMINANCE = 0x00400000L,
        D3DTEXTUREOPCAPS_DOTPRODUCT3 = 0x00800000L,
        D3DTEXTUREOPCAPS_MULTIPLYADD = 0x01000000L,
        D3DTEXTUREOPCAPS_LERP = 0x02000000L,
    };

    const DX8Caps *caps = DX8Wrapper::Get_Current_Caps();

    unsigned int texture_op_caps = caps->Get_Filter_Caps();

    unsigned int shader_bits_1;
    unsigned int shader_bits_2;

    if (s_shaderDirty) {
        shader_bits_1 = 0xFFFFFFFF;
        shader_bits_2 = 0xFFFFFFFF;
    } else {
        shader_bits_1 = s_currentShader ^ m_shaderBits;
        shader_bits_2 = s_currentShader ^ m_shaderBits;
        if (shader_bits_2 == 0) {
            return;
        }
    }

    s_currentShader = m_shaderBits;

    s_shaderDirty = false;

    if (shader_bits_1 & 0x4C0F0) {

        int src_blend;
        int dst_blend;

        if ((m_shaderBits & 0x10) == 0x10) {
            src_blend = _srcBlendLUT[Get_Src_Blend_Func()].func;
            dst_blend = _dstBlendLUT[Get_Dst_Blend_Func()].func;
        } else {
            src_blend = 1;
            dst_blend = 2;
        }

        BOOL alpha_blend = FALSE;

        if (src_blend != 2 || dst_blend != 1) {
            DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, src_blend);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, dst_blend);
            alpha_blend = 1;
        }

        DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, alpha_blend);

        BOOL alpha_test = FALSE;

        if ((m_shaderBits & 0x40000) == 0x40000) {

            if (src_blend == 6) {
                DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHAREF, 0x9F);
                DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHAFUNC, 4);
            } else {
                DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHAREF, 0x60);
                DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHAFUNC, 7);
            }
            alpha_test = 1;
        }

        DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHATESTENABLE, alpha_test);
        shader_bits_2 &= 0xFFFB3F0F;

        if (!shader_bits_2) {
            return;
        }
    }

    if (!(shader_bits_2 & 0x300)) {
        goto LABEL_37;
    }

    if (caps->Supports_Fog() && DX8Wrapper::Get_Fog_Enable()) {

        D3DCOLOR fog_color = DX8Wrapper::Get_Fog_Color();
        unsigned int fog = 0;

        switch ((m_shaderBits >> 8) & 3) {
            case 0u:
                fog = 0;
                break;
            case 1u:
                fog = 1;
                break;
            case 2u:
                fog_color = 0;
                fog = 1;
                break;
            case 3u:
                fog_color = 0xFFFFFF;
                fog = 1;
                break;
            default:
                break;
        }
        DX8Wrapper::Set_DX8_Render_State(D3DRS_FOGENABLE, fog);

        if (fog) {
            DX8Wrapper::Set_DX8_Render_State(D3DRS_FOGCOLOR, fog_color);
        }

    } else {
        DX8Wrapper::Set_DX8_Render_State(D3DRS_FOGENABLE, 0);
    }

    shader_bits_2 &= 0xFFFFFCFF;

    if (shader_bits_2) {
    LABEL_37:
        int color_op_1 = 2;
        int color_op_1_arg_1 = 0;
        int color_op_1_arg_2 = 0;
        int color_op_1_alpha_op = 2;
        int color_op_1_alpha_op_arg_1 = 0;
        int color_op_1_alpha_op_arg_2 = 0;
        int color_op_2 = 1;
        int color_op_2_arg_1 = 2;
        int color_op_2_arg_2 = 1;
        int color_op_2_alpha_op = 1;
        int color_op_2_alpha_op_arg_1 = 2;
        int color_op_2_alpha_op_arg_2 = 1;

        if (caps->Get_Vendor_Number() != 7) {
            goto LABEL_182;
        }

        bool set_something = true;

        if (caps->Get_Device_Number() != 2) {
        LABEL_182:
            set_something = false;
        }

        unsigned int shader_bits_4 = 0x11C00;
        unsigned int shader_bits_5 = 0x7F10000;

        if (set_something) {
            shader_bits_4 = 0x7F11C00;
            shader_bits_5 = 0x7F11C00;
        }

        unsigned int shader_bits_6 = shader_bits_2 & shader_bits_4;

        int tex_op_flags;

        if (shader_bits_2 & shader_bits_4) {

            unsigned int shader_bits_7 = m_shaderBits;

            if ((m_shaderBits & 0x10000) == 0x10000) {

                switch ((shader_bits_7 >> 0xA) & 7) {
                    case 0u:
                        color_op_1 = 2;
                        color_op_1_arg_1 = 2;
                        color_op_1_arg_2 = 1;
                        color_op_1_alpha_op = 2;
                        goto LABEL_66;
                    case 2u:
                        tex_op_flags = (texture_op_caps & 0x40) != 0 ? 3 : 0;
                        goto LABEL_53;
                    case 3u:
                        if (!(texture_op_caps & 0x200000)) {
                            goto LABEL_57;
                        }

                        color_op_1 = 0x16;
                        color_op_1_arg_1 = 2;
                        color_op_1_arg_2 = 0;
                        break;
                    case 4u:
                        if (!(texture_op_caps & 0x400000)) {
                        LABEL_57:
                            color_op_1 = 2;
                            color_op_1_arg_1 = 0;
                            color_op_1_arg_2 = 0;
                            color_op_1_alpha_op = 2;
                            color_op_1_alpha_op_arg_1 = 0;
                            color_op_1_alpha_op_arg_2 = 0;
                            goto LABEL_67;
                        }
                        color_op_1 = 0x17;
                        color_op_1_arg_1 = 2;
                        color_op_1_arg_2 = 0;
                        break;
                    case 5u:
                        tex_op_flags = (texture_op_caps & 5) != 0;
                    LABEL_53:
                        color_op_1 = tex_op_flags + 4;
                        goto LABEL_54;
                    default:
                        color_op_1 = 4;
                    LABEL_54:
                        color_op_1_arg_1 = 2;
                        color_op_1_arg_2 = 0;
                        color_op_1_alpha_op = 4;
                        color_op_1_alpha_op_arg_1 = 2;
                        color_op_1_alpha_op_arg_2 = 0;
                        goto LABEL_67;
                }

            } else {

                if ((shader_bits_7 >> 0xA) & 7) {
                    color_op_1 = 3;
                    color_op_1_arg_1 = 2;
                    color_op_1_arg_2 = 0;
                    color_op_1_alpha_op = 3;
                    color_op_1_alpha_op_arg_1 = 2;
                    color_op_1_alpha_op_arg_2 = 0;
                    goto LABEL_67;
                }

                color_op_1 = 1;
                color_op_1_arg_1 = 2;
                color_op_1_arg_2 = 1;
            }
            color_op_1_alpha_op = 1;
        LABEL_66:
            color_op_1_alpha_op_arg_1 = 2;
            color_op_1_alpha_op_arg_2 = 1;
        }
    LABEL_67:

        if (shader_bits_2 & shader_bits_5) {

            if ((m_shaderBits & 0x10000) == 0x10000) {

                switch ((m_shaderBits >> 0x14) & 0xF) {
                    case 1u:
                        if (texture_op_caps & 2) {
                            color_op_2 = 2;
                            color_op_2_arg_1 = 2;
                            color_op_2_arg_2 = 1;
                        } else if (log_shader_warnings) {
                            captainslog_warn("Warning: Using unsupported texture op: SELECTARG1\n");
                        }
                        break;
                    case 2u:
                        if (texture_op_caps & 8) {
                            goto LABEL_115;
                        }
                        if (log_shader_warnings) {
                            captainslog_warn("Warning: Using unsupported texture op: MODULATE\n");
                        }
                        break;
                    case 3u:
                        if (texture_op_caps & 0x400) {
                            color_op_2 = 0xB;
                            color_op_2_arg_1 = 2;
                            color_op_2_arg_2 = 1;
                        } else {
                            if (texture_op_caps & 0x40) {
                                goto LABEL_109;
                            }
                            if (log_shader_warnings) {
                                captainslog_warn("Warning: Using unsupported texture op: ADDSMOOTH\n");
                            }
                        }
                        break;
                    case 4u:
                        if (texture_op_caps & 0x40) {
                            goto LABEL_109;
                        }
                        if (log_shader_warnings) {
                            captainslog_warn("Warning: Using unsupported texture op: ADD\n");
                        }
                        break;
                    case 5u:
                        if (!(texture_op_caps & 0x200)) {
                            goto LABEL_89;
                        }
                        color_op_2 = 0xA;
                        color_op_2_arg_1 = 2;
                        color_op_2_arg_2 = 1;
                        break;
                    case 6u:
                        if (texture_op_caps & 0x200) {
                            color_op_2 = 0xA;
                            color_op_2_arg_1 = 1;
                            color_op_2_arg_2 = 2;
                        } else {
                        LABEL_89:
                            if (log_shader_warnings) {
                                captainslog_warn("Warning: Using unsupported texture op: SUBTRACT\n");
                            }
                        }
                        break;
                    case 7u:
                        if (texture_op_caps & 0x1000) {
                            color_op_2 = 0xD;
                            color_op_2_arg_1 = 2;
                            color_op_2_arg_2 = 1;
                        } else if (log_shader_warnings) {
                            captainslog_warn("Warning: Using unsupported texture op: BLENDTEXTUREALPHA\n");
                        }
                        break;
                    case 8u:
                        if ((texture_op_caps & 0x8000) != 0) {
                            color_op_2 = 0x10;
                            color_op_2_arg_1 = 2;
                            color_op_2_arg_2 = 1;
                        } else if (log_shader_warnings) {
                            captainslog_warn("Warning: Using unsupported texture op: BLENDCURRENTALPHA\n");
                        }
                        break;
                    case 9u:
                        if ((texture_op_caps & 0x80u) != 0) {
                            color_op_2 = 8;
                            color_op_2_arg_1 = 2;
                            color_op_2_arg_2 = 1;
                        } else {
                            if (texture_op_caps & 0x40) {
                                goto LABEL_109;
                            }
                            if (log_shader_warnings) {
                                captainslog_warn("Warning: Using unsupported texture op: ADDSIGNED\n");
                            }
                        }
                        break;
                    case 10u:
                        if (texture_op_caps & 0x100) {
                            color_op_2 = 9;
                            color_op_2_arg_1 = 2;
                            color_op_2_arg_2 = 1;
                        } else if ((texture_op_caps & 0x80u) != 0) {
                            color_op_2 = 8;
                            color_op_2_arg_1 = 2;
                            color_op_2_arg_2 = 1;
                        } else if (texture_op_caps & 0x40) {
                        LABEL_109:
                            color_op_2 = 7;
                            color_op_2_arg_1 = 2;
                            color_op_2_arg_2 = 1;
                        } else if (log_shader_warnings) {
                            captainslog_warn("Warning: Using unsupported texture op: ADDSIGNED2X\n");
                        }
                        break;
                    case 11u:
                        if (texture_op_caps & 0x10) {
                            color_op_2 = 5;
                            color_op_2_arg_1 = 2;
                            color_op_2_arg_2 = 1;
                        } else if (texture_op_caps & 8) {
                        LABEL_115:
                            color_op_2 = 4;
                            color_op_2_arg_1 = 2;
                            color_op_2_arg_2 = 1;
                        } else if (log_shader_warnings) {
                            captainslog_warn("Warning: Using unsupported texture op: MODULATE2X\n");
                        }
                        break;
                    case 12u:
                        if (caps->Supports_Mod_Alpha_Add_Color()) {
                            color_op_2 = 0x12;
                            color_op_2_arg_1 = 1;
                            color_op_2_arg_2 = 2;
                        } else if (texture_op_caps & 0x40) {
                            color_op_2 = 7;
                            color_op_2_arg_1 = 2;
                            color_op_2_arg_2 = 1;
                        } else if (log_shader_warnings) {
                            captainslog_warn("Warning: Using unsupported texture op: MODULATEALPHA_ADDCOLOR\n");
                        }
                        break;
                    default:
                        break;
                }

                switch ((m_shaderBits >> 0x18) & 7) {
                    case 1u:
                        if (texture_op_caps & 2) {
                            color_op_2_alpha_op = 2;
                            color_op_2_alpha_op_arg_2 = 1;
                        } else if (log_shader_warnings) {
                            captainslog_warn("Warning: Using unsupported texture op: SELECTARG1\n");
                        }
                        break;
                    case 2u:
                        if (texture_op_caps & 8) {
                            color_op_2_alpha_op = 4;
                            color_op_2_alpha_op_arg_2 = 1;
                        } else if (log_shader_warnings) {
                            captainslog_warn("Warning: Using unsupported texture op: MODULATE\n");
                        }
                        break;
                    case 3u:
                        if (texture_op_caps & 0x400) {
                            color_op_2_alpha_op = 0xB;
                            color_op_2_alpha_op_arg_2 = 1;
                        } else if (log_shader_warnings) {
                            captainslog_warn("Warning: Using unsupported texture op: ADDSMOOTH\n");
                        }
                        break;
                }

                if (color_op_2 == 1) {

                    if (color_op_2_alpha_op != 1) {
                        color_op_2 = 3;
                        color_op_2_arg_2 = 1;
                    }
                } else if (color_op_2_alpha_op == 1) {
                    color_op_2_alpha_op = 3;
                    color_op_2_alpha_op_arg_2 = 1;
                }
            }
        }

        bool set_something_2 = false;
        if (!shader_bits_6) {
            goto LABEL_163;
        }

        if (set_something && !color_op_1_arg_2 && (color_op_2_alpha_op != 1 || color_op_2 != 1)) {

            int color_arg_common = 1;

            if ((m_shaderBits & 0x10000) == 0x10000) {
                color_arg_common = 2;
            }

            if (color_op_1 != 2 || color_op_1_arg_1) {
                DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, 2);
                DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, color_arg_common);
                DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP, 2);
                DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAARG1, color_arg_common);
                DX8CALL(SetTextureStageState(2, D3DTSS_COLOROP, color_op_1));
                DX8CALL(SetTextureStageState(2, D3DTSS_COLORARG1, 1));
                DX8CALL(SetTextureStageState(2, D3DTSS_COLORARG2, 0));
                DX8CALL(SetTextureStageState(2, D3DTSS_ALPHAOP, color_op_1_alpha_op));
                DX8CALL(SetTextureStageState(2, D3DTSS_ALPHAARG1, 1));
                DX8CALL(SetTextureStageState(2, D3DTSS_ALPHAARG2, 0));
                DX8CALL(SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0));
                DX8CALL(SetTexture(2u, nullptr));
                set_something_2 = false;
                s_shaderDirty = true;
            LABEL_162:
                shader_bits_2 &= 0xFFFFE3FF;

            LABEL_163:

                if (shader_bits_2 & shader_bits_5) {
                    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, color_op_2);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG1, color_op_2_arg_1);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG2, color_op_2_arg_2);

                    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP, color_op_2_alpha_op);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAARG1, color_op_2_alpha_op_arg_1);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAARG2, color_op_2_alpha_op_arg_2);

                    shader_bits_2 &= 0xF80EFFFF;
                }

                if (set_something) {
                    if (set_something_2) {
                        if (color_op_2 != 1 && color_op_2_alpha_op != 1) {
                            DX8CALL(SetTextureStageState(2, D3DTSS_COLOROP, 2));
                            DX8CALL(SetTextureStageState(2, D3DTSS_COLORARG1, 1));
                            DX8CALL(SetTextureStageState(2, D3DTSS_ALPHAOP, 2));
                            DX8CALL(SetTextureStageState(2, D3DTSS_ALPHAARG1, 1));
                        } else {
                            DX8CALL(SetTextureStageState(2, D3DTSS_COLOROP, 1));
                            DX8CALL(SetTextureStageState(2, D3DTSS_ALPHAOP, 1));
                        }

                        DX8CALL(SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0));
                        DX8CALL(SetTexture(2u, 0));
                    }
                }

                if (!shader_bits_2) {
                    return;
                }

                DX8Wrapper::Set_DX8_Render_State(D3DRS_SPECULARENABLE, (m_shaderBits >> 0xD) & 1);
                DX8Wrapper::Set_DX8_Render_State(D3DRS_ZFUNC, (m_shaderBits & 7) + 1);
                DX8Wrapper::Set_DX8_Render_State(D3DRS_ZWRITEENABLE, (m_shaderBits >> 3) & 1);

                int cull_mode = s_polygonCullMode;

                if (!(m_shaderBits & 0x80000)) {
                    cull_mode = 1;
                }

                DX8Wrapper::Set_DX8_Render_State(D3DRS_CULLMODE, cull_mode);

                // NPatches are not supported on modern systems
#if 0
                if (shader_bits_2 & 0x20000) {
                    float level = 1.0;
                    if (m_shaderBits & 0x20000) {
                        level = (double)WW3D::NPatchesLevel;
                    }
                    DX8Wrapper::Set_DX8_Render_State(D3DRS_PATCHSEGMENTS, F2DW(level));
                }
#endif

                DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHATESTENABLE, (m_shaderBits >> 0x12) & 1);
                return;
            }
            captainslog_warn("Wasted Stage 0 in shader-vertex diffuse only");
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, 1);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP, 1);

            if (color_op_2_arg_2 == 1) {
                color_op_2_arg_2 = 0;
            }

            color_op_2_alpha_op_arg_2 = 0;

        } else {
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, color_op_1);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, color_op_1_arg_1);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, color_op_1_arg_2);

            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP, color_op_1_alpha_op);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAARG1, color_op_1_alpha_op_arg_1);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAARG2, color_op_1_alpha_op_arg_2);
        }
        set_something_2 = true;
        goto LABEL_162;
    }
#endif
}