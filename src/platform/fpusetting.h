/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Map file handling utility functions.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef FPUSETTING_H
#define FPUSETTING_H

#include "always.h"

void Set_FP_Mode();

#ifndef THYME_STANDALONE
#include "hooker.h"
#endif

#endif // FPUSETTING_H

