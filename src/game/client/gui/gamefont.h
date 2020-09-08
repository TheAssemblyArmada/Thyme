/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Information relating to a font to use in game.
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
#include "asciistring.h"
#include "mempoolobj.h"

class GameFont : public MemoryPoolObject
{
    IMPLEMENT_NAMED_POOL(GameFont, GameFont);

public:
    GameFont() {}
    virtual ~GameFont() {}

public:
    GameFont *m_next;
    Utf8String m_nameString;
    int m_pointSize;
    int m_height;
    void *m_fontData;
    bool m_bold;
};
