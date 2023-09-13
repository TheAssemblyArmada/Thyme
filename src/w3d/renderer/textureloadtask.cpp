/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 * @author tomsons26
 *
 * @brief Class for holding a texture loading task for the loader.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "textureloadtask.h"
#include "bitmaphandler.h"
#include "ddsfile.h"
#include "dx8wrapper.h"
#include "missingtexture.h"
#include "targa.h"
#include "texture.h"
#include "textureloader.h"
#include "textureloadtasklist.h"
#include "thumbnail.h"
#include "thumbnailmanager.h"
#include "w3d.h"
#include <algorithm>
#include <cstring>

using std::max;
using std::memset;
using std::min;

/**
 * 0x0082FF20
 */
TextureLoadTaskClass::TextureLoadTaskClass() :
    m_listNode{ nullptr, nullptr },
    m_parent(nullptr),
    m_texture(nullptr),
    m_d3dTexture(W3D_TYPE_INVALID_TEXTURE),
    m_format(WW3D_FORMAT_UNKNOWN),
    m_width(0),
    m_height(0),
    m_mipLevelCount(0),
    m_reduction(0),
    m_hsvAdjust(0.0f, 0.0f, 0.0f),
    m_type(TASK_NONE),
    m_priority(PRIORITY_BACKGROUND),
    m_loadState(STATE_NONE)
{
    memset(m_lockedSurfacePtr, 0, sizeof(m_lockedSurfacePtr));
    memset(m_lockedSurfacePitch, 0, sizeof(m_lockedSurfacePitch));
}

/**
 * 0x0082FF80
 */
TextureLoadTaskClass::~TextureLoadTaskClass()
{
    Deinit();
}

/**
 * Fetched the locked surface pointer, performs checks when built with asserts.
 */
uint8_t *TextureLoadTaskClass::Get_Locked_Surface_Pointer(unsigned level)
{
    captainslog_assert(level < m_mipLevelCount);
    captainslog_assert(m_lockedSurfacePtr[level] != nullptr);
    return m_lockedSurfacePtr[level];
}

/**
 * Fetched the locked surface pitch, performs checks when built with asserts.
 */
unsigned TextureLoadTaskClass::Get_Locked_Surface_Pitch(unsigned level)
{
    captainslog_assert(level < m_mipLevelCount);
    captainslog_assert(m_lockedSurfacePitch[level] != 0);
    return m_lockedSurfacePitch[level];
}

/**
 * Deinitialize this task and add it to the list of free task objects.
 *
 * 0x008301D0
 */
void TextureLoadTaskClass::Destroy()
{
    Deinit();
    g_freeList.Push_Front(this);
}

/**
 * Initialize this task to load to the specified texture object.
 *
 * 0x00830210
 */
void TextureLoadTaskClass::Init(TextureBaseClass *texture, TaskType type, PriorityType priority)
{
    captainslog_assert(texture != nullptr);
    Ref_Ptr_Set(m_texture, texture);
    m_type = type;
    m_priority = priority;
    m_loadState = STATE_NONE;
    m_d3dTexture = W3D_TYPE_INVALID_TEXTURE;

    if (texture->As_Texture() != nullptr) {
        m_format = texture->As_Texture()->Get_Texture_Format();
    } else {
        m_format = WW3D_FORMAT_UNKNOWN;
    }

    m_width = 0;
    m_height = 0;
    m_mipLevelCount = m_texture->m_mipLevelCount;
    m_reduction = m_texture->Get_Reduction();
    m_hsvAdjust = m_texture->m_hsvShift;
    memset(m_lockedSurfacePtr, 0, sizeof(m_lockedSurfacePtr));
    memset(m_lockedSurfacePitch, 0, sizeof(m_lockedSurfacePitch));

    switch (m_type) {
        case TASK_THUMBNAIL:
            m_texture->m_thumbnailTextureLoadTask = this;
            break;
        case TASK_LOAD:
            m_texture->m_normalTextureLoadTask = this;
            break;
        default:
            break;
    }
}

