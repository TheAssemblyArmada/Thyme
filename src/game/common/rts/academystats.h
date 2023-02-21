/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Academy Stats
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

class AcademyStats
{
public:
    char unk[0x9C];
    unsigned int m_minesClearedCount;
    char unk3[0x14];

    void Record_Income();
    void Increment_Mines_Disarmed() { m_minesClearedCount++; }
};
