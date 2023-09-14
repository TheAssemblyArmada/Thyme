/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Supply Draw Module
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dsupplydraw.h"
#include "drawable.h"

void W3DSupplyDrawModuleData::Build_Field_Parse(MultiIniFieldParse &p)
{
    // clang-format off
    static const FieldParse dataFieldParse[] = {
        { "SupplyBonePrefix", &INI::Parse_AsciiString, nullptr, offsetof(W3DSupplyDrawModuleData, m_supplyBonePrefix) },
        { nullptr, nullptr, nullptr, 0 },
    };
    // clang-format on

    W3DModelDrawModuleData::Build_Field_Parse(p);
    p.Add(dataFieldParse, 0);
}

NameKeyType W3DSupplyDraw::Get_Module_Name_Key() const
{
    static const NameKeyType nk = g_theNameKeyGenerator->Name_To_Key("W3DSupplyDraw");
    return nk;
}

void W3DSupplyDraw::Update_Draw_Module_Supply_Status(int max, int current)
{
    W3DModelDraw::Update_Draw_Module_Supply_Status(max, current);
    const Utf8String &prefix = Get_W3D_Supply_Draw_Module_Data()->m_supplyBonePrefix;

    if (m_totalSupplyBoneCount == -1) {
        m_totalSupplyBoneCount = Get_Drawable()->Get_Pristine_Bone_Positions(prefix.Str(), 1, nullptr, nullptr, INT_MAX);
        m_currentSupplyBoneCount = m_totalSupplyBoneCount;
    }

    int newcount = GameMath::Ceil((float)m_totalSupplyBoneCount * ((float)current / (float)max));
    newcount = std::min(newcount, m_totalSupplyBoneCount);

    if (newcount != m_currentSupplyBoneCount) {
        int minbone = std::min(m_currentSupplyBoneCount, newcount);
        int maxbone = std::max(m_currentSupplyBoneCount, newcount);
        bool hide = newcount < m_currentSupplyBoneCount;
        int bonecount = minbone + 1;
        std::vector<ModelConditionInfo::HideShowSubObjInfo> subobjs;

        while (bonecount <= maxbone) {
            char name[16];
            snprintf(name, 16, "%s%02d", prefix.Str(), bonecount);
            ModelConditionInfo::HideShowSubObjInfo subobj;
            subobj.hide = hide;
            subobj.sub_obj_name = name;
            subobjs.push_back(subobj);
            bonecount++;
        }

        Do_Hide_Show_Sub_Objs(&subobjs);
        m_currentSupplyBoneCount = newcount;
    }
}

void W3DSupplyDraw::CRC_Snapshot(Xfer *xfer)
{
    W3DModelDraw::CRC_Snapshot(xfer);
}

void W3DSupplyDraw::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t ver = 1;
    xfer->xferVersion(&ver, 1);
    W3DModelDraw::Xfer_Snapshot(xfer);
}

void W3DSupplyDraw::Load_Post_Process()
{
    W3DModelDraw::Load_Post_Process();
}

ModuleData *W3DSupplyDraw::Friend_New_Module_Data(INI *ini)
{
    W3DSupplyDrawModuleData *data = new W3DSupplyDrawModuleData();

    if (ini != nullptr) {
        ini->Init_From_INI_Multi_Proc(data, W3DSupplyDrawModuleData::Build_Field_Parse);
    }

    return data;
}

Module *W3DSupplyDraw::Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data)
{
    return NEW_POOL_OBJ(W3DSupplyDraw, thing, module_data);
}
