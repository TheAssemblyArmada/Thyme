/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Class for drawing small debug icons on screen.
 * Commonly used for debugging things like pathfinding, trajectories, flight paths.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3ddebugicons.h"
#include "gamelogic.h"
#include "globaldata.h"
#include "vertmaterial.h"
#include <new>

#ifdef BUILD_WITH_D3D8
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#endif

DebugIcon *W3DDebugIcons::s_debugIcons = nullptr;
int W3DDebugIcons::s_numDebugIcons = 0;
int W3DDebugIcons::s_numUnknown = 0;

W3DDebugIcons::W3DDebugIcons() : m_vertexMaterial(VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE))
{
    Allocate_Icons_Array();
}

W3DDebugIcons::~W3DDebugIcons()
{
    Ref_Ptr_Release(m_vertexMaterial);

    if (s_debugIcons != nullptr) {
        delete[] s_debugIcons;
        s_debugIcons = nullptr;
    }

    s_numDebugIcons = 0;
    s_numUnknown = 0;
}

void W3DDebugIcons::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    Vector3 pos(g_theWriteableGlobalData->m_waterExtentX, g_theWriteableGlobalData->m_waterExtentY, 500.0f);
    sphere.Init(pos, pos.Length());
}

void W3DDebugIcons::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    Vector3 c(-2.0f * g_theWriteableGlobalData->m_waterExtentX, -2.0f * g_theWriteableGlobalData->m_waterExtentY, 0.0f);
    Vector3 e(2.0f * g_theWriteableGlobalData->m_waterExtentX, 2.0f * g_theWriteableGlobalData->m_waterExtentY, 1000.0f);

    box.Init(c, e);
}

RenderObjClass *W3DDebugIcons::Clone() const
{
    captainslog_warn("Cannot clone W3DDebugIcons!");
    return nullptr;
}

