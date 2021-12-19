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

enum // Helper enums to avoid including d3d8types.h
{
    DX8_FVF_XYZ = 0x002, //        D3DFVF_XYZ | D3DFVF_TEX0
    DX8_FVF_XYZUV1 = 0x102, //     D3DFVF_XYZ | D3DFVF_TEX1
    DX8_FVF_XYZUV2 = 0x202, //     D3DFVF_XYZ | D3DFVF_TEX2

    DX8_FVF_XYZN = 0x012, //       D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX0
    DX8_FVF_XYZNUV1 = 0x112, //    D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1
    DX8_FVF_XYZNUV2 = 0x212, //    D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2

    DX8_FVF_XYZD = 0x042, //       D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX0
    DX8_FVF_XYZDUV1 = 0x142, //    D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1
    DX8_FVF_XYZDUV2 = 0x242, //    D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2

    DX8_FVF_XYZND = 0x052, //      D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX0
    DX8_FVF_XYZNDUV1 = 0x152, //   D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1
    DX8_FVF_XYZNDUV2 = 0x252, //   D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2

    DX8_FVF_XYZRHW = 0x004, //     D3DFVF_XYZRHW | D3DFVF_TEX0
    DX8_FVF_XYZRHWUV1 = 0x104, //  D3DFVF_XYZRHW | D3DFVF_TEX1
    DX8_FVF_XYZRHWUV2 = 0x204, //  D3DFVF_XYZRHW | D3DFVF_TEX2

    DX8_FVF_XYZRHWD = 0x044, //    D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX0
    DX8_FVF_XYZRHWDUV1 = 0x144, // D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1
    DX8_FVF_XYZRHWDUV2 = 0x244, // D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX2
};

class FVFInfoClass : public W3DMPO
{
    ALLOW_HOOKING
    IMPLEMENT_W3D_POOL(FVFInfoClass);

public:
    FVFInfoClass(unsigned int fvf_, unsigned int fvf_size_);
    unsigned int Get_Location_Offset() const { return m_locationOffset; }
    unsigned int Get_Normal_Offset() const { return m_normalOffset; }
    unsigned int Get_Tex_Offset(unsigned int texture) const { return m_texcoordOffset[texture]; }
    unsigned int Get_Diffuse_Offset() const { return m_diffuseOffset; }
    unsigned int Get_Specular_Offset() const { return m_specularOffset; }
    unsigned int Get_FVF() const { return m_FVF; }
    unsigned int Get_FVF_Size() const { return m_fvfSize; }
    void Get_FVF_Name(StringClass &fvfname) const;

private:
#ifdef GAME_DLL
    FVFInfoClass *Hook_Ctor(unsigned int fvf_, unsigned int fvf_size_) { return new (this) FVFInfoClass(fvf_, fvf_size_); }
#endif
    unsigned int m_FVF;
    unsigned int m_fvfSize;
    unsigned int m_locationOffset;
    unsigned int m_normalOffset;
    unsigned int m_blendOffset;
    unsigned int m_texcoordOffset[8];
    unsigned int m_diffuseOffset;
    unsigned int m_specularOffset;
};
