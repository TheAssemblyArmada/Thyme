/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Projectile Stream Update Module
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

enum
{
    MAX_SEGMENT_COUNT = 20
};

class ProjectileStreamUpdate : public UpdateModule
{
    IMPLEMENT_POOL(ProjectileStreamUpdate)

public:
    ProjectileStreamUpdate(Thing *thing, ModuleData const *module_data);

    virtual ~ProjectileStreamUpdate() override {}
    virtual NameKeyType Get_Module_Name_Key() const override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual UpdateSleepTime Update() override;

    static Module *Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data);

    void Add_Projectile(ObjectID source_id, ObjectID projectile_id, ObjectID target_id, const Coord3D *target_pos);
    void Cull_Front_Of_List();
    bool Consider_Dying();
    void Get_All_Points(Vector3 *points, int *count);
    void Set_Position(const Coord3D *new_position);

private:
    ObjectID m_projectileIDs[MAX_SEGMENT_COUNT];
    int m_nextFreeIndex;
    int m_firstValidIndex;
    ObjectID m_owningObject;
    ObjectID m_targetID;
    Coord3D m_targetPos;
};
