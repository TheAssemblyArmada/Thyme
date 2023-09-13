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
#include "texturebase.h"
#include "assetmgr.h"
#include "dx8texman.h"
#include "dx8wrapper.h"
#include "missingtexture.h"
#include "textureloader.h"
#include "textureloadtask.h"
#include "w3d.h"
#include <algorithm>

#ifdef GAME_DLL
#include "hooker.h"
#else
unsigned TextureBaseClass::s_unusedTextureID = 0;
#endif

/**
 * 0x0081A3A0
 */
TextureBaseClass::TextureBaseClass(
    unsigned width, unsigned height, MipCountType mip_count, PoolType pool, bool render_target, bool allow_reduction) :
    m_mipLevelCount(mip_count),
    m_initialized(false),
    m_isLightMap(false),
    m_compressionAllowed(false),
    m_isProcedural(false),
    m_allowReduction(allow_reduction),
    m_inactivationTime(0),
    m_someTimeVal(0),
    m_startTime(0),
    m_lastAccess(0),
    m_hsvShift(),
    m_width(width),
    m_height(height),
    m_d3dTexture(W3D_TYPE_INVALID_TEXTURE),
    m_name(),
    m_fullPath(),
    m_id(s_unusedTextureID++),
    m_pool(pool),
    m_dirty(false),
    m_normalTextureLoadTask(nullptr),
    m_thumbnailTextureLoadTask(nullptr)
{
}

/**
 * 0x0081A570
 */
TextureBaseClass::~TextureBaseClass()
{
    delete m_normalTextureLoadTask;
    m_normalTextureLoadTask = nullptr;
    delete m_thumbnailTextureLoadTask;
    m_thumbnailTextureLoadTask = nullptr;

#ifdef BUILD_WITH_D3D8
    if (m_d3dTexture != W3D_TYPE_INVALID_TEXTURE) {
        m_d3dTexture->Release();
        m_d3dTexture = W3D_TYPE_INVALID_TEXTURE;
    }
#endif

    DX8TextureManagerClass::Remove(this);
}

/**
 * Invalidates this texture objects and releases any resources held.
 *
 * 0x0081A760
 */
void TextureBaseClass::Invalidate()
{
    if (!m_normalTextureLoadTask && !m_thumbnailTextureLoadTask && !m_isProcedural) {
#ifdef BUILD_WITH_D3D8
        if (m_d3dTexture != W3D_TYPE_INVALID_TEXTURE) {
            m_d3dTexture->Release();
            m_d3dTexture = W3D_TYPE_INVALID_TEXTURE;
        }
#endif
        m_initialized = false;
        m_lastAccess = W3D::Get_Sync_Time();
    }
}

/**
 * Set a HSV adjustment.
 */
void TextureBaseClass::Set_HSV_Shift(const Vector3 &shift)
{
    Invalidate();
    m_hsvShift = shift;
}

/**
 * Gets the number of times the textures dimensions will be halved to reduce its resolution.
 *
 * 0x0081A830
 */
unsigned TextureBaseClass::Get_Reduction() const
{
    if (m_mipLevelCount == MIP_LEVELS_1 || m_width <= 32 || m_height <= 32) {
        return 0;
    }

    unsigned reduction = W3D::Get_Texture_Reduction();

    // Should we further reduce a texture greater than 256 in some dimension?
    // Effect should be to reduce texture by half again after normal reduction is applied.
    if (W3D::Is_Large_Texture_Extra_Reduction_Enabled() && (m_width > 256 || m_height > 256)) {
        ++reduction;
    }

    if (m_mipLevelCount != MIP_LEVELS_ALL) {
        reduction = std::min<unsigned>(reduction, m_mipLevelCount);
    }

    return reduction;
}

/**
 * Request load to a locked surface.
 */
void TextureBaseClass::Load_Locked_Surface()
{
    if (m_d3dTexture != W3D_TYPE_INVALID_TEXTURE) {
#ifdef BUILD_WITH_D3D8
        m_d3dTexture->Release();
#endif
        m_d3dTexture = W3D_TYPE_INVALID_TEXTURE;
    }

    TextureLoader::Request_Thumbnail(this);
    m_initialized = false;
}

/**
 * Checks if this texture object is a missing texture.
 */
bool TextureBaseClass::Is_Missing_Texture() const
{
    w3dbasetexture_t missing = MissingTexture::Get_Missing_Texture();
    bool ret = missing == m_d3dTexture;

    if (missing != W3D_TYPE_INVALID_TEXTURE) {
#ifdef BUILD_WITH_D3D8
        missing->Release();
#endif
    }

    return ret;
}

/**
 * Gets the texure priority.
 */
unsigned TextureBaseClass::Get_Priority()
{
#ifdef BUILD_WITH_D3D8
    if (m_d3dTexture != W3D_TYPE_INVALID_TEXTURE) {
        return m_d3dTexture->GetPriority();
    }
#endif

    return 0;
}

/**
 * Sets the texure priority.
 */
void TextureBaseClass::Set_Priority(unsigned priority)
{
#ifdef BUILD_WITH_D3D8
    if (m_d3dTexture != W3D_TYPE_INVALID_TEXTURE) {
        m_d3dTexture->SetPriority(priority);
    }
#endif
}

/**
 * Access the underlying platform representation of the texture.
 *
 * 0x0081A7A0
 */
w3dbasetexture_t TextureBaseClass::Peek_Platform_Base_Texture() const
{
    m_lastAccess = W3D::Get_Sync_Time();

    return m_d3dTexture;
}

/**
 * Sets the underlying platform representation of the texture.
 *
 * 0x0081A7B0
 */
void TextureBaseClass::Set_Platform_Base_Texture(w3dbasetexture_t tex)
{
    w3dbasetexture_t old = Peek_Platform_Base_Texture();
#ifdef BUILD_WITH_D3D8
    if (old != W3D_TYPE_INVALID_TEXTURE) {
        old->Release();
    }
#endif

    m_d3dTexture = tex;

#ifdef BUILD_WITH_D3D8
    if (tex != W3D_TYPE_INVALID_TEXTURE) {
        tex->AddRef();
    }
#endif
}

/**
 * Does something with W3DAssetManager.
 *
 * 0x0081A620
 */
void TextureBaseClass::Invalidate_Old_Unused_Textures(unsigned age)
{
    if (W3D::Is_Thumbnail_Enabled()) {
        unsigned int syncTime = W3D::Get_Sync_Time();

        for (HashTemplateIterator<StringClass, TextureClass *> textureIter(W3DAssetManager::Get_Instance()->Texture_Hash());
             !textureIter.Is_Done();
             textureIter.Next()) {
            TextureClass *texture = textureIter.Peek_Value();
            if (texture->m_initialized && texture->m_inactivationTime != 0) {
                if (syncTime - texture->m_lastAccess > (age ? age : texture->m_someTimeVal + texture->m_inactivationTime)) {
                    texture->Invalidate();
                    texture->m_startTime = syncTime;
                }
            }
        }
    }
}

/**
 * Nulls DX8Wrappers textures array.
 *
 * 0x0081A890
 */
void TextureBaseClass::Apply_Null(unsigned stage)
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_DX8_Texture(stage, W3D_TYPE_INVALID_TEXTURE);
#endif
}
