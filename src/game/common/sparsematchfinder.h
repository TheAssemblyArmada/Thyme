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
        // std::map requires a < operation to compare keys
        // This class implements a < operation for BitFlags
        bool operator()(const Key &left, const Key &right) const
        {
            if (left.Size() < right.Size()) {
                return true;
            }
            for (auto bit = 0; bit < left.Size(); ++bit) {
                const auto left_bit = left.Test(bit);
                const auto right_bit = right.Test(bit);
                if ((left_bit && right_bit) || (!left_bit && !right_bit)) {
                    continue;
                }
                return !left_bit;
            }
            return false;
        }
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
            m_bestMatches[key] = ret;
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
        const Type *best_match = nullptr;
        // The higher the intersection count the better the match
        int best_match_count = 0;
        // The lower the inverse intersection count the better the match
        int best_match_inv_count = 999;
        int extramatches = 0;
        Utf8String best_match_definition;
        Utf8String ambiguous_match_definition;

        for (auto &template_set : vector) {
            for (int condition_idx = template_set.Get_Conditions_Count() - 1; condition_idx >= 0; condition_idx--) {
                const Key &conditions = template_set.Get_Conditions_Yes(condition_idx);
                int intersection_count = Count_Condition_Intersection(key, conditions);
                int inverse_intersection_count = Count_Condition_Inverse_Intersection(key, conditions);

                if (intersection_count == best_match_count && inverse_intersection_count == best_match_inv_count) {
                    // We have at least two equally good matches!
                    // This is bad as it is ambiguous, hopefully there is a better match later in the search
                    extramatches++;
                    ambiguous_match_definition = template_set.Get_Definition();
                }

                // The higher the intersection count the better the match
                // If of equal intersections then the lowest inverse intersections the better the match
                if (intersection_count > best_match_count
                    || (intersection_count >= best_match_count && inverse_intersection_count < best_match_inv_count)) {
                    best_match = &template_set;
                    best_match_count = intersection_count;
                    best_match_inv_count = inverse_intersection_count;
                    extramatches = 0;
                    best_match_definition = template_set.Get_Definition();
                }
            }
        }

        if (extramatches > 0) {
            Utf8String bits;
            key.Get_Name_For_Bits(&bits);
            captainslog_debug("ambiguous model match in findBestInfoSlow \n\nbetween \n(%s)\n<and>\n(%s)\n\n(%d extra "
                              "matches found)\n\ncurrent bits are (\n%s)",
                best_match_definition.Str(),
                ambiguous_match_definition.Str(),
                extramatches,
                bits.Str());
        }

        return best_match;
    }

    mutable std::map<Key const, const Type *, MapHelper> m_bestMatches;
};
