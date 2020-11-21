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
#include "refcount.h"
#include "vector.h"
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

    MaterialInfoClass *Clone() const;
    void Free();

    int Add_Texture(TextureClass *tex);

    int Get_Texture_Index(const char *name) const;
    TextureClass *Get_Texture(int index);

    int Vertex_Material_Count() const { return m_vertexMaterials.Count(); }
    int Texture_Count() const { return m_textures.Count(); }

    VertexMaterialClass *Peek_Vertex_Material(int index);
    TextureClass *Peek_Texture(int index);

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
