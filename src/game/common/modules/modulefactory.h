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
#include "namekeygenerator.h"
#include "snapshot.h"
#include "subsysteminterface.h"
#include <map>
#include <vector>

// TODO move/remove these as needed.
enum ModuleType
{
    MODULE_FIRST,
    MODULE_DEFAULT = 0,
    MODULE_W3D = 1,
    MODULE_UNK = 2,
};

class Module;
class W3DModelDrawModuleData;
class W3DTreeDrawModuleData;
class Thing;

// TODO possibly move to more appropriate location?
class ModuleData : public SnapShot
{
    friend class ModuleFactory;
public:
    virtual void CRC_Snapshot(Xfer *xfer) {}
    virtual void Xfer_Snapshot(Xfer *xfer) {}
    virtual void Load_Post_Process() {}

    virtual ~ModuleData() {}
    virtual bool Is_AI_Module_Data() const { return false; }
    virtual W3DModelDrawModuleData *Get_As_W3D_Model_Draw_Module_Data() const { return nullptr; }
    virtual W3DTreeDrawModuleData *Get_As_W3D_Tree_Draw_Module_Data() const { return nullptr; }
    virtual void *Get_Minimum_Required_Game_LOD() const { return nullptr; } // Not sure what this actually returns.

private:
    NameKeyType m_tagKey;
};

typedef Module *(*modcreateproc_t)(Thing *, ModuleData *);
typedef ModuleData *(*moddatacreateproc_t)(INI *);

class ModuleFactory : public SubsystemInterface, public SnapShot
{
    ALLOW_HOOKING
    struct ModuleTemplate
    {
        modcreateproc_t create_proc;
        moddatacreateproc_t create_data_proc;
        int which_interfaces;
    };

public:
    virtual ~ModuleFactory() {}

    virtual void Init() override;
    virtual void Reset() override {}
    virtual void Update() override {}

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    int Find_Interface_Mask(const Utf8String &name, ModuleType type);
    Module *New_Module(Thing *thing, const Utf8String &name, ModuleData *data, ModuleType type);
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
#include "hooker.h"
extern ModuleFactory *&g_theModuleFactory;
#else
extern ModuleFactory *g_theModuleFactory;
#endif
