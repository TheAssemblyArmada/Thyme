/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Waypoint Drawing Code
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dwaypointbuffer.h"
#include "aiupdate.h"
#include "assetmgr.h"
#include "camera.h"
#include "gameclient.h"
#include "geometry.h"
#include "ingameui.h"
#include "lightenv.h"
#include "object.h"
#include "opencontain.h"
#include "rendobj.h"
#include "rinfo.h"
#include "segline.h"
#include "w3d.h"

W3DWaypointBuffer::W3DWaypointBuffer()
{
    m_renderObj = W3DAssetManager::Get_Instance()->Create_Render_Obj("SCMNode");
    m_segLine = new SegmentedLineClass();
    m_texture = W3DAssetManager::Get_Instance()->Get_Texture("EXLaser.tga");
    Set_Default_Line_Style();
}

W3DWaypointBuffer::~W3DWaypointBuffer()
{
    Ref_Ptr_Release(m_renderObj);
    Ref_Ptr_Release(m_segLine);
    Ref_Ptr_Release(m_texture);
}

void W3DWaypointBuffer::Free_Waypoint_Buffers() {}

void W3DWaypointBuffer::Set_Default_Line_Style()
{
    if (m_texture != nullptr) {
        m_segLine->Set_Texture(m_texture);
    }

    ShaderClass shader(ShaderClass::s_presetAdditiveShader);
    shader.Set_Depth_Compare(ShaderClass::PASS_ALWAYS);
    m_segLine->Set_Shader(shader);
    m_segLine->Set_Width(1.5f);
    m_segLine->Set_Color(Vector3(0.25f, 0.5f, 1.0f));
    m_segLine->Set_Texture_Mapping_Mode(SegLineRendererClass::TILED_TEXTURE_MAP);
}

