/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Module
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
#include "gamelod.h"
#include "namekeygenerator.h"
#include "snapshot.h"
#include "xfer.h"

class Drawable;
class Object;
class Player;
class Thing;
class W3DModelDrawModuleData;
class W3DTreeDrawModuleData;

// TODO move/remove these as needed.
enum ModuleType
{
    MODULE_FIRST,
    MODULE_DEFAULT = 0,
    MODULE_DRAW = 1,
    MODULE_CLIENT_UPDATE = 2,

    NUM_DRAWABLE_MODULE_TYPES = 2,
};

class ModuleData : public SnapShot
{
    friend class ModuleFactory;

public:
    ModuleData() {}
    virtual ~ModuleData() {}

    virtual bool Is_AI_Module_Data() const { return false; }
    virtual W3DModelDrawModuleData *Get_As_W3D_Model_Draw_Module_Data() const { return nullptr; }
    virtual W3DTreeDrawModuleData *Get_As_W3D_Tree_Draw_Module_Data() const { return nullptr; }
    virtual StaticGameLODLevel Get_Minimum_Required_Game_LOD() const { return STATLOD_LOW; }

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override {}
    virtual void Load_Post_Process() override {}

    static void Build_Field_Parse(MultiIniFieldParse &p) {}

private:
    NameKeyType m_tagKey;
};

class Module : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_ABSTRACT_POOL(Module)

protected:
    virtual ~Module() override {}

public:
    Module(const ModuleData *module_data) : m_moduleData(module_data) {}

    virtual NameKeyType Get_Module_Name_Key() const = 0;
    virtual void On_Object_Created() {}
    virtual void On_Drawable_Bound_To_Object() {}
    virtual void Preload_Assets(TimeOfDayType time_of_day) {}
    virtual void On_Delete() {}

    virtual void CRC_Snapshot(Xfer *xfer) override {}

    virtual void Xfer_Snapshot(Xfer *xfer) override
    {
        unsigned char version = 1;
        xfer->xferVersion(&version, 1);
    }

    virtual void Load_Post_Process() override {}

    const ModuleData *Get_Module_Data() const { return m_moduleData; }

    static ModuleData *Friend_New_Module_Data(INI *ini)
    {
        ModuleData *data = new ModuleData;

        if (ini) {
            ini->Init_From_INI(data, nullptr);
        }

        return data;
    }

    static ModuleType Get_Module_Type() { return MODULE_DEFAULT; }
    static int Get_Interface_Mask() { return 0; }

private:
    const ModuleData *m_moduleData;
};

class ObjectModule : public Module
{
    IMPLEMENT_ABSTRACT_POOL(ObjectModule)

protected:
    virtual ~ObjectModule() override;

public:
    ObjectModule(Thing *thing, const ModuleData *module_data);

    virtual void On_Capture(Player *p1, Player *p2);
    virtual void On_Disabled_Edge(bool b);

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    Object *Get_Object() { return m_object; }

private:
    Object *m_object;
};

class DrawableModule : public Module
{
    IMPLEMENT_ABSTRACT_POOL(DrawableModule)

protected:
    virtual ~DrawableModule() override {}

public:
    DrawableModule(Thing *thing, const ModuleData *module_data);

    virtual void CRC_Snapshot(Xfer *xfer) override {}

    virtual void Xfer_Snapshot(Xfer *xfer) override
    {
        unsigned char version = 1;
        xfer->xferVersion(&version, 1);
    }

    virtual void Load_Post_Process() override {}

    Drawable *Get_Drawable() { return m_drawable; }
    const Drawable *Get_Drawable() const { return m_drawable; }

private:
    Drawable *m_drawable;
};

// class UpgradeMuxData still todo
