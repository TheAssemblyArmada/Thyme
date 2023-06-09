/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Bib buffer
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dbibbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include "globaldata.h"
#include "texture.h"

// BUGFIX added extra variable initializations.
W3DBib::W3DBib() :
    m_highlighted(false), m_unknown(0), m_objectID(INVALID_OBJECT_ID), m_drawableID(INVALID_DRAWABLE_ID), m_isUnused(true)
{
    m_points[0].Set(0.0f, 0.0f, 0.0f);
    m_points[1].Set(0.0f, 0.0f, 0.0f);
    m_points[2].Set(0.0f, 0.0f, 0.0f);
    m_points[3].Set(0.0f, 0.0f, 0.0f);
}

W3DBibBuffer::W3DBibBuffer() :
    m_vertexBib(nullptr),
    m_maxBibVertex(0),
    m_indexBib(nullptr),
    m_maxBibIndex(0),
    m_bibTexture(nullptr),
    m_redBibTexture(nullptr),
    m_numBibVertices(0),
    m_numBibIndices(0),
    m_firstIndex(0),
    m_firstVertex(0),
    m_numBibs(0),
    m_anythingChanged(false),
    m_updateVis(false),
    m_initialized(false)
{
    Clear_All_Bibs();

    m_maxBibIndex = 384;
    m_maxBibVertex = 256;

    Allocate_Bib_Buffers();

    m_bibTexture = new TextureClass("TBBib.tga", nullptr, MIP_LEVELS_ALL, WW3D_FORMAT_UNKNOWN, true, true);
    m_redBibTexture = new TextureClass("TBRedBib.tga", nullptr, MIP_LEVELS_ALL, WW3D_FORMAT_UNKNOWN, true, true);

    m_bibTexture->Get_Texture_Filter()->Set_U_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
    m_bibTexture->Get_Texture_Filter()->Set_V_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
    m_redBibTexture->Get_Texture_Filter()->Set_U_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
    m_redBibTexture->Get_Texture_Filter()->Set_V_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);

    m_initialized = true;
}

W3DBibBuffer::~W3DBibBuffer()
{
    Free_Bib_Buffers();

    Ref_Ptr_Release(m_bibTexture);
    Ref_Ptr_Release(m_redBibTexture);
}

void W3DBibBuffer::Load_Bibs_In_Vertex_And_Index_Buffers()
{
    if (m_indexBib != nullptr && m_vertexBib != nullptr) {
        if (m_initialized && m_anythingChanged) {
            m_numBibVertices = 0;
            m_numBibIndices = 0;
            m_firstIndex = 0;
            m_firstVertex = 0;

            if (m_numBibs > 0) {
                IndexBufferClass::WriteLockClass index_lock(m_indexBib, 8192);

                VertexBufferClass::WriteLockClass vertex_lock(m_vertexBib, 8192);

                VertexFormatXYZDUV1 *vertices = (VertexFormatXYZDUV1 *)vertex_lock.Get_Vertex_Array();
                unsigned short *indices = index_lock.Get_Index_Array();

                float red = g_theWriteableGlobalData->m_terrainAmbient[0].red;
                float grn = g_theWriteableGlobalData->m_terrainAmbient[0].green;
                float blu = g_theWriteableGlobalData->m_terrainAmbient[0].blue;
                red = red + g_theWriteableGlobalData->m_terrainDiffuse[0].red;
                grn = grn + g_theWriteableGlobalData->m_terrainDiffuse[0].green;
                blu = blu + g_theWriteableGlobalData->m_terrainDiffuse[0].blue;

                if (red > 1.0f) {
                    red = 1.0f;
                }

                if (grn > 1.0f) {
                    grn = 1.0f;
                }

                if (blu > 1.0f) {
                    blu = 1.0f;
                }

                red = red * 255.0f;
                grn = grn * 255.0f;
                blu = blu * 255.0f;

                unsigned int color = (GameMath::Fast_To_Int_Truncate(red) << 16) | (GameMath::Fast_To_Int_Truncate(grn) << 8)
                    | GameMath::Fast_To_Int_Truncate(blu) | 0xFF000000;

                for (int i = 0; i <= 1; ++i) {

                    if (i == 1) {
                        m_firstIndex = m_numBibIndices;
                        m_firstVertex = m_numBibVertices;
                    }

                    for (int j = 0; j < m_numBibs; ++j) {
                        if (!m_bibs[j].m_isUnused && m_bibs[j].m_highlighted == (i != 0)) {

                            int index = m_numBibVertices;

                            if (m_numBibVertices + 6 >= m_maxBibVertex) {
                                break;
                            }

                            if (m_numBibIndices + 12 >= m_maxBibIndex) {
                                break;
                            }

                            for (int k = 0; k < ARRAY_SIZE(W3DBib::m_points); ++k) {
                                float v = 0.0f;
                                float u = 0.0f;
                                Vector3 point(m_bibs[j].m_points[k]);

                                switch (k) {
                                    case 0:
                                        u = 0.0f;
                                        v = 1.0f;
                                        break;
                                    case 1:
                                        u = 1.0f;
                                        v = 1.0f;
                                        break;
                                    case 2:
                                        u = 1.0f;
                                        v = 0.0f;
                                        break;
                                    case 3:
                                        u = 0.0f;
                                        v = 0.0f;
                                        break;
                                    default:
                                        break;
                                }

                                vertices->u1 = u;
                                vertices->v1 = v;
                                vertices->x = point.X;
                                vertices->y = point.Y;
                                vertices->z = point.Z;
                                vertices->diffuse = color;

                                ++vertices;
                                ++m_numBibVertices;
                            }

                            *indices = index + 0;
                            ++indices;
                            *indices = index + 1;
                            ++indices;
                            *indices = index + 2;
                            ++indices;
                            *indices = index + 0;
                            ++indices;
                            *indices = index + 2;
                            ++indices;
                            *indices = index + 3;
                            ++indices;

                            m_numBibIndices += 6;
                        }
                    }
                }
                // incriment_IndexBufferTotalIndices();
            }
        }
    }
}

