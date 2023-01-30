/**
 * @file
 *
 * @author xezon
 *
 * @brief Utilities, equivalent to std utility. (Thyme Feature)
          Can be removed if STL Port is abandoned.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

namespace rts
{

template<class T> T &&forward(typename remove_reference<T>::type &t) noexcept
{
    return static_cast<T &&>(t);
}

template<class T> T &&forward(typename remove_reference<T>::type &&t) noexcept
{
    return static_cast<T &&>(t);
}

} // namespace rts
