/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Laser Update
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "laserupdate.h"
#include "drawable.h"
#include "drawmodule.h"
#include "gameclient.h"
#include "gamelogic.h"
#include "particlesys.h"
#include "player.h"
#include "playerlist.h"
#include "w3dlaserdraw.h"

LaserUpdateModuleData::LaserUpdateModuleData() : m_punchThroughScalar(0.0f) {}

LaserUpdateModuleData::~LaserUpdateModuleData() {}

void LaserUpdateModuleData::Build_Field_Parse(MultiIniFieldParse &p)
{
    static const FieldParse dataFieldParse[] = {
        { "MuzzleParticleSystem",
            &INI::Parse_AsciiString,
            nullptr,
            offsetof(LaserUpdateModuleData, m_muzzleParticleSystem) },
        { "TargetParticleSystem",
            &INI::Parse_AsciiString,
            nullptr,
            offsetof(LaserUpdateModuleData, m_targetParticleSystem) },
        { "PunchThroughScalar", &INI::Parse_Real, nullptr, offsetof(LaserUpdateModuleData, m_punchThroughScalar) },
        { nullptr, nullptr, nullptr, 0 },
    };

    ModuleData::Build_Field_Parse(p);
    p.Add(dataFieldParse, 0);
}

LaserUpdate::LaserUpdate(Thing *thing, ModuleData const *module_data) :
    ClientUpdateModule(thing, module_data),
    m_sourceDrawableID(INVALID_DRAWABLE_ID),
    m_victimDrawableID(INVALID_DRAWABLE_ID),
    m_dirty(false),
    m_muzzleParticleSystemID(PARTSYS_ID_NONE),
    m_targetParticleSystemID(PARTSYS_ID_NONE),
    m_grow(false),
    m_shrink(false),
    m_growStartFrame(0),
    m_growEndFrame(0),
    m_width(1.0f),
    m_shrinkStartFrame(0),
    m_shrinkEndFrame(0)
{
    m_endPos.Zero();
    m_startPos.Zero();
}

LaserUpdate::~LaserUpdate()
{
    if (m_muzzleParticleSystemID) {
        g_theParticleSystemManager->Destroy_Particle_System_By_ID(m_muzzleParticleSystemID);
    }

    if (m_targetParticleSystemID) {
        g_theParticleSystemManager->Destroy_Particle_System_By_ID(m_targetParticleSystemID);
    }
}

NameKeyType LaserUpdate::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("LaserUpdate");
    return _key;
}

void LaserUpdate::Client_Update()
{
    Update_Start_Pos();
    Update_End_Pos();

    if (m_shrink) {
        m_width = 1.0f
            - (float)(g_theGameLogic->Get_Frame() - m_shrinkStartFrame) / (float)(m_shrinkEndFrame - m_shrinkStartFrame);
        m_dirty = true;

        if (m_width <= 0.0f) {
            m_width = 0.0f;
        }
    } else if (m_grow) {
        m_width = (float)(g_theGameLogic->Get_Frame() - m_growStartFrame) / (float)(m_growEndFrame - m_growStartFrame);
        m_dirty = true;

        if (m_width >= 1.0f) {
            m_width = 1.0f;
            m_grow = false;
        }
    }
}

void LaserUpdate::CRC_Snapshot(Xfer *xfer)
{
    DrawableModule::CRC_Snapshot(xfer);
}

void LaserUpdate::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    DrawableModule::Xfer_Snapshot(xfer);
    xfer->xferCoord3D(&m_startPos);
    xfer->xferCoord3D(&m_endPos);
    xfer->xferBool(&m_dirty);
    xfer->xferUser(&m_muzzleParticleSystemID, 4);
    xfer->xferUser(&m_targetParticleSystemID, 4);
    xfer->xferBool(&m_grow);
    xfer->xferBool(&m_shrink);
    xfer->xferUnsignedInt(&m_growStartFrame);
    xfer->xferUnsignedInt(&m_growEndFrame);
    xfer->xferReal(&m_width);
    xfer->xferUnsignedInt(&m_shrinkStartFrame);
    xfer->xferUnsignedInt(&m_shrinkEndFrame);
    xfer->xferDrawableID(&m_sourceDrawableID);
    xfer->xferDrawableID(&m_victimDrawableID);
    xfer->xferAsciiString(&m_masterBone);
}

