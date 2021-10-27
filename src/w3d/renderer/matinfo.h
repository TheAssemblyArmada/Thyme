/**
 * @file
 *
 * @author tomsons26
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once
#include "always.h"
#include "shader.h"
#include "texture.h"
#include "vector.h"
#include "vertmaterial.h"
#include "w3dmpo.h"

class TextureClass;
class VertexMaterialClass;
class MeshMatDescClass;

class MaterialInfoClass : public W3DMPO, public RefCountClass
{
    IMPLEMENT_W3D_POOL(MaterialInfoClass)
public:
    MaterialInfoClass() {}
    ~MaterialInfoClass();

    MaterialInfoClass(const MaterialInfoClass &src);

    void Reset() { Free(); }
    MaterialInfoClass *Clone() const;
    void Free();

    int Add_Vertex_Material(VertexMaterialClass *vmat);
    int Add_Texture(TextureClass *tex);

    int Get_Vertex_Material_Index(const char *name);
    int Get_Texture_Index(const char *name);

    VertexMaterialClass *Get_Vertex_Material(int index);
    VertexMaterialClass *Get_Vertex_Material(const char *name);

    TextureClass *Get_Texture(int index);
    TextureClass *Get_Texture(const char *name);

    int Vertex_Material_Count() const { return m_vertexMaterials.Count(); }
    int Texture_Count() const { return m_textures.Count(); }

    VertexMaterialClass *Peek_Vertex_Material(int index);
    VertexMaterialClass *Peek_Vertex_Material(const char *name);

    TextureClass *Peek_Texture(int index);

    void Replace_Material(int index, VertexMaterialClass *newMaterial);
    void Replace_Texture(int index, TextureClass *newTexture);

    void Reset_Texture_Mappers();
    void Make_Vertex_Materials_Unique();
    bool Has_Time_Variant_Texture_Mappers();

private:
    DynamicVectorClass<VertexMaterialClass *> m_vertexMaterials;
    DynamicVectorClass<TextureClass *> m_textures;
};

class MaterialRemapperClass
{
public:
    MaterialRemapperClass(MaterialInfoClass *src, MaterialInfoClass *dest);
    ~MaterialRemapperClass();

    TextureClass *Remap_Texture(TextureClass *src);
    VertexMaterialClass *Remap_Vertex_Material(VertexMaterialClass *src);
    void Remap_Mesh(const MeshMatDescClass *src_desc, MeshMatDescClass *dest_desc);

private:
    struct VmatRemapStruct
    {
        VertexMaterialClass *m_src;
        VertexMaterialClass *m_dest;
    };

    struct TextureRemapStruct
    {
        TextureClass *m_src;
        TextureClass *m_dest;
    };

    MaterialInfoClass *m_srcMatInfo;
    MaterialInfoClass *m_destMatInfo;
    int m_textureCount;
    TextureRemapStruct *m_textureRemaps;
    int m_vertexMaterialCount;
    VmatRemapStruct *m_vertexMaterialRemaps;
    VertexMaterialClass *m_lastSrcVmat;
    VertexMaterialClass *m_lastDestVmat;
    TextureClass *m_lastSrcTex;
    TextureClass *m_lastDestTex;
};

inline VertexMaterialClass *MaterialInfoClass::Peek_Vertex_Material(int index)
{
    captainslog_assert(index >= 0);
    captainslog_assert(index < m_vertexMaterials.Count());

    return m_vertexMaterials[index];
}

inline TextureClass *MaterialInfoClass::Peek_Texture(int index)
{
    captainslog_assert(index >= 0);
    captainslog_assert(index < m_textures.Count());

    return m_textures[index];
}

inline int MaterialInfoClass::Add_Vertex_Material(VertexMaterialClass *vmat)
{
    if (vmat != nullptr) {
        vmat->Add_Ref();
    }

    int index = m_vertexMaterials.Count();
    m_vertexMaterials.Add(vmat);
    return index;
}

inline int MaterialInfoClass::Get_Vertex_Material_Index(const char *name)
{
    for (int i = 0; i < m_vertexMaterials.Count(); i++) {
        if (strcasecmp(name, m_vertexMaterials[i]->Get_Name()) == 0) {
            return i;
        }
    }

    return -1;
}

inline VertexMaterialClass *MaterialInfoClass::Get_Vertex_Material(int index)
{
    captainslog_assert(index >= 0);
    captainslog_assert(index < m_vertexMaterials.Count());

    if (m_vertexMaterials[index]) {
        m_vertexMaterials[index]->Add_Ref();
    }

    return m_vertexMaterials[index];
}

inline VertexMaterialClass *MaterialInfoClass::Get_Vertex_Material(const char *name)
{
    int index = Get_Vertex_Material_Index(name);
    if (index == -1) {
        return nullptr;
    } else {
        return Get_Vertex_Material(index);
    }
}

inline VertexMaterialClass *MaterialInfoClass::Peek_Vertex_Material(const char *name)
{
    int index = Get_Vertex_Material_Index(name);
    if (index == -1) {
        return nullptr;
    } else {
        return Peek_Vertex_Material(index);
    }
}

inline void MaterialInfoClass::Replace_Material(int index, VertexMaterialClass *newMaterial)
{
    Ref_Ptr_Set(m_vertexMaterials[index], newMaterial);
}

inline void MaterialInfoClass::Reset_Texture_Mappers(void)
{
    int vmat_count = m_vertexMaterials.Count();
    for (int i = 0; i < vmat_count; i++) {
        m_vertexMaterials[i]->Reset_Mappers();
    }
}

inline bool MaterialInfoClass::Has_Time_Variant_Texture_Mappers(void)
{
    int vmat_count = m_vertexMaterials.Count();
    for (int i = 0; i < vmat_count; i++) {
        if (m_vertexMaterials[i]->Are_Mappers_Time_Variant())
            return true;
    }
    return false;
}

inline void MaterialInfoClass::Make_Vertex_Materials_Unique(void)
{
    int vmat_count = m_vertexMaterials.Count();
    for (int i = 0; i < vmat_count; i++) {
        m_vertexMaterials[i]->Make_Unique();
    }
}

inline TextureClass *MaterialInfoClass::Get_Texture(const char *name)
{
    int index = Get_Texture_Index(name);
    if (index == -1) {
        return nullptr;
    } else {
        return Get_Texture(index);
    }
}

inline void MaterialInfoClass::Replace_Texture(int index, TextureClass *newTexture)
{
    Ref_Ptr_Set(m_textures[index], newTexture);
}
