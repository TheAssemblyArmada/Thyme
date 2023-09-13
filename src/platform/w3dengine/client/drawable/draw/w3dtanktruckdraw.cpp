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
#include "w3dtanktruckdraw.h"
#include "aiupdate.h"
#include "audiomanager.h"
#include "drawable.h"
#include "mapper.h"
#include "matinfo.h"
#include "object.h"
#include "particlesys.h"
#include "particlesysmanager.h"
#include "physicsupdate.h"
#include "scriptengine.h"
#include "view.h"

W3DTankTruckDrawModuleData::W3DTankTruckDrawModuleData() :
    m_rotationSpeedMultiplier(0.0f),
    m_powerslideRotationAddition(0.0f),
    m_treadDebrisNameLeft("TrackDebrisDirtLeft"),
    m_treadDebrisNameRight("TrackDebrisDirtRight"),
    m_treadAnimationRate(0.0f),
    m_treadPivotSpeedFraction(0.6f),
    m_treadDriveSpeedFraction(0.3f)
{
}

void W3DTankTruckDrawModuleData::Build_Field_Parse(MultiIniFieldParse &p)
{
    // clang-format off
    static FieldParse dataFieldParse[] = {
        { "Dust", &INI::Parse_AsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_dustEffectName) },
        { "DirtSpray", &INI::Parse_AsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_dirtEffectName) },
        { "PowerslideSpray", &INI::Parse_AsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_powerslideEffectName) },
        { "LeftFrontTireBone", &INI::Parse_AsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_frontLeftTireBoneName) },
        { "RightFrontTireBone", &INI::Parse_AsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_frontRightTireBoneName) },
        { "LeftRearTireBone", &INI::Parse_AsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_rearLeftTireBoneName) },
        { "RightRearTireBone", &INI::Parse_AsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_rearRightTireBoneName) },
        { "MidLeftFrontTireBone", &INI::Parse_AsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_midFrontLeftTireBoneName) },
        { "MidRightFrontTireBone", &INI::Parse_AsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_midFrontRightTireBoneName) },
        { "MidLeftRearTireBone", &INI::Parse_AsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_midRearLeftTireBoneName) },
        { "MidRightRearTireBone", &INI::Parse_AsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_midRearRightTireBoneName) },
        { "TireRotationMultiplier", &INI::Parse_Real, nullptr, offsetof(W3DTankTruckDrawModuleData, m_rotationSpeedMultiplier) },
        { "PowerslideRotationAddition", &INI::Parse_Real, nullptr, offsetof(W3DTankTruckDrawModuleData, m_powerslideRotationAddition) },
        { "TreadDebrisLeft", &INI::Parse_AsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_treadDebrisNameLeft) },
        { "TreadDebrisRight", &INI::Parse_AsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_treadDebrisNameRight) },
        { "TreadAnimationRate", &INI::Parse_Velocity_Real, nullptr, offsetof(W3DTankTruckDrawModuleData, m_treadAnimationRate) },
        { "TreadPivotSpeedFraction", &INI::Parse_Real, nullptr, offsetof(W3DTankTruckDrawModuleData, m_treadPivotSpeedFraction) },
        { "TreadDriveSpeedFraction", &INI::Parse_Real, nullptr, offsetof(W3DTankTruckDrawModuleData, m_treadDriveSpeedFraction) },
        { nullptr, nullptr, nullptr, 0 },
    };
    // clang-format on

    W3DModelDrawModuleData::Build_Field_Parse(p);
    p.Add(dataFieldParse, 0);
}