/**
 * Deinitialize this task.
 *
 * 0x00830210
 */
void TextureLoadTaskClass::Deinit()
{
    if (m_texture != nullptr) {
        switch (m_type) {
            case TASK_THUMBNAIL:
                m_texture->m_thumbnailTextureLoadTask = nullptr;
                break;
            case TASK_LOAD:
                m_texture->m_normalTextureLoadTask = nullptr;
                break;
            default:
                break;
        }

        Ref_Ptr_Release(m_texture);
    }
}

/**
 * Begin loading a "compressed" (dds format) texture.
 *
 * 0x008303B0
 */
bool TextureLoadTaskClass::Begin_Compressed_Load()
{
    captainslog_assert(m_texture != nullptr);
    const StringClass *name = m_texture->Get_Full_Path().Is_Empty() ? &m_texture->Get_Name() : &m_texture->Get_Full_Path();

    unsigned reduction;
    unsigned width;
    unsigned height;
    unsigned vol;
    WW3DFormat format;
    unsigned levels;

    if (!Get_Texture_Information(*name, reduction, width, height, vol, format, levels, true)) {
        return false;
    }

    unsigned v_width = width;
    unsigned v_height = height;
    TextureLoader::Validate_Texture_Size(v_width, v_height, vol);

    if ((v_width != width || v_height != height) && levels > 1) {
        for (unsigned l = 1; l < levels; ++l) {
            unsigned reduced_width = width >> l;
            unsigned reduced_height = height >> l;
            unsigned v_reduced_width = reduced_width;
            unsigned v_reduced_height = height;

            if (reduced_width < 4) {
                reduced_width = 4;
                v_reduced_width = 4;
            }

            if (reduced_height < 4) {
                reduced_height = 4;
                v_reduced_height = 4;
            }

            TextureLoader::Validate_Texture_Size(v_reduced_width, v_reduced_height, vol);

            if (reduced_width == v_reduced_width && reduced_height == v_reduced_height) {
                m_reduction += l;
                v_width = v_reduced_width;
                v_height = v_reduced_height;

                break;
            }
        }
    }

    m_width = v_width;
    m_height = v_height;
    m_format = Get_Valid_Texture_Format(format, m_texture->m_compressionAllowed);
    m_reduction = reduction;

    if (m_texture->m_allowReduction && m_texture->m_mipLevelCount != 1) {
        if (m_texture->m_mipLevelCount != 0 && (m_texture->m_mipLevelCount - m_reduction) < 1) {
            m_reduction = m_texture->m_mipLevelCount - 1;
        }
    } else {
        m_reduction = 0;
    }

    if (m_reduction >= levels) {
        m_reduction = 0;
    }

    unsigned mip_level_count = m_mipLevelCount;
    unsigned reduced_width = m_width;
    unsigned reduced_height = m_height;

    if (mip_level_count != 0) {
        mip_level_count = min(mip_level_count, levels);

        if (m_reduction != 0) {
            mip_level_count -= m_reduction;
            reduced_width >>= m_reduction;
            reduced_height >>= m_reduction;
        }
    } else {
        mip_level_count = max((int)(levels - reduction), 1);
        reduced_width = m_width >> m_reduction;
        reduced_height = m_height >> m_reduction;
    }

    unsigned mip_limit = 1;

    for (unsigned i = 4; i < m_width && i < m_height; i *= 2) {
        ++mip_limit;
    }

    mip_level_count = min(mip_level_count, mip_limit);
    m_d3dTexture = DX8Wrapper::Create_Texture(
        reduced_width, reduced_height, m_format, (MipCountType)mip_level_count, (w3dpool_t)1, false);
    m_mipLevelCount = mip_level_count;

    return true;
}

/**
 * Begin loading an "uncompressed" (tga format) texture.
 *
 * 0x00830950
 */
