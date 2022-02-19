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
#pragma once
#include "always.h"
#include "asciistring.h"
#include "multilist.h"
#include "sphere.h"
#include "vector2.h"
class TextureClass;
class DX8VertexBufferClass;
class DX8IndexBufferClass;
class WorldHeightMap;
class RenderObjClass;
struct VertexFormatXYZDUV1;
class CameraClass;
class IRegion2D;

class RoadType
{
    ALLOW_HOOKING

public:
    RoadType();
    ~RoadType();
    void Load_Texture(Utf8String path, int32_t ID);
    void Apply_Texture();
    int32_t Get_Stacking() { return m_stackingOrder; }
    void Set_Stacking(int32_t stacking) { m_stackingOrder = stacking; }
    int32_t Get_Unique_ID() { return m_uniqueID; }
    DX8VertexBufferClass *Get_VB() { return m_vertexRoad; }
    DX8IndexBufferClass *Get_IB() { return m_indexRoad; }
    int32_t Get_Num_Vertices() { return m_numRoadVertices; }
    void Set_Num_Indices(int32_t indices) { m_numRoadIndices = indices; }
    void Set_Num_Vertices(int32_t vertices) { m_numRoadVertices = vertices; }
    int32_t Get_Num_Indices() { return m_numRoadIndices; }
    void Set_Auto_Loaded() { m_isAutoLoaded = true; }
    int32_t Is_Auto_Loaded() { return m_isAutoLoaded; }
    Utf8String Get_Path() { return m_texturePath; }
    void Load_Test_Texture();

protected:
    TextureClass *m_roadTexture;
    DX8VertexBufferClass *m_vertexRoad;
    DX8IndexBufferClass *m_indexRoad;
    int32_t m_numRoadVertices;
    int32_t m_numRoadIndices;
    int32_t m_uniqueID;
    bool m_isAutoLoaded;
    int32_t m_stackingOrder;
    Utf8String m_texturePath;
};

struct TRoadPt
{
    Vector2 loc;
    Vector2 top;
    Vector2 bottom;
    int32_t count;
    bool last;
    bool multi;
    bool is_angled;
    bool is_join;
};

enum TCorner
{
    SEGMENT,
    CURVE,
    TEE,
    FOUR_WAY,
    THREE_WAY_Y,
    THREE_WAY_H,
    THREE_WAY_H_FLIP,
    ALPHA_JOIN,
    NUM_JOINS,
};

struct TRoadSegInfo
{
    Vector2 loc;
    Vector2 road_normal;
    Vector2 road_vector;
    Vector2 corners[4];
    float u_offset;
    float v_offset;
    float scale;
};

class RoadSegment
{
    ALLOW_HOOKING

public:
    RoadSegment();
    ~RoadSegment();
    void Set_Vertex_Buffer(VertexFormatXYZDUV1 *vb, int32_t num_vertex);
    void Set_Index_Buffer(unsigned short *ib, int32_t num_index);
    void Set_Road_Seg_Info(TRoadSegInfo *info) { m_info = *info; }
    void Get_Road_Seg_Info(TRoadSegInfo *info) { *info = m_info; }
    SphereClass &Get_Bounds() { return m_bounds; }
    int32_t Get_Num_Vertex() { return m_numVertex; }
    int32_t Get_Num_Index() { return m_numIndex; }
    int32_t Get_Vertices(VertexFormatXYZDUV1 *destination_vb, int32_t num_to_copy);
    int32_t Get_Indices(unsigned short *destination_ib, int32_t num_to_copy, int32_t offset);
    void Update_Seg_Lighting();

    TRoadPt m_pt1;
    TRoadPt m_pt2;
    float m_curveRadius;
    TCorner m_type;
    float m_scale;
    float m_widthInTexture;
    int32_t m_uniqueID;
    bool m_isVisible;

protected:
    int32_t m_numVertex;
    VertexFormatXYZDUV1 *m_vb;
    int32_t m_numIndex;
    unsigned short *m_ib;
    TRoadSegInfo m_info;
    SphereClass m_bounds;
};