W3DTankTruckDraw::W3DTankTruckDraw(Thing *thing, ModuleData const *module_data) :
    W3DModelDraw(thing, module_data),
    m_effectsInitialized(false),
    m_wasAirborne(false),
    m_isPowersliding(false),
    m_dustEffect(nullptr),
    m_dirtEffect(nullptr),
    m_powerslideEffect(nullptr),
    m_frontWheelRotation(0.0f),
    m_rearWheelRotation(0.0f),
    m_midFrontWheelRotation(0.0f),
    m_midRearWheelRotation(0.0f),
    m_frontLeftTireBone(0),
    m_frontRightTireBone(0),
    m_rearLeftTireBone(0),
    m_rearRightTireBone(0),
    m_midFrontLeftTireBone(0),
    m_midFrontRightTireBone(0),
    m_midRearLeftTireBone(0),
    m_midRearRightTireBone(0),
    m_treadDebrisLeft(nullptr),
    m_treadDebrisRight(nullptr),
    m_treadCount(0),
    m_prevRenderObj(nullptr)
{
    m_landingSound = *thing->Get_Template()->Get_Per_Unit_Sound("TruckLandingSound");
    m_powerslideSound = *thing->Get_Template()->Get_Per_Unit_Sound("TruckPowerslideSound");

    for (int i = 0; i < MAX_TREADS_PER_TANK; i++) {
        m_treads[i].m_robj = nullptr;
    }
}

W3DTankTruckDraw::~W3DTankTruckDraw()
{
    Toss_Emitters();

    for (int i = 0; i < MAX_TREADS_PER_TANK; i++) {
        Ref_Ptr_Release(m_treads[i].m_robj);
    }
}

NameKeyType W3DTankTruckDraw::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("W3DTankTruckDraw");
    return _key;
}

