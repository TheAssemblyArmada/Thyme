/**
 * @file
 *
 * @author OmniBlade
 * @author tomsons26
 *
 * @brief Font information and handling classes.
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
#include "subsysteminterface.h"

class FontCharsClass;

class GameFont : public MemoryPoolObject
{
    IMPLEMENT_POOL(GameFont);

protected:
    virtual ~GameFont() override {}

public:
    // #BUGFIX Initialize important members
    GameFont() : m_next(nullptr), m_fontData(nullptr) {}

    GameFont *m_next;
    Utf8String m_nameString;
    int m_pointSize;
    int m_height;
    FontCharsClass *m_fontData;
    bool m_bold;
};

class FontLibrary : public SubsystemInterface
{
public:
    FontLibrary();
    virtual ~FontLibrary() override;

    virtual void Init() {}
    virtual void Reset();
    virtual void Update() {}

    virtual bool Load_Font_Data(GameFont *font) = 0;
    virtual void Release_Font_Data(GameFont *font) {}

    void Link_Font(GameFont *font);
    void Unlink_Font(GameFont *font);
    void Delete_All_Fonts();

    GameFont *Get_Font(Utf8String name, int point_size, bool bold);
    GameFont *First_Font() const { return m_fontList; }
    GameFont *Next_Font(GameFont *font) const
    {
        if (font != nullptr) {
            return font->m_next;
        } else {
            return nullptr;
        }
    }

private:
    GameFont *m_fontList;
    int m_count;
};

#ifdef GAME_DLL
extern FontLibrary *&g_theFontLibrary;
#else
extern FontLibrary *g_theFontLibrary;
#endif
