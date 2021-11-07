/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Line 3D Object
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "line3d.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"

unsigned int s_indices[36] = {
    3, 5, 1, 7, 5, 3, 1, 5, 0, 5, 4, 0, 4, 2, 0, 4, 6, 2, 7, 3, 2, 6, 7, 2, 7, 6, 5, 5, 6, 4, 2, 3, 1, 2, 1, 0
};

void Line3DClass::Render(RenderInfoClass &rinfo)
{
#ifdef BUILD_WITH_D3D8
    if (!Is_Not_Hidden_At_All()) {
        return;
    }

    if (W3D::Are_Static_Sort_Lists_Enabled() && Get_Sort_Level()) {
        W3D::Add_To_Static_Sort_List(this, Get_Sort_Level());
    } else {
        DX8Wrapper::Set_Shader(m_shader);

        for (int i = 0; i < MAX_TEXTURE_STAGES; ++i) {
            DX8Wrapper::Set_Texture(i, nullptr);
        }

        VertexMaterialClass *material = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
        DX8Wrapper::Set_Material(material);
        material->Release_Ref();

        DX8Wrapper::Set_Transform(D3DTS_WORLD, Matrix4(m_transform));

        DynamicVBAccessClass vb(2, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2, 8);
        {
            DynamicVBAccessClass::WriteLockClass lock(&vb);
            unsigned int color = DX8Wrapper::Convert_Color(m_color);
            VertexFormatXYZNDUV2 *vertices = lock.Get_Formatted_Vertex_Array();

            for (int i = 0; i < 8; ++i) {
                vertices[i].x = m_vertices[i].X;
                vertices[i].y = m_vertices[i].Y;
                vertices[i].z = m_vertices[i].Z;
                vertices[i].diffuse = color;
            }
        }

        DynamicIBAccessClass ib(2, 36);
        {
            DynamicIBAccessClass::WriteLockClass lock(&ib);
            unsigned short *indices = lock.Get_Index_Array();

            for (int i = 0; i < 36; ++i) {
                indices[i] = s_indices[i];
            }
        }

        DX8Wrapper::Set_Vertex_Buffer(vb);
        DX8Wrapper::Set_Index_Buffer(ib, 0);
        DX8Wrapper::Draw_Triangles(0, 12, 0, 8);
    }
#endif
}

void Line3DClass::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    sphere.Init(Vector3(m_length * 0.5f, 0, 0), m_length * 0.5f);
}

void Line3DClass::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    box.Init(Vector3(m_length * 0.5f, 0, 0), Vector3(m_length * 0.5f, 0, 0));
}

void Line3DClass::Scale(float scale)
{
    for (int i = 0; i < 8; ++i) {
        m_vertices[i] *= scale;
    }

    m_length *= scale;
    m_thickness *= scale;

    Invalidate_Cached_Bounding_Volumes();
    RenderObjClass *container = Get_Container();

    if (container) {
        container->Update_Obj_Space_Bounding_Volumes();
    }
}

void Line3DClass::Scale(float scaleX, float scaleY, float scaleZ)
{
    for (int i = 0; i < 8; ++i) {
        m_vertices[i].X *= scaleX;
        m_vertices[i].Y *= scaleY;
        m_vertices[i].Z *= scaleZ;
    }

    m_length *= scaleX;
    m_thickness *= scaleY;

    Invalidate_Cached_Bounding_Volumes();
    RenderObjClass *container = Get_Container();

    if (container) {
        container->Update_Obj_Space_Bounding_Volumes();
    }
}

Line3DClass::Line3DClass(const Vector3 &start, const Vector3 &end, float width, float r, float g, float b, float opacity)
{
    m_length = (end - start).Length();
    m_thickness = width;

    float f = width * 0.5f;
    m_vertices[0] = Vector3(0, -f, -f);
    m_vertices[1] = Vector3(0, f, -f);
    m_vertices[2] = Vector3(0, -f, f);
    m_vertices[3] = Vector3(0, f, f);
    m_vertices[4] = Vector3(m_length, -f, -f);
    m_vertices[5] = Vector3(m_length, f, -f);
    m_vertices[6] = Vector3(m_length, -f, f);
    m_vertices[7] = Vector3(m_length, f, f);

    m_color.X = r;
    m_color.Y = g;
    m_color.Z = b;
    Set_Opacity(opacity);

    Matrix3D transform(true);
    transform.Obj_Look_At(start, end, 0.0f);
    Set_Transform(transform);
}

Line3DClass::Line3DClass(const Line3DClass &src) :
    RenderObjClass(src), m_length(src.m_length), m_thickness(src.m_thickness), m_shader(src.m_shader), m_color(src.m_color)
{
    for (int i = 0; i < 8; i++) {
        m_vertices[i] = src.m_vertices[i];
    }
}

void Line3DClass::Reset(const Vector3 &new_start, const Vector3 &new_end)
{
    float length = (new_end - new_start).Length();

    if (length == 0.0f) {
        length = 0.001f;
    }

    Scale(length / m_length, 1.0f, 1.0f);
    m_length = length;

    Matrix3D m(true);
    m.Obj_Look_At(new_start, new_end, 0.0f);
    Set_Transform(m);

    Invalidate_Cached_Bounding_Volumes();
    RenderObjClass *container = Get_Container();

    if (container) {
        container->Update_Obj_Space_Bounding_Volumes();
    }
}

void Line3DClass::Reset(const Vector3 &new_start, const Vector3 &new_end, float new_width)
{
    float length = (new_end - new_start).Length();

    if (length == 0.0f) {
        length = 0.001f;
    }

    Scale(length / m_length, new_width / m_thickness, new_width / m_thickness);
    m_thickness = new_width;
    m_length = length;

    Matrix3D m(true);
    m.Obj_Look_At(new_start, new_end, 0.0f);
    Set_Transform(m);

    Invalidate_Cached_Bounding_Volumes();
    RenderObjClass *container = Get_Container();

    if (container) {
        container->Update_Obj_Space_Bounding_Volumes();
    }
}

void Line3DClass::Re_Color(float r, float g, float b)
{
    m_color = Vector4(r, g, b, m_color.W);
}

void Line3DClass::Set_Opacity(float opacity)
{
    if (opacity >= 1.0f) {
        m_shader = ShaderClass::s_presetOpaqueSolidShader;
        Set_Sort_Level(0);
    } else {
        m_shader = ShaderClass::s_presetAlphaSolidShader;
        Set_Sort_Level(1);
    }

    m_color.W = opacity;
}
