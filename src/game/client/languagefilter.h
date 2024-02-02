/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Language Filter
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
#include "subsysteminterface.h"
#include "unicodestring.h"
#include <map>

class File;

class LanguageFilter : public SubsystemInterface
{
public:
    LanguageFilter();
    virtual ~LanguageFilter() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    void Filter_Line(Utf16String &line);
    void Un_Haxor(Utf16String &word);
    bool Read_Word(File *file, unichar_t *buf);

private:
    std::map<Utf16String, bool, UnicodeStringLessThan> m_wordList;
    std::map<Utf16String, bool, UnicodeStringLessThan> m_subWordList;
};

LanguageFilter *Create_Language_Filter();

#ifdef GAME_DLL
extern LanguageFilter *&g_theLanguageFilter;
#else
extern LanguageFilter *g_theLanguageFilter;
#endif