class W3DRoadBuffer
{
    ALLOW_HOOKING

public:
    W3DRoadBuffer();
    ~W3DRoadBuffer();
    void Load_Roads();
    void Clear_All_Roads();
    void Draw_Roads(CameraClass *camera,
        TextureClass *cloud_texture,
        TextureClass *noise_texture,
        bool wireframe,
        int32_t minx,
        int32_t maxx,
        int32_t miny,
        int32_t maxy,
        RefMultiListIterator<RenderObjClass> *dynamic_lights_iterator);
    void Set_Map(WorldHeightMap *map);
    void Update_Lighting();

protected:
    void Add_Map_Objects();
    void Add_Map_Object(RoadSegment *road, bool update_the_counts);
    void Adjust_Stacking(int32_t top_unique_id, int32_t bottom_unique_id);
    int32_t Find_Cross_Type_Join_Vector(Vector2 loc, Vector2 *join_vector, int32_t unique_id);
    void Insert_Curve_Segment_At(int32_t ndx1, int32_t ndx2);
    void Insert_Cross_Type_Joins();
    void Miter(int32_t ndx1, int32_t ndx2);
    void Move_Road_Seg_To(int32_t fromndx, int32_t tondx);
    void Check_Link_After(int32_t ndx);
    void Check_Link_Before(int32_t ndx);
    void Update_Counts(RoadSegment *road);
    void Update_Counts_And_Flags();
    void Insert_Curve_Segments();
    void Insert_Tee_Intersections();
    void Insert_Tee(Vector2 loc, int32_t index1, float scale);
    bool Insert_Y(Vector2 loc, int32_t index1, float scale);
    void Insert_4Way(Vector2 loc, int32_t index1, float scale);
    void Offset_4Way(TRoadPt *pc1,
        TRoadPt *pc2,
        TRoadPt *pc3,
        TRoadPt *pr3,
        TRoadPt *pc4,
        Vector2 loc,
        Vector2 align_vector,
        float width_in_texture);
    void Offset_3Way(TRoadPt *pc1,
        TRoadPt *pc2,
        TRoadPt *pc3,
        Vector2 loc,
        Vector2 up_vector,
        Vector2 tee_vector,
        float width_in_texture);
    void Offset_Y(TRoadPt *pc1, TRoadPt *pc2, TRoadPt *pc3, Vector2 loc, Vector2 up_vector, float width_in_texture);
    void Offset_H(TRoadPt *pc1,
        TRoadPt *pc2,
        TRoadPt *pc3,
        Vector2 loc,
        Vector2 up_vector,
        Vector2 tee_vector,
        bool flip,
        bool mirror,
        float width_in_texture);
    void Preload_Roads_In_Vertex_And_Index_Buffers();
    void Preload_Road_Segment(RoadSegment *road);
    void Load_Curve(RoadSegment *road, Vector2 loc1, Vector2 loc2, float scale);
    void Load_Tee(RoadSegment *road, Vector2 loc1, Vector2 loc2, bool is4way, float scale);
    void Load_Y(RoadSegment *road, Vector2 loc1, Vector2 loc2, float scale);
    void Load_Alpha_Join(RoadSegment *road, Vector2 loc1, Vector2 loc2, float scale);
    void Load_H(RoadSegment *road, Vector2 loc1, Vector2 loc2, bool flip, float scale);
    void Load_Float_Section(RoadSegment *road,
        Vector2 loc,
        Vector2 road_vector,
        float half_height,
        float left,
        float right,
        float uoffset,
        float voffset,
        float scale);
    void Load_Float_4Pt_Section(RoadSegment *road,
        Vector2 loc,
        Vector2 road_normal,
        Vector2 road_vector,
        Vector2 *cornersp,
        float uoffset,
        float voffset,
        float uscale,
        float vscale);
    void Load_Lit_4Pt_Section(RoadSegment *road,
        unsigned short *ib,
        VertexFormatXYZDUV1 *vb,
        RefMultiListIterator<RenderObjClass> *dynamic_lights_iterator); // not used, unimplemented
    void Load_Roads_In_Vertex_And_Index_Buffers();
    void Load_Lit_Roads_In_Vertex_And_Index_Buffers(
        RefMultiListIterator<RenderObjClass> *dynamic_lights_iterator); // not used, unimplemented
    void Load_Road_Segment(unsigned short *ib, VertexFormatXYZDUV1 *vb, RoadSegment *road);
    void Allocate_Road_Buffers();
    void Free_Road_Buffers();
    void Rotate_About(Vector2 *pt, Vector2 center, float angle);
    bool Visibility_Changed(const IRegion2D &region);
    void Update_Center();
    void Flip_The_Road(RoadSegment *road)
    {
        TRoadPt temp = road->m_pt1;
        road->m_pt1 = road->m_pt2;
        road->m_pt2 = temp;
    }

    RoadType *m_roadTypes;
    RoadSegment *m_roads;
    int32_t m_numRoads;
    bool m_initialized;
    WorldHeightMap *m_map;
    RefMultiListIterator<RenderObjClass> *m_lightsIterator;
    int32_t m_curUniqueID;
    int32_t m_curRoadType;
    int32_t m_maxUID;
    int32_t m_curOpenRoad;
    int32_t m_maxRoadSegments;
    int32_t m_maxRoadVertex;
    int32_t m_maxRoadIndex;
    int32_t m_maxRoadTypes;
    int32_t m_curNumRoadVertices;
    int32_t m_curNumRoadIndices;
    bool m_dirty;
#ifdef GAME_DLL
    W3DRoadBuffer *Hook_Ctor() { return new (this) W3DRoadBuffer(); }
    void Hook_Dtor() { W3DRoadBuffer::~W3DRoadBuffer(); }
#endif
};

#ifdef GAME_DLL
#include "hooker.h"
#endif
