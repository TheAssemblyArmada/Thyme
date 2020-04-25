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
#include "dx8fvf.h"
#ifdef BUILD_WITH_D3D8
#include <d3dx8.h>
#endif
#ifdef GAME_DLL
#include "hooker.h"
#endif

unsigned int Get_FVF_Vertex_Size(unsigned int fvf)
{
#ifdef BUILD_WITH_D3D8
    return D3DXGetFVFVertexSize(fvf);
#else
    return 0;
#endif
}

FVFInfoClass::FVFInfoClass(unsigned int FVF_, unsigned int fvf_size_)
{
#ifdef BUILD_WITH_D3D8
    FVF = FVF_;
    if (FVF_) {
        fvf_size = Get_FVF_Vertex_Size(FVF);
    } else {
        fvf_size = fvf_size_;
    }
    location_offset = 0;
    blend_offset = 0;
    if ((FVF & D3DFVF_XYZ) == D3DFVF_XYZ) {
        blend_offset = 0x0C;
    }
    normal_offset = blend_offset;
    if (((FVF & D3DFVF_XYZB4) == D3DFVF_XYZB4) && ((FVF & D3DFVF_LASTBETA_UBYTE4) == D3DFVF_LASTBETA_UBYTE4)) {
        normal_offset = blend_offset + 0x10;
    }
    diffuse_offset = normal_offset;
    if ((FVF & D3DFVF_NORMAL) == D3DFVF_NORMAL) {
        diffuse_offset = normal_offset + 0x0C;
    }
    specular_offset = diffuse_offset;
    if ((FVF & D3DFVF_DIFFUSE) == D3DFVF_DIFFUSE) {
        specular_offset = diffuse_offset + 4;
    }
    texcoord_offset[0] = specular_offset;
    if ((FVF & D3DFVF_SPECULAR) == D3DFVF_SPECULAR) {
        texcoord_offset[0] = specular_offset + 4;
    }
    int a = 1;
    for (unsigned int i = 0x0F; (i - 0xE) < 8; i++) {
        texcoord_offset[a] = texcoord_offset[a - 1];
        if ((FVF & (3 << i)) == (unsigned int)(3 << i)) {
            texcoord_offset[a] = texcoord_offset[a - 1] + 4;
        } else {
            texcoord_offset[a] = texcoord_offset[a - 1] + 8;
        }
        a++;
    }
#endif
}

//unimplemented, not required
//void FVFInfoClass::Get_FVF_Name(StringClass &fvfname) const
