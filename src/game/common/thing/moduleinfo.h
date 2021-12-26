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
class ThingTemplate;

enum ModuleInterfaceMaskType
{
    MODULEINTERFACE_UPDATE = 1,
    MODULEINTERFACE_DIE = 2,
    MODULEINTERFACE_DAMAGE = 4,
    MODULEINTERFACE_CREATE = 8,
    MODULEINTERFACE_COLLIDE = 0x10,
    MODULEINTERFACE_BODY = 0x20,
    MODULEINTERFACE_CONTAIN = 0x40,
    MODULEINTERFACE_UPGRADE = 0x80,
    MODULEINTERFACE_SPECIAL_POWER = 0x100,
    MODULEINTERFACE_DESTROY = 0x200,
    MODULEINTERFACE_DRAW = 0x400,
    MODULEINTERFACE_CLIENT_UPDATE = 0x800,
};

class ModuleInfo
{
    struct Nugget
    {
        Utf8String name;
        Utf8String tag_name;
        ModuleData *data;
        int interface_mask;
        bool copied_from_default;
        bool inherited;
        bool like_kind_override;
        Nugget(const Utf8String &n, const Utf8String &tag, ModuleData *d, int mask, bool inherit, bool kind) :
            name(n),
            tag_name(tag),
            data(d),
            interface_mask(mask),
            copied_from_default(false),
            inherited(inherit),
            like_kind_override(kind)
        {
        }
    };

public:
    ModuleInfo() {}
    ~ModuleInfo() {}

    void Clear() { m_info.clear(); }

    void Set_Copied_From_Default(bool b)
    {
        for (unsigned int i = 0; i < m_info.size(); i++) {
            m_info[i].copied_from_default = b;
        }
    }

    unsigned int Get_Count() const { return m_info.size(); }

    Utf8String Get_Nth_Name(int i) const
    {
        if (i < 0 || i >= (int)m_info.size()) {
            return Utf8String::s_emptyString;
        } else {
            return m_info[i].name;
        }
    }

    ModuleData *Friend_Get_Nth_Data(int i) const
    {
        if (i < 0 || i >= (int)m_info.size()) {
            return nullptr;
        } else {
            return m_info[i].data;
        }
    }

    const ModuleData *Get_Nth_Data(int i) const
    {
        if (i < 0 || i >= (int)m_info.size()) {
            return nullptr;
        } else {
            return m_info[i].data;
        }
    }

    void Add_Module_Info(ThingTemplate *thing_template,
        const Utf8String &module_name,
        const Utf8String &tag_name,
        ModuleData *mod_data,
        int interface_mask,
        bool inherited,
        bool like_kind_override);
    bool Clear_Module_Data_With_Tag(const Utf8String &tag, Utf8String &name);
    bool Clear_Copied_From_Default_Entries(int interface_mask, Utf8String const &name, ThingTemplate *tmplate);
    bool Clear_Ai_Module_Info();
    Nugget *Try_Find_Nugget(Utf8String const &name);

private:
    std::vector<Nugget> m_info;
};
