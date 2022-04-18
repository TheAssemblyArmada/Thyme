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
#include "dx8texman.h"
#include "dx8wrapper.h"
#include "surfaceclass.h"
#include "textureloader.h"
#include "w3d.h"
#include <algorithm>
#include <cstring>

#ifdef GAME_DLL
ARRAY2D_DEF(PICK_ADDRESS(0x00A4C284, 0x00DF68D8),
    unsigned,
    g_minTextureFilters,
    MAX_TEXTURE_STAGES,
    TextureFilterClass::FILTER_TYPE_COUNT);
ARRAY2D_DEF(PICK_ADDRESS(0x00A4C304, 0x00DF6958),
    unsigned,
    g_magTextureFilters,
    MAX_TEXTURE_STAGES,
    TextureFilterClass::FILTER_TYPE_COUNT);
ARRAY2D_DEF(PICK_ADDRESS(0x00A4C204, 0x00DF6858),
    unsigned,
    g_mipMapFilters,
    MAX_TEXTURE_STAGES,
    TextureFilterClass::FILTER_TYPE_COUNT);
#else
static unsigned g_minTextureFilters[MAX_TEXTURE_STAGES][TextureFilterClass::FILTER_TYPE_COUNT];
static unsigned g_magTextureFilters[MAX_TEXTURE_STAGES][TextureFilterClass::FILTER_TYPE_COUNT];
static unsigned g_mipMapFilters[MAX_TEXTURE_STAGES][TextureFilterClass::FILTER_TYPE_COUNT];
#endif

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

    if (DX8Wrapper::Get_Current_Caps()->Get_Filter_Caps() & D3DPTFILTERCAPS_MAGFLINEAR) {
        g_magTextureFilters[0][FILTER_TYPE_BEST] = D3DTEXF_LINEAR;
    }

    if (DX8Wrapper::Get_Current_Caps()->Get_Filter_Caps() & D3DPTFILTERCAPS_MINFLINEAR) {
        g_minTextureFilters[0][FILTER_TYPE_BEST] = D3DTEXF_LINEAR;
    }

    switch (mode) {
        case FILTER_MODE_ANISOTROPIC2X:
        case FILTER_MODE_ANISOTROPIC4X:
        case FILTER_MODE_ANISOTROPIC8X:
        case FILTER_MODE_ANISOTROPIC16X:
            if (DX8Wrapper::Get_Current_Caps()->Get_Filter_Caps() & D3DPTFILTERCAPS_MAGFANISOTROPIC) {
                g_magTextureFilters[0][FILTER_TYPE_BEST] = D3DTEXF_ANISOTROPIC;
            }

            if (DX8Wrapper::Get_Current_Caps()->Get_Filter_Caps() & D3DPTFILTERCAPS_MINFANISOTROPIC) {
                g_minTextureFilters[0][FILTER_TYPE_BEST] = D3DTEXF_ANISOTROPIC;
            }
        case FILTER_MODE_TRILINEAR: // fall-through
            if (DX8Wrapper::Get_Current_Caps()->Get_Filter_Caps() & D3DPTFILTERCAPS_MIPFLINEAR) {
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

TextureClass::TextureClass(unsigned width,
    unsigned height,
    WW3DFormat format,
    MipCountType mip_count,
    PoolType pool,
    bool render_target,
    bool allow_reduction) :
    TextureBaseClass(width, height, mip_count, pool, render_target, allow_reduction),
    m_textureFormat(format),
    m_textureFilter(mip_count)
{
    m_initialized = true;
    m_isProcedural = true;
    m_allowReduction = false;
    m_compressionAllowed = Is_Compressed(format);

#ifdef BUILD_WITH_D3D8
    switch (pool) {
        case POOL_DEFAULT:
            m_d3dTexture = DX8Wrapper::Create_Texture(width, height, format, mip_count, D3DPOOL_DEFAULT, render_target);
            break;
        case POOL_MANAGED:
            m_d3dTexture = DX8Wrapper::Create_Texture(width, height, format, mip_count, D3DPOOL_MANAGED, render_target);
            break;
        case POOL_SYSTEMMEM:
            m_d3dTexture = DX8Wrapper::Create_Texture(width, height, format, mip_count, D3DPOOL_SYSTEMMEM, render_target);
            break;
        default:
            captainslog_warn("Attempted to create texture with unknown pool, using default.");
            m_d3dTexture = DX8Wrapper::Create_Texture(width, height, format, mip_count, D3DPOOL_DEFAULT, render_target);
            break;
    }
#endif

    if (pool == POOL_DEFAULT) {
        m_dirty = true;
        DX8TextureManagerClass::Add(new DX8TextureTrackerClass(width, height, mip_count, this, format, render_target));
    }

    m_lastAccess = W3D::Get_Sync_Time();
}

TextureClass::TextureClass(char const *name,
    char const *full_path,
    MipCountType mip_level_count,
    WW3DFormat format,
    bool allow_compression,
    bool allow_reduction) :
    TextureBaseClass(0, 0, mip_level_count, POOL_MANAGED, false, true),
    m_textureFormat(format),
    m_textureFilter(mip_level_count)
{
    m_compressionAllowed = allow_compression;
    m_inactivationTime = 20000;
    m_allowReduction = allow_reduction;

    Set_Texture_Name(name);

    if (!W3D::Is_Texturing_Enabled()) {
        m_initialized = false;
        m_d3dTexture = W3D_TYPE_INVALID_TEXTURE;
    }

    m_lastAccess = W3D::Get_Sync_Time();

    if (!W3D::Is_Thumbnail_Enabled() && TextureLoader::Is_DX8_Thread()) {
        TextureClass::Init();
    }
}

TextureClass::TextureClass(SurfaceClass *surface, MipCountType mip_level_count) :
    TextureBaseClass(0, 0, mip_level_count, POOL_MANAGED, false, true),
    m_textureFormat(surface->Get_Surface_Format()),
    m_textureFilter(mip_level_count)
{
    m_isProcedural = true;
    m_initialized = true;
    m_allowReduction = false;

    SurfaceClass::SurfaceDescription sd;
    surface->Get_Description(sd);
    m_width = sd.width;
    m_height = sd.height;
    m_compressionAllowed = Is_Compressed(sd.format);
    m_d3dTexture = DX8Wrapper::Create_Texture(surface->Peek_D3D_Surface(), mip_level_count);
    m_lastAccess = W3D::Get_Sync_Time();
}

TextureClass::TextureClass(w3dbasetexture_t d3d_texture) :
#ifdef BUILD_WITH_D3D8
    TextureBaseClass(0, 0, (MipCountType)d3d_texture->GetLevelCount(), POOL_DEFAULT, false, true),
    m_textureFilter((MipCountType)d3d_texture->GetLevelCount())
#else
    TextureBaseClass(0, 0, MIP_LEVELS_ALL, POOL_DEFAULT, false, true), m_textureFilter(MIP_LEVELS_ALL)
#endif
{
    m_initialized = true;
    m_isProcedural = true;
    m_allowReduction = false;
    Set_Platform_Base_Texture(d3d_texture);

#ifdef BUILD_WITH_D3D8
    w3dsurface_t surf = nullptr;
    DX8Wrapper::Handle_DX8_ErrorCode(
        reinterpret_cast<w3dtexture_t>(Peek_Platform_Base_Texture())->GetSurfaceLevel(0, &surf));
    D3DSURFACE_DESC desc;
    memset(&desc, 0, sizeof(desc));
    DX8Wrapper::Handle_DX8_ErrorCode(surf->GetDesc(&desc));
    m_width = desc.Width;
    m_height = desc.Height;
    m_textureFormat = D3DFormat_To_WW3DFormat(desc.Format);
#endif

    m_compressionAllowed = Is_Compressed(m_textureFormat);
    m_lastAccess = W3D::Get_Sync_Time();
}

/**
 * Initialise the texture and request that the data for it be loaded by the texture loader.
 */
void TextureClass::Init()
{
    if (!m_initialized) {
        unsigned inact = m_inactivationTime;

        if (inact != 0) {
            unsigned start = m_startTime;

            if (start != 0) {
                if (W3D::Get_Sync_Time() - start < inact) {
                    m_someTimeVal = 3 * inact;
                }

                m_startTime = 0;
            }
        }

        if (Peek_Platform_Base_Texture() == W3D_TYPE_INVALID_TEXTURE) {
            if (W3D::Is_Thumbnail_Enabled() && m_mipLevelCount != 1) {
                WW3DFormat format = m_textureFormat;
                Load_Locked_Surface();
                m_textureFormat = format;
            } else {
                TextureLoader::Request_Foreground_Loading(this);
            }
        }

        if (!m_initialized) {
            TextureLoader::Request_Background_Loading(this);
        }

        m_lastAccess = W3D::Get_Sync_Time();
    }
}

/**
 * Apply a new platform texture to this texture object.
 */
void TextureClass::Apply_New_Surface(w3dbasetexture_t d3d_texture, bool initialized, bool reset)
{
#ifdef BUILD_WITH_D3D8
    w3dbasetexture_t old = Peek_Platform_Base_Texture();
    if (old != W3D_TYPE_INVALID_TEXTURE) {
        old->Release();
    }

    m_d3dTexture = d3d_texture;
    d3d_texture->AddRef();

    if (initialized) {
        m_initialized = true;
    }

    if (reset) {
        m_inactivationTime = 0;
    }

    captainslog_assert(d3d_texture != W3D_TYPE_INVALID_TEXTURE);
    w3dsurface_t surf = nullptr;
    DX8Wrapper::Handle_DX8_ErrorCode(
        reinterpret_cast<w3dtexture_t>(Peek_Platform_Base_Texture())->GetSurfaceLevel(0, &surf));
    D3DSURFACE_DESC desc;
    memset(&desc, 0, sizeof(desc));
    DX8Wrapper::Handle_DX8_ErrorCode(surf->GetDesc(&desc));

    if (initialized) {
        m_textureFormat = D3DFormat_To_WW3DFormat(desc.Format);
        m_width = desc.Width;
        m_height = desc.Height;
    }

    surf->Release();
#endif
}

/**
 * Apply the texture stage requested.
 */
void TextureClass::Apply(unsigned stage)
{
    if (!m_initialized) {
        Init();
    }

    m_lastAccess = W3D::Get_Sync_Time();

    // Debug_Statistics::Record_Texture();

#ifdef BUILD_WITH_D3D8
    if (W3D::Is_Texturing_Enabled()) {
        DX8Wrapper::Set_DX8_Texture(stage, Peek_Platform_Base_Texture());
    } else {
        DX8Wrapper::Set_DX8_Texture(stage, nullptr);
    }
#endif
    m_textureFilter.Apply(stage);
}

/**
 * Get a surface class representing the relevant mip level.
 */
SurfaceClass *TextureClass::Get_Surface_Level(unsigned level)
{
    w3dsurface_t d3d_surface = Get_D3D_Surface_Level(level);
    SurfaceClass *surface = new SurfaceClass(d3d_surface);
#ifdef BUILD_WITH_D3D8
    d3d_surface->Release();
#endif

    return surface;
}

/**
 * Get the platform surface representing the relevant mip level.
 */
w3dsurface_t TextureClass::Get_D3D_Surface_Level(unsigned level)
{
    if (Peek_Platform_Base_Texture() == W3D_TYPE_INVALID_TEXTURE) {
        captainslog_assert(false);

        return W3D_TYPE_INVALID_SURFACE;
    }

    w3dsurface_t surf = W3D_TYPE_INVALID_SURFACE;
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Handle_DX8_ErrorCode(
        reinterpret_cast<w3dtexture_t>(Peek_Platform_Base_Texture())->GetSurfaceLevel(level, &surf));
#endif

    return surf;
}

/**
 * Calculate the memory used for this texture.
 */
unsigned TextureClass::Get_Texture_Memory_Usage()
{
    unsigned usage = 0;

    if (Peek_Platform_Base_Texture() == W3D_TYPE_INVALID_SURFACE) {
        return usage;
    }

#ifdef BUILD_WITH_D3D8
    int count = reinterpret_cast<w3dtexture_t>(Peek_Platform_Base_Texture())->GetLevelCount();

    for (int i = 0; i < count; ++i) {
        D3DSURFACE_DESC desc;
        DX8Wrapper::Handle_DX8_ErrorCode(
            reinterpret_cast<w3dtexture_t>(Peek_Platform_Base_Texture())->GetLevelDesc(i, &desc));
        usage += desc.Size;
    }
#endif

    return usage;
}

void TextureClass::Get_Level_Description(SurfaceClass::SurfaceDescription &surface_desc, unsigned int level)
{
    SurfaceClass *surface = Get_Surface_Level(level);

    if (surface != nullptr) {
        surface->Get_Description(surface_desc);
        surface->Release_Ref();
    }
}
