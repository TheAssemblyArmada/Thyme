/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Class implementing the texture interface for surface textures.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "texture.h"
#include "dx8wrapper.h"

static unsigned g_minTextureFilters[MAX_TEXTURE_STAGES][TextureFilterClass::FILTER_TYPE_COUNT];
static unsigned g_magTextureFilters[MAX_TEXTURE_STAGES][TextureFilterClass::FILTER_TYPE_COUNT];
static unsigned g_mipMapFilters[MAX_TEXTURE_STAGES][TextureFilterClass::FILTER_TYPE_COUNT];

TextureFilterClass::TextureFilterClass(MipCountType mip_count) :
    m_minTextureFilter(FILTER_TYPE_DEFAULT),
    m_magTextureFilter(FILTER_TYPE_DEFAULT),
    m_mipMapFilter(mip_count == MIP_LEVELS_1 ? FILTER_TYPE_NONE : FILTER_TYPE_DEFAULT),
    m_uAddressMode(TEXTURE_ADDRESS_REPEAT),
    m_vAddressMode(TEXTURE_ADDRESS_REPEAT)
{
}

void TextureFilterClass::Apply(unsigned stage)
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_MINFILTER, g_minTextureFilters[stage][m_minTextureFilter]);
    DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_MAGFILTER, g_magTextureFilters[stage][m_magTextureFilter]);
    DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_MIPFILTER, g_mipMapFilters[stage][m_mipMapFilter]);

    switch (m_uAddressMode) {
        case 0:
            DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
            break;
        case 1:
            DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
            break;
        default:
            break;
    }

    switch (m_vAddressMode) {
        case 0:
            DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
            break;
        case 1:
            DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
            break;
        default:
            break;
    }
#endif
}

