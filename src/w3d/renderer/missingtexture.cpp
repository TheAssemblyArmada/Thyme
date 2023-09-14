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
#include "missingtexture.h"
#include "dx8wrapper.h"
#include "w3dformat.h"
#include <captainslog.h>

#ifndef GAME_DLL
w3dtexture_t MissingTexture::s_missingTexture;
#endif

w3dtexture_t MissingTexture::Get_Missing_Texture()
{
#ifdef BUILD_WITH_D3D8
    s_missingTexture->AddRef();
#endif

    return s_missingTexture;
}

void MissingTexture::Init()
{
    static const int _missing_width = 256;
    static const int _missing_height = 256;

#ifdef BUILD_WITH_D3D8
    captainslog_assert(s_missingTexture == W3D_TYPE_INVALID_TEXTURE);
    w3dsurface_t dest = W3D_TYPE_INVALID_SURFACE;
    w3dsurface_t src = W3D_TYPE_INVALID_SURFACE;
    RECT rect{ 0, 0, _missing_width, _missing_height };
    D3DLOCKED_RECT locked_rect;

    w3dtexture_t texture =
        DX8Wrapper::Create_Texture(_missing_width, _missing_height, WW3D_FORMAT_A8R8G8B8, (MipCountType)0, (w3dpool_t)1, 0);
    texture->LockRect(0, &locked_rect, &rect, 0);
    uint32_t *pixels = static_cast<uint32_t *>(locked_rect.pBits);

    // Builds the texture, jut pink for now as original had.
    for (int h = 0; h < _missing_height; ++h) {
        for (int w = 0; w < _missing_width; ++w) {
            pixels[h * (locked_rect.Pitch / 4) + w] = 0x7FFF00FF;
        }
    }

    texture->UnlockRect(0);

    s_missingTexture = texture;
#endif
}

void MissingTexture::Deinit()
{
#ifdef BUILD_WITH_D3D8
    s_missingTexture->Release();
#endif
    s_missingTexture = W3D_TYPE_INVALID_TEXTURE;
}

w3dsurface_t MissingTexture::Create_Missing_Surface()
{
    w3dsurface_t surf = W3D_TYPE_INVALID_SURFACE;
#ifdef BUILD_WITH_D3D8
    w3dsurface_t tsurf = W3D_TYPE_INVALID_SURFACE;
    s_missingTexture->GetSurfaceLevel(0, &tsurf);
    D3DSURFACE_DESC tex_surf_desc;
    memset(&tex_surf_desc, 0, sizeof(tex_surf_desc));
    tsurf->GetDesc(&tex_surf_desc);
    DX8CALL(CreateImageSurface(tex_surf_desc.Width, tex_surf_desc.Height, tex_surf_desc.Format, &surf));
    DX8CALL(CopyRects(tsurf, nullptr, 0, surf, 0));
    tsurf->Release();
#endif
    return surf;
}
