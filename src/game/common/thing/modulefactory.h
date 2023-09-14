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
#pragma once

#include "always.h"
#include "ini.h"
#include "module.h"
#include "namekeygenerator.h"
#include "snapshot.h"
#include "subsysteminterface.h"
#include <map>
#include <vector>

class Module;
class W3DModelDrawModuleData;
class W3DTreeDrawModuleData;
class Thing;

typedef Module *(*modcreateproc_t)(Thing *, const ModuleData *);
typedef ModuleData *(*moddatacreateproc_t)(INI *);

class ModuleFactory : public SubsystemInterface, public SnapShot
{
    ALLOW_HOOKING
    class ModuleTemplate
    {
    public:
        modcreateproc_t create_proc;
        moddatacreateproc_t create_data_proc;
        int which_interfaces;
        ModuleTemplate() : create_proc(nullptr), create_data_proc(nullptr), which_interfaces(0) {}
    };

public:
    virtual ~ModuleFactory() {}

    virtual void Init() override;
    virtual void Reset() override {}
    virtual void Update() override {}

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    int Find_Module_Interface_Mask(const Utf8String &name, ModuleType type);
    Module *New_Module(Thing *thing, const Utf8String &name, const ModuleData *data, ModuleType type);
    ModuleData *New_Module_Data_From_INI(INI *ini, const Utf8String &name, ModuleType type, const Utf8String &tag);

protected:
    static NameKeyType Make_Decorated_Name_Key(const Utf8String &name, ModuleType type);
    void Add_Module_Internal(
        modcreateproc_t proc, moddatacreateproc_t data_proc, ModuleType type, const Utf8String &name, int iface);
    const ModuleTemplate *Find_Module_Template(const Utf8String &name, ModuleType type) const;

protected:
    std::map<NameKeyType, ModuleTemplate> m_moduleTemplateMap;
    std::vector<const ModuleData *> m_moduleDataList;
};

#ifdef GAME_DLL
extern ModuleFactory *&g_theModuleFactory;
#else
extern ModuleFactory *g_theModuleFactory;
#endif
