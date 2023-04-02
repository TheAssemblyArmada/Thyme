/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Object Helper
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

class ObjectHelper : public UpdateModule
{
    IMPLEMENT_ABSTRACT_POOL(ObjectHelper)

public:
    ObjectHelper(Thing *thing, const ModuleData *module_data);
    virtual ~ObjectHelper() override {}

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    void Sleep_Until(unsigned int frame);
};
