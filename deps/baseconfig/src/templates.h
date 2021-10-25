/**
 * @file
 *
 * @author xezon
 *
 * @brief various useful Standard Template Library functions
 *
 * @copyright Baseconfig is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#ifndef BASE_TEMPLATES_H
#define BASE_TEMPLATES_H

#ifdef __cplusplus

#include <algorithm>

namespace stl
{
// Unable to put remove_if in std namespace. STL Port will report std::remove_if missing, but at the same time complain if
// added in std namespace here.

// Sorts elements and returns iterator from where to erase from.
template<class ForwardIterator, class UnaryPredicate>
ForwardIterator remove_if(ForwardIterator first, ForwardIterator last, UnaryPredicate pred)
{
    ForwardIterator result = first;
    while (first != last) {
        if (!pred(*first)) {
            if (result != first)
                *result = *first;
            ++result;
        }
        ++first;
    }
    return result;
}

// Erases all elements where predicate is true.
template<class Container, class UnaryPredicate>
typename Container::size_type erase_if(Container &container, UnaryPredicate pred)
{
    typename Container::iterator remove_at = ::stl::remove_if(container.begin(), container.end(), pred);
    typename Container::size_type count = ::std::distance(remove_at, container.end());
    container.erase(remove_at, container.end());
    return count;
}

// Fill array with value.
template<typename T, size_t Size> void fill(T (&array)[Size], const T &val)
{
    ::std::fill_n(array, Size, val);
}

} // namespace stl

#endif // __cplusplus

#endif // BASE_TEMPLATES_H