void TextureFilterClass::Init_Filters(TextureFilterMode mode)
{
#ifdef BUILD_WITH_D3D8
    g_minTextureFilters[0][FILTER_TYPE_NONE] = D3DTEXF_POINT;
    g_magTextureFilters[0][FILTER_TYPE_NONE] = D3DTEXF_POINT;
    g_mipMapFilters[0][FILTER_TYPE_NONE] = D3DTEXF_NONE;

    g_minTextureFilters[0][FILTER_TYPE_FAST] = D3DTEXF_LINEAR;
    g_magTextureFilters[0][FILTER_TYPE_FAST] = D3DTEXF_LINEAR;
    g_mipMapFilters[0][FILTER_TYPE_FAST] = D3DTEXF_POINT;

    g_minTextureFilters[0][FILTER_TYPE_BEST] = D3DTEXF_POINT;
    g_magTextureFilters[0][FILTER_TYPE_BEST] = D3DTEXF_POINT;
    g_mipMapFilters[0][FILTER_TYPE_BEST] = D3DTEXF_POINT;

    if (DX8Wrapper::Get_Caps()->Get_Filter_Caps() & D3DPTFILTERCAPS_MAGFLINEAR) {
        g_magTextureFilters[0][FILTER_TYPE_BEST] = D3DTEXF_LINEAR;
    }

    if (DX8Wrapper::Get_Caps()->Get_Filter_Caps() & D3DPTFILTERCAPS_MINFLINEAR) {
        g_minTextureFilters[0][FILTER_TYPE_BEST] = D3DTEXF_LINEAR;
    }

    switch (mode) {
        case FILTER_MODE_ANISOTROPIC2X:
        case FILTER_MODE_ANISOTROPIC4X:
        case FILTER_MODE_ANISOTROPIC8X:
        case FILTER_MODE_ANISOTROPIC16X:
            if (DX8Wrapper::Get_Caps()->Get_Filter_Caps() & D3DPTFILTERCAPS_MAGFANISOTROPIC) {
                g_magTextureFilters[0][FILTER_TYPE_BEST] = D3DTEXF_ANISOTROPIC;
            }

            if (DX8Wrapper::Get_Caps()->Get_Filter_Caps() & D3DPTFILTERCAPS_MINFANISOTROPIC) {
                g_minTextureFilters[0][FILTER_TYPE_BEST] = D3DTEXF_ANISOTROPIC;
            }
        case FILTER_MODE_TRILINEAR: // fall-through
            if (DX8Wrapper::Get_Caps()->Get_Filter_Caps() & D3DPTFILTERCAPS_MIPFLINEAR) {
                g_mipMapFilters[0][FILTER_TYPE_BEST] = D3DTEXF_LINEAR;
            }
        default:
            break;
    };

    for (int i = 0; i < MAX_TEXTURE_STAGES - 1; ++i) {
        g_magTextureFilters[i + 1][FILTER_TYPE_NONE] = g_magTextureFilters[i][FILTER_TYPE_NONE];
        g_minTextureFilters[i + 1][FILTER_TYPE_NONE] = g_minTextureFilters[i][FILTER_TYPE_NONE];
        g_mipMapFilters[i + 1][FILTER_TYPE_NONE] = g_mipMapFilters[i][FILTER_TYPE_NONE];
        g_magTextureFilters[i + 1][FILTER_TYPE_FAST] = g_magTextureFilters[i][FILTER_TYPE_FAST];
        g_minTextureFilters[i + 1][FILTER_TYPE_FAST] = g_minTextureFilters[i][FILTER_TYPE_FAST];
        g_mipMapFilters[i + 1][FILTER_TYPE_FAST] = g_mipMapFilters[i][FILTER_TYPE_FAST];

        if (g_magTextureFilters[i][FILTER_TYPE_BEST] != D3DTEXF_ANISOTROPIC) {
            g_magTextureFilters[i + 1][FILTER_TYPE_BEST] = g_magTextureFilters[i][FILTER_TYPE_BEST];
        } else {
            g_magTextureFilters[i + 1][FILTER_TYPE_BEST] = D3DTEXF_LINEAR;
        }

        if (g_minTextureFilters[i][FILTER_TYPE_BEST] != D3DTEXF_ANISOTROPIC) {
            g_minTextureFilters[i + 1][FILTER_TYPE_BEST] = g_minTextureFilters[i][FILTER_TYPE_BEST];
        } else {
            g_minTextureFilters[i + 1][FILTER_TYPE_BEST] = D3DTEXF_LINEAR;
        }

        g_mipMapFilters[i + 1][FILTER_TYPE_BEST] = g_mipMapFilters[i][FILTER_TYPE_BEST];
    }

    for (int i = 0; i < MAX_TEXTURE_STAGES; ++i) {
        g_magTextureFilters[i][FILTER_TYPE_DEFAULT] = g_magTextureFilters[i][FILTER_TYPE_BEST];
        g_minTextureFilters[i][FILTER_TYPE_DEFAULT] = g_minTextureFilters[i][FILTER_TYPE_BEST];
        g_mipMapFilters[i][FILTER_TYPE_DEFAULT] = g_mipMapFilters[i][FILTER_TYPE_BEST];
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_MAXANISOTROPY, 2);
    }
#endif
}

void TextureFilterClass::Set_Default_Min_Filter(FilterType type)
{
    for (int i = 0; i < MAX_TEXTURE_STAGES; ++i) {
        g_minTextureFilters[i][FILTER_TYPE_DEFAULT] = g_minTextureFilters[i][type];
    }
}

void TextureFilterClass::Set_Default_Mag_Filter(FilterType type)
{
    for (int i = 0; i < MAX_TEXTURE_STAGES; ++i) {
        g_magTextureFilters[i][FILTER_TYPE_DEFAULT] = g_magTextureFilters[i][type];
    }
}

void TextureFilterClass::Set_Default_Mip_Filter(FilterType type)
{
    for (int i = 0; i < MAX_TEXTURE_STAGES; ++i) {
        g_mipMapFilters[i][FILTER_TYPE_DEFAULT] = g_mipMapFilters[i][type];
    }
}
