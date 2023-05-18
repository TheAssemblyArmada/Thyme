/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D status circle
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dstatuscircle.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include "gamelogic.h"
#include "globaldata.h"
#include "scriptengine.h"
#include "vertmaterial.h"

int W3DStatusCircle::g_diffuse = 0xFF;
bool W3DStatusCircle::g_needUpdate;

W3DStatusCircle::W3DStatusCircle() :
    // BUGFIX: Init m_numTriangles
    m_numTriangles(0),
    m_indexBuffer(nullptr),
    m_vertexMaterialClass(nullptr),
    m_vertexBufferCircle(nullptr),
    m_vertexBufferScreen(nullptr)
{
}

W3DStatusCircle::W3DStatusCircle(const W3DStatusCircle &src)
{
    *this = src;
}

W3DStatusCircle &W3DStatusCircle::operator=(const W3DStatusCircle &that)
{
    return *this;
}

W3DStatusCircle::~W3DStatusCircle()
{
    Free_Map_Resources();
}

bool W3DStatusCircle::Cast_Ray(RayCollisionTestClass &raytest)
{
    return false;
}

void W3DStatusCircle::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    Vector3 v(500.0f, 500.0f, 0.0f);
    sphere.Init(v, v.Length());
}

void W3DStatusCircle::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    box.Init(Vector3(0.0f, 0.0f, 0.0f), Vector3(1000.0f, 1000.0f, 1000.0f));
}

int W3DStatusCircle::Class_ID() const
{
    return CLASSID_UNKNOWN;
}

RenderObjClass *W3DStatusCircle::Clone() const
{
    return new W3DStatusCircle(*this);
}

int W3DStatusCircle::Free_Map_Resources()
{
    Ref_Ptr_Release(m_indexBuffer);
    Ref_Ptr_Release(m_vertexBufferScreen);
    Ref_Ptr_Release(m_vertexBufferCircle);
    Ref_Ptr_Release(m_vertexMaterialClass);
    return 0;
}

int W3DStatusCircle::Init_Data()
{
    g_needUpdate = true;
    Free_Map_Resources();
    m_numTriangles = 20;
    m_indexBuffer = new DX8IndexBufferClass(3 * m_numTriangles, DX8IndexBufferClass::USAGE_DEFAULT);
    IndexBufferClass::WriteLockClass lock(m_indexBuffer, 0);
    unsigned short *indices = lock.Get_Index_Array();

    for (int i = 0; i < 3 * m_numTriangles; i += 3) {
        indices[0] = i;
        indices[1] = i + 1;
        indices[2] = i + 2;
        indices += 3;
    }

    m_vertexBufferCircle =
        new DX8VertexBufferClass(DX8_FVF_XYZDUV1, 3 * m_numTriangles, DX8VertexBufferClass::USAGE_DEFAULT, 0);
    m_vertexBufferScreen = new DX8VertexBufferClass(DX8_FVF_XYZDUV1, 6, DX8VertexBufferClass::USAGE_DEFAULT, 0);
    m_vertexMaterialClass = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);

    // TODO resolve this
    m_shaderClass = ShaderClass(0x984B7);
    return 0;
}

int W3DStatusCircle::Update_Circle_VB()
{
    if (!m_vertexBufferCircle) {
        return -1;
    }

    g_needUpdate = false;
    VertexBufferClass::WriteLockClass lock(m_vertexBufferCircle, 0);
    VertexFormatXYZDUV1 *vertices = (VertexFormatXYZDUV1 *)lock.Get_Vertex_Array();
    float z = 0.0f;
    float x = 0.02f;
    int diffuse = g_diffuse + 0x7F000000;
    int count = m_numTriangles;
    float angle = 0.0f;
    float delta_angle = (GAMEMATH_PI * 2) / (float)count;

    for (int i = 0; i < count; i++) {
        for (int j = 0; j < 3; j++) {
            vertices->z = z;

            switch (j) {
                case 0: {
                    vertices->x = 0.0f;
                    vertices->y = 0.0f;
                    break;
                }
                case 1: {
                    Vector3 vec(x, 0.0f, z);
                    vec.Rotate_Z(angle);
                    vertices->x = vec.X;
                    vertices->y = vec.Y;
                    break;
                }
                case 2: {
                    float f = angle + delta_angle;
                    if (i == count - 1) {
                        f = 0.0f;
                    }
                    Vector3 vec(x, 0.0f, z);
                    vec.Rotate_Z(f);
                    vertices->x = vec.X;
                    vertices->y = vec.Y;
                    break;
                }
            }

            vertices->diffuse = diffuse;
            vertices->u1 = 0.0f;
            vertices->v1 = 0.0f;
            vertices++;
        }

        angle += delta_angle;
    }

    return 0;
}

