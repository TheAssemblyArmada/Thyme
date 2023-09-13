/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Dependency Model Draw Module
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3ddependencymodeldraw.h"
#include "drawable.h"
#include "object.h"
#include "opencontain.h"

void W3DDependencyModelDrawModuleData::Build_Field_Parse(MultiIniFieldParse &p)
{
    // clang-format off
    static const FieldParse dataFieldParse[] = {
        { "AttachToBoneInContainer", &INI::Parse_AsciiString, nullptr, offsetof(W3DDependencyModelDrawModuleData, m_attachToDrawableBoneInContainer) },
        { nullptr, nullptr, nullptr, 0 },
    };
    // clang-format on

    W3DModelDrawModuleData::Build_Field_Parse(p);
    p.Add(dataFieldParse, 0);
}

NameKeyType W3DDependencyModelDraw::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("W3DDependencyModelDraw");
    return _key;
}

void W3DDependencyModelDraw::Do_Draw_Module(const Matrix3D *transform)
{
    if (m_doDrawModule) {
        W3DModelDraw::Do_Draw_Module(transform);
        m_doDrawModule = false;

        Drawable *drawable = Get_Drawable();

        if (drawable != nullptr) {
            Object *object = drawable->Get_Object();

            if (object != nullptr) {
                Drawable *stealth = nullptr;

                if (object->Get_Contained_By()) {

                    if (!object->Get_Contained_By()->Get_Contain()->Is_Enclosing_Container_For(object)) {
                        stealth = object->Get_Contained_By()->Get_Drawable();
                    }
                }

                if (stealth) {
                    drawable->Imitate_Stealth_Look(*stealth);
                }
            }
        }
    }
}

void W3DDependencyModelDraw::Adjust_Transform_Mtx(Matrix3D &transform) const
{
    W3DModelDraw::Adjust_Transform_Mtx(transform);
    const Drawable *drawable = Get_Drawable();
    const Object *object = drawable->Get_Object();

    const W3DDependencyModelDrawModuleData *data = Get_W3D_Dependency_Model_Draw_Module_Data();

    if (data->m_attachToDrawableBoneInContainer.Is_Not_Empty()) {
        if (object != nullptr) {
            if (object->Get_Contained_By()) {
                if (!object->Get_Contained_By()->Get_Contain()->Is_Enclosing_Container_For(object)) {
                    const Drawable *contained_drawable = object->Get_Contained_By()->Get_Drawable();

                    if (contained_drawable != nullptr) {
                        Matrix3D m;

                        if (contained_drawable->Get_Current_Worldspace_Client_Bone_Positions(
                                data->m_attachToDrawableBoneInContainer.Str(), m)) {
                            transform = m;
                        } else {
                            transform = *contained_drawable->Get_Transform_Matrix();
                            captainslog_debug("m_attachToDrawableBoneInContainer %s not found",
                                data->m_attachToDrawableBoneInContainer.Str());
                        }
                    }
                }
            }
        }
    }
}

void W3DDependencyModelDraw::CRC_Snapshot(Xfer *xfer)
{
    W3DModelDraw::CRC_Snapshot(xfer);
}

void W3DDependencyModelDraw::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char ver = 1;
    xfer->xferVersion(&ver, 1);
    W3DModelDraw::Xfer_Snapshot(xfer);
    xfer->xferBool(&m_doDrawModule);
}

void W3DDependencyModelDraw::Load_Post_Process()
{
    W3DModelDraw::Load_Post_Process();
}

ModuleData *W3DDependencyModelDraw::Friend_New_Module_Data(INI *ini)
{
    W3DDependencyModelDrawModuleData *data = new W3DDependencyModelDrawModuleData();

    if (ini) {
        ini->Init_From_INI_Multi_Proc(data, W3DDependencyModelDrawModuleData::Build_Field_Parse);
    }

    return data;
}

Module *W3DDependencyModelDraw::Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data)
{
    return NEW_POOL_OBJ(W3DDependencyModelDraw, thing, module_data);
}
