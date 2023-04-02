/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Object Defection Helper
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
#include "objecthelper.h"

class ObjectDefectionHelperModuleData : public ModuleData
{
public:
    ObjectDefectionHelperModuleData() {}
    ~ObjectDefectionHelperModuleData() override {}
};

class ObjectDefectionHelper : public ObjectHelper
{
    IMPLEMENT_NAMED_POOL(ObjectDefectionHelper, ObjectDefectionHelperPool)

public:
    ObjectDefectionHelper(Thing *thing, const ModuleData *module_data);

    virtual ~ObjectDefectionHelper() override;
    virtual NameKeyType Get_Module_Name_Key() const override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual BitFlags<DISABLED_TYPE_COUNT> Get_Disabled_Types_To_Process() const override;
    virtual UpdateSleepTime Update() override;

    void Start_Defection_Timer(unsigned int timer, bool do_effect);

private:
    unsigned int m_startFrame;
    unsigned int m_endFrame;
    float m_effectTimer;
    bool m_doEffect;
};