bool TextureLoadTaskClass::Begin_Uncompressed_Load()
{
    captainslog_assert(m_texture != nullptr);
    const StringClass *name = m_texture->Get_Full_Path().Is_Empty() ? &m_texture->Get_Name() : &m_texture->Get_Full_Path();

    unsigned reduction;
    unsigned width;
    unsigned height;
    unsigned vol;
    WW3DFormat format;
    unsigned levels;

    if (!Get_Texture_Information(*name, reduction, width, height, vol, format, levels, false)) {
        return false;
    }

    format = Get_Valid_Texture_Format(format, false);
    TextureLoader::Validate_Texture_Size(width, height, vol);

    m_width = width;
    m_height = height;
    m_reduction = reduction;

    if (m_texture->m_allowReduction && m_texture->m_mipLevelCount != 1) {
        if (m_texture->m_mipLevelCount != 0 && (m_mipLevelCount - m_reduction) == 0) {
            --m_reduction;
        }
    } else {
        m_reduction = 0;
    }

    if (m_reduction >= levels) {
        m_reduction = 0;
    }

    if (m_format != WW3D_FORMAT_UNKNOWN) {
        m_format = Get_Valid_Texture_Format(m_format, false);
    } else {
        m_format = format;
    }

    unsigned mip_level_count = m_texture->m_mipLevelCount;
    unsigned reduced_width = m_width;
    unsigned reduced_height = m_height;

    if (m_reduction != 0) {
        mip_level_count -= m_reduction;
        reduced_width >>= m_reduction;
        reduced_height >>= m_reduction;
    }

    m_d3dTexture = DX8Wrapper::Create_Texture(
        reduced_width, reduced_height, m_format, (MipCountType)mip_level_count, (w3dpool_t)1, false);

    return true;
}

/**
 * Load a "compressed" (dds format) texture mipmap.
 *
 * 0x00830B30
 */
bool TextureLoadTaskClass::Load_Compressed_Mipmap()
{
    captainslog_assert(m_texture != nullptr);
    const StringClass *name = m_texture->Get_Full_Path().Is_Empty() ? &m_texture->Get_Name() : &m_texture->Get_Full_Path();
    DDSFileClass dds(*name, m_reduction);

    if (!dds.Have_Level_Sizes() || !dds.Load()) {
        return false;
    }

    captainslog_assert(m_width != 0 && m_height != 0);
    unsigned reduced_width = m_width;
    unsigned reduced_height = m_height;
    unsigned reduction = m_reduction;

    while (reduction-- > 0) {
        reduced_width /= 2;
        reduced_height /= 2;
    }

    for (unsigned i = 0; i < m_mipLevelCount; ++i) {
        dds.Copy_Level_To_Surface(i,
            m_format,
            reduced_width,
            reduced_height,
            Get_Locked_Surface_Pointer(i),
            Get_Locked_Surface_Pitch(i),
            m_hsvAdjust);
        reduced_width /= 2;
        reduced_height /= 2;
    }

    return true;
}

/**
 * Load an "uncompressed" (tga format) texture mipmap.
 *
 * 0x00830C40
 */
