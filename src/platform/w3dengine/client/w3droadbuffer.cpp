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
#include "assetmgr.h"
#include "baseheightmap.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include "gamemath.h"
#include "globaldata.h"
#include "mapobject.h"
#include "shadermanager.h"
#include "terrainroads.h"
#include "texture.h"
#include "worldheightmap.h"

using GameMath::Fabs;
bool g_dynamic;
bool g_warn_segments = false;

enum
{
    MAX_SEG_VERTEX = 500,
    MAX_SEG_INDEX = 2000,
};

RoadType::RoadType() :
    m_roadTexture(nullptr), m_vertexRoad(nullptr), m_indexRoad(nullptr), m_uniqueID(-1), m_stackingOrder(0)
{
    // #BUGFIX Initialize all members
    m_numRoadVertices = 0;
    m_numRoadIndices = 0;
    m_isAutoLoaded = false;
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
    m_roadTexture = W3DAssetManager::Get_Instance()->Get_Texture(path.Str(), MIP_LEVELS_3);
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
            m_roadTexture =
                new TextureClass(m_texturePath.Str(), m_texturePath.Str(), MIP_LEVELS_3, WW3D_FORMAT_UNKNOWN, true, true);
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
    m_bounds(Vector3(0, 0, 0), 1)
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

    Vector3 pos[MAX_SEG_VERTEX];

    if (num_vertex >= 1 && num_vertex <= MAX_SEG_VERTEX) {
        m_vb = new VertexFormatXYZDUV1[num_vertex];

        if (m_vb != nullptr) {
            m_numVertex = num_vertex;
            memcpy(m_vb, vb, num_vertex * sizeof(VertexFormatXYZDUV1));

            for (int i = 0; i < num_vertex; i++) {
                pos[i].X = m_vb[i].x;
                pos[i].Y = m_vb[i].y;
                pos[i].Z = m_vb[i].z;
            }

            m_bounds = SphereClass(pos, num_vertex);
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

    if (num_index >= 1 && num_index <= MAX_SEG_INDEX) {
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
    int size = g_theTerrainRenderObject->Get_Map()->Border_Size();

    for (int i = 0; i < m_numVertex; i++) {
        m_vb[i].diffuse = g_theTerrainRenderObject->Get_Static_Diffuse(
                              size + (int)(m_vb[i].x / 10.0f + 0.5f), size + (int)(m_vb[i].y / 10.0f + 0.5f))
            | 0xFF000000;
    }
}

W3DRoadBuffer::W3DRoadBuffer() :
    m_roads(nullptr),
    m_numRoads(0),
    m_initialized(false),
    m_map(nullptr),
    m_lightsIterator(nullptr),
    m_curRoadType(0),
    m_maxUID(0),
    m_maxRoadSegments(500),
    m_maxRoadVertex(1000),
    m_maxRoadIndex(MAX_SEG_INDEX),
    m_maxRoadTypes(8)
{
    Allocate_Road_Buffers();
}

W3DRoadBuffer::~W3DRoadBuffer()
{
    Free_Road_Buffers();
    Ref_Ptr_Release(m_map);
}

void W3DRoadBuffer::Allocate_Road_Buffers()
{
    m_maxRoadSegments = g_theWriteableGlobalData->m_maxRoadSegments;
    m_maxRoadVertex = g_theWriteableGlobalData->m_maxRoadVertex;
    m_maxRoadIndex = g_theWriteableGlobalData->m_maxRoadIndex;
    m_maxRoadTypes = g_theWriteableGlobalData->m_maxRoadTypes;
    m_maxRoadTypes += 4;
    m_curNumRoadVertices = 0;
    m_curNumRoadIndices = 0;
    m_roads = new RoadSegment[m_maxRoadSegments];
    m_roadTypes = new RoadType[m_maxRoadTypes];
    int count = 0;

    for (TerrainRoadType *road = g_theTerrainRoads->First_Road(); road; road = g_theTerrainRoads->Next_Road(road)) {
        if (count < m_maxRoadTypes) {
            int ID = road->Get_ID();
            m_roadTypes[count++].Load_Texture(road->Get_Texture(), ID);
            if (m_maxUID < ID) {
                m_maxUID = ID;
            }
        }
    }

    while (count < m_maxRoadTypes && m_roadTypes[count].Is_Auto_Loaded()) {
        m_roadTypes[count++].Load_Test_Texture();
    }

    m_curOpenRoad = count;
    m_initialized = true;
}

void W3DRoadBuffer::Free_Road_Buffers()
{
    if (m_roads) {
        delete[] m_roads;
        m_roads = nullptr;
    }

    if (m_roadTypes) {
        delete[] m_roadTypes;
        m_roadTypes = nullptr;
    }
}

void W3DRoadBuffer::Load_Tee(RoadSegment *road, Vector2 loc1, Vector2 loc2, bool is4way, float scale)
{
    Vector2 v1(loc2.X - loc1.X, loc2.Y - loc1.Y);
    Vector2 v2(-v1.Y, v1.X);

    if ((float)abs((int)v1.X) >= 0.25f || (float)abs((int)v1.Y) >= 0.25f) {
        v1.Normalize();
        v2.Normalize();
    } else {
        v1.Set(1, 0);
        v2.Set(0, 1);
    }

    float uOffset = 0.83007812f;
    float vOffset = 0.49804688f;

    if (is4way) {
        uOffset = 0.83007812f;
        vOffset = 0.83007812f;
    }

    float halfHeight = scale * 1.03f / 2.0f;
    float left = scale * road->m_widthInTexture / 2.0f;
    Vector2 roadVector = loc2 - loc1;
    Vector2 loc(loc1);
    Load_Float_Section(road, loc, roadVector, halfHeight, left, halfHeight, uOffset, vOffset, scale);
}

void W3DRoadBuffer::Load_Roads()
{
    if (m_initialized) {
        Clear_All_Roads();
        Add_Map_Objects();
        Update_Counts_And_Flags();
        Insert_Tee_Intersections();
        Insert_Curve_Segments();
        Insert_Cross_Type_Joins();
        Preload_Roads_In_Vertex_And_Index_Buffers();
        m_dirty = true;
    }
}

void W3DRoadBuffer::Clear_All_Roads()
{
    if (m_roads) {
        for (int i = 0; i < m_numRoads; i++) {
            m_roads[i].Set_Index_Buffer(nullptr, 0);
            m_roads[i].Set_Vertex_Buffer(nullptr, 0);
        }
    }

    m_numRoads = 0;

    if (m_roadTypes) {
        for (int i = 0; i < m_maxRoadTypes; i++) {
            m_roadTypes[i].Set_Stacking(false);
            m_roadTypes[i].Set_Num_Vertices(0);
            m_roadTypes[i].Set_Num_Indices(0);
        }
    }
}

void W3DRoadBuffer::Set_Map(WorldHeightMap *map)
{
    Ref_Ptr_Set(m_map, map);
}

void W3DRoadBuffer::Update_Lighting()
{
    if (m_roads) {
        for (int i = 0; i < m_numRoads; i++) {
            m_roads[i].Update_Seg_Lighting();
        }
        m_dirty = true;
    }
}

void W3DRoadBuffer::Update_Center()
{
    m_dirty = true;
}

void W3DRoadBuffer::Draw_Roads(CameraClass *camera,
    TextureClass *cloud_texture,
    TextureClass *noise_texture,
    bool wireframe,
    int minx,
    int maxx,
    int miny,
    int maxy,
    RefMultiListIterator<RenderObjClass> *dynamic_lights_iterator)
{
    IRegion2D region;
    region.lo.x = (int)((float)minx * 10.0f);
    region.hi.x = (int)((float)maxx * 10.0f);
    region.lo.y = (int)((float)miny * 10.0f);
    region.hi.y = (int)((float)maxy * 10.0f);
    int stacking = 0;

    for (int i = 0; i < m_maxRoadTypes; i++) {
        if (m_roadTypes[i].Get_Stacking() > stacking) {
            stacking = m_roadTypes[i].Get_Stacking();
        }
    }

    W3DShaderManager::ShaderTypes shader = W3DShaderManager::ST_ROAD;

    if (cloud_texture) {
        shader = W3DShaderManager::ST_ROAD_NOISE1;

        if (noise_texture) {
            shader = W3DShaderManager::ST_ROAD_NOISE12;
        }
    } else if (noise_texture) {
        shader = W3DShaderManager::ST_ROAD_NOISE2;
    }

    int passes = W3DShaderManager::Get_Shader_Passes(shader);
    W3DShaderManager::Set_Texture(1, cloud_texture);
    W3DShaderManager::Set_Texture(2, noise_texture);
    bool loadroads = false;

    if (m_dirty) {
        if (Visibility_Changed(region)) {
            loadroads = true;
        }
    }

    m_dirty = false;

    for (int i = 0; i <= stacking; i++) {
        for (int j = 0; j < m_maxRoadTypes; j++) {
            if (i == m_roadTypes[j].Get_Stacking()) {
                m_curUniqueID = m_roadTypes[j].Get_Unique_ID();
                m_curRoadType = j;

                if (loadroads) {
                    Load_Roads_In_Vertex_And_Index_Buffers();
                }

                if (m_roadTypes[j].Get_Num_Indices()) {
                    if (wireframe) {
                        m_roadTypes[j].Apply_Texture();
#ifdef BUILD_WITH_D3D8
                        DX8Wrapper::Set_DX8_Texture(0, nullptr);
#endif
                    } else {
                        m_roadTypes[j].Apply_Texture();
                    }

                    for (int k = 0; k < passes; k++) {
                        if (!wireframe) {
                            W3DShaderManager::Set_Shader(shader, k);
                        }

                        DX8Wrapper::Draw_Triangles(
                            0, m_roadTypes[j].Get_Num_Indices() / 3, 0, m_roadTypes[j].Get_Num_Vertices());
                    }

                    if (!wireframe) {
                        W3DShaderManager::Reset_Shader(shader);
                    }
                }
            }
        }
    }

    m_curRoadType = 0;
}

void W3DRoadBuffer::Rotate_About(Vector2 *pt, Vector2 center, float angle)
{
    Vector2 v1;
    v1.X = pt->X - center.X;
    v1.Y = pt->Y - center.Y;
    Vector2 v2(v1);
    v1.Rotate(angle);
    v1.X = v1.X - v2.X;
    v1.Y = v1.Y - v2.Y;
    *pt += v1;
}

bool W3DRoadBuffer::Visibility_Changed(const IRegion2D &region)
{
    if (!m_initialized) {
        return false;
    }

    bool changed = false;

    for (int i = 0; i < m_numRoads; i++) {
        bool visible = true;
        float scale = m_roads[i].m_scale / 2.0f;

        if ((float)region.lo.x > scale + m_roads[i].m_pt1.loc.X && (float)region.lo.x > scale + m_roads[i].m_pt2.loc.X) {
            visible = false;
        }

        if ((float)region.hi.x < m_roads[i].m_pt1.loc.X - scale && (float)region.hi.x < m_roads[i].m_pt2.loc.X - scale) {
            visible = false;
        }

        if ((float)region.lo.y > scale + m_roads[i].m_pt1.loc.Y && (float)region.lo.y > scale + m_roads[i].m_pt2.loc.Y) {
            visible = false;
        }

        if ((float)region.hi.y < m_roads[i].m_pt1.loc.Y - scale && (float)region.hi.y < m_roads[i].m_pt2.loc.Y - scale) {
            visible = false;
        }

        if (visible != m_roads[i].m_isVisible) {
            changed = true;
        }

        m_roads[i].m_isVisible = visible;
    }

    return changed;
}

void W3DRoadBuffer::Add_Map_Objects()
{
    for (MapObject *o = MapObject::Get_First_Map_Object(); o; o = o->Get_Next()) {
        if (o->Get_Flag(FLAG_ROAD_POINT1)) {
            MapObject *o2 = o->Get_Next();

            if (!o2) {
                break;
            }

            if (o2->Get_Flag(FLAG_ROAD_POINT2)) {
                Vector2 v1;
                Vector2 v2;
                v1.Set(o->Get_Location()->x, o->Get_Location()->y);
                v2.Set(o2->Get_Location()->x, o2->Get_Location()->y);

                if (v1.X == v2.X && v1.Y == v2.Y) {
                    v2.X = v2.X + 0.25;
                }

                RoadSegment segment;
                segment.m_scale = 8.0f;
                segment.m_widthInTexture = 1.0f;
                segment.m_uniqueID = 1;
                bool b = false;
                TerrainRoadType *road = g_theTerrainRoads->Find_Road(o->Get_Name());

                if (road) {
                    segment.m_widthInTexture = road->Get_Road_Width_In_Texture();
                    segment.m_scale = road->Get_Road_Width();
                    segment.m_uniqueID = road->Get_ID();
                    b = true;
                }

                if (!b) {
                    for (int i = 0; i < m_maxRoadTypes; i++) {
                        if (o->Get_Name() == m_roadTypes[i].Get_Path()) {
                            segment.m_scale = 30.0f;
                            segment.m_uniqueID = m_roadTypes[i].Get_Unique_ID();
                            b = true;
                        }
                    }
                }

                if (!b && m_curOpenRoad < m_maxRoadTypes) {
                    m_maxUID++;
                    segment.m_scale = 30.0f;
                    segment.m_uniqueID = m_maxUID;
                    m_roadTypes[m_curOpenRoad].Load_Texture(o->Get_Name(), m_maxUID);
                    m_curOpenRoad++;
                }

                segment.m_pt1.loc = v1;
                segment.m_pt1.is_angled = o->Get_Flag(FLAG_ROAD_CORNER_ANGLED);
                segment.m_pt1.is_join = o->Get_Flag(FLAG_ROAD_JOIN);
                segment.m_pt2.loc = v2;
                segment.m_pt2.is_angled = o2->Get_Flag(FLAG_ROAD_CORNER_ANGLED);
                segment.m_pt2.is_join = o2->Get_Flag(FLAG_ROAD_JOIN);
                segment.m_type = SEGMENT;

                if (o->Get_Flag(FLAG_ROAD_CORNER_TIGHT)) {
                    segment.m_curveRadius = 0.5f;
                } else {
                    segment.m_curveRadius = 1.5f;
                }

                Add_Map_Object(&segment, true);
                o = o2;
            }
        }
    }

    int count = m_numRoads;
    m_numRoads = 0;
    for (int i = 0; i < count; i++) {
        RoadSegment segment(m_roads[i]);
        Add_Map_Object(&segment, false);
    }
}

void W3DRoadBuffer::Add_Map_Object(RoadSegment *road, bool update_the_counts)
{
    RoadSegment segment(*road);
    Vector2 v1;
    Vector2 v2;
    v1 = segment.m_pt1.loc;
    v2 = segment.m_pt2.loc;
    Vector2 v3(v2.X - v1.X, v2.Y - v1.Y);
    Vector2 v4(-v3.Y, v3.X);
    v4.Normalize();
    v4 *= segment.m_scale * segment.m_widthInTexture / 2.0f;
    segment.m_pt1.top = v1 + v4;
    segment.m_pt1.bottom = v1 - v4;
    segment.m_pt2.top = v2 + v4;
    segment.m_pt2.bottom = v2 - v4;

    if (update_the_counts) {
        Update_Counts(&segment);
    }

    if (m_numRoads < m_maxRoadSegments) {
        bool b1 = false;
        bool b2 = false;
        bool b3 = false;
        bool b4 = false;
        int i;
        for (i = 0; i < m_numRoads; i++) {
            b4 = false;

            if (m_roads[i].m_pt1.loc == v1 && m_roads[i].m_pt2.loc == v2
                || m_roads[i].m_pt1.loc == v2 && m_roads[i].m_pt2.loc == v1) {
                b4 = true;
                break;
            }

            if (segment.m_pt1.count == 1) {
                if (m_roads[i].m_pt1.loc == v1) {
                    b1 = true;
                    b3 = true;
                }

                if (m_roads[i].m_pt2.loc == v1) {
                    b1 = false;
                    b2 = true;
                }
            }

            if (segment.m_pt2.count == 1) {
                if (m_roads[i].m_pt1.loc == v2) {
                    b1 = false;
                    b3 = true;
                }

                if (m_roads[i].m_pt2.loc == v2) {
                    b1 = true;
                    b2 = true;
                }
            }

            if (b2 || b3) {
                break;
            }
        }

        if (!b4) {
            int ndx = i;

            if (b3) {
                ndx++;
            }

            if (ndx < m_numRoads) {
                for (i = m_numRoads; i > ndx; i--) {
                    m_roads[i] = m_roads[i - 1];
                }
            }

            m_roads[ndx] = segment;

            if (b1) {
                Flip_The_Road(&m_roads[ndx]);
            }

            m_numRoads++;

            if (b2) {
                Check_Link_Before(ndx);
            } else if (b3) {
                Check_Link_After(ndx);
            }
        }
    }
}

void W3DRoadBuffer::Check_Link_Before(int ndx)
{
    if (m_roads[ndx].m_pt2.count == 1) {
        Vector2 v1(m_roads[ndx].m_pt2.loc);
        int i;

        for (i = ndx + 1; i < m_numRoads - 1 && !(m_roads[i].m_pt1.loc != m_roads[i + 1].m_pt2.loc); i++) {
        }

        int fromNdx = i + 1;

        while (fromNdx < m_numRoads) {
            if (m_roads[fromNdx].m_pt1.loc == v1) {
                Move_Road_Seg_To(fromNdx, ndx);
                v1 = m_roads[ndx].m_pt2.loc;

                if (m_roads[ndx].m_pt2.count != 1) {
                    return;
                }

                i++;
            } else if (m_roads[fromNdx].m_pt2.loc == v1) {
                Flip_The_Road(&m_roads[fromNdx]);
                Move_Road_Seg_To(fromNdx, ndx);
                v1 = m_roads[ndx].m_pt2.loc;

                if (m_roads[ndx].m_pt2.count != 1) {
                    return;
                }

                i++;
            } else {
                fromNdx++;
            }

            if (fromNdx <= i) {
                fromNdx = i + 1;
            }
        }
    }
}

void W3DRoadBuffer::Check_Link_After(int ndx)
{
    if (m_roads[ndx].m_pt1.count == 1 && ndx < m_numRoads - 1) {
        Vector2 v1(m_roads[ndx].m_pt1.loc);
        int fromNdx = ndx + 1;

        while (fromNdx < m_numRoads && ndx < m_numRoads - 1) {
            if (m_roads[fromNdx].m_pt2.loc == v1) {
                ndx++;
                Move_Road_Seg_To(fromNdx, ndx);
                v1 = m_roads[ndx].m_pt1.loc;

                if (m_roads[ndx].m_pt1.count != 1) {
                    return;
                }
            } else if (m_roads[fromNdx].m_pt1.loc == v1) {
                Flip_The_Road(&m_roads[fromNdx]);
                ndx++;
                Move_Road_Seg_To(fromNdx, ndx);
                v1 = m_roads[ndx].m_pt1.loc;

                if (m_roads[ndx].m_pt1.count != 1) {
                    return;
                }
            } else {
                fromNdx++;
            }
        }
    }
}

void W3DRoadBuffer::Adjust_Stacking(int top_unique_id, int bottom_unique_id)
{
    int i;

    for (i = 0; i < m_maxRoadTypes && m_roadTypes[i].Get_Unique_ID() != top_unique_id; i++) {
    }

    if (i < m_maxRoadTypes) {
        int j;

        for (j = 0; j < m_maxRoadTypes && m_roadTypes[j].Get_Unique_ID() != bottom_unique_id; j++) {
        }

        if (j < m_maxRoadTypes) {
            int i1 = m_roadTypes[i].Get_Stacking();

            if (i1 <= m_roadTypes[j].Get_Stacking()) {
                int i2 = m_roadTypes[j].Get_Stacking();

                for (int k = 0; k < m_maxRoadTypes; k++) {
                    if (m_roadTypes[k].Get_Stacking() > i2) {
                        int i3 = m_roadTypes[k].Get_Stacking();
                        m_roadTypes[k].Set_Stacking(i3 + 1);
                    }
                }

                m_roadTypes[i].Set_Stacking(i2 + 1);
            }
        }
    }
}

int xp_sign(const Vector2 &v1, const Vector2 &v2)
{
    float f = v1.X * v2.Y - v1.Y * v2.X;
    if (f >= 0) {
        return f > 0;
    } else {
        return -1;
    }
}

void W3DRoadBuffer::Load_Alpha_Join(RoadSegment *road, Vector2 loc1, Vector2 loc2, float scale)
{
    Vector2 v1(loc2.X - loc1.X, loc2.Y - loc1.Y);
    Vector2 v2(-v1.Y, v1.X);

    if ((float)abs((int)v1.X) >= 0.25f || (float)abs((int)v1.Y) >= 0.25f) {
        v1.Normalize();
        v2.Normalize();
    } else {
        v1.Set(1, 0);
        v2.Set(0, 1);
    }

    float uOffset = 0.20703125f;
    float vOffset = 0.83007812f;
    v1 *= scale * 48.0f / 128.0f;
    v2 *= road->m_widthInTexture * 1.0625f;
    Vector2 corners[4];
    Vector2 v3 = v1 * 0.64999998f;
    Vector2 v4 = v2 * 0.5f;
    Vector2 v5 = loc1 + v4;
    Vector2 v6 = v5 - v3;
    corners[2] = v6;
    corners[0] = corners[2] - v2;
    corners[1] = corners[0] + v1;
    corners[3] = corners[2] + v1;
    Load_Float_4Pt_Section(road, loc1, v2, v1, corners, uOffset, vOffset, scale, road->m_widthInTexture);
}

void W3DRoadBuffer::Load_Y(RoadSegment *road, Vector2 loc1, Vector2 loc2, float scale)
{
    Vector2 v1(loc2.X - loc1.X, loc2.Y - loc1.Y);
    Vector2 v2(-v1.Y, v1.X);

    if ((float)abs((int)v1.X) >= 0.25f || (float)abs((int)v1.Y) >= 0.25f) {
        v1.Normalize();
        v2.Normalize();
    } else {
        v1.Set(1, 0);
        v2.Set(0, 1);
    }

    float uOffset = 0.49804688f;
    float vOffset = 0.44140625f;
    v1 *= scale;
    v2 *= scale;
    Vector2 corners[4];
    v1 *= 1.59f;
    Vector2 v3 = v1 * 0.5f;
    Vector2 v4 = v2 * 0.28999999f;
    Vector2 v5 = loc1 + v4;
    Vector2 v6 = v5 - v3;
    corners[2] = v6;
    Vector2 v7 = v2 * 1.08f;
    corners[0] = corners[2] - v7;
    corners[1] = corners[0] + v1;
    corners[3] = corners[2] + v1;
    Load_Float_4Pt_Section(road, loc1, v2, v1, corners, uOffset, vOffset, scale, scale);
}

void W3DRoadBuffer::Move_Road_Seg_To(int fromndx, int tondx)
{
    if (fromndx >= 0 && fromndx < m_numRoads && tondx >= 0 && tondx < m_numRoads && fromndx != tondx) {
        RoadSegment road(m_roads[fromndx]);

        if (fromndx >= tondx) {
            for (int i = fromndx; i > tondx; i--) {
                m_roads[i] = m_roads[i - 1];
            }
        } else {
            for (int i = fromndx; i < tondx; i++) {
                m_roads[i] = m_roads[i + 1];
            }
        }

        m_roads[tondx] = road;
    }
}

void W3DRoadBuffer::Insert_Tee_Intersections()
{
    int count = m_numRoads;
    for (int i = 0; i < count; i++) {
        if (m_roads[i].m_type == SEGMENT) {
            if (m_roads[i].m_pt1.count == 2) {
                Insert_Tee(m_roads[i].m_pt1.loc, i, m_roads[i].m_scale);
            }

            if (m_roads[i].m_pt2.count == 2) {
                Insert_Tee(m_roads[i].m_pt2.loc, i, m_roads[i].m_scale);
            }

            if (m_roads[i].m_pt1.count == 3) {
                Insert_4Way(m_roads[i].m_pt1.loc, i, m_roads[i].m_scale);
            }

            if (m_roads[i].m_pt2.count == 3) {
                Insert_4Way(m_roads[i].m_pt2.loc, i, m_roads[i].m_scale);
            }
        }
    }
}

void W3DRoadBuffer::Insert_Curve_Segments()
{
    int count = m_numRoads;
    int ndx2 = -1;

    for (int ndx1 = 0; ndx1 < count; ndx1++) {
        if (ndx1 < count - 1 && m_roads[ndx1].m_pt1.loc == m_roads[ndx1 + 1].m_pt2.loc) {
            if (m_roads[ndx1 + 1].m_pt2.count == 1 && m_roads[ndx1].m_pt1.count == 1) {
                Insert_Curve_Segment_At(ndx1, ndx1 + 1);

                if (ndx2 < 0) {
                    ndx2 = ndx1;
                }
            }
        } else if (ndx2 >= 0) {
            if (m_roads[ndx1].m_pt1.loc == m_roads[ndx2].m_pt2.loc && m_roads[ndx2].m_pt2.count == 1
                && m_roads[ndx1].m_pt1.count == 1) {
                Insert_Curve_Segment_At(ndx1, ndx2);
            }

            ndx2 = -1;
        }
    }
}

void W3DRoadBuffer::Load_Roads_In_Vertex_And_Index_Buffers()
{
#ifdef BUILD_WITH_D3D8 // Uses D3D defines.
    if (m_initialized) {
        m_curNumRoadVertices = 0;
        m_curNumRoadIndices = 0;

        if (m_roadTypes[m_curRoadType].Get_IB()) {
            IndexBufferClass::WriteLockClass iblock(
                m_roadTypes[m_curRoadType].Get_IB(), g_dynamic != 0 ? D3DLOCK_DISCARD : 0);
            VertexBufferClass::WriteLockClass vblock(
                m_roadTypes[m_curRoadType].Get_VB(), g_dynamic != 0 ? D3DLOCK_DISCARD : 0);
            VertexFormatXYZDUV1 *vb = static_cast<VertexFormatXYZDUV1 *>(vblock.Get_Vertex_Array());
            unsigned short *ib = iblock.Get_Index_Array();

            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < m_numRoads; j++) {
                    if (m_roads[j].m_type == i) {
                        Load_Road_Segment(ib, vb, &m_roads[j]);
                    }
                }
            }

            m_roadTypes[m_curRoadType].Set_Num_Vertices(m_curNumRoadVertices);
            m_roadTypes[m_curRoadType].Set_Num_Indices(m_curNumRoadIndices);
        } else {
            m_roadTypes[m_curRoadType].Set_Num_Vertices(0);
            m_roadTypes[m_curRoadType].Set_Num_Indices(0);
        }
    }

#endif
}

void W3DRoadBuffer::Load_Road_Segment(unsigned short *ib, VertexFormatXYZDUV1 *vb, RoadSegment *road)
{
    if (road->m_uniqueID == m_curUniqueID) {
        if (road->m_isVisible) {
            int count = m_curNumRoadVertices;
            if (road->Get_Num_Vertex() + count < m_maxRoadVertex
                && road->Get_Num_Index() + m_curNumRoadIndices < m_maxRoadIndex) {
                m_curNumRoadVertices += road->Get_Vertices(&vb[count], road->Get_Num_Vertex());
                m_curNumRoadIndices += road->Get_Indices(&ib[m_curNumRoadIndices], road->Get_Num_Index(), count);
            }
        }
    }
}

void W3DRoadBuffer::Preload_Roads_In_Vertex_And_Index_Buffers()
{
    if (m_initialized) {
        for (int i = 0; i < m_numRoads; i++) {
            if (m_roads[i].m_type == SEGMENT) {
                Preload_Road_Segment(&m_roads[i]);
            }
        }

        for (int i = 0; i < m_numRoads; i++) {
            if (m_roads[i].m_type == CURVE) {
                Load_Curve(&m_roads[i], m_roads[i].m_pt1.loc, m_roads[i].m_pt2.loc, m_roads[i].m_scale);
            }
        }

        for (int i = 0; i < m_numRoads; i++) {
            if (m_roads[i].m_type == THREE_WAY_Y) {
                Load_Y(&m_roads[i], m_roads[i].m_pt1.loc, m_roads[i].m_pt2.loc, m_roads[i].m_scale);
            }
        }

        for (int i = 0; i < m_numRoads; i++) {
            if (m_roads[i].m_type == THREE_WAY_H || m_roads[i].m_type == THREE_WAY_H_FLIP) {
                Load_H(&m_roads[i],
                    m_roads[i].m_pt1.loc,
                    m_roads[i].m_pt2.loc,
                    m_roads[i].m_type == THREE_WAY_H_FLIP,
                    m_roads[i].m_scale);
            }
        }

        for (int i = 0; i < m_numRoads; i++) {
            if (m_roads[i].m_type == TEE || m_roads[i].m_type == FOUR_WAY) {
                Load_Tee(&m_roads[i],
                    m_roads[i].m_pt1.loc,
                    m_roads[i].m_pt2.loc,
                    m_roads[i].m_type == FOUR_WAY,
                    m_roads[i].m_scale);
            }
        }

        for (int i = 0; i < m_numRoads; i++) {
            if (m_roads[i].m_type == ALPHA_JOIN) {
                Load_Alpha_Join(&m_roads[i], m_roads[i].m_pt1.loc, m_roads[i].m_pt2.loc, m_roads[i].m_scale);
            }
        }

        Update_Lighting();
    }
}

void W3DRoadBuffer::Preload_Road_Segment(RoadSegment *road)
{
    Vector2 v1 = road->m_pt2.loc - road->m_pt1.loc;
    Vector2 v2(-v1.Y, v1.X);
    float uOffset = 0.0f;
    float vOffset = 0.16601562f;
    v2.Normalize();
    v2 *= road->m_widthInTexture * road->m_scale / 2.0f;
    Vector2 corners[4];
    corners[0] = road->m_pt1.bottom;
    corners[2] = road->m_pt1.top;
    corners[1] = road->m_pt2.bottom;
    corners[3] = road->m_pt2.top;
    Load_Float_4Pt_Section(road, road->m_pt1.loc, v2, v1, corners, uOffset, vOffset, road->m_scale, road->m_scale);
}

void W3DRoadBuffer::Load_Float_4Pt_Section(RoadSegment *road,
    Vector2 loc,
    Vector2 road_normal,
    Vector2 road_vector,
    Vector2 *cornersp,
    float uoffset,
    float voffset,
    float uscale,
    float vscale)
{
    struct TColumn
    {
        bool collapsed;
        bool deleted;
        Vector3 vtx[100];
        int diffuse_red;
        bool light_gradient;
        int vertex_index[100];
        float u_index;
    };
    VertexFormatXYZDUV1 vb[MAX_SEG_VERTEX];
    unsigned short ib[MAX_SEG_INDEX];
    float f1 = 0.078125f;
    float f2 = 0.6875f;
    int numVertex = 0;
    int numIndex = 0;
    TRoadSegInfo segment;
    segment.loc = loc;
    segment.road_normal = road_normal;
    segment.road_vector = road_vector;
    segment.corners[0] = cornersp[0];
    segment.corners[1] = cornersp[1];
    segment.corners[2] = cornersp[2];
    segment.corners[3] = cornersp[3];
    segment.u_offset = uoffset;
    segment.v_offset = voffset;
    segment.scale = uscale;
    road->Set_Road_Seg_Info(&segment);
    float f3 = road_vector.Length();
    float f4 = road_normal.Length();
    road_normal.Normalize();
    road_vector.Normalize();
    Vector2 v1;
    int i1 = (int)(f3 / 10.0f + 1.0f);

    if (i1 < 2) {
        i1 = 2;
    }

    int i2 = (int)(2.0f * f4 / 10.0f + 1.0f);

    if (i2 < 2) {
        i2 = 2;
    }

    if (i2 > 100) {
        i2 = 100;
    }

    TColumn column1;
    TColumn column2;
    TColumn column3;
    column1.deleted = true;
    column2.deleted = true;
    Vector2 v2(cornersp[0]);
    Vector3 v3(v2.X, v2.Y, 0.0f);
    Vector2 v4 = cornersp[1] - cornersp[0];
    v2 = v4;
    Vector3 v5(v2.X, v2.Y, 0.0f);
    Vector2 v6 = cornersp[3] - cornersp[2];
    v2 = v6;
    Vector3 v7(v2.X, v2.Y, 0.0f);
    v2 = cornersp[2];
    Vector3 v8(v2.X, v2.Y, 0.0f);
    Vector2 v9 = cornersp[2] - cornersp[0];
    v2 = v9;
    Vector3 v10(v2.X, v2.Y, 0.0f);
    Vector2 v11 = cornersp[3] - cornersp[1];
    v2 = v11;
    Vector3 v12(v2.X, v2.Y, 0.0f);
    Vector3 v13 = v10 - v12;
    v7 += v13;

    for (int i = 0; i <= i1; i++) {
        float f5 = (float)i / (float)(i1 - 1);
        float f6 = 1.0f - f5;

        if (i < i1) {
            column3.collapsed = false;
            column3.deleted = false;
            column3.light_gradient = false;
            column3.u_index = (float)i;
            float f7 = (float)WorldHeightMap::Get_Max_Height_Value() * HEIGHTMAP_SCALE;
            float f8 = (float)WorldHeightMap::Get_Min_Height_Value() * HEIGHTMAP_SCALE;

            for (int j = 0; j < i2; j++) {
                float f9 = (float)j / (float)(i2 - 1);
                float f10 = 1.0f - f9;
                Vector3 v14 = v12 * f5;
                Vector3 v15 = v14 * f9;
                Vector3 v16 = v10 * f6;
                Vector3 v17 = v16 * f9;
                Vector3 v18 = v7 * f9;
                Vector3 v19 = v18 * f5;
                Vector3 v20 = v5 * f10;
                Vector3 v21 = v20 * f5;
                Vector3 v22 = v3 + v21;
                Vector3 v23 = v22 + v19;
                Vector3 v24 = v23 + v17;
                Vector3 v25 = v24 + v15;
                column3.vtx[j] = v25;
                float height = g_theTerrainRenderObject->Get_Max_Cell_Height(column3.vtx[j].X, column3.vtx[j].Y);

                if (height < f7) {
                    f7 = height;
                }

                if (height > f8) {
                    f8 = height;
                }

                column3.vertex_index[j] = -1;
                column3.vtx[j].Z = height;
                int i3 = 0;

                if (j) {
                    if (abs((unsigned char)i3 - column3.diffuse_red) > 25) {
                        column3.light_gradient = true;
                    }
                } else {
                    column3.diffuse_red = (unsigned char)i3;
                }
            }

            column3.collapsed = true;
            column3.vtx[0].Z = f8;
            column3.vtx[1] = column3.vtx[i2 - 1];
            column3.vtx[1].Z = f8;

            if (i >= 2) {
                if (column1.collapsed) {
                    if (column2.collapsed) {
                        if (column3.collapsed) {
                            bool deleted = false;
                            float f12 = (column2.u_index - column1.u_index) * column1.vtx[0].Z
                                + (column3.u_index - column2.u_index) * column3.vtx[0].Z;
                            f12 = f12 / (column3.u_index - column1.u_index);

                            if (f12 >= column2.vtx[0].Z && column2.vtx[0].Z + f2 > f12) {
                                f12 = (column2.u_index - column1.u_index) * column1.vtx[1].Z
                                    + (column3.u_index - column2.u_index) * column3.vtx[1].Z;
                                f12 = f12 / (column3.u_index - column1.u_index);

                                if (f12 >= column2.vtx[1].Z && column2.vtx[1].Z + f2 > f12) {
                                    deleted = true;
                                }
                            }

                            if (deleted) {
                                column2.deleted = true;
                            }
                        }
                    }
                }
            } else {
                column2 = column3;
            }
        }

        if (!column2.deleted && i != 1) {
            for (int j = 0; j < i2; j++) {
                if (numVertex >= 2000) {
                    break;
                }

                v1.Set(column2.vtx[j].X - loc.X, column2.vtx[j].Y - loc.Y);
                float f13 = Vector2::Dot_Product(road_normal, v1);
                float f14 = Vector2::Dot_Product(road_vector, v1);
                vb[numVertex].u1 = f14 / (uscale * 4.0f) + uoffset;
                vb[numVertex].v1 = voffset - f13 / (vscale * 4.0f);
                vb[numVertex].x = column2.vtx[j].X;
                vb[numVertex].y = column2.vtx[j].Y;
                vb[numVertex].z = f1 + column2.vtx[j].Z;
                vb[numVertex].diffuse = 0;
                column2.vertex_index[j] = numVertex++;

                if (j == 1) {
                    if (column2.collapsed) {
                        break;
                    }
                }
            }

            if (numVertex >= 2000) {
                break;
            }

            if (i <= 1) {
                if (!i) {
                    column1 = column2;
                }
            } else {
                int i4 = 0;
                int i5 = 0;

                while (i4 < i2 - 1) {
                    if (i5 >= i2 - 1) {
                        break;
                    }

                    if (numIndex >= MAX_SEG_INDEX) {
                        break;
                    }

                    unsigned short *s = &ib[numIndex];

                    if (!i5 || !column1.collapsed) {
                        *s = column1.vertex_index[i4 + 1];
                        s++;
                        *s = column1.vertex_index[i4];
                        s++;
                        *s = column2.vertex_index[i5];
                        s++;
                        numIndex += 3;
                    }

                    if (!i4 || !column2.collapsed) {
                        int i6 = 1;

                        if (column2.collapsed) {
                            if (!column1.collapsed) {
                                i6 = i2 - 1;
                            }
                        }

                        *s = column1.vertex_index[i6 + i4];
                        s++;
                        *s = column2.vertex_index[i5];
                        s++;
                        *s = column2.vertex_index[i5 + 1];
                        s++;
                        numIndex += 3;
                    }

                    if (column1.collapsed && column2.collapsed) {
                        break;
                    }

                    if (!column1.collapsed) {
                        i4++;
                    }

                    if (!column2.collapsed) {
                        i5++;
                    }
                }

                column1 = column2;
            }

            if (numIndex >= MAX_SEG_INDEX) {
                break;
            }
        }

        column2 = column3;
    }

    road->Set_Vertex_Buffer(vb, numVertex);
    road->Set_Index_Buffer(ib, numIndex);
}

void W3DRoadBuffer::Load_H(RoadSegment *road, Vector2 loc1, Vector2 loc2, bool flip, float scale)
{
    Vector2 v1(loc2.X - loc1.X, loc2.Y - loc1.Y);
    Vector2 v2(-v1.Y, v1.X);

    if ((float)abs((int)v1.X) >= 0.25f || (float)abs((int)v1.Y) >= 0.25f) {
        v1.Normalize();
        v2.Normalize();
    } else {
        v1.Set(1, 0);
        v2.Set(0, 1);
    }

    float uOffset = 0.39453125f;
    float vOffset = 0.7109375f;
    v1 *= scale;
    v2 *= scale;
    Vector2 corners[4];
    v2 *= 1.35f;

    if (flip) {
        Vector2 v3 = v1 * road->m_widthInTexture;
        Vector2 v4 = v3 / 2.0f;
        Vector2 v5 = v2 * 0.2f;
        Vector2 v6 = loc1 - v5;
        Vector2 v7 = v6 - v4;
        corners[0] = v7;
    } else {
        Vector2 v3 = v1 * road->m_widthInTexture;
        Vector2 v4 = v3 / 2.0f;
        Vector2 v5 = v2 * 0.8f;
        Vector2 v6 = loc1 - v5;
        Vector2 v7 = v6 - v4;
        corners[0] = v7;
    }

    Vector2 v8 = v1 * road->m_widthInTexture;
    Vector2 v9 = v8 / 2.0f;
    Vector2 v10 = v1 * 1.2f;
    Vector2 v11 = v9 + v10;
    v9 = v11;
    corners[1] = corners[0] + v9;
    corners[3] = corners[1] + v2;
    corners[2] = corners[0] + v2;

    if (flip) {
        v2 = -v2;
    }

    Load_Float_4Pt_Section(road, loc1, v2, v1, corners, uOffset, vOffset, scale, scale);
}

void W3DRoadBuffer::Load_Float_Section(RoadSegment *road,
    Vector2 loc,
    Vector2 road_vector,
    float half_height,
    float left,
    float right,
    float uoffset,
    float voffset,
    float scale)
{
    if (m_map) {
        Vector2 v1(-road_vector.Y, road_vector.X);
        road_vector.Normalize();
        road_vector *= right;
        v1.Normalize();

        if (half_height < 0.0f) {
            half_height = -half_height;
        }

        v1 *= half_height;
        Vector2 v2(road_vector);
        v2.Normalize();
        v2 *= left;
        road_vector += v2;
        Vector2 v3(loc);
        v3 -= v2;
        Vector2 corners[4];
        corners[0] = v3 - v1;
        corners[1] = corners[0];
        corners[1] += road_vector;
        corners[3] = corners[1];
        corners[3] += 2.0f * v1;
        corners[2] = corners[0];
        corners[2] += 2.0f * v1;
        Load_Float_4Pt_Section(road, loc, v1, road_vector, corners, uoffset, voffset, scale, scale);
    }
}

void W3DRoadBuffer::Load_Curve(RoadSegment *road, Vector2 loc1, Vector2 loc2, float scale)
{
    float uOffset = 0.0078125f;
    float vOffset = 0.49804688f;

    if (road->m_curveRadius == 0.5f) {
        vOffset = 0.83007812f;
    }

    Vector2 v1(loc2.X - loc1.X, loc2.Y - loc1.Y);
    Vector2 v2(-v1.Y, v1.X);
    v1.Normalize();
    v1 *= scale;
    v2.Normalize();
    v2 *= (float)abs((int)(scale * road->m_widthInTexture / 2.0f));
    Vector2 corners[4];
    corners[0] = loc1 - v2;
    corners[1] = corners[0];
    corners[1] += v1;
    corners[3] = corners[1];
    corners[3] += 2.0f * v2;
    corners[2] = corners[0];
    corners[2] += 2.0f * v2;

    if (road->m_curveRadius == 0.5f) {
        corners[0] = loc1 - v2;
        corners[1] = corners[0];
        corners[1] += v1 * 0.5f;
        corners[3] = corners[1];
        corners[3] += 2.0f * v2;
        corners[2] = corners[0];
        corners[2] += 2.0f * v2;
        corners[1] += v1 * 0.1f;
        corners[1] += v2 * 0.2f;
        corners[0] -= v2 * 0.1f;
        corners[0] -= v1 * 0.02f;
        corners[2] -= v1 * 0.02f;
        corners[3] -= v1 * 0.4f;
        corners[3] += v2 * 0.2f;
    } else {
        corners[0] = loc1 - v2;
        corners[1] = corners[0];
        corners[1] += v1;
        corners[3] = corners[1];
        corners[3] += 2.0f * v2;
        corners[2] = corners[0];
        corners[2] += 2.0f * v2;
        corners[1] += v1 * 0.1f;
        corners[1] += v2 * 0.4f;
        corners[0] -= v2 * 0.2f;
        corners[0] -= v1 * 0.02f;
        corners[2] -= v1 * 0.02f;
        corners[3] -= v1 * 0.4f;
        corners[3] += v2 * 0.4f;
    }

    Load_Float_4Pt_Section(road, loc1, v2, v1, corners, uOffset, vOffset, scale, scale);
}

void W3DRoadBuffer::Update_Counts(RoadSegment *road)
{
    road->m_pt1.last = true;
    road->m_pt2.last = true;
    road->m_pt1.multi = false;
    road->m_pt2.multi = false;
    road->m_pt1.count = 0;
    road->m_pt2.count = 0;
    Vector2 v1;
    Vector2 v2;
    v1 = road->m_pt1.loc;
    v2 = road->m_pt2.loc;

    for (int i = 0; i < m_numRoads; i++) {
        if (m_roads[i].m_uniqueID == road->m_uniqueID) {
            if (m_roads[i].m_pt1.loc == v1) {
                m_roads[i].m_pt1.count++;
                road->m_pt1.count++;
            }

            if (m_roads[i].m_pt1.loc == v2) {
                m_roads[i].m_pt1.count++;
                road->m_pt2.count++;
            }

            if (m_roads[i].m_pt2.loc == v1) {
                m_roads[i].m_pt2.count++;
                road->m_pt1.count++;
            }

            if (m_roads[i].m_pt2.loc == v2) {
                m_roads[i].m_pt2.count++;
                road->m_pt2.count++;
            }

            m_roads[i].m_pt1.multi = m_roads[i].m_pt1.count > 1;
            m_roads[i].m_pt2.multi = m_roads[i].m_pt2.count > 1;
        }
    }

    road->m_pt1.multi = road->m_pt1.count > 1;
    road->m_pt2.multi = road->m_pt2.count > 1;
}

void W3DRoadBuffer::Update_Counts_And_Flags()
{
    for (int i = 0; i < m_numRoads; i++) {
        m_roads[i].m_pt1.last = true;
        m_roads[i].m_pt2.last = true;
        m_roads[i].m_pt1.count = 0;
        m_roads[i].m_pt2.count = 0;
    }

    for (int i = m_numRoads - 1; i > 0; i--) {
        Vector2 v1;
        Vector2 v2;
        v1 = m_roads[i].m_pt1.loc;
        v2 = m_roads[i].m_pt2.loc;

        for (int j = 0; j < i; j++) {
            if (m_roads[j].m_uniqueID == m_roads[i].m_uniqueID) {
                if (m_roads[j].m_pt1.loc == v1) {
                    m_roads[j].m_pt1.last = false;
                    m_roads[j].m_pt1.count++;
                    m_roads[i].m_pt1.count++;
                }

                if (m_roads[j].m_pt1.loc == v2) {
                    m_roads[j].m_pt1.last = false;
                    m_roads[j].m_pt1.count++;
                    m_roads[i].m_pt2.count++;
                }

                if (m_roads[j].m_pt2.loc == v1) {
                    m_roads[j].m_pt2.last = false;
                    m_roads[j].m_pt2.count++;
                    m_roads[i].m_pt1.count++;
                }

                if (m_roads[j].m_pt2.loc == v2) {
                    m_roads[j].m_pt2.last = false;
                    m_roads[j].m_pt2.count++;
                    m_roads[i].m_pt2.count++;
                }
            }
        }
    }
}

int W3DRoadBuffer::Find_Cross_Type_Join_Vector(Vector2 loc, Vector2 *join_vector, int unique_id)
{
    Vector2 v1 = *join_vector;
    int count = m_numRoads;
    for (int i = 0; i < count; i++) {
        if (m_roads[i].m_uniqueID != unique_id && m_roads[i].m_type == SEGMENT) {
            Vector2 v2;
            Vector2 v3;
            v2 = m_roads[i].m_pt1.loc;
            v3 = m_roads[i].m_pt2.loc;
            Vector2 v4 = v2;
            Vector2 v5 = v2;

            if (v4.X > v3.X) {
                v4.X = v3.X;
            }

            if (v4.Y > v3.Y) {
                v4.Y = v3.Y;
            }

            if (v5.X < v3.X) {
                v5.X = v3.X;
            }

            if (v5.Y < v3.Y) {
                v5.Y = v3.Y;
            }

            v4.X = v4.X - m_roads[i].m_scale / 2.0f;
            v4.Y = v4.Y - m_roads[i].m_scale / 2.0f;
            v5.X = m_roads[i].m_scale / 2.0f + v5.X;
            v5.Y = m_roads[i].m_scale / 2.0f + v5.Y;

            if (loc.X >= v4.X && loc.Y >= v4.Y && loc.X <= v5.X && loc.Y <= v5.Y) {
                Vector3 v6(v2.X, v2.Y, 0.0f);
                Vector3 v7(v3.X, v3.Y, 0.0f);
                LineSegClass line(v6, v7);
                Vector3 v8(loc.X, loc.Y, 0.0f);
                Vector3 v9 = line.Find_Point_Closest_To(v8);

                if (m_roads[i].m_scale * 0.55f > Vector3::Distance(v9, v8)) {
                    Vector2 v10 = v3 - v2;
                    if (xp_sign(v10, *join_vector) == 1) {
                        v10.Rotate(DEG_TO_RADF(90.f));
                    } else {
                        v10.Rotate(DEG_TO_RADF(-90.f));
                    }

                    v1 = v10;
                    *join_vector = v1;
                    return m_roads[i].m_uniqueID;
                }
            }
        }
    }
    return 0;
}

void W3DRoadBuffer::Insert_Cross_Type_Joins()
{
    int count = m_numRoads;
    for (int i = 0; i < count; i++) {
        Vector2 v1;
        Vector2 v2;
        bool b = false;
        if (m_roads[i].m_pt2.count || !m_roads[i].m_pt2.is_join) {
            if (m_roads[i].m_pt1.count || !m_roads[i].m_pt1.is_join) {
                continue;
            }

            v1 = m_roads[i].m_pt1.loc;
            v2 = m_roads[i].m_pt2.loc;
            b = true;
        } else {
            v1 = m_roads[i].m_pt2.loc;
            v2 = m_roads[i].m_pt1.loc;
        }

        Vector2 v3(1.0f, 0.0f);
        Vector2 v4(v2.X - v1.X, v2.Y - v1.Y);
        v4.Normalize();
        v3 = v4;
        int bottomUniqueID = Find_Cross_Type_Join_Vector(v1, &v3, m_roads[i].m_uniqueID);

        if (!bottomUniqueID) {
            v3 *= 100.0f;
        }

        Vector2 v5(-v4.Y, v4.X);
        Vector2 v6(-v3.Y, v3.X);
        Vector2 v7 = v5 * m_roads[i].m_scale;
        Vector2 v8 = v7 * m_roads[i].m_widthInTexture;
        Vector2 v9 = v8 / 2.0f;
        Vector2 v10 = v1 + v9;
        Vector2 v11 = v5 * m_roads[i].m_scale;
        Vector2 v12 = v11 * m_roads[i].m_widthInTexture;
        Vector2 v13 = v12 / 2.0f;
        Vector2 v14 = v2 + v13;
        LineSegClass line1(Vector3(v10.X, v10.Y, 0.0f), Vector3(v14.X, v14.Y, 0.0f));
        Vector3 v15(v1.X, v1.Y, 0.0f);
        Vector3 v16(v6.X, v6.Y, 0.0f);
        Vector3 v17 = v16 + v15;
        LineSegClass line2(v15, v17);
        Vector3 p1;
        Vector3 p2;
        Vector2 v18(m_roads[i].m_pt1.top);
        float f1;

        if (line2.Find_Intersection(line1, &p1, &f1, &p2, &f1)) {
            if (b) {
                m_roads[i].m_pt1.top.Set(p1.X, p1.Y);
                v18 = m_roads[i].m_pt1.top;
            } else {
                m_roads[i].m_pt2.bottom.Set(p1.X, p1.Y);
                v18 = m_roads[i].m_pt2.bottom;
            }
        }

        Vector2 v19 = v5 * m_roads[i].m_scale;
        Vector2 v20 = v19 * m_roads[i].m_widthInTexture;
        Vector2 v21 = v20 / 2.0f;
        v10 = v1 - v21;
        Vector2 v22 = v5 * m_roads[i].m_scale;
        Vector2 v23 = v22 * m_roads[i].m_widthInTexture;
        Vector2 v24 = v23 / 2.0f;
        v14 = v2 - v24;
        line1.Set(Vector3(v10.X, v10.Y, 0.0f), Vector3(v14.X, v14.Y, 0.0f));
        Vector2 v25 = m_roads[i].m_pt1.bottom;

        if (line2.Find_Intersection(line1, &p1, &f1, &p2, &f1)) {
            if (b) {
                m_roads[i].m_pt1.bottom.Set(p1.X, p1.Y);
                v25 = m_roads[i].m_pt1.bottom;
            } else {
                m_roads[i].m_pt2.top.Set(p1.X, p1.Y);
                v25 = m_roads[i].m_pt2.top;
            }
        }

        v25 = v25 - v18;
        float f2 = v25.Length() / (m_roads[i].m_scale * m_roads[i].m_widthInTexture);

        if (bottomUniqueID) {
            Adjust_Stacking(m_roads[i].m_uniqueID, bottomUniqueID);
        }

        if (m_numRoads >= m_maxRoadSegments) {
            return;
        }

        m_roads[m_numRoads].m_pt1.loc.Set(v1);
        m_roads[m_numRoads].m_pt2.loc.Set(v1 + v3);
        m_roads[m_numRoads].m_pt1.last = true;
        m_roads[m_numRoads].m_pt2.last = true;
        m_roads[m_numRoads].m_scale = m_roads[i].m_scale;
        m_roads[m_numRoads].m_widthInTexture = f2 * m_roads[i].m_scale;
        m_roads[m_numRoads].m_pt1.count = 0;
        m_roads[m_numRoads].m_type = ALPHA_JOIN;
        m_roads[m_numRoads].m_uniqueID = m_roads[i].m_uniqueID;
        m_numRoads++;
    }
}

void W3DRoadBuffer::Miter(int ndx1, int ndx2)
{
    LineSegClass line1(Vector3(m_roads[ndx1].m_pt1.top.X, m_roads[ndx1].m_pt1.top.Y, 0.0f),
        Vector3(m_roads[ndx1].m_pt2.top.X, m_roads[ndx1].m_pt2.top.Y, 0.0f));
    LineSegClass line2(Vector3(m_roads[ndx2].m_pt1.top.X, m_roads[ndx2].m_pt1.top.Y, 0.0f),
        Vector3(m_roads[ndx2].m_pt2.top.X, m_roads[ndx2].m_pt2.top.Y, 0.0f));
    Vector3 p1;
    Vector3 p2;
    float f1;

    if (line1.Find_Intersection(line2, &p1, &f1, &p2, &f1)) {
        m_roads[ndx2].m_pt2.top.X = p1.X;
        m_roads[ndx2].m_pt2.top.Y = p1.Y;
        m_roads[ndx1].m_pt1.top.X = p1.X;
        m_roads[ndx1].m_pt1.top.Y = p1.Y;
    }

    line1 = LineSegClass(Vector3(m_roads[ndx1].m_pt1.bottom.X, m_roads[ndx1].m_pt1.bottom.Y, 0.0f),
        Vector3(m_roads[ndx1].m_pt2.bottom.X, m_roads[ndx1].m_pt2.bottom.Y, 0.0f));
    line2 = LineSegClass(Vector3(m_roads[ndx2].m_pt1.bottom.X, m_roads[ndx2].m_pt1.bottom.Y, 0.0f),
        Vector3(m_roads[ndx2].m_pt2.bottom.X, m_roads[ndx2].m_pt2.bottom.Y, 0.0f));

    if (line1.Find_Intersection(line2, &p1, &f1, &p2, &f1)) {
        m_roads[ndx2].m_pt2.bottom.X = p1.X;
        m_roads[ndx2].m_pt2.bottom.Y = p1.Y;
        m_roads[ndx1].m_pt1.bottom.X = p1.X;
        m_roads[ndx1].m_pt1.bottom.Y = p1.Y;
    }
}

void W3DRoadBuffer::Insert_Tee(Vector2 loc, int index1, float scale)
{
    if (!Insert_Y(loc, index1, scale)) {
        TRoadPt *pt1 = nullptr;
        TRoadPt *pt2 = nullptr;
        TRoadPt *pt3 = nullptr;
        TRoadPt *pt4 = nullptr;
        TRoadPt *pt5 = nullptr;
        TRoadPt *pt6 = nullptr;

        if (m_roads[index1].m_pt1.loc == loc) {
            pt1 = &m_roads[index1].m_pt2;
            pt4 = &m_roads[index1].m_pt1;
        } else {
            pt1 = &m_roads[index1].m_pt1;
            pt4 = &m_roads[index1].m_pt2;
        }

        int i1 = 0;
        int i2 = 0;

        for (int i = index1 + 1; i < m_numRoads; i++) {
            if (m_roads[i].m_pt1.loc == loc) {
                m_roads[i].m_pt1.count = -2;

                if (pt2) {
                    pt3 = &m_roads[i].m_pt2;
                    pt6 = &m_roads[i].m_pt1;
                    i2 = i;
                } else {
                    pt2 = &m_roads[i].m_pt2;
                    pt5 = &m_roads[i].m_pt1;
                    i1 = i;
                }
            }

            if (m_roads[i].m_pt2.loc == loc) {
                m_roads[i].m_pt2.count = -2;

                if (pt2) {
                    pt3 = &m_roads[i].m_pt1;
                    pt6 = &m_roads[i].m_pt2;
                    i2 = i;
                } else {
                    pt2 = &m_roads[i].m_pt1;
                    pt5 = &m_roads[i].m_pt2;
                    i1 = i;
                }
            }
        }

        if (pt2) {
            if (pt3) {
                Vector2 v1 = pt1->loc - loc;
                v1.Normalize();
                Vector2 v2 = pt2->loc - loc;
                v2.Normalize();
                Vector2 v3 = pt3->loc - loc;
                v3.Normalize();
                float f1 = Vector2::Dot_Product(v1, v2);
                float f2 = Vector2::Dot_Product(v1, v3);
                float f3 = Vector2::Dot_Product(v3, v2);
                bool b1 = false;
                bool b2 = false;
                bool b3 = false;

                if (f1 >= f2) {
                    if (f2 >= f3) {
                        b3 = true;
                    } else {
                        b2 = true;
                    }
                } else if (f1 >= f3) {
                    b3 = true;
                } else {
                    b1 = true;
                }

                Vector2 v4;
                Vector2 v5;

                if (b1) {
                    v4 = v2 - v1;
                    v5 = v3;
                }

                if (b2) {
                    v4 = v3 - v1;
                    v5 = v2;
                }

                if (b3) {
                    v4 = v2 - v3;
                    v5 = v1;
                }

                v4.Normalize();

                if (Fabs(Vector2::Dot_Product(v4, v5)) <= 0.5f) {
                    float f4 = DEG_TO_RADF(90.f);

                    if (Vector3::Cross_Product_Z(Vector3(v4.X, v4.Y, 0.0f), Vector3(v5.X, v5.Y, 0.0f)) < 0.0f) {
                        f4 = -f4;
                    }

                    v4.Normalize();
                    v4 *= scale * 0.5f;
                    Vector2 v6(v4);
                    v6.Rotate(f4);

                    if (b1) {
                        Offset_3Way(pt4, pt5, pt6, loc, v4, v6, m_roads[index1].m_widthInTexture);
                    }

                    if (b2) {
                        Offset_3Way(pt4, pt6, pt5, loc, v4, v6, m_roads[index1].m_widthInTexture);
                    }

                    if (b3) {
                        Offset_3Way(pt6, pt5, pt4, loc, v4, v6, m_roads[index1].m_widthInTexture);
                    }

                    pt4->last = true;
                    pt4->count = 0;
                    pt5->last = true;
                    pt5->count = 0;
                    pt6->last = true;
                    pt6->count = 0;

                    if (m_numRoads < m_maxRoadSegments) {
                        m_roads[m_numRoads].m_pt1.loc.Set(loc);
                        m_roads[m_numRoads].m_pt2.loc.Set(loc + v6);
                        m_roads[m_numRoads].m_pt1.last = true;
                        m_roads[m_numRoads].m_pt2.last = true;
                        m_roads[m_numRoads].m_scale = m_roads[index1].m_scale;
                        m_roads[m_numRoads].m_widthInTexture = m_roads[index1].m_widthInTexture;
                        m_roads[m_numRoads].m_pt1.count = -3;
                        m_roads[m_numRoads].m_type = TEE;
                        m_roads[m_numRoads].m_uniqueID = m_roads[index1].m_uniqueID;
                        m_numRoads++;
                    }
                } else {
                    float f4 = DEG_TO_RADF(90.f);
                    bool mirror = false;

                    if (Vector3::Cross_Product_Z(Vector3(v4.X, v4.Y, 0.0f), Vector3(v5.X, v5.Y, 0.0f)) < 0.0f) {
                        f4 = -f4;
                        mirror = true;
                    }

                    v4.Normalize();
                    v4 *= scale * 0.5f;
                    Vector2 v6(v4);
                    v6.Rotate(f4);

                    bool flip;

                    if (b1) {
                        flip = xp_sign(v6, v3) == 1;
                        Offset_H(pt4, pt5, pt6, loc, v4, v6, flip, mirror, m_roads[index1].m_widthInTexture);
                    }

                    if (b2) {
                        flip = xp_sign(v6, v2) == 1;
                        Offset_H(pt4, pt6, pt5, loc, v4, v6, flip, mirror, m_roads[index1].m_widthInTexture);
                    }

                    if (b3) {
                        flip = xp_sign(v6, v1) == 1;
                        Offset_H(pt6, pt5, pt4, loc, v4, v6, flip, mirror, m_roads[index1].m_widthInTexture);
                    }

                    pt4->last = true;
                    pt4->count = 0;
                    pt5->last = true;
                    pt5->count = 0;
                    pt6->last = true;
                    pt6->count = 0;

                    if (m_numRoads < m_maxRoadSegments) {
                        m_roads[m_numRoads].m_pt1.loc.Set(loc);
                        m_roads[m_numRoads].m_pt2.loc.Set(loc + v6);
                        m_roads[m_numRoads].m_pt1.last = true;
                        m_roads[m_numRoads].m_pt2.last = true;
                        m_roads[m_numRoads].m_scale = m_roads[index1].m_scale;
                        m_roads[m_numRoads].m_widthInTexture = m_roads[index1].m_widthInTexture;
                        m_roads[m_numRoads].m_pt1.count = -3;
                        m_roads[m_numRoads].m_type = flip ? THREE_WAY_H_FLIP : THREE_WAY_H;
                        m_roads[m_numRoads].m_uniqueID = m_roads[index1].m_uniqueID;
                        m_numRoads++;
                    }
                }
            }
        }
    }
}

bool W3DRoadBuffer::Insert_Y(Vector2 loc, int index1, float scale)
{
    TRoadPt *pt1 = nullptr;
    TRoadPt *pt2 = nullptr;
    TRoadPt *pt3 = nullptr;
    TRoadPt *pt4 = nullptr;
    TRoadPt *pt5 = nullptr;
    TRoadPt *pt6 = nullptr;

    if (m_roads[index1].m_pt1.loc == loc) {
        pt1 = &m_roads[index1].m_pt2;
        pt4 = &m_roads[index1].m_pt1;
    } else {
        pt1 = &m_roads[index1].m_pt1;
        pt4 = &m_roads[index1].m_pt2;
    }

    int i1;
    int i2;

    for (int i = index1 + 1; i < m_numRoads; i++) {
        if (m_roads[i].m_pt1.loc == loc) {
            m_roads[i].m_pt1.count = -2;

            if (pt2) {
                pt3 = &m_roads[i].m_pt2;
                pt6 = &m_roads[i].m_pt1;
                i2 = i;
            } else {
                pt2 = &m_roads[i].m_pt2;
                pt5 = &m_roads[i].m_pt1;
                i1 = i;
            }
        }

        if (m_roads[i].m_pt2.loc == loc) {
            m_roads[i].m_pt2.count = -2;

            if (pt2) {
                pt3 = &m_roads[i].m_pt1;
                pt6 = &m_roads[i].m_pt2;
                i2 = i;
            } else {
                pt2 = &m_roads[i].m_pt1;
                pt5 = &m_roads[i].m_pt2;
                i1 = i;
            }
        }
    }

    if (!pt2 || !pt3) {
        return false;
    }

    Vector2 v1 = pt1->loc - loc;
    v1.Normalize();
    Vector2 v2 = pt2->loc - loc;
    v2.Normalize();
    Vector2 v3 = pt3->loc - loc;
    v3.Normalize();
    bool b1 = false;
    bool b2 = false;
    bool b3 = false;
    float f1 = Vector2::Dot_Product(v1, v2);
    float f2 = Vector2::Dot_Product(v1, v3);
    float f3 = Vector2::Dot_Product(v3, v2);
    float f4 = 2.0f;
    float f5 = 2.0f;
    float f6 = 2.0f;

    if (-0.866 > f1) {
        return false;
    }

    if (-0.866 > f2) {
        return false;
    }

    if (-0.866 > f3) {
        return false;
    }

    int i3 = 0;
    int i4 = xp_sign(v1, v2);
    int i5 = xp_sign(v1, v3);

    if (i4 != i5 && !(i5 + i4)) {
        Vector2 v4(-v1.Y, v1.X);
        if (xp_sign(v4, v2) == 1 && xp_sign(v4, v3) == 1) {
            b3 = true;
            f6 = Fabs(f2 + 0.70700002f) + Fabs(f1 + 0.70700002f);
        }
    }

    i3 = xp_sign(v3, v1);
    i4 = xp_sign(v3, v2);

    if (i4 != i3 && !(i3 + i4)) {
        Vector2 v4(-v3.Y, v3.X);
        if (xp_sign(v4, v2) == 1 && xp_sign(v4, v1) == 1) {
            b1 = true;
            f4 = Fabs(f3 + 0.70700002f) + Fabs(f2 + 0.70700002f);
        }
    }

    i3 = xp_sign(v2, v1);
    i5 = xp_sign(v2, v3);

    if (i5 != i3 && !(i3 + i5)) {
        Vector2 v4(-v3.Y, v3.X);
        if (xp_sign(v4, v3) == 1 && xp_sign(v4, v1) == 1) {
            b2 = true;
            f5 = Fabs(f3 + 0.70700002f) + Fabs(f1 + 0.70700002f);
        }
    }

    if (f4 >= f5) {
        b1 = false;

        if (f5 >= f6) {
            b2 = false;
        } else {
            b3 = false;
        }
    } else {
        b2 = false;

        if (f4 >= f6) {
            b1 = false;
        } else {
            b3 = false;
        }
    }

    Vector2 v5;

    if (b1) {
        v5 = v3;
    } else if (b2) {
        v5 = v2;
    } else {
        if (!b3) {
            return false;
        }

        v5 = v1;
    }

    v5.Normalize();
    v5 *= scale * 0.5f;
    Vector2 v6(v5);
    v6.Rotate(DEG_TO_RADF(-90.f));

    if (b1) {
        if (xp_sign(v3, v1) == -1) {
            Offset_Y(pt4, pt5, pt6, loc, v5, m_roads[index1].m_widthInTexture);
        } else {
            Offset_Y(pt5, pt4, pt6, loc, v5, m_roads[index1].m_widthInTexture);
        }
    }

    if (b2) {
        if (xp_sign(v2, v1) == -1) {
            Offset_Y(pt4, pt6, pt5, loc, v5, m_roads[index1].m_widthInTexture);
        } else {
            Offset_Y(pt6, pt4, pt5, loc, v5, m_roads[index1].m_widthInTexture);
        }
    }

    if (b3) {
        if (xp_sign(v1, v3) == -1) {
            Offset_Y(pt6, pt5, pt4, loc, v5, m_roads[index1].m_widthInTexture);
        } else {
            Offset_Y(pt5, pt6, pt4, loc, v5, m_roads[index1].m_widthInTexture);
        }
    }

    pt4->last = true;
    pt4->count = 0;
    pt5->last = true;
    pt5->count = 0;
    pt6->last = true;
    pt6->count = 0;

    if (m_numRoads >= m_maxRoadSegments) {
        return false;
    }

    m_roads[m_numRoads].m_pt1.loc = loc;
    m_roads[m_numRoads].m_pt2.loc = loc + v6;
    m_roads[m_numRoads].m_pt1.last = true;
    m_roads[m_numRoads].m_pt2.last = true;
    m_roads[m_numRoads].m_scale = m_roads[index1].m_scale;
    m_roads[m_numRoads].m_widthInTexture = m_roads[index1].m_widthInTexture;
    m_roads[m_numRoads].m_pt1.count = -3;
    m_roads[m_numRoads].m_type = THREE_WAY_Y;
    m_roads[m_numRoads].m_uniqueID = m_roads[index1].m_uniqueID;
    m_numRoads++;
    return true;
}

void W3DRoadBuffer::Insert_4Way(Vector2 loc, int index1, float scale)
{

    TRoadPt *pr1 = nullptr;
    TRoadPt *pr2 = nullptr;
    TRoadPt *pr3 = nullptr;
    TRoadPt *pr4 = nullptr;

    TRoadPt *pc1 = nullptr;
    TRoadPt *pc2 = nullptr;
    TRoadPt *pc3 = nullptr;
    TRoadPt *pc4 = nullptr;

    if (m_roads[index1].m_pt1.loc == loc) {
        pr1 = &m_roads[index1].m_pt2;
        pc1 = &m_roads[index1].m_pt1;
    } else {
        pr1 = &m_roads[index1].m_pt1;
        pc1 = &m_roads[index1].m_pt2;
    }

    for (int i = index1 + 1; i < m_numRoads; i++) {
        if (m_roads[i].m_pt1.loc == loc) {
            m_roads[i].m_pt1.count = -2;

            if (!pr2) {
                pr2 = &m_roads[i].m_pt2;
                pc2 = &m_roads[i].m_pt1;
            } else if (!pr3) {
                pr3 = &m_roads[i].m_pt2;
                pc3 = &m_roads[i].m_pt1;
            } else {
                pr4 = &m_roads[i].m_pt2;
                pc4 = &m_roads[i].m_pt1;
            }
        }

        if (m_roads[i].m_pt2.loc == loc) {
            m_roads[i].m_pt2.count = -2;

            if (!pr2) {
                pr2 = &m_roads[i].m_pt1;
                pc2 = &m_roads[i].m_pt2;
            } else if (!pr3) {
                pr3 = &m_roads[i].m_pt1;
                pc3 = &m_roads[i].m_pt2;
            } else {
                pr4 = &m_roads[i].m_pt1;
                pc4 = &m_roads[i].m_pt2;
            }
        }
    }

    if (pr2) {
        if (pr3) {
            if (pr4) {
                Vector2 v1 = pr1->loc - loc;
                v1.Normalize();
                Vector2 v2 = pr2->loc - loc;
                v2.Normalize();
                Vector2 v3 = pr3->loc - loc;
                v3.Normalize();
                Vector2 v4 = pr4->loc - loc;
                v4.Normalize();
                float dot12 = Vector2::Dot_Product(v1, v2);
                float dot13 = Vector2::Dot_Product(v1, v3);
                float dot14 = Vector2::Dot_Product(v1, v4);
                float dot23 = Vector2::Dot_Product(v2, v3);
                float dot24 = Vector2::Dot_Product(v2, v4);
                float dot34 = Vector2::Dot_Product(v3, v4);
                int dp = 12;
                float dot = dot12;

                if (dot13 < dot12) {
                    dp = 13;
                    dot = dot13;
                }

                if (dot14 < dot) {
                    dp = 14;
                    dot = dot14;
                }

                if (dot23 < dot) {
                    dp = 23;
                    dot = dot23;
                }

                if (dot24 < dot) {
                    dp = 24;
                    dot = dot24;
                }

                if (dot34 < dot) {
                    dp = 34;
                    dot = dot34;
                }

                bool do12 = dp == 12;
                bool do13 = dp == 13;
                bool do14 = dp == 14;
                bool do23 = dp == 23;
                bool do24 = dp == 24;
                bool do34 = dp == 34;

                Vector2 align_vector;

                if (do12) {
                    align_vector = v2 - v1;
                }

                if (do13) {
                    align_vector = v3 - v1;
                }

                if (do14) {
                    align_vector = v4 - v1;
                }

                if (do23) {
                    align_vector = v3 - v2;
                }

                if (do24) {
                    align_vector = v4 - v2;
                }

                if (do34) {
                    align_vector = v4 - v3;
                }

                align_vector.Normalize();
                align_vector *= scale * 0.5f;

                if (do12) {
                    Offset_4Way(pc1, pc2, pc3, pr3, pc4, loc, align_vector, m_roads[index1].m_widthInTexture);
                }

                if (do13) {
                    Offset_4Way(pc1, pc3, pc2, pr2, pc4, loc, align_vector, m_roads[index1].m_widthInTexture);
                }

                if (do14) {
                    Offset_4Way(pc1, pc4, pc3, pr3, pc2, loc, align_vector, m_roads[index1].m_widthInTexture);
                }

                if (do23) {
                    Offset_4Way(pc2, pc3, pc1, pr1, pc4, loc, align_vector, m_roads[index1].m_widthInTexture);
                }

                if (do24) {
                    Offset_4Way(pc2, pc4, pc1, pr1, pc3, loc, align_vector, m_roads[index1].m_widthInTexture);
                }

                if (do34) {
                    Offset_4Way(pc3, pc4, pc1, pr1, pc2, loc, align_vector, m_roads[index1].m_widthInTexture);
                }

                if (align_vector.X < 0.0f) {
                    align_vector.X = -align_vector.X;
                    align_vector.Y = -align_vector.Y;
                }

                if (m_numRoads < m_maxRoadSegments) {
                    m_roads[m_numRoads].m_pt1.loc.Set(loc);
                    m_roads[m_numRoads].m_pt2.loc.Set(loc + align_vector);
                    m_roads[m_numRoads].m_pt1.last = true;
                    m_roads[m_numRoads].m_pt2.last = true;
                    m_roads[m_numRoads].m_scale = m_roads[index1].m_scale;
                    m_roads[m_numRoads].m_widthInTexture = 1.03f;
                    m_roads[m_numRoads].m_pt1.count = -4;
                    m_roads[m_numRoads].m_type = FOUR_WAY;
                    m_roads[m_numRoads++].m_uniqueID = m_roads[index1].m_uniqueID;
                } else {
                    if (g_warn_segments) {
                        captainslog_debug("****** Too many road segments.  Need to increase ini values");
                    }
                    g_warn_segments = false;
                }
            }
        }
    }
}

void W3DRoadBuffer::Offset_4Way(TRoadPt *pc1,
    TRoadPt *pc2,
    TRoadPt *pc3,
    TRoadPt *pr3,
    TRoadPt *pc4,
    Vector2 loc,
    Vector2 align_vector,
    float width_in_texture)
{
    pc1->loc = loc - align_vector;
    pc2->loc = loc + align_vector;
    Vector2 v1 = pr3->loc - loc;
    float f1 = DEG_TO_RADF(90.f);

    if (Vector3::Cross_Product_Z(Vector3(align_vector.X, align_vector.Y, 0.0f), Vector3(v1.X, v1.Y, 0.0f)) < 0.0f) {
        f1 = -f1;
    }

    Vector2 v2(align_vector);
    v2.Rotate(f1);
    pc3->loc = loc + v2;
    pc4->loc = loc - v2;
    Vector2 v3(align_vector);
    v3.Rotate(DEG_TO_RADF(90.f));
    v3 *= width_in_texture;

    if (Vector3::Cross_Product_Z(
            Vector3(align_vector.X, align_vector.Y, 0.0f), Vector3(pc1->top.X - pc1->loc.X, pc1->top.Y - pc1->loc.Y, 0.0f))
        > 0.0f) {
        pc1->bottom = pc1->loc - v3;
        pc1->top = pc1->loc + v3;
    } else {
        pc1->bottom = pc1->loc + v3;
        pc1->top = pc1->loc - v3;
    }

    if (Vector3::Cross_Product_Z(
            Vector3(align_vector.X, align_vector.Y, 0.0f), Vector3(pc2->top.X - pc2->loc.X, pc2->top.Y - pc2->loc.Y, 0.0f))
        > 0.0f) {
        pc2->bottom = pc2->loc - v3;
        pc2->top = pc2->loc + v3;
    } else {
        pc2->bottom = pc2->loc + v3;
        pc2->top = pc2->loc - v3;
    }

    align_vector *= width_in_texture;

    if (Vector3::Cross_Product_Z(Vector3(v3.X, v3.Y, 0.0f), Vector3(pc3->top.X - pc3->loc.X, pc3->top.Y - pc3->loc.Y, 0.0f))
        < 0.0f) {
        pc3->bottom = pc3->loc - align_vector;
        pc3->top = pc3->loc + align_vector;
    } else {
        pc3->bottom = pc3->loc + align_vector;
        pc3->top = pc3->loc - align_vector;
    }

    if (Vector3::Cross_Product_Z(Vector3(v3.X, v3.Y, 0.0f), Vector3(pc4->top.X - pc4->loc.X, pc4->top.Y - pc4->loc.Y, 0.0f))
        < 0.0f) {
        pc4->bottom = pc4->loc - align_vector;
        pc4->top = pc4->loc + align_vector;
    } else {
        pc4->bottom = pc4->loc + align_vector;
        pc4->top = pc4->loc - align_vector;
    }

    pc1->last = true;
    pc1->count = 0;
    pc2->last = true;
    pc2->count = 0;
    pc3->last = true;
    pc3->count = 0;
    pc4->last = true;
    pc4->count = 0;
}

void W3DRoadBuffer::Offset_3Way(
    TRoadPt *pc1, TRoadPt *pc2, TRoadPt *pc3, Vector2 loc, Vector2 up_vector, Vector2 tee_vector, float width_in_texture)
{
    pc1->loc = loc - up_vector;
    pc2->loc = loc + up_vector;
    pc3->loc = loc + tee_vector;
    Vector2 v1(tee_vector);

    if (Vector3::Cross_Product_Z(Vector3(up_vector.X, up_vector.Y, 0.0f), Vector3(tee_vector.X, tee_vector.Y, 0.0f))
        < 0.0f) {
        v1.X = -tee_vector.X;
        v1.Y = -tee_vector.Y;
    }

    v1 *= width_in_texture;

    if (Vector3::Cross_Product_Z(
            Vector3(up_vector.X, up_vector.Y, 0.0f), Vector3(pc1->top.X - pc1->loc.X, pc1->top.Y - pc1->loc.Y, 0.0f))
        > 0.0f) {
        pc1->bottom = pc1->loc - v1;
        pc1->top = pc1->loc + v1;
    } else {
        pc1->bottom = pc1->loc + v1;
        pc1->top = pc1->loc - v1;
    }

    if (Vector3::Cross_Product_Z(
            Vector3(up_vector.X, up_vector.Y, 0.0f), Vector3(pc2->top.X - pc2->loc.X, pc2->top.Y - pc2->loc.Y, 0.0f))
        > 0.0f) {
        pc2->bottom = pc2->loc - v1;
        pc2->top = pc2->loc + v1;
    } else {
        pc2->bottom = pc2->loc + v1;
        pc2->top = pc2->loc - v1;
    }

    up_vector *= width_in_texture;

    if (Vector3::Cross_Product_Z(Vector3(v1.X, v1.Y, 0.0f), Vector3(pc3->top.X - pc3->loc.X, pc3->top.Y - pc3->loc.Y, 0.0f))
        < 0.0f) {
        pc3->bottom = pc3->loc - up_vector;
        pc3->top = pc3->loc + up_vector;
    } else {
        pc3->bottom = pc3->loc + up_vector;
        pc3->top = pc3->loc - up_vector;
    }
}

void W3DRoadBuffer::Offset_Y(
    TRoadPt *pc1, TRoadPt *pc2, TRoadPt *pc3, Vector2 loc, Vector2 up_vector, float width_in_texture)
{
    pc3->loc += up_vector * 0.55f;
    pc3->top += up_vector * 0.55f;
    pc3->bottom += up_vector * 0.55f;
    Vector2 v1(up_vector);
    v1.Rotate(DEG_TO_RADF(135.f));
    Vector2 v2(-v1.Y, v1.X);
    v2 *= width_in_texture;
    pc2->loc += v1 * 1.1f;

    if (xp_sign(v1, pc2->top - loc) == 1) {
        pc2->top = pc2->loc + v2;
        pc2->bottom = pc2->loc - v2;
    } else {
        pc2->top = pc2->loc - v2;
        pc2->bottom = pc2->loc + v2;
    }

    v1 = up_vector;
    v1.Rotate(DEG_TO_RADF(-135.f));
    v2.Set(-v1.Y, v1.X);
    v2 *= width_in_texture;
    pc1->loc += v1 * 1.1f;

    if (xp_sign(v1, pc1->top - loc) == 1) {
        pc1->top = pc1->loc + v2;
        pc1->bottom = pc1->loc - v2;
    } else {
        pc1->top = pc1->loc - v2;
        pc1->bottom = pc1->loc + v2;
    }
}

void W3DRoadBuffer::Offset_H(TRoadPt *pc1,
    TRoadPt *pc2,
    TRoadPt *pc3,
    Vector2 loc,
    Vector2 up_vector,
    Vector2 tee_vector,
    bool flip,
    bool mirror,
    float width_in_texture)
{
    if (flip == mirror) {
        pc1->loc = loc - up_vector * 0.46f;
        pc2->loc = loc + up_vector * 2.05f;
    } else {
        pc1->loc = loc - up_vector * 2.05f;
        pc2->loc = loc + up_vector * 0.46f;
    }

    Vector2 v1(tee_vector);

    if (Vector3::Cross_Product_Z(Vector3(up_vector.X, up_vector.Y, 0.0f), Vector3(tee_vector.X, tee_vector.Y, 0.0f))
        < 0.0f) {
        v1.X = -tee_vector.X;
        v1.Y = -tee_vector.Y;
    }

    v1 *= width_in_texture;

    if (Vector3::Cross_Product_Z(
            Vector3(up_vector.X, up_vector.Y, 0.0f), Vector3(pc1->top.X - pc1->loc.X, pc1->top.Y - pc1->loc.Y, 0.0f))
        > 0.0f) {
        pc1->bottom = pc1->loc - v1;
        pc1->top = pc1->loc + v1;
    } else {
        pc1->bottom = pc1->loc + v1;
        pc1->top = pc1->loc - v1;
    }

    if (Vector3::Cross_Product_Z(
            Vector3(up_vector.X, up_vector.Y, 0.0f), Vector3(pc2->top.X - pc2->loc.X, pc2->top.Y - pc2->loc.Y, 0.0f))
        > 0.0f) {
        pc2->bottom = pc2->loc - v1;
        pc2->top = pc2->loc + v1;
    } else {
        pc2->bottom = pc2->loc + v1;
        pc2->top = pc2->loc - v1;
    }

    Vector2 v2(tee_vector);
    if (flip) {
        v2.Rotate(DEG_TO_RADF(45.f));
    } else {
        v2.Rotate(DEG_TO_RADF(-45.f));
    }

    Vector2 v3(-v2.Y, v2.X);
    v3 *= width_in_texture;
    pc3->loc += v2 * 2.0999999f;

    if ((float)xp_sign(v2, pc3->top - loc) == 1.0f) {
        pc3->top = pc3->loc + v3;
        pc3->bottom = pc3->loc - v3;
    } else {
        pc3->top = pc3->loc - v3;
        pc3->bottom = pc3->loc + v3;
    }
}

void W3DRoadBuffer::Insert_Curve_Segment_At(int ndx1, int ndx2)
{
    float f1 = m_roads[ndx1].m_curveRadius * m_roads[ndx1].m_scale;
    Vector2 v1(m_roads[ndx1].m_pt1.loc);
    LineSegClass line1(Vector3(m_roads[ndx1].m_pt1.loc.X, m_roads[ndx1].m_pt1.loc.Y, 0.0f),
        Vector3(m_roads[ndx1].m_pt2.loc.X, m_roads[ndx1].m_pt2.loc.Y, 0.0f));
    LineSegClass line2(Vector3(m_roads[ndx2].m_pt1.loc.X, m_roads[ndx2].m_pt1.loc.Y, 0.0f),
        Vector3(m_roads[ndx2].m_pt2.loc.X, m_roads[ndx2].m_pt2.loc.Y, 0.0f));

    if (m_roads[ndx1].m_uniqueID == m_roads[ndx2].m_uniqueID) {
        float f2 = line1.Get_Dir() * line2.Get_Dir();
        TRoadPt *pt1;
        TRoadPt *pt2;
        TRoadPt *pt3;
        TRoadPt *pt4;
        bool b;

        if (Vector3::Cross_Product_Z(line1.Get_Dir(), line2.Get_Dir()) <= 0.0f) {
            pt1 = &m_roads[ndx1].m_pt1;
            pt2 = &m_roads[ndx1].m_pt2;
            pt3 = &m_roads[ndx2].m_pt1;
            pt4 = &m_roads[ndx2].m_pt2;
            b = false;
            line1.Set(Vector3(pt4->loc.X, pt4->loc.Y, 0.0f), Vector3(pt3->loc.X, pt3->loc.Y, 0.0f));
            line2.Set(Vector3(pt2->loc.X, pt2->loc.Y, 0.0f), Vector3(pt1->loc.X, pt1->loc.Y, 0.0f));
        } else {
            pt4 = &m_roads[ndx1].m_pt1;
            pt3 = &m_roads[ndx1].m_pt2;
            pt2 = &m_roads[ndx2].m_pt1;
            pt1 = &m_roads[ndx2].m_pt2;
            b = true;
        }

        float f4 = GameMath::Acos(f2) / 0.52359879f;
        if (f4 < 0.9f || m_roads[ndx1].m_pt1.is_angled) {
            Miter(ndx1, ndx2);
            return;
        }

        Vector3 v2 = f1 * line1.Get_Dir();
        Vector3 v3 = f1 * line2.Get_Dir();
        v2.Rotate_Z(DEG_TO_RADF(-90.f));
        v3.Rotate_Z(DEG_TO_RADF(-90.f));
        Vector3 v4 = Vector3(pt4->loc.X, pt4->loc.Y, 0.0f) + v2;
        Vector3 v5 = Vector3(pt3->loc.X, pt3->loc.Y, 0.0f) + v2;
        LineSegClass line3(v4, v5);
        v4 = Vector3(pt2->loc.X, pt2->loc.Y, 0.0f) + v3;
        v5 = Vector3(pt1->loc.X, pt1->loc.Y, 0.0f) + v3;
        LineSegClass line4(v4, v5);
        Vector3 p1;
        Vector3 p2;
        Vector3 p3;
        Vector3 p4;
        float f3;
        if (line3.Find_Intersection(line4, &p1, &f3, &p2, &f3)) {
            m_roads[ndx2].m_pt2.last = true;
            LineSegClass line5(p1, p1 - v3);
            LineSegClass line6(p1, p1 - v2);
            line5.Find_Intersection(line2, &p1, &f3, &p2, &f3);
            line6.Find_Intersection(line1, &p3, &f3, &p4, &f3);

            if (line2.Get_Dir() * (p1 - Vector3(pt2->loc.X, pt2->loc.Y, 0.0f)) < 0.5f) {
                pt4->loc = v1;
                pt1->loc = v1;
                Miter(ndx1, ndx2);
                return;
            }

            if (line1.Get_Dir() * (Vector3(pt3->loc.X, pt3->loc.Y, 0.0f) - p3) < 0.5f) {
                pt4->loc = v1;
                pt1->loc = v1;
                Miter(ndx1, ndx2);
                return;
            }

            pt1->loc = Vector2(p1.X, p1.Y);
            float angle = DEG_TO_RADF(-30.f);
            Vector2 v6 = pt1->loc;
            Vector2 v7 = pt2->loc;
            Vector2 v8(v7.X - v6.X, v7.Y - v6.Y);
            Vector2 v9(-v8.Y, v8.X);
            v9.Normalize();
            v9 *= m_roads[ndx1].m_curveRadius * m_roads[ndx1].m_scale;
            v9 += v6;
            Rotate_About(&v6, v9, angle);
            v8.Rotate(angle);
            v7 = v6 + v8;
            m_roads[m_numRoads].m_pt1.loc = v6;
            m_roads[m_numRoads].m_pt2.loc = v7;

            if (m_numRoads >= m_maxRoadSegments) {
                return;
            }

            m_roads[m_numRoads].m_pt1.last = true;
            m_roads[m_numRoads].m_pt2.last = true;
            m_roads[m_numRoads].m_scale = m_roads[ndx1].m_scale;
            m_roads[m_numRoads].m_widthInTexture = m_roads[ndx1].m_widthInTexture;
            m_roads[m_numRoads].m_type = CURVE;
            m_roads[m_numRoads].m_curveRadius = m_roads[ndx1].m_curveRadius;
            m_roads[m_numRoads].m_uniqueID = m_roads[ndx1].m_uniqueID;
            m_numRoads++;

            if (f4 > 2.0f) {
                for (int i = 2; (float)i < f4; i++) {
                    v8.Rotate(angle);
                    Rotate_About(&v6, v9, angle);
                    v7 = v6 + v8;

                    if (m_numRoads >= m_maxRoadSegments) {
                        return;
                    }

                    m_roads[m_numRoads].m_pt1.loc.Set(v6);
                    m_roads[m_numRoads].m_pt2.loc.Set(v7);
                    m_roads[m_numRoads].m_pt1.last = true;
                    m_roads[m_numRoads].m_pt2.last = true;
                    m_roads[m_numRoads].m_scale = m_roads[ndx1].m_scale;
                    m_roads[m_numRoads].m_widthInTexture = m_roads[ndx1].m_widthInTexture;
                    m_roads[m_numRoads].m_type = CURVE;
                    m_roads[m_numRoads].m_curveRadius = m_roads[ndx1].m_curveRadius;
                    m_roads[m_numRoads].m_uniqueID = m_roads[ndx1].m_uniqueID;
                    m_numRoads++;
                }
            }

            pt4->loc = Vector2(p3.X, p3.Y);
            m_roads[ndx1].m_pt1.last = true;

            if (f4 > 1.0f) {
                v6 = pt4->loc;
                v7 = (pt4->loc + pt4->loc) - pt3->loc;
                v8.Set(v7.X - v6.X, v7.Y - v6.Y);
                v7 = v6 + v8;

                if (m_numRoads >= m_maxRoadSegments) {
                    return;
                }

                m_roads[m_numRoads].m_pt1.loc.Set(v6);
                m_roads[m_numRoads].m_pt2.loc.Set(v7);
                m_roads[m_numRoads].m_pt1.last = true;
                m_roads[m_numRoads].m_pt2.last = true;
                m_roads[m_numRoads].m_scale = m_roads[ndx1].m_scale;
                m_roads[m_numRoads].m_widthInTexture = m_roads[ndx1].m_widthInTexture;
                m_roads[m_numRoads].m_type = CURVE;
                m_roads[m_numRoads].m_curveRadius = m_roads[ndx1].m_curveRadius;
                m_roads[m_numRoads].m_uniqueID = m_roads[ndx1].m_uniqueID;
                m_numRoads++;
            }

            Vector2 v11 = m_roads[ndx1].m_pt2.loc - m_roads[ndx1].m_pt1.loc;
            Vector2 v12(-v11.Y, v11.X);
            v12.Normalize();
            v12 *= m_roads[ndx1].m_scale * m_roads[ndx1].m_widthInTexture / 2.0f;
            m_roads[ndx1].m_pt1.top = m_roads[ndx1].m_pt1.loc + v12;
            m_roads[ndx1].m_pt1.bottom = m_roads[ndx1].m_pt1.loc - v12;
            v11 = m_roads[ndx2].m_pt2.loc - m_roads[ndx2].m_pt1.loc;
            v12 = Vector2(-v11.Y, v11.X);
            v12.Normalize();
            v12 *= m_roads[ndx2].m_scale * m_roads[ndx2].m_widthInTexture / 2.0f;
            m_roads[ndx2].m_pt2.top = m_roads[ndx2].m_pt2.loc + v12;
            m_roads[ndx2].m_pt2.bottom = m_roads[ndx2].m_pt2.loc - v12;
            return;
        }
    }
}

#ifdef GAME_DLL
#include <new>
W3DRoadBuffer *W3DRoadBuffer::Hook_Ctor()
{
    return new (this) W3DRoadBuffer();
}
void W3DRoadBuffer::Hook_Dtor()
{
    W3DRoadBuffer::~W3DRoadBuffer();
}
#endif
