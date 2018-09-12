/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Information relating to a font to use in game.
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
#include "asciistring.h"

struct GameFont
{
	Utf8String name_string;
	int point_size;
	int bold;
	int height;
	void *font_data;
	GameFont *next;
};