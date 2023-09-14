/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Particle System
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dparticlesys.h"
#include "camera.h"
#include "globaldata.h"
#include "heightmap.h"
#include "particle.h"
#include "particlesys.h"
#include "pointgr.h"
#include "randomvalue.h"
#include "rinfo.h"
#include "streak.h"
#include "w3dassetmanager.h"
#include "w3ddisplay.h"
#include "w3dsmudge.h"
#include "w3dsnow.h"

W3DParticleSystemManager::W3DParticleSystemManager() :
    m_pointGroup(nullptr),
    m_streakLine(nullptr),
    m_posBuffer(nullptr),
    m_RGBABuffer(nullptr),
    m_sizeBuffer(nullptr),
    m_angleBuffer(nullptr),
    m_readyToRender(false)
{
    m_onScreenParticleCount = 0;
    m_pointGroup = new PointGroupClass();
    m_streakLine = new StreakLineClass();
    m_posBuffer = new ShareBufferClass<Vector3>(MAX_POINTS_PER_GROUP);
    m_RGBABuffer = new ShareBufferClass<Vector4>(MAX_POINTS_PER_GROUP);
    m_sizeBuffer = new ShareBufferClass<float>(MAX_POINTS_PER_GROUP);
    m_angleBuffer = new ShareBufferClass<unsigned char>(MAX_POINTS_PER_GROUP);
}

W3DParticleSystemManager::~W3DParticleSystemManager()
{
    if (m_pointGroup != nullptr) {
        delete m_pointGroup;
    }

    if (m_streakLine != nullptr) {
        delete m_streakLine;
        m_streakLine = nullptr;
    }

    if (m_posBuffer != nullptr) {
        delete m_posBuffer;
        m_posBuffer = nullptr;
    }

    if (m_RGBABuffer != nullptr) {
        delete m_RGBABuffer;
        m_RGBABuffer = nullptr;
    }

    if (m_sizeBuffer != nullptr) {
        delete m_sizeBuffer;
        m_sizeBuffer = nullptr;
    }

    if (m_angleBuffer != nullptr) {
        delete m_angleBuffer;
        m_angleBuffer = nullptr;
    }
}

int W3DParticleSystemManager::Get_On_Screen_Particle_Count()
{
    return m_onScreenParticleCount;
}

