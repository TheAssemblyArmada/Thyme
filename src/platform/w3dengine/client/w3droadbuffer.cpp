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
