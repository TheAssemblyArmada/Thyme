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
#pragma once

#include "always.h"

class StringClass;
struct W3dMaterial3Struct;

enum ShaderShiftConstants
{
    SHIFT_DEPTHCOMPARE = 0,
    SHIFT_DEPTHMASK = 3,
    SHIFT_COLORMASK = 4,
    SHIFT_DSTBLEND = 5,
    SHIFT_FOG = 8,
    SHIFT_PRIGRADIENT = 10,
    SHIFT_SECGRADIENT = 13,
    SHIFT_SRCBLEND = 14,
    SHIFT_TEXTURING = 16,
    SHIFT_NPATCHENABLE = 17,
    SHIFT_ALPHATEST = 18,
    SHIFT_CULLMODE = 19,
    SHIFT_POSTDETAILCOLORFUNC = 20,
    SHIFT_POSTDETAILALPHAFUNC = 24,
};

// Utility function to convert enums to bitfield. Based on a macro provided by TT team.
inline constexpr uint32_t Shader_Const(uint32_t depth_compare,
    uint32_t depth_mask,
    uint32_t color_mask,
    uint32_t dst_blend,
    uint32_t fog,
    uint32_t pri_grad,
    uint32_t sec_grad,
    uint32_t src_blend,
    uint32_t texture,
    uint32_t npatch,
    uint32_t alpha_test,
    uint32_t cullmode,
    uint32_t post_det_color,
    uint32_t post_det_alpha)
{
    return depth_compare << SHIFT_DEPTHCOMPARE | depth_mask << SHIFT_DEPTHMASK | color_mask << SHIFT_COLORMASK
        | dst_blend << SHIFT_DSTBLEND | fog << SHIFT_FOG | pri_grad << SHIFT_PRIGRADIENT | sec_grad << SHIFT_SECGRADIENT
        | src_blend << SHIFT_SRCBLEND | texture << SHIFT_TEXTURING | npatch << SHIFT_NPATCHENABLE
        | alpha_test << SHIFT_ALPHATEST | cullmode << SHIFT_CULLMODE | post_det_color << SHIFT_POSTDETAILCOLORFUNC
        | post_det_alpha << SHIFT_POSTDETAILALPHAFUNC;
}

class ShaderClass
{
    friend class DX8Wrapper;

public:
    enum AlphaTestType
    {
        ALPHATEST_DISABLE,
        ALPHATEST_ENABLE,
        ALPHATEST_MAX
    };

    enum DepthCompareType
    {
        PASS_NEVER,
        PASS_LESS,
        PASS_EQUAL,
        PASS_LEQUAL,
        PASS_GREATER,
        PASS_NOTEQUAL,
        PASS_GEQUAL,
        PASS_ALWAYS,
        PASS_MAX
    };

    enum DepthMaskType
    {
        DEPTH_WRITE_DISABLE,
        DEPTH_WRITE_ENABLE,
        DEPTH_WRITE_MAX
    };

    enum ColorMaskType
    {
        COLOR_WRITE_DISABLE,
        COLOR_WRITE_ENABLE,
        COLOR_WRITE_MAX
    };

    enum DetailAlphaFuncType
    {
        DETAILALPHA_DISABLE,
        DETAILALPHA_DETAIL,
        DETAILALPHA_SCALE,
        DETAILALPHA_INVSCALE,
        DETAILALPHA_MAX
    };

    enum DetailColorFuncType
    {
        DETAILCOLOR_DISABLE,
        DETAILCOLOR_DETAIL,
        DETAILCOLOR_SCALE,
        DETAILCOLOR_INVSCALE,
        DETAILCOLOR_ADD,
        DETAILCOLOR_SUB,
        DETAILCOLOR_SUBR,
        DETAILCOLOR_BLEND,
        DETAILCOLOR_DETAILBLEND,
        DETAILCOLOR_ADDSIGNED, // following are new in ZH
        DETAILCOLOR_ADDSIGNED2X,
        DETAILCOLOR_SCALE2X,
        DETAILCOLOR_MODALPHAADDCOLOR,
        DETAILCOLOR_MAX
    };

    enum CullModeType
    {
        CULL_MODE_DISABLE,
        CULL_MODE_ENABLE,
        CULL_MODE_MAX
    };

    enum NPatchEnableType
    {
        NPATCH_DISABLE,
        NPATCH_ENABLE,
        NPATCH_TYPE_MAX
    };

    enum DstBlendFuncType
    {
        DSTBLEND_ZERO,
        DSTBLEND_ONE,
        DSTBLEND_SRC_COLOR,
        DSTBLEND_ONE_MINUS_SRC_COLOR,
        DSTBLEND_SRC_ALPHA,
        DSTBLEND_ONE_MINUS_SRC_ALPHA,
        DSTBLEND_MAX
    };

