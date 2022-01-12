/**
 * @file
 *
 * @author xezon
 *
 * @brief Common structures for Game Localization.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "asciistring.h"
#include "bittype.h"
#include "unicodestring.h"
#include "utility/type_traits.h"
#include <vector>

// This enum applies to RA2/YR and Generals/ZH, BFME ID's are slightly different.
enum class LanguageID : int32_t
{
    // Official game languages.
    US = 0,
    UK = 1,
    GERMAN = 2,
    FRENCH = 3,
    SPANISH = 4,
    ITALIAN = 5,
    JAPANESE = 6,
    JABBER = 7,
    KOREAN = 8,
    CHINESE = 9,
    UNUSED_1 = 10,
    BRAZILIAN = 11,
    POLISH = 12,

    // Unspecified language. Default in GameTextFile class.
    UNKNOWN = 13,

    // Community game languages.
    RUSSIAN = 14,
    ARABIC = 15,

    COUNT
};

DEFINE_RTS_UNDERLYING_TYPE(LanguageID, int32_t);

constexpr size_t g_languageCount = static_cast<size_t>(LanguageID::COUNT);

struct CSFHeader
{
    uint32_t id;
    int32_t version;
    int32_t num_labels;
    int32_t num_strings;
    int32_t skip;
    LanguageID langid;
};

struct StringInfo
{
    Utf8String label;
    Utf16String text;
    Utf8String speech;
};

struct NoString
{
    NoString *next;
    Utf16String text;
};

namespace Thyme
{

struct MultiStringInfo
{
    Utf8String label;
    Utf16String text[g_languageCount];
    Utf8String speech[g_languageCount];
};

using StringInfos = std::vector<StringInfo>;
using MultiStringInfos = std::vector<MultiStringInfo>;

bool Name_To_Language(const char *localization, LanguageID &language);
const char *Get_Language_Name(LanguageID language);

} // namespace Thyme
