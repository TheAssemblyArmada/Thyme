/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Quoted Printable
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

Utf8String Ascii_String_To_Quoted_Printable(Utf8String string);
Utf8String Quoted_Printable_To_Ascii_String(Utf8String string);