void LaserUpdate::Load_Post_Process()
{
    DrawableModule::Load_Post_Process();
}

ModuleData *LaserUpdate::Friend_New_Module_Data(INI *ini)
{
    LaserUpdateModuleData *data = new LaserUpdateModuleData();

    if (ini) {
        ini->Init_From_INI_Multi_Proc(data, LaserUpdateModuleData::Build_Field_Parse);
    }

    return data;
}

Module *LaserUpdate::Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data)
{
    return NEW_POOL_OBJ(LaserUpdate, thing, module_data);
}

float LaserUpdate::Get_Current_Laser_Radius() const
{
    const Drawable *drawable = Get_Drawable();

    for (const DrawModule **m = drawable->Get_Draw_Modules(); *m != nullptr; m++) {
        const LaserDrawInterface *l = (*m)->Get_Laser_Draw_Interface();

        if (l) {
            return l->Get_Laser_Template_Width() * m_width;
        }
    }

    return 0.0f;
}

void LaserUpdate::Init_Laser(Object const *source_obj,
    Object const *victim_obj,
    Coord3D const *start_pos,
    Coord3D const *end_pos,
    Utf8String master_bone,
    int width)
{
    const LaserUpdateModuleData *data = Get_Laser_Update_Module_Data();

    if (width > 0) {
        m_grow = true;
        m_growStartFrame = g_theGameLogic->Get_Frame();
        m_growEndFrame = width + m_growStartFrame;
        m_width = 0.0f;
    } else if (width < 0) {
        m_shrink = true;
        m_shrinkStartFrame = g_theGameLogic->Get_Frame();
        m_shrinkEndFrame = width + m_shrinkStartFrame;
        m_width = 1.0f;
    }

    m_masterBone = master_bone;

    if (!source_obj) {
        if (!start_pos) {
            g_theGameClient->Destroy_Drawable(Get_Drawable());
            return;
        }

        m_startPos = *start_pos;
    } else {
        if (source_obj->Get_Drawable()) {
            m_sourceDrawableID = source_obj->Get_Drawable()->Get_ID();
        }

        Update_Start_Pos();
    }

    if (victim_obj && !end_pos) {
        if (victim_obj->Get_Drawable()) {
            m_victimDrawableID = victim_obj->Get_Drawable()->Get_ID();
        }

        m_endPos = *victim_obj->Get_Position();
    } else {
        if (!end_pos) {
            g_theGameClient->Destroy_Drawable(Get_Drawable());
            return;
        }

        m_endPos = *end_pos;
    }

    if (!m_muzzleParticleSystemID) {
        Player *player = g_thePlayerList->Get_Local_Player();

        if (!source_obj || source_obj->Get_Shrouded_Status(player->Get_Player_Index()) <= SHROUDED_TRANSITION) {
            if (data->m_muzzleParticleSystem.Is_Not_Empty()) {
                ParticleSystemTemplate *tmplate = g_theParticleSystemManager->Find_Template(data->m_muzzleParticleSystem);

                if (tmplate) {
                    ParticleSystem *system = g_theParticleSystemManager->Create_Particle_System(tmplate, true);

                    if (system) {
                        m_muzzleParticleSystemID = system->Get_System_ID();
                    }
                }
            }

            if (data->m_targetParticleSystem.Is_Not_Empty()) {
                ParticleSystemTemplate *tmplate = g_theParticleSystemManager->Find_Template(data->m_targetParticleSystem);

                if (tmplate) {
                    ParticleSystem *system = g_theParticleSystemManager->Create_Particle_System(tmplate, true);

                    if (system) {
                        m_targetParticleSystemID = system->Get_System_ID();
                    }
                }
            }
        }
    }

    if (m_muzzleParticleSystemID) {
        ParticleSystem *system = g_theParticleSystemManager->Find_Particle_System(m_muzzleParticleSystemID);

        if (system) {
            system->Set_Position(m_startPos);
        }
    }

    if (m_targetParticleSystemID) {
        ParticleSystem *system = g_theParticleSystemManager->Find_Particle_System(m_targetParticleSystemID);

        if (system) {
            system->Set_Position(m_endPos);
        }
    }

    Coord3D pos;

    if (source_obj) {
        pos = *source_obj->Get_Position();
    } else {
        pos.Set(start_pos);
        pos.Add(end_pos);
        pos.Scale(0.5f);
    }

    Drawable *drawable = Get_Drawable();

    if (drawable) {
        drawable->Set_Position(&pos);
    }

    m_dirty = true;
}

