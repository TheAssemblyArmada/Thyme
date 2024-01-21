/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Dock Update
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
#include "updatemodule.h"

class DockUpdate : public UpdateModule, public DockUpdateInterface
{
    IMPLEMENT_ABSTRACT_POOL(DockUpdate)

public:
    virtual ~DockUpdate() override;
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;
    virtual UpdateSleepTime Update() override;
    virtual bool Is_Clear_To_Approach(const Object *docker) const override;
    virtual bool Reserve_Approach_Position(Object *docker, Coord3D *pos, int *index) override;
    virtual bool Advance_Approach_Position(Object *docker, Coord3D *pos, int *index) override;
    virtual bool Is_Clear_To_Enter(const Object *docker) const override;
    virtual bool Is_Clear_To_Advance(const Object *docker, int index) const override;
    virtual void Get_Enter_Position(Object *docker, Coord3D *pos) override;
    virtual void Get_Dock_Position(Object *docker, Coord3D *pos) override;
    virtual void Get_Exit_Position(Object *docker, Coord3D *pos) override;
    virtual void On_Approach_Reached(Object *docker) override;
    virtual void On_Enter_Reached(Object *docker) override;
    virtual void On_Dock_Reached(Object *docker) override;
    virtual void On_Exit_Reached(Object *docker) override;
    virtual void Cancel_Dock(Object *docker) override;
    virtual bool Is_Dock_Open() override;
    virtual void Set_Dock_Open(bool open) override;
    virtual void Set_Dock_Crippled(bool crippled) override;
    virtual bool Is_Allow_Passthrough_Type() override;
    virtual bool Is_Rally_Point_After_Dock_Type() override;

private:
    Coord3D m_enterPosition;
    Coord3D m_dockPosition;
    Coord3D m_exitPosition;
    int m_numberApproachPositions;
    int m_numberApproachBones;
    bool m_dockPositionsLoaded;
    std::vector<Coord3D> m_approachPositions;
    std::vector<ObjectID> m_approachPositionOwners;
    std::vector<bool> m_approachPositionReached;
    ObjectID m_activeDocker;
    bool m_isObjectDocking;
    bool m_dockCrippled;
    bool m_unk;
};
