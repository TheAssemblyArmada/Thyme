/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Tank Draw Module
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dtankdraw.h"
#include "aiupdate.h"
#include "drawable.h"
#include "mapper.h"
#include "matinfo.h"
#include "object.h"
#include "particlesys.h"
#include "particlesysmanager.h"
#include "physicsupdate.h"
#include "scriptengine.h"
#include "vertmaterial.h"
#include "view.h"
#include <cstring>

using std::strchr;

W3DTankDrawModuleData::W3DTankDrawModuleData() :
    m_treadDebrisNameLeft("TrackDebrisDirtLeft"),
    m_treadDebrisNameRight("TrackDebrisDirtRight"),
    m_treadAnimationRate(0.0f),
    m_treadPivotSpeedFraction(0.6f),
    m_treadDriveSpeedFraction(0.3f)
{
}

void W3DTankDrawModuleData::Build_Field_Parse(MultiIniFieldParse &p)
{
    // clang-format off
    static const FieldParse dataFieldParse[] = {
        { "TreadDebrisLeft", &INI::Parse_AsciiString, nullptr, offsetof(W3DTankDrawModuleData, m_treadDebrisNameLeft) },
        { "TreadDebrisRight", &INI::Parse_AsciiString, nullptr, offsetof(W3DTankDrawModuleData, m_treadDebrisNameRight) },
        { "TreadAnimationRate", &INI::Parse_Velocity_Real, nullptr, offsetof(W3DTankDrawModuleData, m_treadAnimationRate) },
        { "TreadPivotSpeedFraction", &INI::Parse_Real, nullptr, offsetof(W3DTankDrawModuleData, m_treadPivotSpeedFraction) },
        { "TreadDriveSpeedFraction", &INI::Parse_Real, nullptr, offsetof(W3DTankDrawModuleData, m_treadDriveSpeedFraction) },
        { nullptr, nullptr, nullptr, 0 },
    };
    // clang-format on

    W3DModelDrawModuleData::Build_Field_Parse(p);
    p.Add(dataFieldParse, 0);
}

W3DTankDraw::W3DTankDraw(Thing *thing, ModuleData const *module_data) :
    W3DModelDraw(thing, module_data),
    m_treadDebrisLeft(nullptr),
    m_treadDebrisRight(nullptr),
    m_prevRenderObj(nullptr),
    m_treadCount(0)
{
    for (int i = 0; i < MAX_TREADS_PER_TANK; i++) {
        m_treads[i].m_robj = nullptr;
    }

    m_treadDirection.x = 1.0f;
    m_treadDirection.y = 0.0f;
    m_treadDirection.z = 0.0f;
    Create_Emitters();
}

W3DTankDraw::~W3DTankDraw()
{
    // BUGFIX: Free emitters to avoid a leak
    Toss_Emitters();

    for (int i = 0; i < MAX_TREADS_PER_TANK; i++) {
        Ref_Ptr_Release(m_treads[i].m_robj);
    }
}

NameKeyType W3DTankDraw::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("W3DTankDraw");
    return _key;
}