void LaserUpdate::Update_Start_Pos()
{
    Coord3D oldpos = m_startPos;

    if (m_sourceDrawableID) {
        Drawable *drawable = g_theGameClient->Find_Drawable_By_ID(m_sourceDrawableID);

        if (drawable) {
            if (m_masterBone.Is_Not_Empty()) {
                Matrix3D m;

                if (!drawable->Get_Current_Worldspace_Client_Bone_Positions(m_masterBone.Str(), m)) {
                    const Coord3D *pos = drawable->Get_Position();
                    m_startPos.Set(pos);
                    captainslog_debug("LaserUpdate::updateStartPos() -- Drawable %s is expecting to find a bone %s but "
                                      "can't. Defaulting to position of drawable.",
                        drawable->Get_Template()->Get_Name().Str(),
                        m_masterBone.Str());
                    return;
                }

                m_startPos.x = m.Get_X_Translation();
                m_startPos.y = m.Get_Y_Translation();
                m_startPos.z = m.Get_Z_Translation();
            } else {
                m_startPos = *drawable->Get_Position();
            }

            if (!(m_startPos == oldpos)) {
                m_dirty = true;
            }
        }
    }
}

void LaserUpdate::Update_End_Pos()
{
    const LaserUpdateModuleData *data = Get_Laser_Update_Module_Data();
    Coord3D oldpos = m_endPos;

    if (m_victimDrawableID) {
        Drawable *drawable = g_theGameClient->Find_Drawable_By_ID(m_victimDrawableID);
        bool dead;

        if (drawable && drawable->Get_Object()) {
            dead = drawable->Get_Object()->Is_Effectively_Dead();
        } else {
            dead = false;
        }

        if (!drawable || dead) {
            if (data->m_punchThroughScalar > 0.0f) {
                Vector3 pos;
                pos.Set(m_endPos.x, m_endPos.y, m_endPos.z);
                pos = pos - Vector3(m_startPos.x, m_startPos.y, m_startPos.z);
                pos *= data->m_punchThroughScalar;
                pos = pos + Vector3(m_startPos.x, m_startPos.y, m_startPos.z);
                m_endPos.x = pos.X;
                m_endPos.y = pos.Y;
                m_endPos.z = pos.Z;
            }

            m_victimDrawableID = INVALID_DRAWABLE_ID;
        } else {
            m_endPos = *drawable->Get_Position();
        }

        if (!(m_startPos == oldpos)) {
            m_dirty = true;
        }
    }
}

void LaserUpdate::Set_Decay_Frames(unsigned int frames)
{
    if (frames != 0) {
        m_shrink = true;
        m_shrinkStartFrame = g_theGameLogic->Get_Frame();
        m_shrinkEndFrame = frames + m_shrinkStartFrame;
        m_width = 1.0f;
    }
}
