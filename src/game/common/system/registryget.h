/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Recovers configuration information.
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

Utf8String Get_Registry_Language();
bool Get_String_From_Registry(Utf8String subkey, Utf8String value, Utf8String &destination);
bool Get_String_From_Generals_Registry(Utf8String subkey, Utf8String value, Utf8String &destination);
bool Get_Unsigned_Int_From_Registry(Utf8String subkey, Utf8String value, unsigned int &destination);
