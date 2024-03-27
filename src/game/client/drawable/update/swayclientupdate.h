/**
 * @file
 *
 * @author xezon
 *
 * @brief Sway client update class. Used for foliage movements.
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
#include "bittype.h"
#include "clientupdatemodule.h"
#include "namekeygenerator.h"

class Thing;
class ModuleData;

// This class can be tested with some trees on Seaside Mutiny (2) map. Not all trees use this module.

class SwayClientUpdate : public ClientUpdateModule
{
    IMPLEMENT_POOL(SwayClientUpdate)

protected:
    SwayClientUpdate(Thing *thing, const ModuleData *module_data);

    virtual ~SwayClientUpdate() override {}

public:
    static SwayClientUpdate *Friend_New_Module_Instance(Thing *thing, const ModuleData *module_data);

    virtual void CRC_Snapshot(Xfer *xfer) override;

    virtual void Xfer_Snapshot(Xfer *xfer) override;

    virtual void Load_Post_Process() override;

    virtual NameKeyType Get_Module_Name_Key() const override;

    virtual void Client_Update() override;

private:
    void Update_Sway();
    void Stop_Sway();

#ifdef GAME_DLL
public:
    SwayClientUpdate *Hook_Ctor(Thing *thing, const ModuleData *module_data)
    {
        return new (this) SwayClientUpdate(thing, module_data);
    }
#endif

private:
    float m_curValue;
    float m_curAngle;
    float m_curDelta;
    float m_curAngleLimit;
    float m_leanAngle;
    int16_t m_curVersion;
    bool m_swaying;
    bool m_unused;
};
