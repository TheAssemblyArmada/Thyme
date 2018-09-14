/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Base interface for fetching a stream of data.
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

enum StrawControl
{
    STRAW_DECODE = 1,
    STRAW_DECRYPT = 1,
    STRAW_UNCOMPRESS = 1,
    STRAW_ENCODE = 0,
    STRAW_ENCRYPT = 0,
    STRAW_COMPRESS = 0,
};

class Straw
{
public:
    Straw() : m_chainTo(nullptr), m_chainFrom(nullptr) {}
    Straw(Straw const &that) : m_chainTo(that.m_chainTo), m_chainFrom(that.m_chainFrom) {}
    virtual ~Straw();

    Straw &operator=(Straw &that);

    virtual void Get_From(Straw *straw);
    virtual int Get(void *source, int length);

protected:
    Straw *m_chainTo;
    Straw *m_chainFrom;
};

inline Straw &Straw::operator=(Straw &that)
{
    if (this != &that) {
        m_chainTo = that.m_chainTo;
        m_chainFrom = that.m_chainFrom;
    }

    return *this;
}
