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

#ifndef REGISTRY_H
#define REGISTRY_H

#include "always.h"
#include "asciistring.h"

AsciiString Get_Registry_Language();
void Get_String_From_Registry(AsciiString subkey, AsciiString value, AsciiString const &destination);
void Get_String_From_Generals_Registry(AsciiString subkey, AsciiString value, AsciiString const &destination);

#ifndef THYME_STANDALONE
#include "hooker.h"
#endif

#endif // REGISTRY_H