void W3DWaypointBuffer::Draw_Waypoints(RenderInfoClass &rinfo)
{
    if (g_theInGameUI != nullptr) {
        Set_Default_Line_Style();

        if (g_theInGameUI->Is_In_Waypoint_Mode()) {
            LightEnvironmentClass lightenv;
            lightenv.Reset(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f));
            lightenv.Pre_Render_Update(rinfo.m_camera.Get_Transform());
            RenderInfoClass newinfo(rinfo.m_camera);
            newinfo.m_lightEnvironment = &lightenv;
            Vector3 points[513];
            auto list = g_theInGameUI->Get_All_Selected_Drawables();

            for (auto it = list->begin(); it != list->end(); it++) {
                Object *obj = (*it)->Get_Object();
                int count = 1;

                if (obj != nullptr && !obj->Is_KindOf(KINDOF_IGNORED_IN_GUI)) {
                    const AIUpdateInterface *update = obj->Get_AI_Update_Interface();
                    int size = update != nullptr ? update->Friend_Get_Waypoint_Goal_Path_Size() : 0;
                    int index = update != nullptr ? update->Get_Current_Goal_Path_Index() : 0;

                    if (update != nullptr && index >= 0 && index < size) {
                        const Coord3D *pos = obj->Get_Position();
                        points[0] = Vector3(pos->x, pos->y, pos->z);

                        for (int i = index; i < size; i++) {
                            const Coord3D *goal = update->Get_Goal_Path_Position(i);

                            if (goal != nullptr) {
                                if (count < 513) {
                                    points[count] = Vector3(goal->x, goal->y, goal->z);
                                    count++;
                                }

                                m_renderObj->Set_Position(Vector3(goal->x, goal->y, goal->z));
                                W3D::Render(*m_renderObj, newinfo);
                            }
                        }

                        m_segLine->Set_Points(count, points);
                        m_segLine->Render(rinfo);
                    }
                }
            }
        } else {
            LightEnvironmentClass lightenv;
            lightenv.Reset(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f));
            lightenv.Pre_Render_Update(rinfo.m_camera.Get_Transform());
            RenderInfoClass newinfo(rinfo.m_camera);
            newinfo.m_lightEnvironment = &lightenv;
            Vector3 points[513];
            auto list = g_theInGameUI->Get_All_Selected_Drawables();

            for (auto it = list->begin(); it != list->end(); it++) {
                Object *obj = (*it)->Get_Object();
                int count = 0;

                if (obj != nullptr && obj->Is_Locally_Controlled()) {
                    if (obj->Is_KindOf(KINDOF_REVEALS_ENEMY_PATHS)) {
                        Drawable *drawable =
                            g_theGameClient->Find_Drawable_By_ID(g_theInGameUI->Get_Moused_Over_Drawable_ID());

                        if (drawable != nullptr) {
                            Object *obj2 = drawable->Get_Object();

                            if (obj2 != nullptr) {
                                if (obj2->Get_Relationship(obj) == ENEMIES) {
                                    Coord3D pos = *obj->Get_Position();
                                    pos.Sub(obj2->Get_Position());

                                    if (obj->Get_Vision_Range() >= pos.Length()) {
                                        const AIUpdateInterface *update = obj->Get_AI_Update_Interface();
                                        int size = update != nullptr ? update->Friend_Get_Waypoint_Goal_Path_Size() : 0;
                                        int index = update != nullptr ? update->Get_Current_Goal_Path_Index() : 0;

                                        if (update != nullptr) {
                                            bool has_points = false;

                                            const Coord3D *pos2 = obj2->Get_Position();
                                            points[count] = Vector3(pos2->x, pos2->y, pos2->z);
                                            count++;

                                            if (index >= 0 && index < size) {
                                                for (int i = index; i < size; i++) {
                                                    const Coord3D *goal = update->Get_Goal_Path_Position(i);

                                                    if (goal != nullptr) {
                                                        if (count < 513) {
                                                            points[count] = Vector3(goal->x, goal->y, goal->z);
                                                            count++;
                                                        }

                                                        m_renderObj->Set_Position(Vector3(goal->x, goal->y, goal->z));
                                                        W3D::Render(*m_renderObj, newinfo);
                                                        has_points = true;
                                                    }
                                                }
                                            } else {
                                                const Coord3D *goal = update->Get_Goal_Position();

                                                if (goal->Length() > 1.0f) {
                                                    points[count] = Vector3(goal->x, goal->y, goal->z);
                                                    count++;
                                                    m_renderObj->Set_Position(Vector3(goal->x, goal->y, goal->z));
                                                    W3D::Render(*m_renderObj, newinfo);
                                                    has_points = true;
                                                }
                                            }

                                            if (has_points) {
                                                m_segLine->Set_Color(Vector3(0.95f, 0.5f, 0.0f));
                                                m_segLine->Set_Width(3.0f);
                                                m_segLine->Set_Points(count, points);
                                                m_segLine->Render(newinfo);
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        break;
                    }

                    ExitInterface *exit = obj->Get_Object_Exit_Interface();

                    if (exit != nullptr) {
                        Coord3D pos;

                        if (!exit->Get_Exit_Position(pos)) {
                            pos = *obj->Get_Position();
                        }

                        points[count] = Vector3(pos.x, pos.y, pos.z);
                        count++;
                        bool add_point = true;
                        Coord3D rally;

                        if (exit->Get_Natural_Rally_Point(rally, false)) {
                            if (rally != pos) {
                                points[count] = Vector3(rally.x, rally.y, rally.z);
                                count++;
                            } else {
                                add_point = false;
                            }

                            const Coord3D *rally2 = exit->Get_Rally_Point();

                            if (rally2 != nullptr) {
                                if (add_point) {
                                    const GeometryInfo &info = obj->Get_Geometry_Info();
                                    const Coord3D *pos2 = obj->Get_Position();
                                    Coord3D pos3;
                                    pos3.x = rally.x - pos.x;
                                    pos3.y = rally.y - pos.y;
                                    pos3.z = 0.0f;
                                    pos3.Normalize();
                                    pos3.Scale(100000.0f);
                                    float length = pos3.Length();
                                    pos3.Add(&rally);
                                    Coord3D pos4 = pos3;
                                    pos4.Sub(rally2);
                                    float length2 = pos4.Length();

                                    if (length2 + 100.0f > length) {
                                        pos3.Normalize();
                                        Coord3D pos5 = rally;
                                        pos5.Sub(rally2);
                                        pos5.Normalize();

                                        if (pos5.x * pos3.x + pos5.y * pos3.y > 0.0f) {
                                            float orientation = obj->Get_Orientation();
                                            float cos = GameMath::Cos(orientation);
                                            float sin = GameMath::Sin(orientation);
                                            float f1 = info.Get_Major_Radius() * cos;
                                            float f2 = info.Get_Minor_Radius() * cos;
                                            float f3 = info.Get_Major_Radius() * sin;
                                            float f4 = info.Get_Minor_Radius() * sin;

                                            Coord2D coords[4];
                                            coords[0].x = pos2->x - f1 - f4;
                                            coords[0].y = f2 + pos2->y - f3;
                                            coords[1].x = f1 + pos2->x - f4;
                                            coords[1].y = f2 + pos2->y + f3;
                                            coords[2].x = f1 + pos2->x + f4;
                                            coords[2].y = pos2->y - f2 + f3;
                                            coords[3].x = pos2->x - f1 + f4;
                                            coords[3].y = pos2->y - f2 - f3;

                                            Coord2D *f6 = nullptr;
                                            Coord2D *f7 = nullptr;
                                            Coord2D *f8 = nullptr;
                                            Coord3D pos6;
                                            pos6.z = 0.0f;
                                            Coord3D pos7;
                                            pos7.z = 0.0f;
                                            float f9 = 100000.0f;
                                            float f10 = 100000.0f;

                                            for (int j = 0; j < 4; j++) {
                                                f8 = &coords[j];
                                                pos7.x = pos.x - f8->x;
                                                pos7.y = pos.y - f8->y;
                                                pos7.Normalize();

                                                if (pos7.x * pos3.x + pos7.y * pos3.y < 0.0f) {
                                                    pos6.x = rally2->x - f8->x;
                                                    pos6.y = rally2->y - f8->y;

                                                    if (pos6.Length() < f9) {
                                                        f9 = pos6.Length();
                                                        f6 = f8;
                                                    }
                                                } else {
                                                    pos6.x = rally2->x - f8->x;
                                                    pos6.y = rally2->y - f8->y;
                                                    if (pos6.Length() < f10) {
                                                        f10 = pos6.Length();
                                                        f7 = f8;
                                                    }
                                                }
                                            }

                                            if (f6 != nullptr) {
                                                m_renderObj->Set_Position(Vector3(f6->x, f6->y, pos2->z));
                                                W3D::Render(*m_renderObj, newinfo);
                                                points[count] = Vector3(f6->x, f6->y, pos2->z);
                                                count++;

                                                if (f7 != nullptr) {
                                                    Coord3D pos8;
                                                    pos8.x = rally.x - f6->x;
                                                    pos8.y = rally.y - f6->y;
                                                    pos8.z = 0.0f;
                                                    pos8.Normalize();

                                                    Coord3D pos9;
                                                    pos9.x = f6->x - rally2->x;
                                                    pos9.y = f6->y - rally2->y;
                                                    pos9.z = 0.0f;
                                                    pos9.Normalize();

                                                    if (pos9.x * pos8.x + pos9.y * pos8.y < 0.0f) {
                                                        m_renderObj->Set_Position(Vector3(f7->x, f7->y, pos2->z));
                                                        W3D::Render(*m_renderObj, newinfo);
                                                        points[count] = Vector3(f7->x, f7->y, pos2->z);
                                                        count++;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                                points[count] = Vector3(rally2->x, rally2->y, rally2->z);
                                count++;
                                m_renderObj->Set_Position(Vector3(rally.x, rally.y, rally.z));
                                W3D::Render(*m_renderObj, newinfo);
                                m_segLine->Set_Points(count, points);
                                m_segLine->Render(newinfo);
                            }
                        }
                    }
                }
            }
        }
    }
}
