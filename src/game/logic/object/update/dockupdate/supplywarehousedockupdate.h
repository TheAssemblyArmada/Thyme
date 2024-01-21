/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Supply Warehouse Dock Update
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
#include "dockupdate.h"

class SupplyWarehouseDockUpdate : public DockUpdate
{
    IMPLEMENT_POOL(SupplyWarehouseDockUpdate)

public:
    virtual ~SupplyWarehouseDockUpdate() override;
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void On_Object_Created() override;
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;
    virtual DockUpdateInterface *Get_Dock_Update_Interface() override;
    virtual bool Action(Object *docker, Object *obj) override;
    virtual void Set_Dock_Crippled(bool crippled) override;

    int Get_Boxes_Stored() const { return m_boxesStored; }

private:
    int m_boxesStored;
};
