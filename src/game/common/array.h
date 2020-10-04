/**
 * @file
 *
 * @author feliwir
 *
 * @brief A minimalistic templated array class
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

template<typename T, int N> class Array
{
public:
    inline Array() {}

    inline Array(T val)
    {
        for (int i = 0; i < N; ++i)
            m_data[i] = val;
    }

    inline int Size() const { return N; }

    inline T &operator[](int idx) { return m_data[idx]; }

    inline T operator[](int idx) const { return m_data[idx]; }

private:
    T m_data[N];
};
