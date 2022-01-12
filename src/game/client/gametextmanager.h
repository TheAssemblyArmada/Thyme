/**
 * @file
 *
 * @author OmniBlade
 * @author xezon
 *
 * @brief Game Localization Manager. (Thyme Feature)
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
#include "gametextcommon.h"
#include "gametextfile.h"
#include "gametextinterface.h"
#include "gametextlookup.h"

namespace Thyme
{
// #FEATURE New GameTextManager with new features. Can read UTF-8 STR Files. GameTextManager is self contained and will
// automatically load and read generals.csf, generals.str and map.str files.
class GameTextManager : public GameTextInterface
{
    using Utf8Strings = std::vector<Utf8String>;

public:
    GameTextManager();
    virtual ~GameTextManager();

    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override {}

    virtual Utf16String Fetch(const char *args, bool *success = nullptr) override;
    virtual Utf16String Fetch(Utf8String args, bool *success = nullptr) override;
    virtual std::vector<Utf8String> *Get_Strings_With_Prefix(Utf8String label) override;
    virtual void Init_Map_String_File(Utf8String const &filename) override;
    virtual void Deinit() override;

    static int Compare_LUT(void const *a, void const *b);
    static GameTextInterface *Create_Game_Text_Interface();

private:
    static void Collect_Labels_With_Prefix(
        Utf8Strings &found_labels, const Utf8String &search_label, const StringInfos &string_infos);

    bool m_initialized;
    bool m_useStringFile;
    Utf16String m_failed;

    // Main localization
    GameTextFile m_textFile;
    ConstGameTextLookup m_textLookup;

    // Map localization
    GameTextFile m_mapTextFile;
    ConstGameTextLookup m_mapTextLookup;

    NoString *m_noStringList;
    Utf8Strings m_stringVector;
};
} // namespace Thyme
