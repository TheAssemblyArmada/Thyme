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

template<typename K, typename T> class SparseMatchFinder
{
    class MapHelper
    {
    public:
        bool operator()(const K &left, const K &right) const { return left == right; }
    };

public:
    // TODO implement more wrappers as they are determined required.
    T &operator[](const K &key) { return m_bestMatches[key]; }

private:
    std::map<K, T, MapHelper> m_bestMatches;
};
