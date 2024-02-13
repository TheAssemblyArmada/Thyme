/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief CPU clock measurement
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "mpu.h"
#include "intrinsics.h"

unsigned long Get_CPU_Clock(unsigned long &high)
{
    uint64_t time = 0;
#ifdef HAVE__RDTSC
    time = __rdtsc();
#endif
    high = time >> 32;
    return time & 0xFFFFFFFF;
}
