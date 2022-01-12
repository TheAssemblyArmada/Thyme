/**
 * @file
 *
 * @author xezon
 *
 * @brief Utilities for Standard Template Library classes. (Thyme Feature)
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

template<class ContainerType> void Free_Container(ContainerType &container)
{
    ContainerType().swap(container);
}

template<class ContainerType> void Append_Container(ContainerType &dest, const ContainerType &src)
{
    dest.insert(dest.end(), src.begin(), src.end());
}

template<class ContainerType> void Shrink_To_Fit(ContainerType &container)
{
#if THYME_USE_STLPORT
    if (container.capacity() > container.size()) {
        ContainerType tmp;
        container.swap(tmp);
        container.swap(ContainerType());
        container.assign(tmp.begin(), tmp.end());
    }
#else
    container.shrink_to_fit();
#endif
}

} // namespace rts
