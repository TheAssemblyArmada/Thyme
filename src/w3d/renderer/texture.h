/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Class implementing the texture interface for surface textures.
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
#include "texturebase.h"
#include "w3dformat.h"

class TextureFilterClass
{
public:
    // TODO
private:
    char *m_minTextureFilters;
    char *m_magTextureFilters;
    char *m_mipMapFilters;
    uint32_t UAddressMode;
    uint32_t VAddressMode;
};


class TextureClass final : public TextureBaseClass
{
public:
    // TODO
    WW3DFormat Texture_Format() const { return m_textureFormat; }

private:
    WW3DFormat m_textureFormat;
    TextureFilterClass m_textureFilter;
};
