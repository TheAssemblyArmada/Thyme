/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief Factory class for modules.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "modulefactory.h"
#include "stringex.h"

#ifndef THYME_STANDALONE
ModuleFactory *&g_theModuleFactory = Make_Global<ModuleFactory *>(0x00A2BE40);
#else
ModuleFactory *g_theModuleFactory = nullptr;
#endif

NameKeyType ModuleFactory::Make_Decorated_Name_Key(AsciiString &name, ModuleType type)
{
    char tmp[256];

    tmp[0] = type + '0';
    strlcpy(&tmp[1], name, sizeof(tmp) - 1);

    return g_theNameKeyGenerator->Name_To_Key(tmp);
}

void ModuleFactory::Add_Module_Internal(
    modcreateproc_t proc, moddatacreateproc_t data_proc, ModuleType type, AsciiString name, int interface)
{
    ModuleTemplate &data = m_moduleTemplateMap[Make_Decorated_Name_Key(name, type)];
    data.create_proc = proc;
    data.create_data_proc = data_proc;
    data.which_interfaces = interface;
}
