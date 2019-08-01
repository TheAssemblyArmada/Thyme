/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Texture mapping related classes.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "mapper.h"

/**
 * 0x00864ED0
 */
TextureMapperClass::TextureMapperClass(uint32_t stage) : m_stage(stage)
{
    if (stage > 2) {
        m_stage = 1;
    }
}
