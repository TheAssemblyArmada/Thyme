/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Factory class for modules.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "modulefactory.h"

#ifndef GAME_DLL
ModuleFactory *g_theModuleFactory = nullptr;
#else
#include "hooker.h"
#endif

/**
 * @brief Initialises the factory with module templates for all supported modules.
 *
 * 0x004D9660
 */
void ModuleFactory::Init()
{
    // TODO, needs most module derived classes implementing.
#ifdef GAME_DLL
    Call_Method<void, ModuleFactory>(PICK_ADDRESS(0x004D9660, 0x0076F6B7), this);
#elif 0 // TODO, complete function with raw addresses until reimplemented code exists.
    Add_Module_Internal(
        (modcreateproc_t)0x004DE090, (moddatacreateproc_t)0x004DE170, MODULE_FIRST, "AutoHealBehaviour", 0x85);
    Add_Module_Internal(
        (modcreateproc_t)0x004DE4A0, (moddatacreateproc_t)0x004DE580, MODULE_FIRST, "GrantStealthBehaviour", 0x1);
#elif 0 // TODO, will look like this when we have actual functions.
    Add_Module_Internal(
        AutoHealBehaviour::New_Module_Instance, AutoHealBehaviour::New_Module_Data, MODULE_FIRST, "AutoHealBehaviour", 0x85);
#endif
}

/**
 * @brief Snapshot class CRC implementation.
 *
 * 0x004F2F70
 */
void ModuleFactory::CRC_Snapshot(Xfer *xfer)
{
    for (auto it = m_moduleDataList.begin(); it != m_moduleDataList.end(); ++it) {
        // TODO Snapshot methods are not const, so need to cast here.
        // Really the ModuleData vector should be none const I guess, but symbols suggest it was const.
        const_cast<ModuleData *>(*it)->CRC_Snapshot(xfer);
    }
}

/**
 * @brief Snapshot class Xfer implementation.
 *
 * 0x004F2F70
 */
void ModuleFactory::Xfer_Snapshot(Xfer *xfer)
{
    for (auto it = m_moduleDataList.begin(); it != m_moduleDataList.end(); ++it) {
        // TODO Snapshot methods are not const, so need to cast here.
        const_cast<ModuleData *>(*it)->Xfer_Snapshot(xfer);
    }
}

/**
 * @brief Internal method for locating a template from a module name and type.
 */
const ModuleFactory::ModuleTemplate *ModuleFactory::Find_Module_Template(const Utf8String &name, ModuleType type) const
{
    auto it = m_moduleTemplateMap.find(Make_Decorated_Name_Key(name, type));

    if (it != m_moduleTemplateMap.end()) {
        return &it->second;
    }

    captainslog_dbgassert(0, "Module name '%s' not found", name.Str());
    return nullptr;
}

/**
 * @brief Locates the interface mask from a module template.
 *
 * 0x004F2B80
 */
int ModuleFactory::Find_Module_Interface_Mask(const Utf8String &name, ModuleType type)
{
    if (name.Is_Not_Empty()) {
        const ModuleTemplate *temp = Find_Module_Template(name, type);

        if (temp != nullptr) {
            return temp->which_interfaces;
        }
    }

    return 0;
}

/**
 * @brief Creates a new module instance using the function pointer stored in a template and provided module data.
 *
 * 0x004F2DD0
 */
Module *ModuleFactory::New_Module(Thing *thing, const Utf8String &name, const ModuleData *data, ModuleType type)
{
    if (name.Is_Not_Empty()) {
        const ModuleTemplate *temp = Find_Module_Template(name, type);

        if (temp != nullptr) {
            return temp->create_proc(thing, data);
        }
    }

    return nullptr;
}

/**
 * @brief Creates new module data using template function pointer and data from an INI file.
 *
 * 0x004F2C20
 */
ModuleData *ModuleFactory::New_Module_Data_From_INI(INI *ini, const Utf8String &name, ModuleType type, const Utf8String &tag)
{
    if (name.Is_Not_Empty()) {
        const ModuleTemplate *temp = Find_Module_Template(name, type);

        if (temp != nullptr) {
            ModuleData *data = temp->create_data_proc(ini);
            data->m_tagKey = g_theNameKeyGenerator->Name_To_Key(tag.Str());
            m_moduleDataList.push_back(data);

            return data;
        }
    }

    return nullptr;
}

/**
 * @brief Internal method that creates a key from the name decorated with the type converted to a printable character.
 *
 * 0x004F2D60
 */
NameKeyType ModuleFactory::Make_Decorated_Name_Key(const Utf8String &name, ModuleType type)
{
    char tmp[256];

    tmp[0] = type + '0';
    strlcpy(&tmp[1], name.Str(), sizeof(tmp) - 1);

    return g_theNameKeyGenerator->Name_To_Key(tmp);
}

/**
 * @brief Internal method that adds a module template, used during factory initialisation.
 *
 * 0x004F2E80
 */
void ModuleFactory::Add_Module_Internal(
    modcreateproc_t proc, moddatacreateproc_t data_proc, ModuleType type, const Utf8String &name, int iface)
{
    ModuleTemplate &data = m_moduleTemplateMap[Make_Decorated_Name_Key(name, type)];
    data.create_proc = proc;
    data.create_data_proc = data_proc;
    data.which_interfaces = iface;
}