void W3DBibBuffer::Free_Bib_Buffers()
{
    Ref_Ptr_Release(m_vertexBib);
    Ref_Ptr_Release(m_indexBib);
}

void W3DBibBuffer::Allocate_Bib_Buffers()
{
    m_vertexBib = new DX8VertexBufferClass(0x142, m_maxBibVertex + 4, DX8VertexBufferClass::USAGE_DYNAMIC, 0);
    m_indexBib = new DX8IndexBufferClass(m_maxBibIndex + 4, DX8IndexBufferClass::USAGE_DYNAMIC);
    m_numBibVertices = 0;
    m_numBibIndices = 0;
}

void W3DBibBuffer::Clear_All_Bibs()
{
    m_numBibs = 0;
    m_anythingChanged = true;
}

void W3DBibBuffer::Remove_Highlighting()
{
    for (int i = 0; i < m_numBibs; ++i) {
        m_bibs[i].m_highlighted = false;
    }
}

void W3DBibBuffer::Add_Bib_To_Object(Vector3 *points, ObjectID id, bool highlighted)
{
    int i;

    for (i = 0; i < m_numBibs && (m_bibs[i].m_isUnused || m_bibs[i].m_objectID != id); ++i) {
        ;
    }

    if (i == m_numBibs) {
        for (i = 0; i < m_numBibs && !m_bibs[i].m_isUnused; ++i) {
            ;
        }
    }

    if (i == m_numBibs) {
        if (m_numBibs >= MAX_BIBS) {
            return;
        }
        ++m_numBibs;
    }

    m_anythingChanged = true;

    m_bibs[i].m_points[0] = points[0];
    m_bibs[i].m_points[1] = points[1];
    m_bibs[i].m_points[2] = points[2];
    m_bibs[i].m_points[3] = points[3];

    m_bibs[i].m_highlighted = highlighted;
    m_bibs[i].m_unknown = 0;
    m_bibs[i].m_isUnused = false;
    m_bibs[i].m_objectID = id;
    m_bibs[i].m_drawableID = INVALID_DRAWABLE_ID;
}

void W3DBibBuffer::Add_Bib_To_Drawable(Vector3 *points, DrawableID id, bool highlighted)
{
    int i;

    for (i = 0; i < m_numBibs && (m_bibs[i].m_isUnused || m_bibs[i].m_drawableID != id); ++i) {
        ;
    }

    if (i == m_numBibs) {
        for (i = 0; i < m_numBibs && !m_bibs[i].m_isUnused; ++i) {
            ;
        }
    }

    if (i == m_numBibs) {
        if (m_numBibs >= MAX_BIBS) {
            return;
        }
        ++m_numBibs;
    }

    m_anythingChanged = true;

    m_bibs[i].m_points[0] = points[0];
    m_bibs[i].m_points[1] = points[1];
    m_bibs[i].m_points[2] = points[2];
    m_bibs[i].m_points[3] = points[3];

    m_bibs[i].m_highlighted = highlighted;
    m_bibs[i].m_unknown = 0;
    m_bibs[i].m_isUnused = false;
    m_bibs[i].m_objectID = INVALID_OBJECT_ID;
    m_bibs[i].m_drawableID = id;
}

void W3DBibBuffer::Remove_Bib_From_Object(ObjectID id)
{
    for (int i = 0; i < m_numBibs; ++i) {
        if (m_bibs[i].m_objectID == id) {
            m_bibs[i].m_isUnused = true;
            m_bibs[i].m_objectID = INVALID_OBJECT_ID;
            m_bibs[i].m_drawableID = INVALID_DRAWABLE_ID;

            m_anythingChanged = true;
        }
    }
}

void W3DBibBuffer::Remove_Bib_From_Drawable(DrawableID id)
{
    for (int i = 0; i < m_numBibs; ++i) {
        if (m_bibs[i].m_drawableID == id) {
            m_bibs[i].m_isUnused = true;
            m_bibs[i].m_objectID = INVALID_OBJECT_ID;
            m_bibs[i].m_drawableID = INVALID_DRAWABLE_ID;

            m_anythingChanged = true;
        }
    }
}

ShaderClass s_detailAlphaShader(0x184B7);

void W3DBibBuffer::Render_Bibs()
{
    Load_Bibs_In_Vertex_And_Index_Buffers();

    if (m_numBibIndices) {
        DX8Wrapper::Set_Index_Buffer(m_indexBib, 0);
        DX8Wrapper::Set_Vertex_Buffer(m_vertexBib, 0);
        DX8Wrapper::Set_Shader(s_detailAlphaShader);

        if (m_firstIndex > 0) {
            DX8Wrapper::Set_Texture(0, m_bibTexture);
            DX8Wrapper::Draw_Triangles(0, m_firstIndex / 3, 0, m_firstVertex);
        }

        if (m_numBibIndices > m_firstIndex) {
            DX8Wrapper::Set_Texture(0, m_redBibTexture);
            DX8Wrapper::Draw_Triangles(
                m_firstIndex, (m_numBibIndices - m_firstIndex) / 3, m_firstVertex, m_numBibVertices - m_firstVertex);
        }
    }
}