    enum FogFuncType
    {
        FOG_DISABLE,
        FOG_ENABLE,
        FOG_SCALE_FRAGMENT,
        FOG_WHITE,
        FOG_MAX
    };

    enum PriGradientType
    {
        GRADIENT_DISABLE,
        GRADIENT_MODULATE,
        GRADIENT_ADD,
        GRADIENT_BUMPENVMAP,
        GRADIENT_BUMPENVMAPLUMINANCE,
        GRADIENT_MODULATE2X, // new in ZH
        GRADIENT_MAX
    };

    enum SecGradientType
    {
        SECONDARY_GRADIENT_DISABLE,
        SECONDARY_GRADIENT_ENABLE,
        SECONDARY_GRADIENT_MAX
    };

    enum SrcBlendFuncType
    {
        SRCBLEND_ZERO,
        SRCBLEND_ONE,
        SRCBLEND_SRC_ALPHA,
        SRCBLEND_ONE_MINUS_SRC_ALPHA,
        SRCBLEND_MAX
    };

    enum TexturingType
    {
        TEXTURING_DISABLE,
        TEXTURING_ENABLE,
        TEXTURING_MAX
    };

    enum StaticSortCategoryType
    {
        SSCAT_OPAQUE,
        SSCAT_ALPHA_TEST,
        SSCAT_ADDITIVE,
        SSCAT_OTHER,
        SSCAT_OTHER2,
    };

    // Masks for where various shader options are held within the bitfield.
    // clang-format off
    enum
    {
        MASK_DEPTHCOMPARE = (7 << 0),          // 0x00000007 -  0 0000 0000 0000 0000 0000 0000 0000 0111
        MASK_DEPTHMASK = (1 << 3),             // 0x00000008 -  3 0000 0000 0000 0000 0000 0000 0000 1000
        MASK_COLORMASK = (1 << 4),             // 0x00000010 -  4 0000 0000 0000 0000 0000 0000 0001 0000
        MASK_DSTBLEND = (7 << 5),              // 0x000000E0 -  5 0000 0000 0000 0000 0000 0000 1110 0000
        MASK_FOG = (3 << 8),                   // 0x00000300 -  8 0000 0000 0000 0000 0000 0011 0000 0000
        MASK_PRIGRADIENT = (7 << 10),          // 0x00001C00 - 10 0000 0000 0000 0000 0001 1100 0000 0000
        MASK_SECGRADIENT = (1 << 13),          // 0x00002000 - 13 0000 0000 0000 0000 0010 0000 0000 0000
        MASK_SRCBLEND = (3 << 14),             // 0x0000C000 - 14 0000 0000 0000 0000 1100 0000 0000 0000
        MASK_TEXTURING = (1 << 16),            // 0x00010000 - 16 0000 0000 0000 0001 0000 0000 0000 0000
        MASK_NPATCHENABLE = (1 << 17),         // 0x00020000 - 17 0000 0000 0000 0010 0000 0000 0000 0000
        MASK_ALPHATEST = (1 << 18),            // 0x00040000 - 18 0000 0000 0000 0100 0000 0000 0000 0000
        MASK_CULLMODE = (1 << 19),             // 0x00080000 - 19 0000 0000 0000 1000 0000 0000 0000 0000
        MASK_POSTDETAILCOLORFUNC = (15 << 20), // 0x00F00000 - 20 0000 0000 1111 0000 0000 0000 0000 0000
        MASK_POSTDETAILALPHAFUNC = (7 << 24),  // 0x07000000 - 24 0000 0111 0000 0000 0000 0000 0000 0000
    };
    // clang-format on

public:
    ShaderClass() { Reset(); }
    ShaderClass(const ShaderClass &that) : m_shaderBits(that.m_shaderBits) {}
    ShaderClass(const uint32_t bits) : m_shaderBits(bits) {}

    bool operator==(const ShaderClass &that) const { return m_shaderBits == that.m_shaderBits; }
    bool operator!=(const ShaderClass &that) const { return m_shaderBits != that.m_shaderBits; }

