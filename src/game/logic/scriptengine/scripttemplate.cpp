/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Template classes for script actions and conditions.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "scripttemplate.h"
#include <cstring>

Template::Template() : m_uiName("UNUSED/(placeholder)/placeholder"), m_uiStringCount(0), m_numParameters(0)
{
    std::memset(m_parameters, 0, sizeof(m_parameters));
}

void Template::Get_UI_Strings(Utf8String *strings) const
{
    for (int i = 0; i < m_numParameters; ++i) {
        strings[i] = m_uiStrings[i];
    }
}

Parameter::ParameterType Template::Get_Parameter_Type(int parameter) const
{
    if (parameter < 0 || parameter > m_numParameters) {
        return Parameter::INT;
    }

    return m_parameters[parameter];
}
