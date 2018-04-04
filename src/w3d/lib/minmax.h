/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Functions for getting min and max of two values and clamping a value to a range.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

/**
 * @brief Returns the minimum of 'minval' and 'maxval'.
 */
template<typename T>
T Min(T minval, T maxval)
{
    return minval < maxval ? minval : maxval;
}

/**
 * @brief Returns the maximum of 'minval' and 'maxval'.
 */
template<typename T>
T Max(T minval, T maxval)
{
    return maxval < minval ? minval : maxval;
}

/**
 * @brief Returns the original value or the min or max value if the original was outside that range.
 */
template<class T>
T Clamp(T original, T minval, T maxval)
{
    if (original < minval) {
        return minval;
    }

    if (original > maxval) {
        return maxval;
    }

    return original;
};