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
#include "module.h"
#include "thing.h"

namespace
{
Object *Thing_As_Object(Thing *thing)
{
    return thing ? thing->As_Object_Meth() : nullptr;
}

Drawable *Thing_As_Drawable(Thing *thing)
{
    return thing ? thing->As_Drawable_Meth() : nullptr;
}
} // namespace

void Module::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
}

const ModuleData *Module::Get_Module_Data() const
{
    return m_moduleData;
}

NameKeyType Module::Get_Tag_Key() const
{
    return Get_Module_Data()->Get_Tag_Key();
}

ModuleData *Module::Friend_New_Module_Data(INI *ini)
{
    ModuleData *data = new ModuleData;

    if (ini) {
        ini->Init_From_INI(data, nullptr);
    }

    return data;
}

ModuleType Module::Get_Module_Type()
{
    return MODULE_DEFAULT;
}

int Module::Get_Interface_Mask()
{
    return 0;
}

ObjectModule::ObjectModule(Thing *thing, const ModuleData *module_data) : Module(module_data)
{
    captainslog_relassert(module_data != nullptr, CODE_06, "module data may not be null");
    captainslog_dbgassert(thing != nullptr, "Thing passed to ObjectModule is NULL!");
    m_object = Thing_As_Object(thing);
    captainslog_dbgassert(thing != nullptr, "Thing passed to ObjectModule is not an Object!");
}

void ObjectModule::CRC_Snapshot(Xfer *xfer)
{
    Module::CRC_Snapshot(xfer);
}

void ObjectModule::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);
    Module::Xfer_Snapshot(xfer);
}

void ObjectModule::Load_Post_Process()
{
    Module::Load_Post_Process();
}

DrawableModule::DrawableModule(Thing *thing, const ModuleData *module_data) : Module(module_data)
{
    captainslog_relassert(module_data != nullptr, CODE_06, "module data may not be null");
    captainslog_dbgassert(thing != nullptr, "Thing passed to DrawableModule is NULL!");
    m_drawable = Thing_As_Drawable(thing);
    captainslog_dbgassert(m_drawable != nullptr, "Thing passed to DrawableModule is not a Drawable!");
}

void DrawableModule::CRC_Snapshot(Xfer *xfer)
{
    Module::CRC_Snapshot(xfer);
}

void DrawableModule::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);
    Module::Xfer_Snapshot(xfer);
}

void DrawableModule::Load_Post_Process()
{
    Module::Load_Post_Process();
}
