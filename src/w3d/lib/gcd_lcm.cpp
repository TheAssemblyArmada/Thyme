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

unsigned int Greatest_Common_Divisor(unsigned int a, unsigned int b)
{
    if (b) {
        a = Greatest_Common_Divisor(b, a % b);
    }
    return a;
}

unsigned int Least_Common_Multiple(unsigned int a, unsigned int b)
{
    return b * a / Greatest_Common_Divisor(a, b);
}
