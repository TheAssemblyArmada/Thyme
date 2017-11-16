/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class representing action and condition parameters.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "scriptparam.h"

Parameter::Parameter(ParameterType type) :
    m_type(type),
    m_initialized(),
    m_int(),
    m_real(),
    m_string(),
    m_coord(),
    m_unkInt1(),
    m_unkInt2()
{
}
