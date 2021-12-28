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

Drawable *Thing_As_Drawable(Thing *thing)
{
    if (thing) {
        return thing->As_Drawable_Meth();
    } else {
        return nullptr;
    }
}

DrawableModule::DrawableModule(Thing *thing, const ModuleData *module_data) : Module(module_data)
{
    captainslog_relassert(module_data, CODE_06, "module data may not be null");
    captainslog_dbgassert(thing, "Thing passed to DrawableModule is NULL!");
    m_drawable = Thing_As_Drawable(thing);
    captainslog_dbgassert(m_drawable, "Thing passed to DrawableModule is not a Drawable!");
}