void W3DTankDraw::Do_Draw_Module(const Matrix3D *transform)
{
    bool frozen = g_theTacticalView->Is_Time_Frozen() && !g_theTacticalView->Is_Camera_Movement_Finished();
    frozen = frozen || g_theScriptEngine->Is_Time_Frozen_Debug() || g_theScriptEngine->Is_Time_Frozen_Script();

    if (!frozen && Get_Render_Object() != nullptr) {
        if (Get_Render_Object() != m_prevRenderObj) {
            Update_Tread_Objects();
        }

        Object *object = Get_Drawable()->Get_Object();

        if (object != nullptr) {
            PhysicsBehavior *physics = object->Get_Physics();

            if (physics) {
                const Coord3D &velocity = physics->Get_Velocity();
                int len = velocity.x * velocity.x + velocity.y * velocity.y;

                if (len > 0.00001f && !Get_Drawable()->Is_Hidden() && !Get_Fully_Obscured_By_Shroud()) {
                    Start_Move_Debris();
                } else {
                    Stop_Move_Debris();
                }

                len = GameMath::Sqrt(len);
                Coord3D mul;
                mul.x = 0.5f * len + 0.1f;

                if (mul.x > 1.0f) {
                    mul.x = 1.0f;
                }

                mul.y = mul.x;
                mul.z = len + 0.1f;

                if (mul.z > 1.0f) {
                    mul.z = 1.0f;
                }

                m_treadDebrisLeft->Set_Velocity_Multiplier(&mul);
                m_treadDebrisRight->Set_Velocity_Multiplier(&mul);
                m_treadDebrisLeft->Set_Burst_Count_Multiplier(mul.z);
                m_treadDebrisRight->Set_Burst_Count_Multiplier(mul.z);

                if (m_treadCount != 0) {
                    PhysicsTurningType turning = physics->Get_Turning();
                    float rate = Get_W3D_Tank_Draw_Module_Data()->m_treadAnimationRate;
                    float speed = object->Get_AI_Update_Interface()->Get_Cur_Locomotor_Speed();

                    if (turning != TURN_NONE
                        && physics->Get_Velocity_Magnitude() / speed
                            < Get_W3D_Tank_Draw_Module_Data()->m_treadPivotSpeedFraction) {
                        Coord3D dir;
                        object->Get_Unit_Dir_Vector2D(dir);

                        if (GameMath::Fabs(1.0f - (dir.x * m_treadDirection.x + dir.y * m_treadDirection.y)) > 0.00001f) {
                            if (turning == TURN_NEGATIVE) {
                                Update_Tread_Positions(-rate);
                            } else {
                                Update_Tread_Positions(rate);
                            }
                        }

                        m_treadDirection = dir;
                    } else if (physics->Is_Motive()) {
                        if (physics->Get_Velocity_Magnitude() / speed
                            >= Get_W3D_Tank_Draw_Module_Data()->m_treadDriveSpeedFraction) {
                            for (int i = 0; i < m_treadCount; i++) {
                                float offset = m_treads[i].m_materialSettings.m_customUVOffset.X - rate;
                                offset -= GameMath::Floor(offset);
                                m_treads[i].m_materialSettings.m_customUVOffset.Set(offset, 0.0f);
                            }
                        }
                    }
                }

                W3DModelDraw::Do_Draw_Module(transform);
            }
        }
    }
}

void W3DTankDraw::Set_Fully_Obscured_By_Shroud(bool obscured)
{
    if (obscured != Get_Fully_Obscured_By_Shroud() && obscured) {
        Stop_Move_Debris();
    }

    W3DModelDraw::Set_Fully_Obscured_By_Shroud(obscured);
}

void W3DTankDraw::On_Render_Obj_Recreated()
{
    Update_Tread_Objects();
}

void W3DTankDraw::CRC_Snapshot(Xfer *xfer)
{
    W3DModelDraw::CRC_Snapshot(xfer);
}

void W3DTankDraw::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char ver = 1;
    xfer->xferVersion(&ver, 1);
    W3DModelDraw::Xfer_Snapshot(xfer);
}

void W3DTankDraw::Load_Post_Process()
{
    W3DModelDraw::Load_Post_Process();
    Toss_Emitters();
    Create_Emitters();
}

void W3DTankDraw::Set_Hidden(bool hidden)
{
    W3DModelDraw::Set_Hidden(hidden);

    if (hidden) {
        Stop_Move_Debris();
    }
}

void W3DTankDraw::Toss_Emitters()
{
    if (m_treadDebrisLeft != nullptr) {
        m_treadDebrisLeft->Attach_To_Object(nullptr);
        m_treadDebrisLeft->Destroy();
        m_treadDebrisLeft = nullptr;
    }

    if (m_treadDebrisRight != nullptr) {
        m_treadDebrisRight->Attach_To_Object(nullptr);
        m_treadDebrisRight->Destroy();
        m_treadDebrisRight = nullptr;
    }
}

void W3DTankDraw::Create_Emitters()
{
    if (m_treadDebrisLeft == nullptr) {
        ParticleSystemTemplate *tmplate =
            g_theParticleSystemManager->Find_Template(Get_W3D_Tank_Draw_Module_Data()->m_treadDebrisNameLeft);

        if (tmplate != nullptr) {
            m_treadDebrisLeft = g_theParticleSystemManager->Create_Particle_System(tmplate, true);
            m_treadDebrisLeft->Attach_To_Drawable(Get_Drawable());
            m_treadDebrisLeft->Set_Saveable(false);
            m_treadDebrisLeft->Stop();
        }
    }

    if (m_treadDebrisRight == nullptr) {
        ParticleSystemTemplate *tmplate =
            g_theParticleSystemManager->Find_Template(Get_W3D_Tank_Draw_Module_Data()->m_treadDebrisNameRight);

        if (tmplate != nullptr) {
            m_treadDebrisRight = g_theParticleSystemManager->Create_Particle_System(tmplate, true);
            m_treadDebrisRight->Attach_To_Drawable(Get_Drawable());
            m_treadDebrisRight->Set_Saveable(false);
            m_treadDebrisRight->Stop();
        }
    }
}

