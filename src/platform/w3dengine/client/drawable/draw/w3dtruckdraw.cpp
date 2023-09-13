/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Truck Draw Module
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dtruckdraw.h"
#include "aipathfind.h"
#include "aiupdate.h"
#include "audiomanager.h"
#include "drawable.h"
#include "globaldata.h"
#include "locomotor.h"
#include "particlesys.h"
#include "particlesysmanager.h"
#include "partitionmanager.h"
#include "physicsupdate.h"
#include "rendobj.h"
#include "scriptengine.h"
#include "w3dview.h"

void W3DTruckDrawModuleData::Build_Field_Parse(MultiIniFieldParse &p)
{
    // clang-format off
    static FieldParse dataFieldParse[] = {
        { "Dust", &INI::Parse_AsciiString, nullptr, offsetof(W3DTruckDrawModuleData, m_dustEffectName) },
        { "DirtSpray", &INI::Parse_AsciiString, nullptr, offsetof(W3DTruckDrawModuleData, m_dirtEffectName) },
        { "PowerslideSpray", &INI::Parse_AsciiString, nullptr, offsetof(W3DTruckDrawModuleData, m_powerslideEffectName) },
        { "LeftFrontTireBone", &INI::Parse_AsciiString, nullptr, offsetof(W3DTruckDrawModuleData, m_frontLeftTireBoneName) },
        { "RightFrontTireBone", &INI::Parse_AsciiString, nullptr, offsetof(W3DTruckDrawModuleData, m_frontRightTireBoneName) },
        { "LeftRearTireBone", &INI::Parse_AsciiString, nullptr, offsetof(W3DTruckDrawModuleData, m_rearLeftTireBoneName) },
        { "RightRearTireBone", &INI::Parse_AsciiString, nullptr, offsetof(W3DTruckDrawModuleData, m_rearRightTireBoneName) },
        { "MidLeftFrontTireBone", &INI::Parse_AsciiString, nullptr, offsetof(W3DTruckDrawModuleData, m_midFrontLeftTireBoneName) },
        { "MidRightFrontTireBone", &INI::Parse_AsciiString, nullptr, offsetof(W3DTruckDrawModuleData, m_midFrontRightTireBoneName) },
        { "MidLeftRearTireBone", &INI::Parse_AsciiString, nullptr, offsetof(W3DTruckDrawModuleData, m_midRearLeftTireBoneName) },
        { "MidRightRearTireBone", &INI::Parse_AsciiString, nullptr, offsetof(W3DTruckDrawModuleData, m_midRearRightTireBoneName) },
        { "MidLeftMidTireBone", &INI::Parse_AsciiString, nullptr, offsetof(W3DTruckDrawModuleData, m_midMidLeftTireBoneName) },
        { "MidRightMidTireBone", &INI::Parse_AsciiString, nullptr, offsetof(W3DTruckDrawModuleData, m_midMidRightTireBoneName) },
        { "TireRotationMultiplier", &INI::Parse_Real, nullptr, offsetof(W3DTruckDrawModuleData, m_rotationSpeedMultiplier) },
        { "PowerslideRotationAddition", &INI::Parse_Real, nullptr, offsetof(W3DTruckDrawModuleData, m_powerslideRotationAddition) },
        { "CabBone", &INI::Parse_AsciiString, nullptr, offsetof(W3DTruckDrawModuleData, m_cabBoneName) },
        { "TrailerBone", &INI::Parse_AsciiString, nullptr, offsetof(W3DTruckDrawModuleData, m_trailerBoneName) },
        { "CabRotationMultiplier", &INI::Parse_Real, nullptr, offsetof(W3DTruckDrawModuleData, m_cabRotationMultiplier) },
        { "TrailerRotationMultiplier", &INI::Parse_Real, nullptr, offsetof(W3DTruckDrawModuleData, m_trailerRotationMultiplier) },
        { "RotationDamping", &INI::Parse_Real, nullptr, offsetof(W3DTruckDrawModuleData, m_rotationDamping) },
        { nullptr, nullptr, nullptr, 0 },
    };
    // clang-format on

    W3DModelDrawModuleData::Build_Field_Parse(p);
    p.Add(dataFieldParse, 0);
}

W3DTruckDraw::W3DTruckDraw(Thing *thing, ModuleData const *module_data) :
    W3DModelDraw(thing, module_data),
    // BUGFIX: Init all members
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
    m_midMidLeftTireBone(0),
    m_midMidRightTireBone(0),
    m_cabBone(0),
    m_cabRotation(0.0f),
    m_trailerBone(0),
    m_trailerRotation(0.0f),
    m_numBones(0),
    m_prevRenderObj(nullptr)
{
    m_landingSound = *thing->Get_Template()->Get_Per_Unit_Sound("TruckLandingSound");
    m_powerslideSound = *thing->Get_Template()->Get_Per_Unit_Sound("TruckPowerslideSound");
}

