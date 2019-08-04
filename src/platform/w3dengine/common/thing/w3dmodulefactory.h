/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Factory class for modules extended to support w3d specific modules.
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
#include "modulefactory.h"

class W3DModuleFactory : public ModuleFactory
{
public:
    virtual ~W3DModuleFactory() {}
    virtual void Init() override;
};

#ifdef GAME_DLL
#include "hooker.h"
#endif