bool TextureLoadTaskClass::Load_Uncompressed_Mipmap()
{
    captainslog_assert(m_texture != nullptr);

    if (m_mipLevelCount == 0) {
        return false;
    }

    const StringClass *name = m_texture->Get_Full_Path().Is_Empty() ? &m_texture->Get_Name() : &m_texture->Get_Full_Path();
    TargaImage tga;

    if (TargaImage::Error_Handler(tga.Open(*name, TargaImage::TARGA_READ), *name) != TGA_RET_OK) {
        return false;
    }

    tga.Toggle_Flip_Y();
    WW3DFormat dst_format;
    WW3DFormat src_format;
    unsigned int dest_bpp = 0;
    char palette[1024];
    Get_WW3D_Format(dst_format, src_format, dest_bpp, tga);

    if (src_format == WW3D_FORMAT_UNKNOWN) {
        return false;
    }

    dst_format = m_format;
    tga.Set_Palette(palette);
    tga.Load(*name, 1, 0);
    int srcwidth = tga.Get_Header().width;
    int srcheight = tga.Get_Header().height;
    int width = m_width;
    int height = m_height;
    uint8_t *dst_surface = 0;
    uint8_t *src_surface = reinterpret_cast<uint8_t *>(tga.Get_Image());

    if (src_format == WW3D_FORMAT_A1R5G5B5 || src_format == WW3D_FORMAT_R5G6B5 || src_format == WW3D_FORMAT_A4R4G4B4
        || src_format == WW3D_FORMAT_P8 || src_format == WW3D_FORMAT_L8 || srcwidth != width || srcheight != height) {
        dst_surface = new uint8_t[4 * width * height];
        dst_format = Get_Valid_Texture_Format(WW3D_FORMAT_A8R8G8B8, 0);
        BitmapHandlerClass::Copy_Image(dst_surface,
            width,
            height,
            4 * width,
            WW3D_FORMAT_A8R8G8B8,
            src_surface,
            srcwidth,
            srcheight,
            dest_bpp * srcwidth,
            src_format,
            (uint8_t *)tga.Get_Palette(),
            tga.Get_Header().cmap_depth >> 3,
            false,
            m_hsvAdjust);
        src_surface = dst_surface;
        src_format = WW3D_FORMAT_A8R8G8B8;
        srcwidth = width;
        srcheight = height;
        dest_bpp = Get_Bytes_Per_Pixel(WW3D_FORMAT_A8R8G8B8);
    }

    int pitch = dest_bpp * srcwidth;

    if (m_reduction != 0) {
        uint8_t *reduction_surface = new uint8_t[4 * width * height];
        for (unsigned i = 0; i < m_reduction; ++i) {
            BitmapHandlerClass::Copy_Image(reduction_surface,
                width,
                height,
                pitch,
                m_format,
                src_surface,
                srcwidth,
                srcheight,
                pitch,
                src_format,
                0,
                0,
                true,
                Vector3(0.0f, 0.0f, 0.0f));
            width >>= 1;
            height >>= 1;
            srcwidth >>= 1;
            srcheight >>= 1;

            if ((width == 0) || (height == 0) || (srcwidth == 0) || (srcheight == 0)) {
                break;
            }
        }

        delete[] reduction_surface;
    }

    for (unsigned i = 0; i < m_mipLevelCount; ++i) {
        BitmapHandlerClass::Copy_Image(Get_Locked_Surface_Pointer(i),
            width,
            height,
            Get_Locked_Surface_Pitch(i),
            m_format,
            src_surface,
            srcwidth,
            srcheight,
            pitch,
            src_format,
            0,
            0,
            true,
            Vector3(0.0f, 0.0f, 0.0f));
        width >>= 1;
        height >>= 1;
        srcwidth >>= 1;
        srcheight >>= 1;

        if ((width == 0) || (height == 0) || (srcwidth == 0) || (srcheight == 0)) {
            break;
        }
    }

    if (dst_surface != nullptr) {
        delete[] dst_surface;
    }

    return true;
}

/**
 * Lock mipmap levels in a texture and save pointers to data.
 *
 * 0x00830A80
 */
void TextureLoadTaskClass::Lock_Surfaces()
{
#ifdef BUILD_WITH_D3D8
    m_mipLevelCount = m_d3dTexture->GetLevelCount();
    captainslog_assert(m_mipLevelCount < MAX_MIPLEVEL_COUNT);

    for (unsigned i = 0; i < m_mipLevelCount; ++i) {
        D3DLOCKED_RECT rect;
        m_d3dTexture->LockRect(i, &rect, nullptr, 0);
        m_lockedSurfacePtr[i] = static_cast<uint8_t *>(rect.pBits);
        m_lockedSurfacePitch[i] = rect.Pitch;
    }
#endif
}

