/**
 * @file
 *
 * @author Jonathan Wilson
 * @author thomsons26
 *
 * @brief FVFInfo class
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
#include "w3dmpo.h"
#include "wwstring.h"
enum
{
    DX8_FVF_XYZ = 0x2,
    DX8_FVF_XYZN = 0x12,
    DX8_FVF_XYZNUV1 = 0x112,
    DX8_FVF_XYZNUV2 = 0x212,
    DX8_FVF_XYZNDUV1 = 0x152,
    DX8_FVF_XYZNDUV2 = 0x252,
    DX8_FVF_XYZDUV1 = 0x142,
    DX8_FVF_XYZDUV2 = 0x242,
    DX8_FVF_XYZUV1 = 0x102,
    DX8_FVF_XYZUV2 = 0x202,
};

class FVFInfoClass : public W3DMPO
{
    ALLOW_HOOKING
public:
    FVFInfoClass(unsigned int FVF_, unsigned int fvf_size_);
    unsigned int Get_Location_Offset() const { return location_offset; }
    unsigned int Get_Normal_Offset() const { return normal_offset; }
    unsigned int Get_Tex_Offset(unsigned int texture) const { return texcoord_offset[texture]; }
    unsigned int Get_Diffuse_Offset() const { return diffuse_offset; }
    unsigned int Get_Specular_Offset() const { return specular_offset; }
    unsigned int Get_FVF() const { return FVF; }
    unsigned int Get_FVF_Size() const { return fvf_size; }
    void Get_FVF_Name(StringClass &fvfname) const;

private:
#ifdef GAME_DLL
    FVFInfoClass *Hook_Ctor(unsigned int FVF_, unsigned int fvf_size_)
    {
        return new (this) FVFInfoClass(FVF_, fvf_size_);
    }
#endif
    unsigned int FVF;
    unsigned int fvf_size;
    unsigned int location_offset;
    unsigned int normal_offset;
    unsigned int blend_offset;
    unsigned int texcoord_offset[8];
    unsigned int diffuse_offset;
    unsigned int specular_offset;
};
