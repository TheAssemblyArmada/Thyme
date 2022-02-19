/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Polygon Renderer
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
#include "dx8wrapper.h"
#include "multilist.h"
#include "sortingrenderer.h"
#include "sphere.h"
class MeshModelClass;
class DX8TextureCategoryClass;

class DX8PolygonRendererClass : public MultiListObjectClass
{
public:
    DX8PolygonRendererClass(uint32_t index_count,
        MeshModelClass *mmc,
        DX8TextureCategoryClass *tex_cat,
        uint32_t vertex_offset,
        uint32_t index_offset,
        bool strip,
        uint32_t pass);
    DX8PolygonRendererClass(const DX8PolygonRendererClass &src, MeshModelClass *mmc);
    virtual ~DX8PolygonRendererClass();

    void Render(int32_t offset);
    void Render_Sorted(int32_t offset, const SphereClass &sphere);

    void Set_Vertex_Index_Range(uint32_t min, uint32_t max);
    void Set_Texture_Category(DX8TextureCategoryClass *category) { m_textureCategory = category; }

    uint32_t Get_Vertex_Offset() { return m_vertexOffset; }
    uint32_t Get_Index_Offset() { return m_indexOffset; }
    MeshModelClass *Get_Mesh_Model_Class() { return m_mmc; }
    DX8TextureCategoryClass *Get_Texture_Category() { return m_textureCategory; }
    uint32_t Get_Pass() { return m_pass; }

    void Log();

private:
    MeshModelClass *m_mmc;
    DX8TextureCategoryClass *m_textureCategory;
    uint32_t m_indexOffset;
    uint32_t m_vertexOffset;
    uint32_t m_indexCount;
    uint32_t m_minVertexIndex;
    uint32_t m_vertexIndexRange;
    bool m_strip;
    uint32_t m_pass;
};

inline void DX8PolygonRendererClass::Render(int32_t offset)
{
    DX8Wrapper::Set_Index_Buffer_Index_Offset(offset);

    if (m_strip) {
        DX8Wrapper::Draw_Strip((unsigned short)m_indexOffset,
            (unsigned short)m_indexCount - 2,
            (unsigned short)m_minVertexIndex,
            (unsigned short)m_vertexIndexRange);
    } else {
        DX8Wrapper::Draw_Triangles((unsigned short)m_indexOffset,
            (unsigned short)m_indexCount / 3,
            (unsigned short)m_minVertexIndex,
            (unsigned short)m_vertexIndexRange);
    }
}

inline void DX8PolygonRendererClass::Render_Sorted(int32_t offset, const SphereClass &sphere)
{
    DX8Wrapper::Set_Index_Buffer_Index_Offset(offset);

    SortingRendererClass::Insert_Triangles(sphere,
        (unsigned short)m_indexOffset,
        (unsigned short)(m_indexCount / 3),
        (unsigned short)m_minVertexIndex,
        (unsigned short)m_vertexIndexRange);
}
inline void DX8PolygonRendererClass::Set_Vertex_Index_Range(uint32_t min, uint32_t max)
{
    m_minVertexIndex = min;
    m_vertexIndexRange = max;
}
