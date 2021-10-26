/**
 * @file
 *
 * @author xezon
 *
 * @brief Utility class that can be privately inherited from to avoid copies.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

class NotCopyable
{
public:
    NotCopyable() = default;

private:
    NotCopyable(const NotCopyable &) = delete;
    NotCopyable &operator=(const NotCopyable &) = delete;
};