void W3DDebugIcons::Render(RenderInfoClass &rinfo)
{
#if BUILD_WITH_D3D8
    if (W3D::Are_Static_Sort_Lists_Enabled()) {
        W3D::Add_To_Static_Sort_List(this, 1);
    } else {
        bool any_ended = false;

        if (s_numDebugIcons) {
            DX8Wrapper::Apply_Render_State_Changes();

            DX8Wrapper::Set_Material(m_vertexMaterial);

            DX8Wrapper::Set_Texture(0, nullptr);

            DX8Wrapper::Apply_Render_State_Changes();

            Matrix3D m(m_transform);
            DX8Wrapper::Set_Transform(D3DTS_WORLD, m);

            int num_icons = std::min(s_numDebugIcons, 5000);

            int index = 0;

            while (index < s_numDebugIcons) {
                int index_count = 0;
                int vertex_count = 0;

                DynamicVBAccessClass vb_access(VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, DX8_FVF_XYZNDUV2, 4 * num_icons);

                DynamicIBAccessClass ib_access(IndexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, 6 * num_icons);

                {
                    DynamicVBAccessClass::WriteLockClass lock_vb(&vb_access);
                    VertexFormatXYZNDUV2 *verts = lock_vb.Get_Formatted_Vertex_Array();

                    // BUGFIX check verts before fetching indices
                    if (verts == nullptr) {
                        return;
                    }

                    DynamicIBAccessClass::WriteLockClass lock_ib(&ib_access);

                    unsigned short *indices = lock_ib.Get_Index_Array();

                    // BUGFIX check indices too
                    if (indices == nullptr) {
                        return;
                    }

                    while (vertex_count < 4 * num_icons && index < s_numDebugIcons) {

                        DebugIcon *icon = &s_debugIcons[index];

                        int alpha = 64;

                        int fade_start = 100;

                        int frame = (int)g_theGameLogic->Get_Frame();
                        int frame_delta = icon->endFrame - frame;

                        if (frame_delta < 1) {
                            any_ended = true;
                        } else {

                            if (frame_delta < fade_start) {
                                alpha = (int)((float)frame_delta / (float)fade_start * (float)alpha);
                            }

                            RGBColor color = icon->color;

                            float offset = icon->width / 2.0f;

                            int diffuse = (alpha << 24) | color.Get_As_Int();

                            float x = icon->position.x;
                            float y = icon->position.y;
                            float z = icon->position.z;

                            verts->x = x - offset;
                            verts->y = y - offset;
                            verts->z = z;
                            verts->diffuse = diffuse;
                            verts->u1 = 0.0f;
                            verts->v1 = 0.0f;
                            ++verts;

                            verts->x = x + offset;
                            verts->y = y - offset;
                            verts->z = z;
                            verts->diffuse = diffuse;
                            verts->u1 = 0.0f;
                            verts->v1 = 0.0f;
                            ++verts;

                            verts->x = x + offset;
                            verts->y = y + offset;
                            verts->z = z;
                            verts->diffuse = diffuse;
                            verts->u1 = 0.0f;
                            verts->v1 = 0.0f;
                            ++verts;

                            verts->x = x - offset;
                            verts->y = y + offset;
                            verts->z = z;
                            verts->diffuse = diffuse;
                            verts->u1 = 0.0f;
                            verts->v1 = 0.0f;
                            ++verts;

                            *indices++ = unsigned short(vertex_count + 0);
                            *indices++ = unsigned short(vertex_count + 1);
                            *indices++ = unsigned short(vertex_count + 2);

                            *indices++ = unsigned short(vertex_count + 0);
                            *indices++ = unsigned short(vertex_count + 2);
                            *indices++ = unsigned short(vertex_count + 3);

                            index_count += 6;

                            vertex_count += 4;
                        }
                        ++index;
                    }
                }

                if (vertex_count == 0) {
                    break;
                }

                // TODO resolve shader
                DX8Wrapper::Set_Shader(ShaderClass(0x984B7));

                DX8Wrapper::Set_Index_Buffer(ib_access, 0);
                DX8Wrapper::Set_Vertex_Buffer(vb_access);

                DX8Wrapper::Draw_Triangles(0, index_count / 3, 0, vertex_count);
            }

            if (any_ended) {
                Compress_Icons_Array();
            }
        }
    }
#endif
}

void W3DDebugIcons::Add_Icon(Coord3D *pos, float width, int num_frames_duration, RGBColor color)
{
    if (pos == nullptr) {
        if (s_numDebugIcons > s_numUnknown) {
            captainslog_warn("Max icons %d", s_numDebugIcons);
            s_numUnknown = s_numDebugIcons;
        }
        s_numDebugIcons = 0;

    } else if (s_numDebugIcons < MAX_ICONS) {

        if (s_debugIcons != nullptr) {
            s_debugIcons[s_numDebugIcons].position = *pos;
            s_debugIcons[s_numDebugIcons].width = width;
            s_debugIcons[s_numDebugIcons].color = color;
            s_debugIcons[s_numDebugIcons++].endFrame = num_frames_duration + g_theGameLogic->Get_Frame();
        }
    }
}

void W3DDebugIcons::Allocate_Icons_Array()
{
    s_debugIcons = new DebugIcon[MAX_ICONS];
    s_numDebugIcons = 0;
    s_numUnknown = 0;
}

void W3DDebugIcons::Compress_Icons_Array()
{
    if (s_debugIcons != nullptr) {
        if (s_numDebugIcons > 0) {

            int j = 0;

            for (int i = 0; i < s_numDebugIcons; ++i) {

                if (static_cast<unsigned int>(s_debugIcons[i].endFrame) >= g_theGameLogic->Get_Frame() && i > j) {
                    s_debugIcons[j++] = s_debugIcons[i];
                }
            }

            s_numDebugIcons = j;
        }
    }
}

void Add_Icon(Coord3D *pos, float width, int num_frames_duration, RGBColor color)
{
    W3DDebugIcons::Add_Icon(pos, width, num_frames_duration, color);
}