/**
 * Unlock mipmap levels in a texture and clear pointers to data.
 *
 * 0x00830AE0
 */
void TextureLoadTaskClass::Unlock_Surfaces()
{
#ifdef BUILD_WITH_D3D8
    captainslog_assert(m_mipLevelCount < MAX_MIPLEVEL_COUNT);

    for (unsigned i = 0; i < m_mipLevelCount; ++i) {
        if (m_lockedSurfacePtr[i] != nullptr) {
            m_d3dTexture->UnlockRect(i);
            m_lockedSurfacePtr[i] = nullptr;
        }
    }
#endif
}

/**
 * Applies the platform level texture to the texture object.
 *
 * 0x00830380
 */
void TextureLoadTaskClass::Apply(bool initialized)
{
    captainslog_assert(m_d3dTexture != W3D_TYPE_INVALID_TEXTURE);
    m_texture->Apply_New_Surface(m_d3dTexture, initialized, false);
#ifdef BUILD_WITH_D3D8
    m_d3dTexture->Release();
    m_d3dTexture = W3D_TYPE_INVALID_TEXTURE;
#endif
}

/**
 * Applies the missing texture as the platform and texture object texture.
 */
void TextureLoadTaskClass::Apply_Missing_Texture()
{
    captainslog_assert(TextureLoader::Is_DX8_Thread());
    captainslog_assert(m_d3dTexture == W3D_TYPE_INVALID_TEXTURE);
    m_d3dTexture = MissingTexture::Get_Missing_Texture();
    Apply(true);
}

/**
 * Begins the process of loading a texture file to the texture object.
 *
 * 0x008302D0
 */
bool TextureLoadTaskClass::Begin_Load()
{
    captainslog_assert(m_texture != 0);
    bool res = false;

    if (m_texture->m_compressionAllowed) {
        res = Begin_Compressed_Load();
    }

    if (!res) {
        res = Begin_Uncompressed_Load();
    }

    if (res) {
        Lock_Surfaces();
        m_loadState = STATE_LOAD_BEGUN;
    }

    return res;
}

/**
 * Performs load of the texture data.
 *
 * 0x00830310
 */
bool TextureLoadTaskClass::Load()
{
    captainslog_assert(m_texture != 0);
    bool res = false;

    if (m_texture->m_compressionAllowed) {
        res = Load_Compressed_Mipmap();
    }

    if (!res) {
        res = Load_Uncompressed_Mipmap();
    }

    m_loadState = STATE_LOADED;

    return res;
}

/**
 * Finalises the load by unlocking surfaces and applying platform texture to the texture object.
 */
void TextureLoadTaskClass::End_Load()
{
    captainslog_assert(TextureLoader::Is_DX8_Thread());
    Unlock_Surfaces();
    Apply(true);
    m_loadState = STATE_LOAD_ENDED;
}

/**
 * Performs the load stages in sequence.
 */
void TextureLoadTaskClass::Finish_Load()
{
    switch (m_loadState) {
        case STATE_NONE:
            if (!Begin_Load()) {
                Apply_Missing_Texture();
                return;
            }
        case STATE_LOAD_BEGUN: // Fallthough
            Load();
        case STATE_LOADED: // Fallthough
            End_Load();
        default:
            break;
    }
}

/**
 * Deletes all TextureLoadTaskClass objects from the free lists.
 */
void TextureLoadTaskClass::Delete_Free_Pool()
{
    for (TextureLoadTaskClass *task = g_freeList.Pop_Front(); task != nullptr; task = g_freeList.Pop_Front()) {
        delete task;
    }

    for (TextureLoadTaskClass *task = g_cubeFreeList.Pop_Front(); task != nullptr; task = g_freeList.Pop_Front()) {
        delete task;
    }

    for (TextureLoadTaskClass *task = g_volFreeList.Pop_Front(); task != nullptr; task = g_freeList.Pop_Front()) {
        delete task;
    }
}

