/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Road Drawing Code
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3droadbuffer.h"
#include "texture.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "shadermanager.h"
#include "dx8wrapper.h"
#include "assetmgr.h"
#include "globaldata.h"
#include "baseheightmap.h"
#include "worldheightmap.h"

bool g_dynamic;

RoadType::RoadType() : m_roadTexture(nullptr), m_vertexRoad(nullptr), m_indexRoad(nullptr), m_uniqueID(-1), m_stackingOrder(0)
{
}

RoadType::~RoadType()
{
    Ref_Ptr_Release(m_roadTexture);
    Ref_Ptr_Release(m_vertexRoad);
    Ref_Ptr_Release(m_indexRoad);
}

void RoadType::Apply_Texture()
{
    W3DShaderManager::Set_Texture(0, m_roadTexture);
    DX8Wrapper::Set_Index_Buffer(m_indexRoad, 0);
    DX8Wrapper::Set_Vertex_Buffer(m_vertexRoad, 0);
}

void RoadType::Load_Texture(Utf8String path, int ID)
{
    m_roadTexture = W3DAssetManager::Get_Instance()->Get_Texture(path, MIP_LEVELS_3);
    m_roadTexture->Get_Texture_Filter()->Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_BEST);
    m_roadTexture->Get_Texture_Filter()->Set_U_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_REPEAT);
    m_roadTexture->Get_Texture_Filter()->Set_V_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_REPEAT);
    m_vertexRoad = new DX8VertexBufferClass(DX8_FVF_XYZDUV1,
        g_theWriteableGlobalData->m_maxRoadVertex + 4,
        g_dynamic != 0 ? DX8VertexBufferClass::USAGE_DYNAMIC : DX8VertexBufferClass::USAGE_DEFAULT,
        0);
    m_indexRoad = new DX8IndexBufferClass(g_theWriteableGlobalData->m_maxRoadIndex + 4,
        g_dynamic != 0 ? DX8IndexBufferClass::USAGE_DYNAMIC : DX8IndexBufferClass::USAGE_DEFAULT);
    m_numRoadVertices = 0;
    m_numRoadIndices = 0;
    m_texturePath = path;
    m_uniqueID = ID;
}

void RoadType::Load_Test_Texture()
{
    if (m_isAutoLoaded) {
        if (m_uniqueID > 0 && !m_texturePath.Is_Empty()) {
            m_roadTexture = new TextureClass(m_texturePath, m_texturePath, MIP_LEVELS_3, WW3D_FORMAT_UNKNOWN, true, true);
            m_roadTexture->Get_Texture_Filter()->Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_BEST);
            m_roadTexture->Get_Texture_Filter()->Set_U_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_REPEAT);
            m_roadTexture->Get_Texture_Filter()->Set_V_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_REPEAT);
        }
    }
}

RoadSegment::RoadSegment() :
    m_curveRadius(0),
    m_type(SEGMENT),
    m_scale(1),
    m_widthInTexture(1),
    m_uniqueID(0),
    m_isVisible(0),
    m_numVertex(0),
    m_vb(nullptr),
    m_numIndex(0),
    m_ib(nullptr),
    m_Bounds(Vector3(0, 0, 0), 1)
{
}

RoadSegment::~RoadSegment()
{
    m_numVertex = 0;

    if (m_vb != nullptr) {
        delete[] m_vb;
        m_vb = nullptr;
    }

    m_numIndex = 0;

    if (m_ib != nullptr) {
        delete[] m_ib;
        m_ib = nullptr;
    }
}

void RoadSegment::Set_Vertex_Buffer(VertexFormatXYZDUV1 *vb, int num_vertex)
{
    if (m_vb != nullptr) {
        delete[] m_vb;
        m_vb = nullptr;
        m_numVertex = 0;
    }

    Vector3 pos[500];

    if (num_vertex >= 1 && num_vertex <= 500) {
        m_vb = new VertexFormatXYZDUV1[num_vertex];

        if (m_vb != nullptr) {
            m_numVertex = num_vertex;
            memcpy(m_vb, vb, num_vertex * sizeof(VertexFormatXYZDUV1));

            for (int i = 0; i < num_vertex; i++) {
                pos[i].X = m_vb[i].x;
                pos[i].Y = m_vb[i].y;
                pos[i].Z = m_vb[i].z;
            }

            m_Bounds = SphereClass(pos, num_vertex);
        }
    }
}

void RoadSegment::Set_Index_Buffer(unsigned short *ib, int num_index)
{
    if (m_ib != nullptr) {
        delete[] m_ib;
        m_ib = nullptr;
        m_numIndex = 0;
    }

    if (num_index >= 1 && num_index <= 2000) {
        m_ib = new unsigned short[num_index];

        if (m_ib != nullptr) {
            m_numIndex = num_index;
            memcpy(m_ib, ib, num_index * sizeof(unsigned short));
        }
    }
}

int RoadSegment::Get_Vertices(VertexFormatXYZDUV1 *destination_vb, int num_to_copy) 
{
    if (m_vb == nullptr || num_to_copy < 1) {
        return 0;
    }

    if (num_to_copy > m_numVertex) {
        return 0;
    }

    memcpy(destination_vb, m_vb, num_to_copy * sizeof(VertexFormatXYZDUV1));
    return num_to_copy;
}

int RoadSegment::Get_Indices(unsigned short *destination_ib, int num_to_copy, int offset)
{
    if (m_ib == nullptr || num_to_copy < 1) {
        return 0;
    }

    if (num_to_copy > m_numIndex) {
        return 0;
    }

    for (int i = 0; i < num_to_copy; i++) {
        destination_ib[i] = offset + m_ib[i];
    }

    return num_to_copy;
}

void RoadSegment::Update_Seg_Lighting()
{
    int size = g_theTerrainRenderObject->Get_Map()->Get_Border_Size();

    for (int i = 0; i < m_numVertex; i++) {
        m_vb[i].diffuse = g_theTerrainRenderObject->Get_Static_Diffuse(
                              size + (int)(m_vb[i].x / 10.0f + 0.5), size + (int)(m_vb[i].x / 10.0f + 0.5))
            | 0xFF000000;
    }
}
