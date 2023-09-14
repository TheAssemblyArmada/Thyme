/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Overlord Tank Draw Module
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3doverlordtankdraw.h"
#include "drawable.h"
#include "object.h"
#include "opencontain.h"

void W3DOverlordTankDrawModuleData::Build_Field_Parse(MultiIniFieldParse &p)
{
    // clang-format off
    static const FieldParse dataFieldParse[] = {
        { nullptr, nullptr, nullptr, 0 },
    };
    // clang-format on

    W3DTankDrawModuleData::Build_Field_Parse(p);
    p.Add(dataFieldParse, 0);
}

NameKeyType W3DOverlordTankDraw::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("W3DOverlordTankDraw");
    return _key;
}

void W3DOverlordTankDraw::Do_Draw_Module(const Matrix3D *transform)
{
    W3DTankDraw::Do_Draw_Module(transform);
    Object *object = Get_Drawable()->Get_Object();

    if (object != nullptr && object->Get_Contain() != nullptr) {
        const Object *rider = object->Get_Contain()->Friend_Get_Rider();

        if (rider != nullptr) {
            if (rider->Get_Drawable() != nullptr) {
                Drawable *drawable = rider->Get_Drawable();
                if (const TintEnvelope *envelope = Get_Drawable()->Get_Tint_Color_Envelope()) {
                    drawable->Set_Tint_Color_Envelope(envelope);
                }
                drawable->Notify_Drawable_Dependency_Cleared();
                drawable->Draw(nullptr);
            }
        }
    }
}

void W3DOverlordTankDraw::CRC_Snapshot(Xfer *xfer)
{
    W3DTankDraw::CRC_Snapshot(xfer);
}

void W3DOverlordTankDraw::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char ver = 1;
    xfer->xferVersion(&ver, 1);
    W3DTankDraw::Xfer_Snapshot(xfer);
}

void W3DOverlordTankDraw::Load_Post_Process()
{
    W3DTankDraw::Load_Post_Process();
}

void W3DOverlordTankDraw::Set_Hidden(bool hidden)
{
    W3DTankDraw::Set_Hidden(hidden);
    Object *object = Get_Drawable()->Get_Object();

    if (object != nullptr && object->Get_Contain() != nullptr) {
        const Object *rider = object->Get_Contain()->Friend_Get_Rider();

        if (rider != nullptr) {
            if (rider->Get_Drawable() != nullptr) {
                rider->Get_Drawable()->Set_Drawable_Hidden(hidden);
            }
        }
    }
}

ModuleData *W3DOverlordTankDraw::Friend_New_Module_Data(INI *ini)
{
    W3DOverlordTankDrawModuleData *data = new W3DOverlordTankDrawModuleData();

    if (ini) {
        ini->Init_From_INI_Multi_Proc(data, W3DOverlordTankDrawModuleData::Build_Field_Parse);
    }

    return data;
}

Module *W3DOverlordTankDraw::Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data)
{
    return NEW_POOL_OBJ(W3DOverlordTankDraw, thing, module_data);
}
