/**
 * @file
 *
 * @author OmniBlade
 * @author tomsons26
 *
 * @brief Missing asset handling.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "missing.h"
#include "dx8wrapper.h"
#include "w3dformat.h"

#ifndef THYME_STANDALONE
w3dtexture_t &MissingTextureClass::s_missingTexture = Make_Global<w3dtexture_t>(0x00A522F8);
#else
w3dtexture_t MissingTextureClass::s_missingTexture;
#endif

w3dtexture_t MissingTextureClass::Get_Missing_Texture()
{
#ifdef BUILD_WITH_D3D8
    s_missingTexture->AddRef();
#endif

    return s_missingTexture;
}

void MissingTextureClass::Init()
{
    static const int _missing_width = 256;
    static const int _missing_height = 256;

    // TODO, looks like it uses some of D3DX8
#ifndef THYME_STANDALONE
    Call_Function<void>(0x0084A610);
#endif
}

void MissingTextureClass::Deinit()
{
#ifdef BUILD_WITH_D3D8
    s_missingTexture->Release();
#endif
    s_missingTexture = W3D_TYPE_INVALID_TEXTURE;
}