W3DTruckDraw::~W3DTruckDraw()
{
    Toss_Emitters();
}

NameKeyType W3DTruckDraw::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("W3DTruckDraw");
    return _key;
}

void W3DTruckDraw::Do_Draw_Module(const Matrix3D *transform)
{
    W3DModelDraw::Do_Draw_Module(transform);

    if (g_theWriteableGlobalData->m_showClientPhysics) {
        const W3DTruckDrawModuleData *data = Get_W3D_Truck_Draw_Module_Data();

        if (data != nullptr) {
            bool frozen = g_theTacticalView->Is_Time_Frozen() && !g_theTacticalView->Is_Camera_Movement_Finished();
            frozen = frozen || g_theScriptEngine->Is_Time_Frozen_Debug() || g_theScriptEngine->Is_Time_Frozen_Script();

            if (!frozen) {
                Object *object = Get_Drawable()->Get_Object();

                if (object != nullptr) {
                    if (Get_Render_Object() != nullptr) {
                        if (Get_Render_Object() != m_prevRenderObj) {
                            captainslog_debug("W3DTruckDraw::Do_Draw_Module - shouldn't update bones.");
                            Update_Bones();
                        }

                        PhysicsBehavior *physics = object->Get_Physics();

                        if (physics != nullptr) {
                            const Coord3D &velocity = physics->Get_Velocity();
                            float magnitude = physics->Get_Velocity_Magnitude();
                            const TWheelInfo *info = Get_Drawable()->Get_Wheel_Info();
                            AIUpdateInterface *update = object->Get_AI_Update_Interface();

                            if (m_cabBone != 0 && info != nullptr) {
                                Matrix3D m;
                                m.Make_Identity();
                                float angle = info->m_wheelAngle * data->m_cabRotationMultiplier;

                                if (update != nullptr && update->Get_Path() != nullptr) {
                                    Coord3D pos;
                                    update->Get_Path()->Get_Point_Pos(&pos);
                                    float angle2d = g_thePartitionManager->Get_Relative_Angle_2D(object, &pos);

                                    if (angle2d < 0.0f) {
                                        if (angle < angle2d) {
                                            angle = angle2d;
                                        }

                                        if (angle > 0.0f) {
                                            angle = 0.0f;
                                        }
                                    } else {
                                        if (angle > angle2d) {
                                            angle = angle2d;
                                        }

                                        if (angle < 0.0f) {
                                            angle = 0.0f;
                                        }
                                    }
                                }

                                m_cabRotation += (angle - m_cabRotation) * data->m_rotationDamping;
                                m.Rotate_Z(m_cabRotation);
                                Get_Render_Object()->Capture_Bone(m_cabBone);
                                Get_Render_Object()->Control_Bone(m_cabBone, m);

                                if (info != nullptr && m_trailerBone != 0) {
                                    angle = -info->m_wheelAngle * data->m_trailerRotationMultiplier;
                                    m_trailerRotation += (angle - m_trailerRotation) * data->m_rotationDamping;
                                    m.Make_Identity();
                                    m.Rotate_Z(m_trailerRotation);
                                    Get_Render_Object()->Capture_Bone(m_trailerBone);
                                    Get_Render_Object()->Control_Bone(m_trailerBone, m);
                                }
                            }

                            if (m_frontLeftTireBone != 0 || m_rearLeftTireBone != 0) {
                                float addition = data->m_powerslideRotationAddition;

                                if (update != nullptr) {
                                    const Locomotor *locomotor = update->Get_Cur_Locomotor();

                                    if (locomotor != nullptr) {
                                        if (locomotor->Is_Moving_Backwards()) {
                                            magnitude = -magnitude;
                                            addition = -addition;
                                        }
                                    }
                                }

                                float rotation = data->m_rotationSpeedMultiplier;
                                m_frontWheelRotation += rotation * magnitude;

                                if (m_isPowersliding) {
                                    m_rearWheelRotation += (magnitude + addition) * rotation;
                                } else {
                                    m_rearWheelRotation += rotation * magnitude;
                                }

                                m_midFrontWheelRotation = m_frontWheelRotation;
                                m_midRearWheelRotation = m_rearWheelRotation;

                                Matrix3D m;

                                if (m_frontLeftTireBone != 0 && info != nullptr) {
                                    m.Make_Identity();
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

                                if (m_rearLeftTireBone != 0 && info != nullptr) {
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

                                if (m_midFrontLeftTireBone != 0 && info != nullptr) {
                                    m.Make_Identity();
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

                                if (m_midRearLeftTireBone != 0 && info != nullptr) {
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

                                if (m_midMidLeftTireBone != 0 && info != nullptr) {
                                    m.Make_Identity();
                                    m.Rotate_Y(m_midRearWheelRotation);
                                    m.Adjust_Z_Translation(info->m_rearLeftHeightOffset);
                                    Get_Render_Object()->Capture_Bone(m_midMidLeftTireBone);
                                    Get_Render_Object()->Control_Bone(m_midMidLeftTireBone, m);

                                    m.Make_Identity();
                                    m.Rotate_Y(m_midRearWheelRotation);
                                    m.Adjust_Z_Translation(info->m_rearRightHeightOffset);
                                    Get_Render_Object()->Capture_Bone(m_midMidRightTireBone);
                                    Get_Render_Object()->Control_Bone(m_midMidRightTireBone, m);
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
                                bool is_moving = len > 0.01f;

                                if (is_moving) {
                                    if (c.x * velocity.x + c.y * velocity.y < 0.0f) {
                                        is_moving = false;
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
                                        m_landingSound.Set_Object_ID(object->Get_ID());
                                        g_theAudio->Add_Audio_Event(&m_landingSound);
                                    } else if (!is_moving || magnitude > 2.0f) {
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

                                if (m_dirtEffect != nullptr && (!is_moving || magnitude > 2.0f)) {
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
                        }
                    }
                }
            }
        }
    }
}

void W3DTruckDraw::Set_Fully_Obscured_By_Shroud(bool obscured)
{
    if (obscured != Get_Fully_Obscured_By_Shroud()) {
        if (obscured) {
            Toss_Emitters();
        } else {
            Create_Emitters();
        }
    }

    W3DModelDraw::Set_Fully_Obscured_By_Shroud(obscured);
}

void W3DTruckDraw::On_Render_Obj_Recreated()
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
    m_midMidLeftTireBone = 0;
    m_midMidRightTireBone = 0;
    Update_Bones();
}

void W3DTruckDraw::CRC_Snapshot(Xfer *xfer)
{
    W3DModelDraw::CRC_Snapshot(xfer);
}

void W3DTruckDraw::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t ver = 1;
    xfer->xferVersion(&ver, 1);
    W3DModelDraw::Xfer_Snapshot(xfer);
}

void W3DTruckDraw::Load_Post_Process()
{
    W3DModelDraw::Load_Post_Process();
    Toss_Emitters();
}

void W3DTruckDraw::Set_Hidden(bool hidden)
{
    W3DModelDraw::Set_Hidden(hidden);

    if (hidden) {
        Enable_Emitters(false);
    }
}

void W3DTruckDraw::Toss_Emitters()
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

void W3DTruckDraw::Create_Emitters()
{
    const W3DTruckDrawModuleData *data = Get_W3D_Truck_Draw_Module_Data();
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

void W3DTruckDraw::Enable_Emitters(bool enable)
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

void W3DTruckDraw::Update_Bones()
{
    const W3DTruckDrawModuleData *data = Get_W3D_Truck_Draw_Module_Data();

    // Thyme specific: All original asserts have been demoted to log messages because they are data issues.

    if (data != nullptr) {
        if (!data->m_frontLeftTireBoneName.Is_Empty()) {
            m_frontLeftTireBone = Get_Render_Object()->Get_Bone_Index(data->m_frontLeftTireBoneName.Str());
            if (m_frontLeftTireBone == 0) {
                captainslog_error("Missing front-left tire bone %s in model %s",
                    data->m_frontLeftTireBoneName.Str(),
                    Get_Render_Object()->Get_Name());
            }
        }

        if (!data->m_frontRightTireBoneName.Is_Empty()) {
            m_frontRightTireBone = Get_Render_Object()->Get_Bone_Index(data->m_frontRightTireBoneName.Str());
            if (m_frontRightTireBone == 0) {
                captainslog_error("Missing front-right tire bone %s in model %s",
                    data->m_frontRightTireBoneName.Str(),
                    Get_Render_Object()->Get_Name());
            }
        }

        if (!data->m_rearLeftTireBoneName.Is_Empty()) {
            m_rearLeftTireBone = Get_Render_Object()->Get_Bone_Index(data->m_rearLeftTireBoneName.Str());
            if (m_rearLeftTireBone == 0) {
                captainslog_error("Missing rear-left tire bone %s in model %s",
                    data->m_rearLeftTireBoneName.Str(),
                    Get_Render_Object()->Get_Name());
            }
        }

        if (!data->m_rearRightTireBoneName.Is_Empty()) {
            m_rearRightTireBone = Get_Render_Object()->Get_Bone_Index(data->m_rearRightTireBoneName.Str());
            if (m_rearRightTireBone == 0) {
                captainslog_error("Missing rear-right tire bone %s in model %s",
                    data->m_rearRightTireBoneName.Str(),
                    Get_Render_Object()->Get_Name());
            }
        }

        if (!data->m_midFrontLeftTireBoneName.Is_Empty()) {
            m_midFrontLeftTireBone = Get_Render_Object()->Get_Bone_Index(data->m_midFrontLeftTireBoneName.Str());
            if (m_midFrontLeftTireBone == 0) {
                captainslog_error("Missing mid-front-left tire bone %s in model %s",
                    data->m_midFrontLeftTireBoneName.Str(),
                    Get_Render_Object()->Get_Name());
            }

            m_midFrontRightTireBone = Get_Render_Object()->Get_Bone_Index(data->m_midFrontRightTireBoneName.Str());
            if (m_midFrontRightTireBone == 0) {
                captainslog_error("Missing mid-front-right tire bone %s in model %s",
                    data->m_midFrontRightTireBoneName.Str(),
                    Get_Render_Object()->Get_Name());
            }

            if (m_midFrontRightTireBone == 0) {
                m_midFrontLeftTireBone = 0;
            }
        }

        if (!data->m_midRearLeftTireBoneName.Is_Empty()) {
            m_midRearLeftTireBone = Get_Render_Object()->Get_Bone_Index(data->m_midRearLeftTireBoneName.Str());
            if (m_midRearLeftTireBone == 0) {
                captainslog_error("Missing mid-rear-left tire bone %s in model %s",
                    data->m_midRearLeftTireBoneName.Str(),
                    Get_Render_Object()->Get_Name());
            }

            m_midRearRightTireBone = Get_Render_Object()->Get_Bone_Index(data->m_midRearRightTireBoneName.Str());
            if (m_midRearRightTireBone == 0) {
                captainslog_error("Missing mid-rear-right tire bone %s in model %s",
                    data->m_midRearRightTireBoneName.Str(),
                    Get_Render_Object()->Get_Name());
            }

            if (m_midRearRightTireBone == 0) {
                m_midRearLeftTireBone = 0;
            }
        }

        if (!data->m_midMidLeftTireBoneName.Is_Empty()) {
            m_midMidLeftTireBone = Get_Render_Object()->Get_Bone_Index(data->m_midMidLeftTireBoneName.Str());
            if (m_midMidLeftTireBone == 0) {
                captainslog_error("Missing mid-mid-left tire bone %s in model %s",
                    data->m_midMidLeftTireBoneName.Str(),
                    Get_Render_Object()->Get_Name());
            }

            m_midMidRightTireBone = Get_Render_Object()->Get_Bone_Index(data->m_midMidRightTireBoneName.Str());
            if (m_midMidRightTireBone == 0) {
                captainslog_error("Missing mid-mid-right tire bone %s in model %s",
                    data->m_midMidRightTireBoneName.Str(),
                    Get_Render_Object()->Get_Name());
            }

            if (m_midMidRightTireBone == 0) {
                m_midMidLeftTireBone = 0;
            }
        }

        if (!data->m_cabBoneName.Is_Empty()) {
            m_cabBone = Get_Render_Object()->Get_Bone_Index(data->m_cabBoneName.Str());
            if (m_cabBone == 0) {
                captainslog_error(
                    "Missing cab bone %s in model %s", data->m_cabBoneName.Str(), Get_Render_Object()->Get_Name());
            }

            m_trailerBone = Get_Render_Object()->Get_Bone_Index(data->m_trailerBoneName.Str());
        }
    }

    m_prevRenderObj = Get_Render_Object();
    m_numBones = m_prevRenderObj->Get_Num_Bones();
}

ModuleData *W3DTruckDraw::Friend_New_Module_Data(INI *ini)
{
    W3DTruckDrawModuleData *data = new W3DTruckDrawModuleData();

    if (ini) {
        ini->Init_From_INI_Multi_Proc(data, W3DTruckDrawModuleData::Build_Field_Parse);
    }

    return data;
}

Module *W3DTruckDraw::Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data)
{
    return NEW_POOL_OBJ(W3DTruckDraw, thing, module_data);
}
