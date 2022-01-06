/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Client Update Module
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
#include "module.h"
#include "moduleinfo.h"

class ClientUpdateModule : public DrawableModule
{
    IMPLEMENT_ABSTRACT_POOL(ClientUpdateModule)

protected:
    virtual ~ClientUpdateModule() override {}

public:
    ClientUpdateModule(Thing *thing, const ModuleData *module_data) : DrawableModule(thing, module_data) {}
    virtual void Client_Update() = 0;

    static ModuleType Get_Module_Type() { return MODULE_CLIENT_UPDATE; }
    static int Get_Interface_Mask() { return MODULEINTERFACE_CLIENT_UPDATE; }
};