void W3DTankTruckDraw::Do_Draw_Module(const Matrix3D *transform)
{
    W3DModelDraw::Do_Draw_Module(transform);

    if (g_theWriteableGlobalData->m_showClientPhysics) {
        bool frozen = g_theTacticalView->Is_Time_Frozen() && !g_theTacticalView->Is_Camera_Movement_Finished();
        frozen = frozen || g_theScriptEngine->Is_Time_Frozen_Debug() || g_theScriptEngine->Is_Time_Frozen_Script();

        if (!frozen) {
            Object *object = Get_Drawable()->Get_Object();

            if (object != nullptr) {
                if (Get_Render_Object() != nullptr) {
                    if (Get_Render_Object() != m_prevRenderObj) {
                        Update_Bones();
                        Update_Tread_Objects();
                    }

                    PhysicsBehavior *physics = object->Get_Physics();

                    if (physics != nullptr) {
                        const Coord3D &velocity = physics->Get_Velocity();
                        float magnitude = physics->Get_Velocity_Magnitude();
                        const TWheelInfo *info = Get_Drawable()->Get_Wheel_Info();

                        if (info && (m_frontLeftTireBone != 0 || m_rearLeftTireBone != 0)) {
                            float rotation = Get_W3D_Tank_Truck_Draw_Module_Data()->m_rotationSpeedMultiplier;
                            m_frontWheelRotation = rotation * magnitude + m_frontWheelRotation;

                            if (m_isPowersliding) {
                                m_rearWheelRotation =
                                    (magnitude + Get_W3D_Tank_Truck_Draw_Module_Data()->m_powerslideRotationAddition)
                                        * rotation
                                    + m_rearWheelRotation;
                            } else {
                                m_rearWheelRotation = rotation * magnitude + m_rearWheelRotation;
                            }

                            Matrix3D m(true);

                            if (m_frontLeftTireBone != 0) {
                                m.Adjust_Z_Translation(info->m_frontLeftHeightOffset);
                                m.Rotate_Z(info->m_wheelAngle);
                                m.Rotate_Y(m_frontWheelRotation);
                                Get_Render_Object()->Capture_Bone(m_frontLeftTireBone);
                                Get_Render_Object()->Control_Bone(m_frontLeftTireBone, m);

                                m.Make_Identity();
                                m.Adjust_Z_Translation(info->m_frontRightHeightOffset);
                                m.Rotate_Z(info->m_wheelAngle);
                                m.Rotate_Y(m_frontWheelRotation);
                                Get_Render_Object()->Capture_Bone(m_frontRightTireBone);
                                Get_Render_Object()->Control_Bone(m_frontRightTireBone, m);
                            }

                            if (m_rearLeftTireBone != 0) {
                                m.Make_Identity();
                                m.Rotate_Y(m_rearWheelRotation);
                                m.Adjust_Z_Translation(info->m_rearLeftHeightOffset);
                                Get_Render_Object()->Capture_Bone(m_rearLeftTireBone);
                                Get_Render_Object()->Control_Bone(m_rearLeftTireBone, m);

                                m.Make_Identity();
                                m.Rotate_Y(m_rearWheelRotation);
                                m.Adjust_Z_Translation(info->m_rearRightHeightOffset);
                                Get_Render_Object()->Capture_Bone(m_rearRightTireBone);
                                Get_Render_Object()->Control_Bone(m_rearRightTireBone, m);
                            }

                            if (m_midFrontLeftTireBone != 0) {
                                m.Adjust_Z_Translation(info->m_frontLeftHeightOffset);
                                m.Rotate_Z(info->m_wheelAngle);
                                m.Rotate_Y(m_midFrontWheelRotation);
                                Get_Render_Object()->Capture_Bone(m_midFrontLeftTireBone);
                                Get_Render_Object()->Control_Bone(m_midFrontLeftTireBone, m);

                                m.Make_Identity();
                                m.Adjust_Z_Translation(info->m_frontRightHeightOffset);
                                m.Rotate_Z(info->m_wheelAngle);
                                m.Rotate_Y(m_midFrontWheelRotation);
                                Get_Render_Object()->Capture_Bone(m_midFrontRightTireBone);
                                Get_Render_Object()->Control_Bone(m_midFrontRightTireBone, m);
                            }

                            if (m_midRearLeftTireBone != 0) {
                                m.Make_Identity();
                                m.Rotate_Y(m_midRearWheelRotation);
                                m.Adjust_Z_Translation(info->m_rearLeftHeightOffset);
                                Get_Render_Object()->Capture_Bone(m_midRearLeftTireBone);
                                Get_Render_Object()->Control_Bone(m_midRearLeftTireBone, m);

                                m.Make_Identity();
                                m.Rotate_Y(m_midRearWheelRotation);
                                m.Adjust_Z_Translation(info->m_rearRightHeightOffset);
                                Get_Render_Object()->Capture_Bone(m_midRearRightTireBone);
                                Get_Render_Object()->Control_Bone(m_midRearRightTireBone, m);
                            }
                        }

                        bool slide = m_isPowersliding;
                        m_isPowersliding = false;

                        if (physics->Is_Motive() && !object->Is_Significantly_Above_Terrain()) {
                            Enable_Emitters(true);
                            Coord3D prev = physics->Get_Prev_Accel();
                            Coord3D c;
                            c.x = prev.x;
                            c.y = prev.y;
                            c.z = 0.0f;
                            float len = c.Length();
                            bool b = len > 0.01f;

                            if (b) {
                                if (c.x * velocity.x + c.y * velocity.y < 0.0f) {
                                    b = false;
                                }
                            }

                            if (m_dustEffect != nullptr) {
                                if (magnitude > 2.0f) {
                                    magnitude = 2.0f;
                                }

                                m_dustEffect->Set_Size_Multiplier(magnitude);
                            }

                            if (m_dirtEffect != nullptr) {
                                if (info != nullptr && info->m_framesAirborne > 3) {
                                    float f = info->m_framesAirborne / 16 + 1;

                                    if (f > 2.0f) {
                                        f = 2.0f;
                                    }

                                    m_dustEffect->Set_Size_Multiplier(f * 2.0f);
                                    m_dustEffect->Reset_Delay();
                                    m_landingSound.Set_Position(object->Get_Position());
                                    g_theAudio->Add_Audio_Event(&m_landingSound);
                                } else if (!b || magnitude > 2.0f) {
                                    m_dirtEffect->Stop();
                                }
                            }

                            if (m_powerslideEffect != nullptr) {
                                if (physics->Get_Turning() != TURN_NONE) {
                                    m_isPowersliding = true;
                                    m_powerslideEffect->Start();
                                } else {
                                    m_powerslideEffect->Stop();
                                }
                            }

                            if (m_dirtEffect != nullptr && (!b || magnitude > 2.0f)) {
                                m_dirtEffect->Stop();
                            }
                        } else {
                            Enable_Emitters(false);
                        }

                        m_wasAirborne = object->Is_Significantly_Above_Terrain();

                        if (!slide && m_isPowersliding) {
                            m_powerslideSound.Set_Object_ID(object->Get_ID());
                            m_powerslideSound.Set_Playing_Handle(g_theAudio->Add_Audio_Event(&m_powerslideSound));
                        } else if (slide && !m_isPowersliding) {
                            g_theAudio->Remove_Audio_Event(m_powerslideSound.Get_Playing_Handle());
                        }

                        if (m_treadCount != 0) {
                            float rate = Get_W3D_Tank_Truck_Draw_Module_Data()->m_treadAnimationRate;
                            float speed = object->Get_AI_Update_Interface()->Get_Cur_Locomotor_Speed();

                            if (physics->Is_Motive()) {
                                if (physics->Get_Velocity_Magnitude() / speed
                                    >= Get_W3D_Tank_Truck_Draw_Module_Data()->m_treadDriveSpeedFraction) {
                                    for (int i = 0; i < m_treadCount; i++) {
                                        float offset = m_treads[i].m_materialSettings.m_customUVOffset.X - rate;
                                        offset -= GameMath::Floor(offset);
                                        m_treads[i].m_materialSettings.m_customUVOffset.Set(offset, 0.0f);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void W3DTankTruckDraw::Set_Fully_Obscured_By_Shroud(bool obscured)
{
    if (obscured != Get_Fully_Obscured_By_Shroud() && obscured) {
        if (obscured) {
            Toss_Emitters();
        } else {
            Create_Emitters();
        }
    }

    W3DModelDraw::Set_Fully_Obscured_By_Shroud(obscured);
}

void W3DTankTruckDraw::On_Render_Obj_Recreated()
{
    m_prevRenderObj = nullptr;
    m_frontLeftTireBone = 0;
    m_frontRightTireBone = 0;
    m_rearLeftTireBone = 0;
    m_rearRightTireBone = 0;
    m_midFrontLeftTireBone = 0;
    m_midFrontRightTireBone = 0;
    m_midRearLeftTireBone = 0;
    m_midRearRightTireBone = 0;
    Update_Bones();
    Update_Tread_Objects();
}

void W3DTankTruckDraw::CRC_Snapshot(Xfer *xfer)
{
    W3DModelDraw::CRC_Snapshot(xfer);
}

void W3DTankTruckDraw::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char ver = 1;
    xfer->xferVersion(&ver, 1);
    W3DModelDraw::Xfer_Snapshot(xfer);
}

void W3DTankTruckDraw::Load_Post_Process()
{
    W3DModelDraw::Load_Post_Process();
    Toss_Emitters();
}

void W3DTankTruckDraw::Set_Hidden(bool hidden)
{
    W3DModelDraw::Set_Hidden(hidden);

    if (hidden) {
        Enable_Emitters(false);
    }
}

void W3DTankTruckDraw::Toss_Emitters()
{
    if (m_dustEffect != nullptr) {
        m_dustEffect->Attach_To_Object(nullptr);
        m_dustEffect->Destroy();
        m_dustEffect = nullptr;
    }

    if (m_dirtEffect != nullptr) {
        m_dirtEffect->Attach_To_Object(nullptr);
        m_dirtEffect->Destroy();
        m_dirtEffect = nullptr;
    }

    if (m_powerslideEffect != nullptr) {
        m_powerslideEffect->Attach_To_Object(nullptr);
        m_powerslideEffect->Destroy();
        m_powerslideEffect = nullptr;
    }
}

void W3DTankTruckDraw::Create_Emitters()
{
    const W3DTankTruckDrawModuleData *data = Get_W3D_Tank_Truck_Draw_Module_Data();
    if (!Get_Drawable()->Is_Hidden() && data != nullptr) {
        if (m_dustEffect == nullptr) {
            ParticleSystemTemplate *tmplate = g_theParticleSystemManager->Find_Template(data->m_dustEffectName);

            if (tmplate != nullptr) {
                m_dustEffect = g_theParticleSystemManager->Create_Particle_System(tmplate, true);
                m_dustEffect->Attach_To_Object(Get_Drawable()->Get_Object());
                m_dustEffect->Set_Saveable(false);
                m_dustEffect->Stop();
            } else {
                if (!data->m_dustEffectName.Is_Empty()) {
                    captainslog_debug("*** ERROR - Missing particle system '%s' in thing '%s'",
                        data->m_dustEffectName.Str(),
                        Get_Drawable()->Get_Object()->Get_Template()->Get_Name().Str());
                }
            }
        }

        if (m_dirtEffect == nullptr) {
            ParticleSystemTemplate *tmplate = g_theParticleSystemManager->Find_Template(data->m_dirtEffectName);

            if (tmplate != nullptr) {
                m_dirtEffect = g_theParticleSystemManager->Create_Particle_System(tmplate, true);
                m_dirtEffect->Attach_To_Object(Get_Drawable()->Get_Object());
                m_dirtEffect->Set_Saveable(false);
                m_dirtEffect->Stop();
            } else {
                if (!data->m_dirtEffectName.Is_Empty()) {
                    captainslog_debug("*** ERROR - Missing particle system '%s' in thing '%s'",
                        data->m_dirtEffectName.Str(),
                        Get_Drawable()->Get_Object()->Get_Template()->Get_Name().Str());
                }
            }
        }

        if (m_powerslideEffect == nullptr) {
            ParticleSystemTemplate *tmplate = g_theParticleSystemManager->Find_Template(data->m_powerslideEffectName);

            if (tmplate != nullptr) {
                m_powerslideEffect = g_theParticleSystemManager->Create_Particle_System(tmplate, true);
                m_powerslideEffect->Attach_To_Object(Get_Drawable()->Get_Object());
                m_powerslideEffect->Set_Saveable(false);
                m_powerslideEffect->Stop();
            } else {
                if (!data->m_powerslideEffectName.Is_Empty()) {
                    captainslog_debug("*** ERROR - Missing particle system '%s' in thing '%s'",
                        data->m_powerslideEffectName.Str(),
                        Get_Drawable()->Get_Object()->Get_Template()->Get_Name().Str());
                }
            }
        }
    }
}

void W3DTankTruckDraw::Enable_Emitters(bool enable)
{
    Create_Emitters();
    m_effectsInitialized = true;

    if (m_dustEffect != nullptr) {
        if (enable) {
            m_dustEffect->Start();
        } else {
            m_dustEffect->Stop();
        }
    }

    if (m_dirtEffect != nullptr) {
        if (enable) {
            m_dirtEffect->Start();
        } else {
            m_dirtEffect->Stop();
        }
    }

    if (m_powerslideEffect != nullptr) {
        if (!enable) {
            m_powerslideEffect->Stop();
        }
    }
}

void W3DTankTruckDraw::Update_Bones()
{
    const W3DTankTruckDrawModuleData *data = Get_W3D_Tank_Truck_Draw_Module_Data();

    if (data != nullptr) {
        if (!data->m_frontLeftTireBoneName.Is_Empty()) {
            m_frontLeftTireBone = Get_Render_Object()->Get_Bone_Index(data->m_frontLeftTireBoneName.Str());
            captainslog_dbgassert(m_frontLeftTireBone != 0,
                "Missing front-left tire bone %s in model %s",
                data->m_frontLeftTireBoneName.Str(),
                Get_Render_Object()->Get_Name());

            m_frontRightTireBone = Get_Render_Object()->Get_Bone_Index(data->m_frontRightTireBoneName.Str());
            captainslog_dbgassert(m_frontRightTireBone != 0,
                "Missing front-right tire bone %s in model %s",
                data->m_frontRightTireBoneName.Str(),
                Get_Render_Object()->Get_Name());

            if (m_frontRightTireBone == 0) {
                m_frontLeftTireBone = 0;
            }
        }

        if (!data->m_rearLeftTireBoneName.Is_Empty()) {
            m_rearLeftTireBone = Get_Render_Object()->Get_Bone_Index(data->m_rearLeftTireBoneName.Str());
            captainslog_dbgassert(m_rearLeftTireBone != 0,
                "Missing rear-left tire bone %s in model %s",
                data->m_rearLeftTireBoneName.Str(),
                Get_Render_Object()->Get_Name());

            m_rearRightTireBone = Get_Render_Object()->Get_Bone_Index(data->m_rearRightTireBoneName.Str());
            captainslog_dbgassert(m_rearRightTireBone != 0,
                "Missing rear-right tire bone %s in model %s",
                data->m_rearRightTireBoneName.Str(),
                Get_Render_Object()->Get_Name());

            if (m_rearRightTireBone == 0) {
                m_rearLeftTireBone = 0;
            }
        }

        if (!data->m_midFrontLeftTireBoneName.Is_Empty()) {
            m_midFrontLeftTireBone = Get_Render_Object()->Get_Bone_Index(data->m_midFrontLeftTireBoneName.Str());
            captainslog_dbgassert(m_midFrontLeftTireBone != 0,
                "Missing mid-front-left tire bone %s in model %s",
                data->m_midFrontLeftTireBoneName.Str(),
                Get_Render_Object()->Get_Name());

            m_midFrontRightTireBone = Get_Render_Object()->Get_Bone_Index(data->m_midFrontRightTireBoneName.Str());
            captainslog_dbgassert(m_midFrontRightTireBone != 0,
                "Missing mid-front-right tire bone %s in model %s",
                data->m_midFrontRightTireBoneName.Str(),
                Get_Render_Object()->Get_Name());

            if (m_midFrontRightTireBone == 0) {
                m_midFrontLeftTireBone = 0;
            }
        }

        if (!data->m_midRearLeftTireBoneName.Is_Empty()) {
            m_midRearLeftTireBone = Get_Render_Object()->Get_Bone_Index(data->m_midRearLeftTireBoneName.Str());
            captainslog_dbgassert(m_midRearLeftTireBone != 0,
                "Missing mid-rear-left tire bone %s in model %s",
                data->m_midRearLeftTireBoneName.Str(),
                Get_Render_Object()->Get_Name());

            m_midRearRightTireBone = Get_Render_Object()->Get_Bone_Index(data->m_midRearRightTireBoneName.Str());
            captainslog_dbgassert(m_midRearRightTireBone != 0,
                "Missing mid-rear-right tire bone %s in model %s",
                data->m_midRearRightTireBoneName.Str(),
                Get_Render_Object()->Get_Name());

            if (m_midRearRightTireBone == 0) {
                m_midRearLeftTireBone = 0;
            }
        }
    }

    m_prevRenderObj = Get_Render_Object();
}

void W3DTankTruckDraw::Start_Move_Debris()
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

void W3DTankTruckDraw::Stop_Move_Debris()
{
    if (m_treadDebrisLeft != nullptr) {
        m_treadDebrisLeft->Stop();
    }

    if (m_treadDebrisRight != nullptr) {
        m_treadDebrisRight->Stop();
    }
}

void W3DTankTruckDraw::Update_Tread_Positions(float uv_delta)
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

void W3DTankTruckDraw::Update_Tread_Objects()
{
    RenderObjClass *robj = Get_Render_Object();

    for (int i = 0; i < m_treadCount; i++) {
        Ref_Ptr_Release(m_treads[i].m_robj);
    }

    m_treadCount = 0;

    if (Get_W3D_Tank_Truck_Draw_Module_Data() != nullptr
        && Get_W3D_Tank_Truck_Draw_Module_Data()->m_treadAnimationRate != 0.0f && robj != nullptr) {
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
                                        m_treads[m_treadCount].m_type = TREAD_MIDDLE;
                                        subobj->Set_User_Data(&m_treads[m_treadCount].m_materialSettings, false);
                                        m_treads[m_treadCount].m_materialSettings.m_customUVOffset = Vector2(0.0f, 0.0f);
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

ModuleData *W3DTankTruckDraw::Friend_New_Module_Data(INI *ini)
{
    W3DTankTruckDrawModuleData *data = new W3DTankTruckDrawModuleData();

    if (ini) {
        ini->Init_From_INI_Multi_Proc(data, W3DTankTruckDrawModuleData::Build_Field_Parse);
    }

    return data;
}

Module *W3DTankTruckDraw::Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data)
{
    return NEW_POOL_OBJ(W3DTankTruckDraw, thing, module_data);
}
