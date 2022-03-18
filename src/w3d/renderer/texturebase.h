/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 * @author tomsons26
 *
 * @brief Base class for texture handling.
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
#include "refcount.h"
#include "vector3.h"
#include "w3dtypes.h"
#include "wwstring.h"

class TextureLoadTaskClass;
class TextureClass;
class CubeTextureClass;
class VolumeTextureClass;

enum MipCountType
{
    MIP_LEVELS_ALL,
    MIP_LEVELS_1,
    MIP_LEVELS_2,
    MIP_LEVELS_3,
    MIP_LEVELS_4,
    MIP_LEVELS_5,
    MIP_LEVELS_6,
    MIP_LEVELS_7,
    MIP_LEVELS_8,
    MIP_LEVELS_10, // TT Scripts has it this way, no 9?
    MIP_LEVELS_11,
    MIP_LEVELS_12,
    MIP_LEVELS_MAX,
};

enum PoolType
{
    POOL_DEFAULT,
    POOL_MANAGED,
    POOL_SYSTEMMEM,
};

enum TexAssetType
{
    ASSET_STANDARD,
    ASSET_CUBE,
    ASSET_VOLUME,
};

class TextureBaseClass : public RefCountClass
{
    friend class TextureLoadTaskClass;

public:
    TextureBaseClass(
        unsigned width, unsigned height, MipCountType mip_count, PoolType pool, bool render_target, bool allow_reduction);
    virtual ~TextureBaseClass();
    virtual int Get_Asset_Type() = 0;
    virtual unsigned Get_Texture_Memory_Usage() = 0;
    virtual void Init() = 0;
    virtual void Apply_New_Surface(w3dbasetexture_t base, bool initialized, bool reset) = 0;
    virtual void Apply(unsigned stage) = 0;
    virtual TextureClass *As_Texture() { return nullptr; }
    virtual CubeTextureClass *As_Cube_Texture() { return nullptr; }
    virtual VolumeTextureClass *As_Volume_Texture() { return nullptr; }

    void Invalidate();
    w3dbasetexture_t Peek_Platform_Base_Texture() const;
    void Release_Platform_Base_Texture()
    {
#ifdef BUILD_WITH_D3D8
        m_d3dTexture->Release();
#endif
        m_d3dTexture = W3D_TYPE_INVALID_TEXTURE;
    }
    void Set_Platform_Base_Texture(w3dbasetexture_t tex);
    void Set_Texture_Name(const char *name) { m_name = name; }
    void Set_HSV_Shift(const Vector3 &shift);
    unsigned Get_Reduction() const;
    void Load_Locked_Surface();
    bool Is_Missing_Texture() const;
    unsigned Get_Priority();
    void Set_Priority(unsigned priority);
    TextureLoadTaskClass *Get_Normal_Load_Task() { return m_normalTextureLoadTask; }
    TextureLoadTaskClass *Get_Thumbnail_Load_Task() { return m_thumbnailTextureLoadTask; }
    const StringClass &Get_Full_Path() const { return m_fullPath; }
    const StringClass &Get_Name() const { return m_name; }
    const Vector3 &Get_HSV_Shift() const { return m_hsvShift; }
    bool Is_Initialized() const { return m_initialized; }
    void Set_Dirty(bool dirty) { m_dirty = dirty; }
    unsigned Get_Width() const { return m_width; }
    unsigned Get_Height() const { return m_height; }
    MipCountType Get_Mip_Level_Count() const { return m_mipLevelCount; }

    static void Invalidate_Old_Unused_Textures(unsigned age);
    static void Apply_Null(unsigned stage);

protected:
    MipCountType m_mipLevelCount;
    bool m_initialized;
    bool m_isLightMap;
    bool m_compressionAllowed;
    bool m_isProcedural;
    bool m_allowReduction;
    unsigned m_inactivationTime;
    unsigned m_someTimeVal;
    unsigned m_startTime;
    mutable unsigned m_lastAccess;
    Vector3 m_hsvShift;
    unsigned m_width;
    unsigned m_height;
    w3dbasetexture_t m_d3dTexture;
    StringClass m_name;
    StringClass m_fullPath;
    unsigned m_id;
    PoolType m_pool;
    bool m_dirty;
    TextureLoadTaskClass *m_normalTextureLoadTask;
    TextureLoadTaskClass *m_thumbnailTextureLoadTask;

private:
#ifdef GAME_DLL
    static unsigned &s_unusedTextureID;
#else
    static unsigned s_unusedTextureID;
#endif
};
