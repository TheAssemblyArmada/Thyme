/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Reads a line of text from a file or straw.
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
#include "straw.h"
#include "wwfile.h"

int Read_Line(FileClass &file, char *buffer, int length, bool &eof);
int Read_Line(Straw &straw, char *buffer, int length, bool &eof);
