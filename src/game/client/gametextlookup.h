/**
 * @file
 *
 * @author xezon
 *
 * @brief Game Localization Lookup. (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "gametextcommon.h"
#include "utility/stlutil.h"
#include <cstdlib>

namespace Thyme
{

struct ConstStringLookup
{
    const char *label;
    const StringInfo *string_info;
};

struct MutableStringLookup
{
    const char *label;
    StringInfo *string_info;
};

struct ConstMultiStringLookup
{
    const char *label;
    const MultiStringInfo *string_info;
};

struct MutableMultiStringLookup
{
    const char *label;
    MultiStringInfo *string_info;
};

// #FEATURE Template class to build a lookup map to help find strings by their label from a given string info container.
// Prefer using the type aliases further down below. An instance of this class must not be used after the source string info
// container has been changed or deleted.
template<typename StringLookup, typename StringInfos> class GameTextLookup
{
    using StringLookups = std::vector<StringLookup>;

public:
    explicit GameTextLookup() {}
    explicit GameTextLookup(StringInfos &string_infos) { Load(string_infos); }

    void Load(StringInfos &string_infos)
    {
        if (string_infos.empty()) {
            return;
        }

        const size_t string_count = string_infos.size();
        m_stringLookups.resize(string_count);

        for (size_t i = 0; i < string_count; ++i) {
            m_stringLookups[i].label = string_infos[i].label.Str();
            m_stringLookups[i].string_info = &string_infos[i];
        }

        void *base = &m_stringLookups[0];
        const size_t item_count = m_stringLookups.size();
        const size_t item_size = sizeof(StringLookup);

        qsort(base, item_count, item_size, Compare_LUT);
    }

    void Unload() { rts::Free_Container(m_stringLookups); }

    const StringLookup *Find(const char *label) const
    {
        if (m_stringLookups.empty()) {
            return nullptr;
        }

        StringLookup key;
        key.label = label;
        key.string_info = nullptr;

        const void *base = &m_stringLookups[0];
        const size_t item_count = m_stringLookups.size();
        const size_t item_size = sizeof(StringLookup);

        return static_cast<const StringLookup *>(bsearch(&key, base, item_count, item_size, Compare_LUT));
    }

private:
    static int Compare_LUT(const void *a, const void *b)
    {
        const char *ac = static_cast<const StringLookup *>(a)->label;
        const char *bc = static_cast<const StringLookup *>(b)->label;

        return strcasecmp(ac, bc);
    }

    StringLookups m_stringLookups;
};

// Aliases for convenience.

using ConstGameTextLookup = GameTextLookup<ConstStringLookup, const StringInfos>;
using MutableGameTextLookup = GameTextLookup<MutableStringLookup, StringInfos>;
using ConstMultiGameTextLookup = GameTextLookup<ConstMultiStringLookup, const MultiStringInfos>;
using MutableMultiGameTextLookup = GameTextLookup<MutableMultiStringLookup, MultiStringInfos>;

} // namespace Thyme
