/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for tracking module information.
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
#include <vector>

class ModuleData;

class ModuleInfo
{
    struct Nugget
    {
        Utf8String unk_string1;
        Utf8String unk_string2;
        ModuleData *data;
        int interface_mask;
        bool copied_from_default;
        bool unk_bool1;
        bool unk_bool2;
    };

public:
    ModuleInfo() {}
    ~ModuleInfo() {}
    void Set_Copied_From_Default(bool b)
    {
        for (unsigned int i = 0; i < m_info.size(); i++) {
            m_info[i].copied_from_default = b;
        }
    }

private:
    std::vector<Nugget> m_info;
};
