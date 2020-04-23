/**
 * @file
 *
 * @author Jonathan Wilson
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

class FVFInfoClass : public W3DMPO
{
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
    unsigned int FVF;
    unsigned int fvf_size;
    unsigned int location_offset;
    unsigned int normal_offset;
    unsigned int blend_offset;
    unsigned int texcoord_offset[8];
    unsigned int diffuse_offset;
    unsigned int specular_offset;
};
