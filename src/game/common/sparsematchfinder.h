/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Template class providing a wrapper around a key/value pair container.
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
#include <map>

template<typename Type, typename Key> class SparseMatchFinder
{
    class MapHelper
    {
    public:
        bool operator()(const Key &left, const Key &right) const { return left == right; }
    };

    static int Count_Condition_Intersection(const Key &key1, const Key &key2) { return key1.Count_Intersection(key2); }
    static int Count_Condition_Inverse_Intersection(const Key &key1, const Key &key2)
    {
        return key1.Count_Inverse_Intersection(key2);
    }

public:
    const Type *Find_Best_Info(std::vector<Type> const &vector, Key const &key) const
    {
        auto iter = m_bestMatches.find(key);
        const Type *ret = nullptr;

        if (iter != m_bestMatches.end()) {
            ret = (*iter).second;
        }

        if (ret != nullptr) {
            return ret;
        }

        ret = Find_Best_Info_Slow(vector, key);
        captainslog_dbgassert(ret, "no suitable match for criteria was found!");

        if (ret != nullptr) {
            const_cast<std::map<Key const, const Type *, MapHelper> &>(m_bestMatches)[key] = ret;
        }

        return ret;
    }

    void Clear() { m_bestMatches.clear(); }

    const SparseMatchFinder &operator=(const SparseMatchFinder &that)
    {
        m_bestMatches = that.m_bestMatches;
        return *this;
    }

private:
    const Type *Find_Best_Info_Slow(std::vector<Type> const &vector, Key const &key) const
    {
        const Type *ret = nullptr;
        int intersection_count = 0;
        int inverse_intersection_count = 999;
        int extramatches = 0;
        Utf8String definition;
        Utf8String definition2;

        for (auto &i : vector) {
            for (int j = i.Unk() - 1; j >= 0; j--) {
                const Key &conditions = i.Get_Conditions_Yes(j);
                int int_count = Count_Condition_Intersection(key, conditions);
                int invintcount = Count_Condition_Inverse_Intersection(key, conditions);

                if (int_count == intersection_count && invintcount == inverse_intersection_count) {
                    extramatches++;
                    definition2 = i.Get_Definition();
                }

                if (int_count > intersection_count
                    || int_count >= intersection_count && invintcount < inverse_intersection_count) {
                    ret = &i;
                    intersection_count = int_count;
                    inverse_intersection_count = invintcount;
                    extramatches = 0;
                    definition = i.Get_Definition();
                }
            }
        }

        if (extramatches > 0) {
            Utf8String bits;
            key.Get_Name_For_Bits(&bits);
            captainslog_debug("ambiguous model match in findBestInfoSlow \n\nbetween \n(%s)\n<and>\n(%s)\n\n(%d extra "
                              "matches found)\n\ncurrent bits are (\n%s)",
                definition.Str(),
                definition2.Str(),
                extramatches,
                bits.Str());
        }

        return ret;
    }

    std::map<Key const, const Type *, MapHelper> m_bestMatches;
};