int W3DStatusCircle::Update_Screen_VB(int diffuse)
{
    if (!m_vertexBufferScreen) {
        return -1;
    }

    g_needUpdate = false;
    VertexBufferClass::WriteLockClass lock(m_vertexBufferScreen, 0);
    VertexFormatXYZDUV1 *vertices = (VertexFormatXYZDUV1 *)lock.Get_Vertex_Array();

    vertices->x = -1.0f;
    vertices->y = -1.0f;
    vertices->z = 0.0f;
    vertices->diffuse = diffuse;
    vertices->u1 = 0.0f;
    vertices->v1 = 0.0f;
    vertices++;

    vertices->x = 1.0f;
    vertices->y = 1.0f;
    vertices->z = 0.0f;
    vertices->diffuse = diffuse;
    vertices->u1 = 0.0f;
    vertices->v1 = 0.0f;
    vertices++;

    vertices->x = -1.0f;
    vertices->y = 1.0f;
    vertices->z = 0.0f;
    vertices->diffuse = diffuse;
    vertices->u1 = 0.0f;
    vertices->v1 = 0.0f;
    vertices++;

    vertices->x = -1.0f;
    vertices->y = -1.0f;
    vertices->z = 0.0f;
    vertices->diffuse = diffuse;
    vertices->u1 = 0.0f;
    vertices->v1 = 0.0f;
    vertices++;

    vertices->x = 1.0f;
    vertices->y = -1.0f;
    vertices->z = 0.0f;
    vertices->diffuse = diffuse;
    vertices->u1 = 0.0f;
    vertices->v1 = 0.0f;
    vertices++;

    vertices->x = 1.0f;
    vertices->y = 1.0f;
    vertices->z = 0.0f;
    vertices->diffuse = diffuse;
    vertices->u1 = 0.0f;
    vertices->v1 = 0.0f;
    vertices++;

    return 0;
}

void W3DStatusCircle::Render(RenderInfoClass &rinfo)
{
#ifdef BUILD_WITH_D3D8
    if (g_theGameLogic->Is_In_Game() && g_theGameLogic->Get_Game_Mode() != GAME_SHELL) {
        if (!m_indexBuffer) {
            Init_Data();
        }

        if (m_indexBuffer) {
            bool b = false;
            Matrix3D m(true);

            if (g_theWriteableGlobalData->m_showTeamDot) {
                if (g_needUpdate) {
                    Update_Circle_VB();
                }

                DX8Wrapper::Set_Material(m_vertexMaterialClass);
                DX8Wrapper::Set_Shader(m_shaderClass);
                DX8Wrapper::Set_Texture(0, nullptr);
                DX8Wrapper::Set_Index_Buffer(m_indexBuffer, 0);
                DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferCircle, 0);
                b = true;
                m.Set_Translation(Vector3(0.95f, 0.67f, 0.0f));
                DX8Wrapper::Set_Transform(D3DTS_WORLD, m);
                DX8Wrapper::Draw_Triangles(0, 20, 0, 3 * m_numTriangles);
            }

            ScriptEngine::TFade fade = g_theScriptEngine->Get_Fade();

            if (fade) {
                if (!b) {
                    DX8Wrapper::Set_Material(m_vertexMaterialClass);
                    DX8Wrapper::Set_Index_Buffer(m_indexBuffer, 0);
                    DX8Wrapper::Set_Texture(0, nullptr);
                }

                m.Make_Identity();
                int fadevalue = (int)(g_theScriptEngine->Get_Fade_Value() * 255.0f);
                int diffuse = fadevalue | (fadevalue << 8) | (fadevalue << 0x10) | 0xFF000000;
                Update_Screen_VB(diffuse);

                DX8Wrapper::Set_Transform(D3DTS_WORLD, m);

                // TODO resolve this
                DX8Wrapper::Set_Shader(ShaderClass(0x94037));
                DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferScreen, 0);
                DX8Wrapper::Apply_Render_State_Changes();

                if (fade == ScriptEngine::FADE_SUBTRACT) {
                    DX8Wrapper::Set_DX8_Render_State(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
                    DX8Wrapper::Draw_Triangles(0, 2, 0, 6);
                    DX8Wrapper::Set_DX8_Render_State(D3DRS_BLENDOP, D3DBLENDOP_ADD);
                } else if (fade == ScriptEngine::FADE_SATURATE) {
                    DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
                    DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
                    DX8Wrapper::Draw_Triangles(0, 2, 0, 6);
                    DX8Wrapper::Draw_Triangles(0, 2, 0, 6);
                } else {
                    if (fade == ScriptEngine::FADE_MULTIPLY) {
                        DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_ZERO);
                        DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
                    }

                    DX8Wrapper::Draw_Triangles(0, 2, 0, 6);
                }

                ShaderClass::Invalidate();
            }
        }
    }
#endif
}
