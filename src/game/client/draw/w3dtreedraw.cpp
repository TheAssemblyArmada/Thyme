/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Tree Draw
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dtreedraw.h"
#include "baseheightmap.h"
#include "drawable.h"
#include "fxlist.h"

W3DTreeDrawModuleData::W3DTreeDrawModuleData() :
    m_moveOutwardTime(1),
    m_moveInwardTime(1),
    m_moveOutwardDistanceFactor(1.0f),
    m_darkeningFactor(0.0f),
    m_toppleFX(nullptr),
    m_bounceFX(nullptr),
    m_killWhenFinishedToppling(true),
    m_doTopple(false),
    m_doShadow(false),
    m_initialVelocityPercent(0.2f),
    m_initialAccelPercent(0.01f),
    m_bounceVelocityPercent(0.3f),
    m_minimumToppleSpeed(0.5f),
    m_sinkTime(300),
    m_sinkDistance(20.0f)
{
}

void W3DTreeDrawModuleData::Build_Field_Parse(MultiIniFieldParse &p)
{
    // clang-format off
    static const FieldParse dataFieldParse[] = {
        FIELD_PARSE_ASCIISTRING("ModelName", W3DTreeDrawModuleData, m_modelName),
        FIELD_PARSE_ASCIISTRING("TextureName", W3DTreeDrawModuleData, m_textureName),
        { "MoveOutwardTime", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(W3DTreeDrawModuleData, m_moveOutwardTime) },
        { "MoveInwardTime", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(W3DTreeDrawModuleData, m_moveInwardTime) },
        FIELD_PARSE_REAL("MoveOutwardDistanceFactor", W3DTreeDrawModuleData, m_moveOutwardDistanceFactor),
        FIELD_PARSE_REAL("DarkeningFactor", W3DTreeDrawModuleData, m_darkeningFactor),
        { "ToppleFX", &FXList::Parse, nullptr, offsetof(W3DTreeDrawModuleData, m_toppleFX) },
        { "BounceFX", &FXList::Parse, nullptr, offsetof(W3DTreeDrawModuleData, m_bounceFX) },
        FIELD_PARSE_ASCIISTRING("StumpName", W3DTreeDrawModuleData, m_stumpName),
        FIELD_PARSE_BOOL("KillWhenFinishedToppling", W3DTreeDrawModuleData, m_killWhenFinishedToppling),
        FIELD_PARSE_BOOL("DoTopple", W3DTreeDrawModuleData, m_doTopple),
        FIELD_PARSE_PERCENT_TO_REAL("InitialVelocityPercent", W3DTreeDrawModuleData, m_initialVelocityPercent),
        FIELD_PARSE_PERCENT_TO_REAL("InitialAccelPercent", W3DTreeDrawModuleData, m_initialAccelPercent),
        FIELD_PARSE_PERCENT_TO_REAL("BounceVelocityPercent", W3DTreeDrawModuleData, m_bounceVelocityPercent),
        FIELD_PARSE_POSITIVE_NON_ZERO_REAL("MinimumToppleSpeed", W3DTreeDrawModuleData, m_minimumToppleSpeed),
        FIELD_PARSE_POSITIVE_NON_ZERO_REAL("SinkDistance", W3DTreeDrawModuleData, m_sinkDistance),
        { "SinkTime", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(W3DTreeDrawModuleData, m_sinkTime) },
        FIELD_PARSE_BOOL("DoShadow", W3DTreeDrawModuleData, m_doShadow),
        FIELD_PARSE_LAST
    };
    // clang-format on

    ModuleData::Build_Field_Parse(p);
    p.Add(dataFieldParse, 0);
}

W3DTreeDraw::W3DTreeDraw(Thing *thing, ModuleData const *module_data) : DrawModule(thing, module_data), m_transformSet(false)
{
}

ModuleData *W3DTreeDraw::Friend_New_Module_Data(INI *ini)
{
    W3DTreeDrawModuleData *data = new W3DTreeDrawModuleData();

    if (ini != nullptr) {
        ini->Init_From_INI_Multi_Proc(data, W3DTreeDrawModuleData::Build_Field_Parse);
    }

    return data;
}

Module *W3DTreeDraw::Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data)
{
    return NEW_POOL_OBJ(W3DTreeDraw, thing, module_data);
}

NameKeyType W3DTreeDraw::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("W3DTreeDraw");
    return _key;
}

void W3DTreeDraw::React_To_Transform_Change(const Matrix3D *matrix, const Coord3D *pos, float angle)
{
    Drawable *drawable = Get_Drawable();

    if (!m_transformSet && (drawable->Get_Position()->x != 0.0f || drawable->Get_Position()->y != 0.0f)) {
        m_transformSet = true;
        const W3DTreeDrawModuleData *data = Get_W3D_Tree_Draw_Module_Data();

        if (data != nullptr) {
            g_theTerrainRenderObject->Add_Tree(drawable->Get_ID(),
                *drawable->Get_Position(),
                drawable->Get_Scale(),
                drawable->Get_Orientation(),
                0.0f,
                data);
        }
    }
}
