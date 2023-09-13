/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "tiledata.h"

void TileData::Do_Mip(unsigned char *hi_res, int hi_row, unsigned char *lo_res)
{
    for (int i = 0; i < hi_row; i += 2) {
        for (int j = 0; j < hi_row; j += 2) {
            int i1 = 4 * (i + hi_row * j);
            int i2 = 4 * (i / 2 + hi_row / 2 * (j / 2));

            for (int k = 0; k < 4; k++) {
                lo_res[i2] = (hi_res[4 * hi_row + i1] + hi_res[i1 + 4] + hi_res[i1] + hi_res[4 * hi_row + 4 + i1] + 2) / 4;
                ++i1;
                ++i2;
            }
        }
    }
}

void TileData::Update_Mips()
{
    Do_Mip(m_tileData, 64, m_tileDataMip32);
    Do_Mip(m_tileDataMip32, 32, m_tileDataMip16);
    Do_Mip(m_tileDataMip16, 16, m_tileDataMip8);
    Do_Mip(m_tileDataMip8, 8, m_tileDataMip4);
    Do_Mip(m_tileDataMip4, 4, m_tileDataMip2);
    Do_Mip(m_tileDataMip2, 2, m_tileDataMip1);
}

bool TileData::Has_RGB_Data_For_Width(int width)
{
    switch (width) {
        case 64:
            return 1;
        case 32:
            return 1;
        case 16:
            return 1;
        case 8:
            return 1;
        case 4:
            return 1;
        case 2:
            return 1;
    }
    return width == 1;
}

unsigned char *TileData::Get_RGB_Data_For_Width(int width)
{
    switch (width) {
        case 32:
            return m_tileDataMip32;
        case 16:
            return m_tileDataMip16;
        case 8:
            return m_tileDataMip8;
        case 4:
            return m_tileDataMip4;
        case 2:
            return m_tileDataMip2;
        case 1:
            return m_tileDataMip1;
    }
    return m_tileData;
}
