/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief AI guard
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
#include "aistates.h"
#include "aiupdate.h"

class AIGuardMachine : public StateMachine
{
    IMPLEMENT_NAMED_POOL(AIGuardMachine, AIGuardMachinePool)

public:
    virtual ~AIGuardMachine() override;
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;
    static float Get_Std_Guard_Range(Object *obj);

private:
    ObjectID m_targetToGuard;
    PolygonTrigger *m_areaToGuard;
    Coord3D m_positionToGuard;
    ObjectID m_nemesisToAttack;
    GuardMode m_guardMode;
};
