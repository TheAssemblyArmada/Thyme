/**
 * @file
 *
 * @author xezon
 *
 * @brief Common structures for Game Localization. (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gametextcommon.h"
#include "utility/enumerator.h"

namespace Thyme
{

constexpr const char *const s_localization_us = "English";
constexpr const char *const s_localization_en = "";
constexpr const char *const s_localization_de = "German";
constexpr const char *const s_localization_fr = "French";
constexpr const char *const s_localization_es = "Spanish";
constexpr const char *const s_localization_it = "Italian";
constexpr const char *const s_localization_ja = "Japanese";
constexpr const char *const s_localization_jb = "";
constexpr const char *const s_localization_ko = "Korean";
constexpr const char *const s_localization_zh = "Chinese";
constexpr const char *const s_localization___ = "";
constexpr const char *const s_localization_bp = "Brazilian";
constexpr const char *const s_localization_pl = "Polish";
constexpr const char *const s_localization_uk = "Unknown";
constexpr const char *const s_localization_ru = "Russian";
constexpr const char *const s_localization_ar = "Arabic";

constexpr const char *const s_localizations[] = {
    s_localization_us,
    s_localization_en,
    s_localization_de,
    s_localization_fr,
    s_localization_es,
    s_localization_it,
    s_localization_ja,
    s_localization_jb,
    s_localization_ko,
    s_localization_zh,
    s_localization___,
    s_localization_bp,
    s_localization_pl,
    s_localization_uk,
    s_localization_ru,
    s_localization_ar,
};

static_assert(s_localization_us == s_localizations[size_t(LanguageID::US)]);
static_assert(s_localization_en == s_localizations[size_t(LanguageID::UK)]);
static_assert(s_localization_de == s_localizations[size_t(LanguageID::GERMAN)]);
static_assert(s_localization_fr == s_localizations[size_t(LanguageID::FRENCH)]);
static_assert(s_localization_es == s_localizations[size_t(LanguageID::SPANISH)]);
static_assert(s_localization_it == s_localizations[size_t(LanguageID::ITALIAN)]);
static_assert(s_localization_ja == s_localizations[size_t(LanguageID::JAPANESE)]);
static_assert(s_localization_jb == s_localizations[size_t(LanguageID::JABBER)]);
static_assert(s_localization_ko == s_localizations[size_t(LanguageID::KOREAN)]);
static_assert(s_localization_zh == s_localizations[size_t(LanguageID::CHINESE)]);
static_assert(s_localization___ == s_localizations[size_t(LanguageID::UNUSED_1)]);
static_assert(s_localization_bp == s_localizations[size_t(LanguageID::BRAZILIAN)]);
static_assert(s_localization_pl == s_localizations[size_t(LanguageID::POLISH)]);
static_assert(s_localization_uk == s_localizations[size_t(LanguageID::UNKNOWN)]);
static_assert(s_localization_ru == s_localizations[size_t(LanguageID::RUSSIAN)]);
static_assert(s_localization_ar == s_localizations[size_t(LanguageID::ARABIC)]);

static_assert(ARRAY_SIZE(s_localizations) == g_languageCount);

bool Name_To_Language(const char *localization, LanguageID &language)
{
    rts::enumerator<LanguageID> it;

    for (const char *name : s_localizations) {
        if (0 == strcasecmp(localization, name)) {
            language = it.value();
            return true;
        }
        ++it;
    }
    return false;
}

const char *Get_Language_Name(LanguageID language)
{
    return s_localizations[static_cast<size_t>(language)];
}

} // namespace Thyme
