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
#include "matinfo.h"
#include "meshmatdesc.h"
#include "meshmdl.h"
#include "texture.h"
#include "vertmaterial.h"

MaterialInfoClass::MaterialInfoClass(const MaterialInfoClass &src)
{
    for (int m = 0; m < src.m_vertexMaterials.Count(); ++m) {
        VertexMaterialClass *vmat;
        vmat = src.m_vertexMaterials[m]->Clone();
        m_vertexMaterials.Add(vmat);
    }

    for (int t = 0; t < src.m_textures.Count(); ++t) {
        TextureClass *tex = src.m_textures[t];
        tex->Add_Ref();
        m_textures.Add(tex);
    }
}

MaterialInfoClass::~MaterialInfoClass()
{
    Free();
}

MaterialInfoClass *MaterialInfoClass::Clone() const
{
    return new MaterialInfoClass(*this);
}

int MaterialInfoClass::Add_Texture(TextureClass *tex)
{
    captainslog_assert(tex != NULL);

    tex->Add_Ref();
    int index = m_textures.Count();
    m_textures.Add(tex);
    return index;
}

int MaterialInfoClass::Get_Texture_Index(const char *name)
{
    for (int i = 0; i < m_textures.Count(); ++i) {
        if (strcasecmp(name, m_textures[i]->Get_Name()) == 0) {
            return i;
        }
    }
    return -1;
}

TextureClass *MaterialInfoClass::Get_Texture(int index)
{
    captainslog_assert(index >= 0);
    captainslog_assert(index < m_textures.Count());

    m_textures[index]->Add_Ref();
    return m_textures[index];
}

void MaterialInfoClass::Free()
{
    int i;

    for (i = 0; i < m_vertexMaterials.Count(); ++i) {
        Ref_Ptr_Release(m_vertexMaterials[i]);
    }
    m_vertexMaterials.Delete_All();

    for (i = 0; i < m_textures.Count(); ++i) {
        Ref_Ptr_Release(m_textures[i]);
    }
    m_textures.Delete_All();
}

MaterialRemapperClass::MaterialRemapperClass(MaterialInfoClass *src, MaterialInfoClass *dest) :
    m_textureCount(0),
    m_textureRemaps(nullptr),
    m_vertexMaterialCount(0),
    m_vertexMaterialRemaps(nullptr),
    m_lastSrcVmat(nullptr),
    m_lastDestVmat(nullptr),
    m_lastSrcTex(nullptr),
    m_lastDestTex(nullptr)
{
    captainslog_assert(src);
    captainslog_assert(dest);
    captainslog_assert(src->Texture_Count() == dest->Texture_Count());
    captainslog_assert(src->Vertex_Material_Count() == dest->Vertex_Material_Count());

    m_srcMatInfo = src;
    m_srcMatInfo->Add_Ref();
    m_destMatInfo = dest;
    m_destMatInfo->Add_Ref();

    if (src->Vertex_Material_Count() > 0) {
        m_vertexMaterialCount = src->Vertex_Material_Count();
        m_vertexMaterialRemaps = new VmatRemapStruct[m_vertexMaterialCount];

        for (int i = 0; i < m_vertexMaterialCount; ++i) {
            m_vertexMaterialRemaps[i].m_src = src->Peek_Vertex_Material(i);
            m_vertexMaterialRemaps[i].m_dest = dest->Peek_Vertex_Material(i);
        }
    }

    if (src->Texture_Count() > 0) {
        m_textureCount = src->Texture_Count();
        m_textureRemaps = new TextureRemapStruct[m_textureCount];

        for (int i = 0; i < m_textureCount; ++i) {
            m_textureRemaps[i].m_src = src->Peek_Texture(i);
            m_textureRemaps[i].m_dest = dest->Peek_Texture(i);
        }
    }
}

MaterialRemapperClass::~MaterialRemapperClass()
{
    m_srcMatInfo->Release_Ref();
    m_destMatInfo->Release_Ref();

    if (m_textureRemaps) {
        delete[] m_textureRemaps;
    }

    if (m_vertexMaterialRemaps) {
        delete[] m_vertexMaterialRemaps;
    }

    // these are weak pointers
    m_lastSrcVmat = nullptr;
    m_lastDestVmat = nullptr;
    m_lastSrcTex = nullptr;
    m_lastDestTex = nullptr;
}

TextureClass *MaterialRemapperClass::Remap_Texture(TextureClass *src)
{
    if (src == nullptr) {
        return src;
    }

    if (src == m_lastSrcTex) {
        return m_lastDestTex;
    }

    for (int i = 0; i < m_textureCount; ++i) {
        if (m_textureRemaps[i].m_src == src) {
            m_lastSrcTex = src;
            m_lastDestTex = m_textureRemaps[i].m_dest;
            return m_textureRemaps[i].m_dest;
        }
    }

    captainslog_assert(0);
    return nullptr;
}

VertexMaterialClass *MaterialRemapperClass::Remap_Vertex_Material(VertexMaterialClass *src)
{
    if (src == nullptr) {
        return src;
    }

    if (src == m_lastSrcVmat) {
        return m_lastDestVmat;
    }

    for (int i = 0; i < m_vertexMaterialCount; ++i) {
        if (m_vertexMaterialRemaps[i].m_src == src) {
            m_lastSrcVmat = src;
            m_lastDestVmat = m_vertexMaterialRemaps[i].m_dest;
            return m_vertexMaterialRemaps[i].m_dest;
        }
    }

    captainslog_assert(0);
    return nullptr;
}

void MaterialRemapperClass::Remap_Mesh(const MeshMatDescClass *src_desc, MeshMatDescClass *dest_desc)
{
    if (m_srcMatInfo->Vertex_Material_Count() >= 1) {
        for (int pass = 0; pass < src_desc->Get_Pass_Count(); ++pass) {
            if (src_desc->Has_Material_Array(pass)) {

                for (int vert_index = 0; vert_index < src_desc->Get_Vertex_Count(); ++vert_index) {
                    VertexMaterialClass *src = src_desc->Peek_Material(vert_index, pass);
                    dest_desc->Set_Material(vert_index, Remap_Vertex_Material(src), pass);
                }

            } else {
                VertexMaterialClass *src = src_desc->Peek_Single_Material(pass);
                dest_desc->Set_Single_Material(Remap_Vertex_Material(src), pass);
            }
        }
    }

    if (m_srcMatInfo->Texture_Count() >= 1) {
        for (int pass = 0; pass < src_desc->Get_Pass_Count(); ++pass) {

            for (int stage = 0; stage < MeshMatDescClass::MAX_TEX_STAGES; ++stage) {
                if (src_desc->Has_Texture_Array(pass, stage)) {

                    for (int poly_index = 0; poly_index < src_desc->Get_Polygon_Count(); ++poly_index) {
                        TextureClass *src = src_desc->Peek_Texture(poly_index, pass, stage);
                        dest_desc->Set_Texture(poly_index, Remap_Texture(src), pass, stage);
                    }

                } else {
                    TextureClass *src = src_desc->Peek_Single_Texture(pass, stage);
                    dest_desc->Set_Single_Texture(Remap_Texture(src), pass, stage);
                }
            }
        }
    }
}
