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
#pragma once

#include "always.h"
#include "surfaceclass.h"
#include "texturebase.h"
#include "w3dformat.h"
#include "w3dmpo.h"
#include <new>

class TextureFilterClass
{
public:
    enum FilterType
    {
        FILTER_TYPE_NONE,
        FILTER_TYPE_FAST,
        FILTER_TYPE_BEST,
        FILTER_TYPE_DEFAULT,
        FILTER_TYPE_COUNT,
    };

    enum TextureFilterMode
    {
        FILTER_MODE_BILINEAR,
        FILTER_MODE_TRILINEAR,
        FILTER_MODE_ANISOTROPIC2X,
        FILTER_MODE_ANISOTROPIC4X, // These appear to have been added by TTScripts?
        FILTER_MODE_ANISOTROPIC8X,
        FILTER_MODE_ANISOTROPIC16X,
    };

    enum TxtAddrMode
    {
        TEXTURE_ADDRESS_REPEAT,
        TEXTURE_ADDRESS_CLAMP,
    };

public:
    TextureFilterClass(MipCountType mip_count);
    void Apply(unsigned stage);
    void Set_Mip_Mapping(FilterType type) { m_mipMapFilter = type; }
    void Set_Min_Filter(FilterType type) { m_minTextureFilter = type; }
    void Set_Mag_Filter(FilterType type) { m_magTextureFilter = type; }
    void Set_Default_Min_Filter(FilterType type);
    void Set_Default_Mag_Filter(FilterType type);
    void Set_Default_Mip_Filter(FilterType type);
    void Set_U_Address_Mode(TxtAddrMode mode) { m_uAddressMode = mode; }
    void Set_V_Address_Mode(TxtAddrMode mode) { m_vAddressMode = mode; }
    FilterType Get_Mag_Filter() const { return m_magTextureFilter; }
    FilterType Get_Min_Filter() const { return m_minTextureFilter; }
    FilterType Get_Mip_Mapping() const { return m_mipMapFilter; }
    TxtAddrMode Get_U_Address_Mode() const { return m_uAddressMode; }
    TxtAddrMode Get_V_Address_Mode() const { return m_vAddressMode; }

    static void Init_Filters(TextureFilterMode mode);

#ifdef GAME_DLL
    TextureFilterClass *Hook_Ctor(MipCountType mip_count) { return new (this) TextureFilterClass(mip_count); }
#endif

private:
    FilterType m_minTextureFilter;
    FilterType m_magTextureFilter;
    FilterType m_mipMapFilter;
    TxtAddrMode m_uAddressMode;
    TxtAddrMode m_vAddressMode;
};

class TextureClass : public TextureBaseClass
{
    IMPLEMENT_W3D_POOL(TextureClass);

public:
    TextureClass(unsigned width,
        unsigned height,
        WW3DFormat format,
        MipCountType mip_level_count,
        PoolType pool,
        bool render_target,
        bool allow_reduction);
    TextureClass(char const *name,
        char const *full_path,
        MipCountType mip_level_count,
        WW3DFormat format,
        bool allow_compression,
        bool allow_reduction);
    TextureClass(SurfaceClass *surface, MipCountType mip_level_count);
    TextureClass(w3dbasetexture_t d3d_texture);
    virtual ~TextureClass() {}

    virtual int Get_Asset_Type() override { return 0; }
    virtual unsigned Get_Texture_Memory_Usage() override;
    virtual void Init() override;
    virtual void Apply_New_Surface(w3dbasetexture_t base, bool initialized, bool reset) override;
    virtual void Apply(unsigned stage) override;
    virtual TextureClass *As_Texture() override { return this; }

    SurfaceClass *Get_Surface_Level(unsigned level);
    w3dsurface_t Get_D3D_Surface_Level(unsigned level);
    void Get_Level_Description(SurfaceClass::SurfaceDescription &surface_desc, unsigned int level);
    WW3DFormat Get_Texture_Format() const { return m_textureFormat; }
    TextureFilterClass *Get_Texture_Filter() { return &m_textureFilter; }

#ifdef GAME_DLL
    TextureClass *Hook_Ctor1(unsigned width,
        unsigned height,
        WW3DFormat format,
        MipCountType mip_level_count,
        PoolType pool,
        bool render_target,
        bool allow_reduction)
    {
        return new (this) TextureClass(width, height, format, mip_level_count, pool, render_target, allow_reduction);
    }
    TextureClass *Hook_Ctor2(char const *name,
        char const *full_path,
        MipCountType mip_level_count,
        WW3DFormat format,
        bool allow_compression,
        bool allow_reduction)
    {
        return new (this) TextureClass(name, full_path, mip_level_count, format, allow_compression, allow_reduction);
    }
    TextureClass *Hook_Ctor3(SurfaceClass *surface, MipCountType mip_level_count)
    {
        return new (this) TextureClass(surface, mip_level_count);
    }
    TextureClass *Hook_Ctor4(w3dbasetexture_t d3d_texture) { return new (this) TextureClass(d3d_texture); }
#endif

private:
    WW3DFormat m_textureFormat;
    TextureFilterClass m_textureFilter;
};
