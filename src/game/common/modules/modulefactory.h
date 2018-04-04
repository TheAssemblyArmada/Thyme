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
#pragma once

#include "always.h"
#include "ini.h"
#include "namekeygenerator.h"
#include "snapshot.h"
#include "subsysteminterface.h"
#include <map>
#include <vector>

#ifndef THYME_STANDALONE
#include "hooker.h"
#endif

// TODO move/remove these as needed.
enum ModuleType
{
    MODULE_FIRST,
};

class Module;
class ModuleData;
class Thing;

typedef Module *(*modcreateproc_t)(Thing *, ModuleData *);
typedef ModuleData *(*moddatacreateproc_t)(INI *);

class ModuleFactory : public SubsystemInterface, public SnapShot
{
    struct ModuleTemplate
    {
        modcreateproc_t create_proc;
        moddatacreateproc_t create_data_proc;
        int which_interfaces;
    };

public:
    ModuleFactory();
    virtual ~ModuleFactory();

    virtual void Init() override;
    virtual void Reset() override {}
    virtual void Update() override {}

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

private:
    static NameKeyType Make_Decorated_Name_Key(AsciiString &name, ModuleType type);
    void Add_Module_Internal(
        modcreateproc_t proc, moddatacreateproc_t data_proc, ModuleType type, AsciiString name, int interface);

protected:
    std::map<NameKeyType, ModuleTemplate> m_moduleTemplateMap;
    std::vector<ModuleData const *> m_moduleDataList;
};

#ifndef THYME_STANDALONE
extern ModuleFactory *&g_theModuleFactory;
#else
extern ModuleFactory *g_theModuleFactory;
#endif