void W3DTankDraw::Start_Move_Debris()
{
    if (!Get_Drawable()->Is_Hidden()) {
        if (m_treadDebrisLeft != nullptr) {
            m_treadDebrisLeft->Start();
        }

        if (m_treadDebrisRight != nullptr) {
            m_treadDebrisRight->Start();
        }
    }
}

void W3DTankDraw::Stop_Move_Debris()
{
    if (m_treadDebrisLeft != nullptr) {
        m_treadDebrisLeft->Stop();
    }

    if (m_treadDebrisRight != nullptr) {
        m_treadDebrisRight->Stop();
    }
}

void W3DTankDraw::Update_Tread_Positions(float uv_delta)
{
    for (int i = 0; i < m_treadCount; i++) {
        float offset = 0;

        if (m_treads[i].m_type == TREAD_LEFT) {
            offset = m_treads[i].m_materialSettings.m_customUVOffset.X + uv_delta;
        } else if (m_treads[i].m_type == TREAD_RIGHT) {
            offset = m_treads[i].m_materialSettings.m_customUVOffset.X - uv_delta;
        }

        offset -= GameMath::Floor(offset);
        m_treads[i].m_materialSettings.m_customUVOffset.Set(offset, 0.0f);
    }
}

void W3DTankDraw::Update_Tread_Objects()
{
    RenderObjClass *robj = Get_Render_Object();

    for (int i = 0; i < m_treadCount; i++) {
        Ref_Ptr_Release(m_treads[i].m_robj);
    }

    m_treadCount = 0;

    if (Get_W3D_Tank_Draw_Module_Data() != nullptr && Get_W3D_Tank_Draw_Module_Data()->m_treadAnimationRate != 0.0f
        && robj != nullptr) {
        for (int j = 0; j < robj->Get_Num_Sub_Objects() && m_treadCount < MAX_TREADS_PER_TANK; j++) {
            RenderObjClass *subobj = robj->Get_Sub_Object(j);

            if (subobj != nullptr) {
                if (subobj->Class_ID() == RenderObjClass::CLASSID_MESH) {
                    if (subobj->Get_Name() != nullptr) {
                        const char *str = strchr(subobj->Get_Name(), '.');
                        if (str != nullptr && *str++ != 0 && !strncasecmp(str, "TREADS", 6)) {
                            MaterialInfoClass *matinfo = subobj->Get_Material_Info();

                            if (matinfo) {
                                for (int index = 0; index < matinfo->Vertex_Material_Count(); index++) {
                                    TextureMapperClass *mapper = matinfo->Get_Vertex_Material(index)->Peek_Mapper(0);

                                    if (mapper != nullptr
                                        && mapper->Mapper_ID() == TextureMapperClass::MAPPER_ID_LINEAR_OFFSET) {
                                        static_cast<LinearOffsetTextureMapperClass *>(mapper)->Set_UV_Offset_Delta(
                                            Vector2(0.0f, 0.0f));
                                        subobj->Add_Ref();
                                        m_treads[m_treadCount].m_robj = subobj;
                                        m_treads[m_treadCount].m_type = W3DTankDraw::TREAD_MIDDLE;
                                        subobj->Set_User_Data(&m_treads[m_treadCount].m_materialSettings, false);
                                        m_treads[m_treadCount].m_materialSettings.m_customUVOffset = Vector2(0.0f, 0.0f);

                                        switch (str[6]) {
                                            case 'L':
                                            case 'l':
                                                m_treads[m_treadCount].m_type = W3DTankDraw::TREAD_LEFT;
                                                break;
                                            case 'R':
                                            case 'r':
                                                m_treads[m_treadCount].m_type = W3DTankDraw::TREAD_RIGHT;
                                                break;
                                        }

                                        m_treadCount++;
                                    }
                                }

                                Ref_Ptr_Release(matinfo);
                            }
                        }
                    }
                }
            }

            Ref_Ptr_Release(subobj);
        }
    }

    m_prevRenderObj = robj;
}

ModuleData *W3DTankDraw::Friend_New_Module_Data(INI *ini)
{
    W3DTankDrawModuleData *data = new W3DTankDrawModuleData();

    if (ini) {
        ini->Init_From_INI_Multi_Proc(data, W3DTankDrawModuleData::Build_Field_Parse);
    }

    return data;
}

Module *W3DTankDraw::Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data)
{
    return NEW_POOL_OBJ(W3DTankDraw, thing, module_data);
}
