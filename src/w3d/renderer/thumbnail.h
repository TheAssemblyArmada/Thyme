/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Object containing thumbnail information.
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
#include "w3dformat.h"
#include "wwstring.h"
#include <new>

class ThumbnailManagerClass;

class ThumbnailClass
{
public:
    ThumbnailClass(ThumbnailManagerClass *manager, char const *texture, uint8_t *bits, unsigned width, unsigned height,
        unsigned maxwidth, unsigned maxheight, unsigned miplevels, WW3DFormat format, bool isallocated, unsigned time);
    ThumbnailClass(ThumbnailManagerClass *manager, const StringClass &texture);
    ~ThumbnailClass();

    const StringClass &Get_Name() const { return m_filename; }
    uint8_t *Get_Bitmap() const { return m_bitmap; }

#ifndef THYME_STANDALONE
    ThumbnailClass *Hook_Ctor(ThumbnailManagerClass *manager, const StringClass &texture)
    {
        return new (this) ThumbnailClass(manager, texture);
    }

    static void Hook_Me();
#endif

private:
    StringClass m_filename;
    uint8_t *m_bitmap;
    int m_width;
    int m_height;
    int m_maxWidth;
    int m_maxHeight;
    int m_mipLevels;
    WW3DFormat m_format;
    unsigned m_time;
    bool m_isAllocated;
    ThumbnailManagerClass *m_manager;
};

#ifndef THYME_STANDALONE
#include "hooker.h"

inline void ThumbnailClass::Hook_Me()
{
    Hook_Method(0x0086A040, &ThumbnailClass::Hook_Ctor);
}
#endif
