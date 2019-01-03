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
#include "texture.h"
#include "textureloadtasklist.h"
#include <cstring>

using std::memset;

TextureLoadTaskClass::TextureLoadTaskClass() :
    m_listNode{nullptr, nullptr},
    m_parent(nullptr),
    m_texture(nullptr),
    m_d3dTexture(W3D_TYPE_INVALID_TEXTURE),
    m_format(WW3D_FORMAT_UNKNOWN),
    m_width(0),
    m_height(0),
    m_mipLevelCount(0),
    m_reduction(0),
    m_hsvAdjust(),
    m_type(TASK_NONE),
    m_priority(PRIORITY_BACKGROUND),
    m_loadState(STATE_NONE)
{
    memset(m_lockedSurfacePtr, 0, sizeof(m_lockedSurfacePtr));
    memset(m_lockedSurfacePitch, 0, sizeof(m_lockedSurfacePitch));
}

TextureLoadTaskClass::~TextureLoadTaskClass()
{
    Deinit();
}

void TextureLoadTaskClass::Destroy()
{
    Deinit();
    g_freeList.Push_Front(this);
}

void TextureLoadTaskClass::Init(TextureBaseClass *texture, TaskType type, PriorityType priority)
{
    texture->Add_Ref();
    m_texture->Release_Ref();

    m_type = type;
    m_texture = texture;
    m_priority = priority;
    m_loadState = STATE_NONE;
    m_d3dTexture = W3D_TYPE_INVALID_TEXTURE;

    if (texture->As_Texture() != nullptr) {
        m_format = texture->As_Texture()->Get_Format();
    } else {
        m_format = WW3D_FORMAT_UNKNOWN;
    }

    m_width = 0;
    m_height = 0;
    m_mipLevelCount = m_texture->m_mipLevelCount;
    m_reduction = m_texture->Get_Reduction();
    m_hsvAdjust = m_texture->m_hsvShift;

    for (int i = 0; i < MAX_SURFACES; ++i) {
        m_lockedSurfacePtr[i] = nullptr;
        m_lockedSurfacePitch[i] = 0;
    }

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

void TextureLoadTaskClass::Begin_Compressed_Load() {}

void TextureLoadTaskClass::Begin_Uncompressed_Load() {}

void TextureLoadTaskClass::Load_Compressed_Mipmap() {}

void TextureLoadTaskClass::Load_Uncompressed_Mipmap() {}

void TextureLoadTaskClass::Lock_Surfaces() {}

void TextureLoadTaskClass::Unlock_Surfaces() {}

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

void TextureLoadTaskClass::Get_Texture_Information(const char *name, unsigned &reduction, unsigned &width, unsigned &height,
    unsigned &depth, WW3DFormat &format, unsigned &levels, bool use_dds)
{
#ifndef THYME_STANDALONE
    Call_Function<void, const char *, unsigned &, unsigned &, unsigned &, unsigned &, WW3DFormat &, unsigned &, bool>(
        0x008305F0, name, reduction, width, height, depth, format, levels, use_dds);
#endif
}