    bool Uses_Alpha() const;
    bool Uses_Fog() const { return Get_Fog_Func() != FOG_DISABLE; }
    bool Uses_Primary_Gradient() const { return Get_Primary_Gradient() != GRADIENT_DISABLE; }
    bool Uses_Secondary_Gradient() const { return Get_Secondary_Gradient() != SECONDARY_GRADIENT_DISABLE; }
    bool Uses_Texture() const { return Get_Texturing() != TEXTURING_DISABLE; }
    bool Uses_Post_Detail_Texture() const;
    void Reset() { m_shaderBits = 0x8441B; } // TODO decode to Shader_Const call.
    void Init_From_Material3(const W3dMaterial3Struct &material);
    void Enable_Fog(const char *source);
    StaticSortCategoryType Get_Static_Sort_Category() const;
    int Guess_Sort_Level() const;
    StringClass &Get_Description(StringClass &desc) const; // new function in ZH

    uint32_t Get_Bits() const { return m_shaderBits; }

    DepthCompareType Get_Depth_Compare() const
    {
        return (DepthCompareType)(m_shaderBits & MASK_DEPTHCOMPARE >> SHIFT_DEPTHCOMPARE);
    }

    DepthMaskType Get_Depth_Mask() const { return (DepthMaskType)((m_shaderBits & MASK_DEPTHMASK) >> SHIFT_DEPTHMASK); }

    ColorMaskType Get_Color_Mask() const { return (ColorMaskType)((m_shaderBits & MASK_COLORMASK) >> SHIFT_COLORMASK); }

    DetailAlphaFuncType Get_Post_Detail_Alpha_Func() const
    {
        return (DetailAlphaFuncType)((m_shaderBits & MASK_POSTDETAILALPHAFUNC) >> SHIFT_POSTDETAILALPHAFUNC);
    }

    DetailColorFuncType Get_Post_Detail_Color_Func() const
    {
        return (DetailColorFuncType)((m_shaderBits & MASK_POSTDETAILCOLORFUNC) >> SHIFT_POSTDETAILCOLORFUNC);
    }

    AlphaTestType Get_Alpha_Test() const { return (AlphaTestType)((m_shaderBits & MASK_ALPHATEST) >> SHIFT_ALPHATEST); }

    CullModeType Get_Cull_Mode() const { return (CullModeType)((m_shaderBits & MASK_CULLMODE) >> SHIFT_CULLMODE); }

    DstBlendFuncType Get_Dst_Blend_Func() const
    {
        return (DstBlendFuncType)((m_shaderBits & MASK_DSTBLEND) >> SHIFT_DSTBLEND);
    }

    FogFuncType Get_Fog_Func() const { return (FogFuncType)((m_shaderBits & MASK_FOG) >> SHIFT_FOG); }

    PriGradientType Get_Primary_Gradient() const
    {
        return (PriGradientType)((m_shaderBits & MASK_PRIGRADIENT) >> SHIFT_PRIGRADIENT);
    }

    SecGradientType Get_Secondary_Gradient() const
    {
        return (SecGradientType)((m_shaderBits & MASK_SECGRADIENT) >> SHIFT_SECGRADIENT);
    }

    SrcBlendFuncType Get_Src_Blend_Func() const
    {
        return (SrcBlendFuncType)((m_shaderBits & MASK_SRCBLEND) >> SHIFT_SRCBLEND);
    }

    TexturingType Get_Texturing() const { return (TexturingType)((m_shaderBits & MASK_TEXTURING) >> SHIFT_TEXTURING); }

    NPatchEnableType Get_NPatch_Enable() const
    {
        return (NPatchEnableType)((m_shaderBits & MASK_NPATCHENABLE) >> SHIFT_NPATCHENABLE);
    }

    void Set_Depth_Compare(DepthCompareType x)
    {
        m_shaderBits &= ~MASK_DEPTHCOMPARE;
        m_shaderBits |= x << SHIFT_DEPTHCOMPARE;
    }

    void Set_Depth_Mask(DepthMaskType x)
    {
        m_shaderBits &= ~MASK_DEPTHMASK;
        m_shaderBits |= x << SHIFT_DEPTHMASK;
    }

    void Set_Color_Mask(ColorMaskType x)
    {
        m_shaderBits &= ~MASK_COLORMASK;
        m_shaderBits |= x << SHIFT_COLORMASK;
    }

    void Set_Post_Detail_Alpha_Func(DetailAlphaFuncType x)
    {
        m_shaderBits &= ~MASK_POSTDETAILALPHAFUNC;
        m_shaderBits |= x << SHIFT_POSTDETAILALPHAFUNC;
    }

    void Set_Post_Detail_Color_Func(DetailColorFuncType x)
    {
        m_shaderBits &= ~MASK_POSTDETAILCOLORFUNC;
        m_shaderBits |= x << SHIFT_POSTDETAILCOLORFUNC;
    }

    void Set_Alpha_Test(AlphaTestType x)
    {
        m_shaderBits &= ~MASK_ALPHATEST;
        m_shaderBits |= x << SHIFT_ALPHATEST;
    }

