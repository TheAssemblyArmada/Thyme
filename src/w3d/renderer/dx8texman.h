/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Keeps track of textures allocated from the graphics library.
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
#include "multilist.h"
#include "texturebase.h"
#include "w3dformat.h"

class DX8TextureTrackerClass;
typedef MultiListClass<DX8TextureTrackerClass> DX8TextureTrackerList;
typedef MultiListIterator<DX8TextureTrackerClass> DX8TextureTrackerListIterator;

class DX8TextureTrackerClass : public MultiListObjectClass
{
public:
    DX8TextureTrackerClass(unsigned width,
        unsigned height,
        MipCountType mip_levels,
        TextureBaseClass *texture,
        WW3DFormat format,
        bool render_target) :
        m_width(width),
        m_height(height),
        m_mipLevelCount(mip_levels),
        m_texture(texture),
        m_format(format),
        m_renderTarget(render_target)
    {
    }
    virtual ~DX8TextureTrackerClass() {}
    virtual void Recreate();

public:
    unsigned m_width;
    unsigned m_height;
    MipCountType m_mipLevelCount;
    TextureBaseClass *m_texture;
    WW3DFormat m_format;
    bool m_renderTarget;
};

class DX8TextureManagerClass
{
public:
    static void Add(DX8TextureTrackerClass *tracker);
    static void Remove(TextureBaseClass *texture);
    static void Release_Textures();
    static void Recreate_Textures();
    static void Shutdown();

private:
    static DX8TextureTrackerList s_managedTextures;
};
