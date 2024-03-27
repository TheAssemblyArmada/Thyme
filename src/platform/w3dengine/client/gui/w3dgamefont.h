/**
 * @file
 *
 * @author tomsons26
 *
 * @brief W3D Font information and handling class.
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
#include "gamefont.h"

class W3DFontLibrary : public FontLibrary
{
public:
    W3DFontLibrary() {}
    virtual ~W3DFontLibrary() override {}

    virtual bool Load_Font_Data(GameFont *font) override;
    virtual void Release_Font_Data(GameFont *font) override;
};