    void Set_Cull_Mode(CullModeType x)
    {
        m_shaderBits &= ~MASK_CULLMODE;
        m_shaderBits |= x << SHIFT_CULLMODE;
    }

    void Set_Dst_Blend_Func(DstBlendFuncType x)
    {
        m_shaderBits &= ~MASK_DSTBLEND;
        m_shaderBits |= x << SHIFT_DSTBLEND;
    }

    void Set_Fog_Func(FogFuncType x)
    {
        m_shaderBits &= ~MASK_FOG;
        m_shaderBits |= x << SHIFT_FOG;
    }

    void Set_Primary_Gradient(PriGradientType x)
    {
        m_shaderBits &= ~MASK_PRIGRADIENT;
        m_shaderBits |= x << SHIFT_PRIGRADIENT;
    }

    void Set_Secondary_Gradient(SecGradientType x)
    {
        m_shaderBits &= ~MASK_SECGRADIENT;
        m_shaderBits |= x << SHIFT_SECGRADIENT;
    }

    void Set_Src_Blend_Func(SrcBlendFuncType x)
    {
        m_shaderBits &= ~MASK_SRCBLEND;
        m_shaderBits |= x << SHIFT_SRCBLEND;
    }

    void Set_Texturing(TexturingType x)
    {
        m_shaderBits &= ~MASK_TEXTURING;
        m_shaderBits |= x << SHIFT_TEXTURING;
    }

    void Set_NPatch_Enable(NPatchEnableType x)
    {
        m_shaderBits &= ~MASK_NPATCHENABLE;
        m_shaderBits |= x << SHIFT_NPATCHENABLE;
    }

    static void Invalidate() { s_shaderDirty = true; }
    static void Invert_Backface_Culling(bool onoff);
    static bool Is_Backface_Culling_Inverted() { return s_polygonCullMode == 3; }

protected:
    void Report_Unable_To_Fog(const char *source);

private:
    void Apply();

public:
    static ShaderClass s_presetOpaqueShader;
    static ShaderClass s_presetAdditiveShader;
    static ShaderClass s_presetBumpenvmapShader;
    static ShaderClass s_presetAlphaShader;
    static ShaderClass s_presetMultiplicativeShader;
    static ShaderClass s_presetOpaque2DShader;
    static ShaderClass s_presetOpaqueSpriteShader;
    static ShaderClass s_presetAdditive2DShader;
    static ShaderClass s_presetAlpha2DShader;
    static ShaderClass s_presetAdditiveSpriteShader;
    static ShaderClass s_presetAlphaSpriteShader;
    static ShaderClass s_presetOpaqueSolidShader;
    static ShaderClass s_presetAdditiveSolidShader;
    static ShaderClass s_presetAlphaSolidShader;
    static ShaderClass s_presetATest2DShader;
    static ShaderClass s_presetATestSpriteShader;
    static ShaderClass s_presetATestBlend2DShader;
    static ShaderClass s_presetATestBlendSpriteShader;
    static ShaderClass s_presetScreen2DShader;
    static ShaderClass s_presetScreenSpriteShader;
    static ShaderClass s_presetMultiplicative2DShader;
    static ShaderClass s_presetMultiplicativeSpriteShader;

protected:
    uint32_t m_shaderBits;
#ifdef GAME_DLL
    static uint32_t &s_currentShader;
    static bool &s_shaderDirty;
    static uint32_t &s_polygonCullMode;
#else
    static uint32_t s_currentShader;
    static bool s_shaderDirty;
    static uint32_t s_polygonCullMode;
#endif
};

inline bool ShaderClass::Uses_Alpha() const
{
    // check if alpha test is enabled
    if (Get_Alpha_Test() != ALPHATEST_DISABLE) {
        return true;
    }

    DstBlendFuncType dst = Get_Dst_Blend_Func();

    if (dst == DSTBLEND_SRC_ALPHA || dst == DSTBLEND_ONE_MINUS_SRC_ALPHA) {
        return true;
    }

    SrcBlendFuncType src = Get_Src_Blend_Func();

    return src == SRCBLEND_SRC_ALPHA || src == SRCBLEND_ONE_MINUS_SRC_ALPHA;
}

inline bool ShaderClass::Uses_Post_Detail_Texture() const
{
    if (Get_Texturing() == TEXTURING_DISABLE) {
        return false;
    }

    return (Get_Post_Detail_Color_Func() != DETAILCOLOR_DISABLE) || (Get_Post_Detail_Alpha_Func() != DETAILALPHA_DISABLE);
}
