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

#include "bittype.h"
#include "clientupdatemodule.h"

enum NameKeyType;
class Thing;
class ModuleData;

// This class can be tested with some trees on Seaside Mutiny (2) map. Interestingly not all trees use this module. In Retail
// game, some trees move faster with higher framerate, while trees using this module do not: They sway in consistent speeds
// regardless of the framerate (good).

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
private:
    void *operator new(size_t size, void *where) { return where; }
    void operator delete(void *ptr, void *where) {}

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