/**
 * Creates a TextureLoadTaskClass to load to a given texture object.
 *
 * 0x0082FFD0
 */
TextureLoadTaskClass *TextureLoadTaskClass::Create(TextureBaseClass *texture, TaskType type, PriorityType priority)
{
    // TODO Original can load volume and cube textures too, test if it actuall does.
    captainslog_dbgassert(
        texture->Get_Asset_Type() == 0, "Attempting to create texture asset of type %d.", texture->Get_Asset_Type());
    captainslog_assert(texture != nullptr);

    TextureLoadTaskClass *task = g_freeList.Pop_Front();

    if (task == nullptr) {
        task = new TextureLoadTaskClass;
    }

    task->Init(texture, type, priority);

    return task;
}

/**
 * Helper to get information about a given texture file.
 *
 * 0x008305F0
 */
bool TextureLoadTaskClass::Get_Texture_Information(const char *name,
    unsigned &reduction,
    unsigned &width,
    unsigned &height,
    unsigned &depth,
    WW3DFormat &format,
    unsigned &levels,
    bool use_dds)
{
    ThumbnailClass *thumb = ThumbnailManagerClass::Peek_Thumbnail_Instance_From_Any_Manager(name);

    if (thumb != nullptr) {
        if (use_dds) {
            WW3DFormat thumb_format = thumb->Get_Format();
            if (thumb_format != WW3D_FORMAT_DXT1 && thumb_format != WW3D_FORMAT_DXT2 && thumb_format != WW3D_FORMAT_DXT3
                && thumb_format != WW3D_FORMAT_DXT4 && thumb_format != WW3D_FORMAT_DXT5) {
                return false;
            }
        }

        width = thumb->Get_Max_Width() >> reduction;
        height = thumb->Get_Max_Height() >> reduction;
        levels = thumb->Get_Mip_Levels();
        format = thumb->Get_Format();
    } else if (use_dds) {
        DDSFileClass dds(name, 0);

        if (!dds.Have_Level_Sizes()) {
            return false;
        }

        width = dds.Get_Width(0);
        height = dds.Get_Height(0);
        depth = dds.Get_Depth(0);
        format = dds.Get_Format();
        levels = dds.Get_Mip_Level_Count();
        unsigned reduct = W3D::Get_Texture_Reduction();

        if (reduct > levels) {
            reduct = levels - 1;
        }

        unsigned w = width;
        unsigned h = height;
        unsigned r;

        for (r = 0; r < reduct; ++r) {
            if (w <= W3D::Get_Texture_Min_Dimension() || h <= W3D::Get_Texture_Min_Dimension()) {
                break;
            }

            w /= 2;
            h /= 2;
        }

        reduction = r;
    } else {
        TargaImage tga;

        if (TargaImage::Error_Handler(tga.Open(name, TargaImage::TARGA_READ), name) != TGA_RET_OK) {
            return false;
        }

        WW3DFormat dst_format;
        unsigned bpp;
        Get_WW3D_Format(dst_format, format, bpp, tga);
        unsigned w = tga.Get_Header().width;
        unsigned h = tga.Get_Header().height;

        for (levels = 0; w > 0 && h > 0; ++levels) {
            w /= 2;
            h /= 2;
        }

        unsigned reduct = W3D::Get_Texture_Reduction();

        if (reduct > levels) {
            reduct = levels - 1;
        }

        w = tga.Get_Header().width;
        h = tga.Get_Header().height;
        unsigned r;

        for (r = 0; r < reduct; ++r) {
            if (w <= W3D::Get_Texture_Min_Dimension() || h <= W3D::Get_Texture_Min_Dimension()) {
                break;
            }

            w /= 2;
            h /= 2;
        }

        reduction = r;
        width = tga.Get_Header().width;
        height = tga.Get_Header().height;
        depth = 1;
    }

    return true;
}
