/**
 * @file
 *
 * @author Jonathan Wilson
 * @author tomsons26
 *
 * @brief Mesh Mat Desc Class
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "meshmatdesc.h"
#include "refcount.h"
#include "texture.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

MatBufferClass::MatBufferClass(const MatBufferClass &that) : ShareBufferClass<VertexMaterialClass *>(that)
{
    for (int i = 0; i < m_count; ++i) {
        if (m_array[i]) {
            m_array[i]->Add_Ref();
        }
    }
}

MatBufferClass::~MatBufferClass()
{
    for (int i = 0; i < m_count; ++i) {
        Ref_Ptr_Release(m_array[i]);
    }
}

TexBufferClass::TexBufferClass(const TexBufferClass &that) : ShareBufferClass<TextureClass *>(that)
{
    for (int i = 0; i < m_count; ++i) {
        if (m_array[i]) {
            m_array[i]->Add_Ref();
        }
    }
}

TexBufferClass::~TexBufferClass(void)
{
    for (int i = 0; i < m_count; ++i) {
        Ref_Ptr_Release(m_array[i]);
    }
}

void TexBufferClass::Set_Element(int index, TextureClass *tex)
{
    Ref_Ptr_Set(m_array[index], tex);
}

TextureClass *TexBufferClass::Get_Element(int index)
{
    if (m_array[index]) {
        m_array[index]->Add_Ref();
    }
    return m_array[index];
}

TextureClass *TexBufferClass::Peek_Element(int index)
{
    return m_array[index];
}

void MatBufferClass::Set_Element(int index, VertexMaterialClass *mat)
{
    Ref_Ptr_Set(m_array[index], mat);
}

VertexMaterialClass *MatBufferClass::Get_Element(int index)
{
    if (m_array[index]) {
        m_array[index]->Add_Ref();
    }
    return m_array[index];
}

VertexMaterialClass *MatBufferClass::Peek_Element(int index)
{
    return m_array[index];
}

ShaderClass MeshMatDescClass::Get_Shader(int pidx, int pass) const
{
    if (m_shaderArray[pass]) {
        return m_shaderArray[pass]->Get_Element(pidx);
    }

    return m_shader[pass];
}

VertexMaterialClass *MeshMatDescClass::Peek_Material(int vidx, int pass) const
{
    if (m_materialArray[pass]) {
        return m_materialArray[pass]->Peek_Element(vidx);
    }

    return m_material[pass];
}

TextureClass *MeshMatDescClass::Peek_Texture(int pidx, int pass, int stage) const
{
    if (m_textureArray[pass][stage]) {
        return m_textureArray[pass][stage]->Peek_Element(pidx);
    }

    return m_texture[pass][stage];
}

void MeshMatDescClass::Set_Single_Material(VertexMaterialClass *vmat, int pass)
{
    Ref_Ptr_Set(m_material[pass], vmat);
}

void MeshMatDescClass::Set_Single_Texture(TextureClass *tex, int pass, int stage)
{
    Ref_Ptr_Set(m_texture[pass][stage], tex);
}

void MeshMatDescClass::Set_Material(int vidx, VertexMaterialClass *vmat, int pass)
{
    MatBufferClass *mats = Get_Material_Array(pass, true);
    mats->Set_Element(vidx, vmat);
}

void MeshMatDescClass::Set_Texture(int pidx, TextureClass *tex, int pass, int stage)
{
    TexBufferClass *textures = Get_Texture_Array(pass, stage, true);
    textures->Set_Element(pidx, tex);
}

TexBufferClass *MeshMatDescClass::Get_Texture_Array(int pass, int stage, bool create)
{
    if (create && m_textureArray[pass][stage] == NULL) {
        m_textureArray[pass][stage] = new TexBufferClass(m_polyCount);
    }
    return m_textureArray[pass][stage];
}

MatBufferClass *MeshMatDescClass::Get_Material_Array(int pass, bool create)
{
    if (create && m_materialArray[pass] == NULL) {
        m_materialArray[pass] = new MatBufferClass(m_vertexCount);
    }
    return m_materialArray[pass];
}