void W3DParticleSystemManager::Do_Particles(RenderInfoClass &rinfo)
{
    if (m_readyToRender) {
        m_readyToRender = false;
        m_onScreenParticleCount = 0;
        int hazecount = 0;

        if (g_theSmudgeManager != nullptr) {
            g_theSmudgeManager->Set_Heat_Haze_Count(0);
        }

        AABoxClass bbox;
        g_theTerrainRenderObject->Get_Maximum_Visible_Box(rinfo.m_camera.Get_Frustum(), &bbox, true);
        float bcx = bbox.m_center.X;
        float bcy = bbox.m_center.Y;
        float bcz = bbox.m_center.Z;
        float bex = bbox.m_extent.X;
        float bey = bbox.m_extent.Y;
        float bez = bbox.m_extent.Z;
        SmudgeSet *set = nullptr;

        if (g_theSmudgeManager != nullptr) {
            set = g_theSmudgeManager->Add_Smudge_Set();
        }

        std::list<ParticleSystem *> particlesyslist = g_theParticleSystemManager->Get_All_Particle_Systems();

        for (auto it = particlesyslist.begin(); it != particlesyslist.end(); it++) {
            ParticleSystem *sys = *it;

            if (sys != nullptr && !sys->Is_Drawable()) {
                if (*reinterpret_cast<const unsigned int *>(sys->Get_Particle_Type_Name().Str()) == 0x44554D53) {
                    if (g_theSmudgeManager != nullptr
                        && g_theSmudgeManager->Get_Hardware_Test_Result() != SmudgeManager::HARDWARE_TEST_NOT_RUN
                        && g_theWriteableGlobalData->m_useHeatEffects) {
                        for (Particle *particle = sys->Get_First_Particle(); particle != nullptr;
                             particle = particle->m_systemNext) {
                            const Coord3D *pos = particle->Get_Position();
                            float size = particle->Get_Size();

                            if (bex + size >= GameMath::Fabs(pos->x - bcx)) {
                                if (bey + size >= GameMath::Fabs(pos->y - bcy)) {
                                    if (bez + size >= GameMath::Fabs(pos->z - bcz)) {
                                        Smudge *smudge = set->Add_Smudge_To_Set();
                                        smudge->m_smudgePos.Set(pos->x, pos->y, pos->z);
                                        smudge->m_smudgeRandom.Set(Get_Client_Random_Value_Real(-0.03f, 0.03f),
                                            Get_Client_Random_Value_Real(-0.06f, 0.06f));
                                        smudge->m_smudgeSize = size;
                                        smudge->m_smudgeAlpha = particle->Get_Alpha();
                                        hazecount++;
                                    }
                                }
                            }
                        }
                    }
                } else {
                    int count = 0;
                    Vector3 *posarray = m_posBuffer->Get_Array();
                    float *sizearray = m_sizeBuffer->Get_Array();
                    Vector4 *colorarray = m_RGBABuffer->Get_Array();
                    unsigned char *anglearray = m_angleBuffer->Get_Array();
                    unsigned int idarray[MAX_POINTS_PER_GROUP];

                    for (Particle *particle = sys->Get_First_Particle(); particle != nullptr;
                         particle = particle->m_systemNext) {
                        const Coord3D *pos = particle->Get_Position();
                        float size = particle->Get_Size();

                        if (bex + size >= GameMath::Fabs(pos->x - bcx)) {
                            if (bey + size >= GameMath::Fabs(pos->y - bcy)) {
                                if (bez + size >= GameMath::Fabs(pos->z - bcz)) {
                                    m_fieldParticleCount +=
                                        sys->Get_Priority() == PARTICLE_PRIORITY_AREA_EFFECT && sys->Is_Ground_Aligned();
                                    idarray[count] = particle->Get_ID();
                                    posarray[count].X = pos->x;
                                    posarray[count].Y = pos->y;
                                    posarray[count].Z = pos->z;
                                    sizearray[count] = size;
                                    const RGBColor *color = particle->Get_Color();
                                    colorarray[count].X = color->red;
                                    colorarray[count].Y = color->green;
                                    colorarray[count].Z = color->blue;
                                    colorarray[count].W = particle->Get_Alpha();
                                    anglearray[count] = particle->Get_Angle() * 255.0f / DEG_TO_RADF(360.0f);
                                    count++;

                                    if (count == 512) {
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    if (count != 0) {
                        TextureClass *texture = W3DDisplay::s_assetManager->Get_Texture(sys->Get_Particle_Type_Name().Str());

                        if (m_streakLine != nullptr && sys->Is_Streak() && count >= 2) {
                            m_streakLine->Reset_Line();
                            m_streakLine->Set_Texture(texture);
                            texture->Release_Ref();

                            switch (sys->Get_Shader_Type()) {
                                case ParticleSystem::PARTICLE_SHADER_ADDITIVE:
                                    m_streakLine->Set_Shader(ShaderClass::s_presetAdditiveSpriteShader);
                                    break;
                                case ParticleSystem::PARTICLE_SHADER_ALPHA:
                                    m_streakLine->Set_Shader(ShaderClass::s_presetAlphaSpriteShader);
                                    break;
                                case ParticleSystem::PARTICLE_SHADER_ALPHA_TEST:
                                    m_streakLine->Set_Shader(ShaderClass::s_presetATestSpriteShader);
                                    break;
                                case ParticleSystem::PARTICLE_SHADER_MULTIPLY:
                                    m_streakLine->Set_Shader(ShaderClass::s_presetMultiplicativeSpriteShader);
                                    break;
                                default:
                                    break;
                            }

                            m_streakLine->Set_Locs_Widths_Colors(count,
                                m_posBuffer->Get_Array(),
                                m_sizeBuffer->Get_Array(),
                                m_RGBABuffer->Get_Array(),
                                idarray);
                            colorarray->X = 0.0f;
                            colorarray->Y = 0.0f;
                            colorarray->Z = 0.0f;
                            colorarray->W = 0.0f;
                            m_streakLine->Render(rinfo);
                        } else {
                            captainslog_assert(m_pointGroup != nullptr);

                            if (m_pointGroup != nullptr) {
                                m_pointGroup->Set_Texture(texture);
                                texture->Release_Ref();
                                m_pointGroup->Set_Flag(PointGroupClass::TRANSFORM, true);

                                switch (sys->Get_Shader_Type()) {
                                    case ParticleSystem::PARTICLE_SHADER_ADDITIVE:
                                        m_pointGroup->Set_Shader(ShaderClass::s_presetAdditiveSpriteShader);
                                        break;
                                    case ParticleSystem::PARTICLE_SHADER_ALPHA:
                                        m_pointGroup->Set_Shader(ShaderClass::s_presetAlphaSpriteShader);
                                        break;
                                    case ParticleSystem::PARTICLE_SHADER_ALPHA_TEST:
                                        m_pointGroup->Set_Shader(ShaderClass::s_presetATestSpriteShader);
                                        break;
                                    case ParticleSystem::PARTICLE_SHADER_MULTIPLY:
                                        m_pointGroup->Set_Shader(ShaderClass::s_presetMultiplicativeSpriteShader);
                                        break;
                                    default:
                                        break;
                                }

                                m_pointGroup->Set_Point_Mode(PointGroupClass::QUADS);
                                m_pointGroup->Set_Arrays(
                                    m_posBuffer, m_RGBABuffer, nullptr, m_sizeBuffer, m_angleBuffer, nullptr, count);
                                m_pointGroup->Set_Billboard(sys->Is_Not_Ground_Aligned());
                                m_pointGroup->Set_Point_Frame(0);

                                if (sys->Is_Volume_Particle() <= 1) {
                                    m_pointGroup->Render(rinfo);
                                } else {
                                    m_pointGroup->Render_Volume_Particle(rinfo, sys->Is_Volume_Particle());
                                }
                            }
                        }

                        m_onScreenParticleCount += count;
                    }
                }
            }
        }

        g_theParticleSystemManager->Set_On_Screen_Particle_Count(m_onScreenParticleCount);

        if (g_theSnowManager != nullptr) {
            static_cast<W3DSnowManager *>(g_theSnowManager)->Render(rinfo);
        }

        if (g_theSmudgeManager != nullptr) {
            g_theSmudgeManager->Render(rinfo);
            g_theSmudgeManager->Reset();
            g_theSmudgeManager->Set_Heat_Haze_Count(hazecount);
        }
    }
}

void W3DParticleSystemManager::Queue_Particle_Render()
{
    m_readyToRender = true;
}

void Do_Particles(RenderInfoClass &rinfo)
{
    if (g_theParticleSystemManager) {
        g_theParticleSystemManager->Do_Particles(rinfo);
    }
}
