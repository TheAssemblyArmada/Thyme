/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Science Model Draw Module
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dsciencemodeldraw.h"
#include "playerlist.h"
#include "science.h"

void W3DScienceModelDrawModuleData::Build_Field_Parse(MultiIniFieldParse &p)
{
    // clang-format off
    static const FieldParse dataFieldParse[] = {
        { "RequiredScience", &ScienceStore::Parse_Science, nullptr, offsetof(W3DScienceModelDrawModuleData, m_requiredScience) },
        { nullptr, nullptr, nullptr, 0 },
    };
    // clang-format on

    W3DModelDrawModuleData::Build_Field_Parse(p);
    p.Add(dataFieldParse, 0);
}

NameKeyType W3DScienceModelDraw::Get_Module_Name_Key() const
{
    static const NameKeyType nk = g_theNameKeyGenerator->Name_To_Key("W3DScienceModelDraw");
    return nk;
}

void W3DScienceModelDraw::Do_Draw_Module(const Matrix3D *transform)
{
    ScienceType science = Get_W3D_Science_Model_Draw_Module_Data()->m_requiredScience;
    if (science == SCIENCE_INVALID) {
        captainslog_debug("ScienceModelDraw has invalid science as condition.");
        Set_Hidden(true);
        return;
    }

    if (!g_thePlayerList->Get_Local_Player()->Has_Science(science)) {
        if (g_thePlayerList->Get_Local_Player()->Is_Player_Active()) {
            Set_Hidden(true);
        }
    }

    W3DModelDraw::Do_Draw_Module(transform);
}

void W3DScienceModelDraw::CRC_Snapshot(Xfer *xfer)
{
    W3DModelDraw::CRC_Snapshot(xfer);
}

void W3DScienceModelDraw::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t ver = 1;
    xfer->xferVersion(&ver, 1);
    W3DModelDraw::Xfer_Snapshot(xfer);
}

void W3DScienceModelDraw::Load_Post_Process()
{
    W3DModelDraw::Load_Post_Process();
}

ModuleData *W3DScienceModelDraw::Friend_New_Module_Data(INI *ini)
{
    W3DScienceModelDrawModuleData *data = new W3DScienceModelDrawModuleData();

    if (ini) {
        ini->Init_From_INI_Multi_Proc(data, W3DScienceModelDrawModuleData::Build_Field_Parse);
    }

    return data;
}

Module *W3DScienceModelDraw::Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data)
{
    return NEW_POOL_OBJ(W3DScienceModelDraw, thing, module_data);
}
