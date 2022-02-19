/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Greatest Common Divisor/Lowest Common Multiple
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gcd_lcm.h"

uint32_t Greatest_Common_Divisor(uint32_t a, uint32_t b)
{
    if (b) {
        a = Greatest_Common_Divisor(b, a % b);
    }
    return a;
}

uint32_t Least_Common_Multiple(uint32_t a, uint32_t b)
{
    return b * a / Greatest_Common_Divisor(a, b);
}
