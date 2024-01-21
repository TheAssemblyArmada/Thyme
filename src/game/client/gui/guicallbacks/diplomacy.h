/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Diplomacy
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

void Toggle_Diplomacy(bool immediate);

void Update_Diplomacy_Briefing_Text(Utf8String text, bool clear);
