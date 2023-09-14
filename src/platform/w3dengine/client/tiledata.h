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
#pragma once

#include "always.h"
#include "coord.h"
#include "refcount.h"

class TileData : public RefCountClass
{
public:
    static void Do_Mip(unsigned char *hi_res, int hi_row, unsigned char *lo_res);
    unsigned char *Get_Data_Ptr() { return m_tileData; }
    void Update_Mips();
    bool Has_RGB_Data_For_Width(int width);
    unsigned char *Get_RGB_Data_For_Width(int width);

protected:
    unsigned char m_tileData[16384];
    unsigned char m_tileDataMip32[4096];
    unsigned char m_tileDataMip16[1024];
    unsigned char m_tileDataMip8[256];
    unsigned char m_tileDataMip4[64];
    unsigned char m_tileDataMip2[16];
    unsigned char m_tileDataMip1[4];

public:
    ICoord2D m_tileLocationInTexture;
};
