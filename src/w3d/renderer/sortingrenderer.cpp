/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Sorting Renderer
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "sortingrenderer.h"
#include "dllist.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include "sphere.h"
#include "w3d.h"
#include <algorithm>
#ifdef BUILD_WITH_D3D8
#include <d3dx8.h>
#endif
#ifdef GAME_DLL
#include "hooker.h"
#endif

struct ShortVectorIStruct
{
    unsigned short i;
    unsigned short j;
    unsigned short k;
};

struct TempIndexStruct
{
    struct ShortVectorIStruct tri;
    unsigned short idx;
    float z;
};

struct SortingNodeStruct : public DLNodeClass<SortingNodeStruct>
{
    IMPLEMENT_W3D_POOL(SortingNodeStruct);

public:
    RenderStateStruct sorting_state;
    SphereClass bounding_sphere;
    Vector3 transformed_center;
    unsigned short start_index;
    unsigned short polygon_count;
    unsigned short min_vertex_index;
    unsigned short vertex_count;
};

enum
{
    MAX_NODES = 4096
};

bool SortingRendererClass::s_EnableTriangleDraw = true;

static TempIndexStruct *g_tempIndexArray;
static unsigned int g_tempIndexArrayCount;

static unsigned int g_defaultSortingPolyCount = 0x4000;
static unsigned int g_defaultSortingVertexCount = 0x8000;

static SortingNodeStruct *g_overlappingNodes[MAX_NODES];
static unsigned int g_overlappingVertexCount;
static unsigned int g_overlappingNodeCount;
static unsigned int g_overlappingPolygonCount;

static unsigned int g_totalSortingVertices;
static DLListClass<SortingNodeStruct> g_cleanList;
static DLListClass<SortingNodeStruct> g_sortedList;

bool operator<(TempIndexStruct const &left, TempIndexStruct const &right)
{
    return left.z < right.z;
}

bool operator<=(TempIndexStruct const &left, TempIndexStruct const &right)
{
    return left.z <= right.z;
}

bool operator>(TempIndexStruct const &left, TempIndexStruct const &right)
{
    return left.z > right.z;
}

bool operator>=(TempIndexStruct const &left, TempIndexStruct const &right)
{
    return left.z >= right.z;
}

bool operator==(TempIndexStruct const &left, TempIndexStruct const &right)
{
    return left.z == right.z;
}

SortingNodeStruct *Get_Sorting_Struct()
{
    SortingNodeStruct *node = g_cleanList.Head();

    if (node) {
        node->Remove();
        return node;
    } else {
        return new SortingNodeStruct();
    }
}

void Release_Refs(SortingNodeStruct *state)
{
    for (int i = 0; i < VERTEX_BUFFERS; i++) {
        Ref_Ptr_Release(state->sorting_state.vertex_buffers[i]);
    }

    Ref_Ptr_Release(state->sorting_state.index_buffer);
    Ref_Ptr_Release(state->sorting_state.material);

    for (unsigned int i = 0; i < DX8Wrapper::Get_Current_Caps()->Max_Textures_Per_Pass(); i++) {
        Ref_Ptr_Release(state->sorting_state.Textures[i]);
    }
}

// void InsertionSort(TempIndexStruct *begin, TempIndexStruct *end)
//{
//    /* This is a stripped down __insertion_sort from stl */
//    for (auto iter = begin + 1; iter < end; ++iter) {
//        /* This is __unguarded_linear_insert from stl */
//        auto val = *iter;
//        auto *next = iter - 1;
//        auto *cur = iter;
//        while (*next > val) {
//            *cur = *next;
//            cur = next;
//            --next;
//        }
//        *cur = val;
//    }
//}

// zh: 0x0080D3C0 wb: 0x0056A9A0
void Sort(TempIndexStruct *begin, TempIndexStruct *end)
{
    std::sort(begin, end);
    // Vanilla used a specialised sort see below
    /*
    // 16 is the __stl_threshold
    for (auto total_elements = end - begin; total_elements > 16; total_elements = end - begin) {
        auto *halfway = &begin[total_elements / 2];
        auto *next = begin + 1; // ebx
        auto *last = end - 1;
        std::swap(*halfway, *next);
        if (*next > *last) {
            std::swap(*next, *last);
        }
        if (*begin > *last) {
            std::swap(*begin, *last);
        }
        if (*next > *begin) {
            std::swap(*next, *begin);
        }

        auto *ptr = next; // ebp
        auto *pivot = last; // esi
        while (1) {
            while (++ptr <= last && *ptr < *begin)
                ;
            while (--pivot > next && *pivot > *begin)
                ;

            if (pivot < ptr) {
                break;
            }
            std::swap(*ptr, *pivot);
        }

        std::swap(*pivot, *begin);

        auto right_num_elements = end - pivot - 1;
        auto left_num_elements = pivot - begin;
        // Sort the smallest side
        if (left_num_elements <= right_num_elements) {
            Sort(begin, pivot);
            begin = pivot + 1;
        } else {
            Sort(pivot + 1, end);
            end = pivot;
        }
    }
    InsertionSort(begin, end);*/
}

TempIndexStruct *Get_Temp_Index_Array(unsigned int count)
{
    if (count < g_defaultSortingPolyCount) {
        count = g_defaultSortingPolyCount;
    }

    if (count > g_tempIndexArrayCount) {
        delete[] g_tempIndexArray;
        g_tempIndexArray = new TempIndexStruct[count];
        g_tempIndexArrayCount = count;
    }

    return g_tempIndexArray;
}

void Apply_Render_State(RenderStateStruct &render_state)
{
    DX8Wrapper::Set_Shader(render_state.shader);
    DX8Wrapper::Set_Material(render_state.material);

    for (unsigned int i = 0; i < DX8Wrapper::Get_Current_Caps()->Max_Textures_Per_Pass(); ++i) {
        DX8Wrapper::Set_Texture(i, render_state.Textures[i]);
    }

#ifdef BUILD_WITH_D3D8
    if (render_state.material->Get_Lighting()) {
        for (int i = 0; i < 4; i++) {
            if (!render_state.LightEnable[i]) {
                DX8Wrapper::Set_DX8_Light(i, nullptr);
                break;
            }
            DX8Wrapper::Set_DX8_Light(i, &render_state.Lights[i]);
        }
    }

    DX8Wrapper::Set_DX8_Transform(D3DTS_WORLD, render_state.world);
    DX8Wrapper::Set_DX8_Transform(D3DTS_VIEW, render_state.view);
#endif
}

void SortingRendererClass::Set_Min_Vertex_Buffer_Size(unsigned int val)
{
    g_defaultSortingVertexCount = val;
    g_defaultSortingPolyCount = val >> 1;
}

void SortingRendererClass::Insert_Triangles(const SphereClass &bounding_sphere,
    unsigned short start_index,
    unsigned short polygon_count,
    unsigned short min_vertex_index,
    unsigned short vertex_count)
{
#ifdef BUILD_WITH_D3D8
    if (W3D::Is_Sorting_Enabled()) {
        // Debug_Statistics::Record_Sorting_Polys_And_Vertices(polygon_count, vertex_count);
        SortingNodeStruct *state = Get_Sorting_Struct();
        DX8Wrapper::Get_Render_State(state->sorting_state);

        captainslog_assert((state->sorting_state.index_buffer_type == IndexBufferClass::BUFFER_TYPE_SORTING
                               || state->sorting_state.index_buffer_type == IndexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING)
            && (state->sorting_state.vertex_buffer_types[0] == VertexBufferClass::BUFFER_TYPE_SORTING
                || state->sorting_state.vertex_buffer_types[0] == VertexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING));

        state->bounding_sphere = bounding_sphere;
        state->polygon_count = polygon_count;
        VertexBufferClass *vertex_buffer = state->sorting_state.vertex_buffers[0];
        state->start_index = start_index;
        state->min_vertex_index = min_vertex_index;
        state->vertex_count = vertex_count;

        captainslog_assert(vertex_buffer);
        captainslog_assert(state->vertex_count <= vertex_buffer->Get_Vertex_Count());

        D3DXMATRIX mtx;
        D3DXMatrixMultiply(&mtx, &state->sorting_state.world, &state->sorting_state.view);
        D3DXVECTOR3 bounding_sphere_center;
        bounding_sphere_center.x = state->bounding_sphere.Center.X;
        bounding_sphere_center.y = state->bounding_sphere.Center.Y;
        bounding_sphere_center.z = state->bounding_sphere.Center.Z;
        D3DXVECTOR4 transformed_center;
        D3DXVec3Transform(&transformed_center, &bounding_sphere_center, &mtx);
        state->transformed_center.X = transformed_center.x;
        state->transformed_center.Y = transformed_center.y;
        state->transformed_center.Z = transformed_center.z;

        SortingNodeStruct *i;

        for (i = g_sortedList.Head(); i; i = i->Succ()) {
            if (state->transformed_center.Z > i->transformed_center.Z) {
                if (g_sortedList.Head() == g_sortedList.Tail()) {
                    g_sortedList.Add_Head(state);
                } else {
                    state->Insert_Before(i);
                }
                break;
            }
        }

        if (!i) {
            g_sortedList.Add_Tail(state);
        }
    } else {
        DX8Wrapper::Draw_Triangles(start_index, polygon_count, min_vertex_index, vertex_count);
    }
#endif
}

void SortingRendererClass::Insert_Triangles(
    unsigned short start_index, unsigned short polygon_count, unsigned short min_vertex_index, unsigned short vertex_count)
{
    SphereClass sphere(Vector3(0, 0, 0), 0);
    Insert_Triangles(sphere, start_index, polygon_count, min_vertex_index, vertex_count);
}

void SortingRendererClass::Insert_To_Sorting_Pool(SortingNodeStruct *state)
{
    if (g_overlappingNodeCount < MAX_NODES) {
        g_overlappingNodes[g_overlappingNodeCount] = state;
        g_overlappingVertexCount += state->vertex_count;
        g_overlappingPolygonCount += state->polygon_count;
        g_overlappingNodeCount++;
    } else {
        Release_Refs(state);
        captainslog_assert(0);
    }
}

void SortingRendererClass::Flush_Sorting_Pool()
{
#ifdef BUILD_WITH_D3D8
    if (g_overlappingNodeCount) {
        TempIndexStruct *index_array = Get_Temp_Index_Array(g_overlappingPolygonCount);
        unsigned int vertexAllocCount = g_overlappingVertexCount;

        if (DynamicVBAccessClass::Get_Default_Vertex_Count() < g_defaultSortingVertexCount) {
            vertexAllocCount = g_defaultSortingVertexCount;
        }

        if (g_overlappingVertexCount > vertexAllocCount) {
            vertexAllocCount = g_overlappingVertexCount;
        }

        captainslog_assert(g_defaultSortingVertexCount == 1 || vertexAllocCount <= g_defaultSortingVertexCount);

        DynamicVBAccessClass dyn_vb_access(VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8,
            (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2 | D3DFVF_DIFFUSE),
            vertexAllocCount);

        {
            DynamicVBAccessClass::WriteLockClass lock(&dyn_vb_access);
            VertexFormatXYZNDUV2 *dest_verts = lock.Get_Formatted_Vertex_Array();
            unsigned int polygon_array_offset = 0;
            unsigned int vertex_array_offset = 0;

            for (unsigned int node_id = 0; node_id < g_overlappingNodeCount; node_id++) {
                SortingNodeStruct *state = g_overlappingNodes[node_id];
                VertexFormatXYZNDUV2 *src_verts = nullptr;
                SortingVertexBufferClass *vertex_buffer =
                    static_cast<SortingVertexBufferClass *>(state->sorting_state.vertex_buffers[0]);

                captainslog_assert(vertex_buffer);
                src_verts = vertex_buffer->Get_Sorting_Vertex_Buffer();
                captainslog_assert(src_verts);

                src_verts +=
                    state->sorting_state.vba_offset + state->sorting_state.index_base_offset + state->min_vertex_index;
                memcpy(dest_verts, src_verts, sizeof(VertexFormatXYZNDUV2) * state->vertex_count);
                dest_verts += state->vertex_count;

                D3DXMATRIX d3d_mtx;
                D3DXMatrixMultiply(&d3d_mtx, &state->sorting_state.world, &state->sorting_state.view);

                SortingIndexBufferClass *index_buffer =
                    static_cast<SortingIndexBufferClass *>(state->sorting_state.index_buffer);
                captainslog_assert(index_buffer);
                unsigned short *indices = index_buffer->Get_Sorting_Index_Buffer();
                captainslog_assert(indices);
                indices += state->start_index + state->sorting_state.iba_offset;

                if (d3d_mtx.m[0][2] == 0.0f && d3d_mtx.m[1][2] == 0.0f && d3d_mtx.m[3][2] == 0.0f
                    && d3d_mtx.m[2][2] == 1.0f) {
                    for (int i = 0; i < state->polygon_count; i++) {
                        unsigned short idx1 = indices[3 * i] - state->min_vertex_index;
                        unsigned short idx2 = indices[3 * i + 1] - state->min_vertex_index;
                        unsigned short idx3 = indices[3 * i + 2] - state->min_vertex_index;

                        captainslog_assert(idx1 < state->vertex_count);
                        captainslog_assert(idx2 < state->vertex_count);
                        captainslog_assert(idx3 < state->vertex_count);

                        VertexFormatXYZNDUV2 *vert1 = &src_verts[idx1];
                        VertexFormatXYZNDUV2 *vert2 = &src_verts[idx2];
                        VertexFormatXYZNDUV2 *vert3 = &src_verts[idx3];
                        unsigned int array_index = polygon_array_offset + i;
                        captainslog_assert(array_index < g_overlappingPolygonCount);

                        TempIndexStruct *tis_ptr = &index_array[array_index];
                        tis_ptr->tri.i = vertex_array_offset + idx1;
                        tis_ptr->tri.j = vertex_array_offset + idx2;
                        tis_ptr->tri.k = vertex_array_offset + idx3;
                        tis_ptr->idx = node_id;
                        tis_ptr->z = (vert1->z + vert2->z + vert3->z) / 3.0f;
                        // assert disabled due to stock bug in W3DLaserDraw
                        // captainslog_assert((!_isnan(tis_ptr->z) && _finite(tis_ptr->z)));
                    }
                } else {
                    for (int i = 0; i < state->polygon_count; i++) {
                        unsigned short idx1 = indices[3 * i] - state->min_vertex_index;
                        unsigned short idx2 = indices[3 * i + 1] - state->min_vertex_index;
                        unsigned short idx3 = indices[3 * i + 2] - state->min_vertex_index;

                        captainslog_assert(idx1 < state->vertex_count);
                        captainslog_assert(idx2 < state->vertex_count);
                        captainslog_assert(idx3 < state->vertex_count);

                        VertexFormatXYZNDUV2 *vert1 = &src_verts[idx1];
                        VertexFormatXYZNDUV2 *vert2 = &src_verts[idx2];
                        VertexFormatXYZNDUV2 *vert3 = &src_verts[idx3];
                        unsigned int array_index = polygon_array_offset + i;
                        captainslog_assert(array_index < g_overlappingPolygonCount);

                        TempIndexStruct *tis_ptr = &index_array[array_index];
                        tis_ptr->tri.i = vertex_array_offset + idx1;
                        tis_ptr->tri.j = vertex_array_offset + idx2;
                        tis_ptr->tri.k = vertex_array_offset + idx3;
                        tis_ptr->idx = node_id;
                        tis_ptr->z = ((vert1->x + vert2->x + vert3->x) * d3d_mtx.m[0][2]
                                         + (vert1->y + vert2->y + vert3->y) * d3d_mtx.m[1][2]
                                         + (vert1->z + vert2->z + vert3->z) * d3d_mtx.m[2][2])
                                / 3.0f
                            + d3d_mtx.m[3][2];
                        // assert disabled due to stock bug in W3DLaserDraw
                        // captainslog_assert((!_isnan(tis_ptr->z) && _finite(tis_ptr->z)));
                    }
                }

                state->min_vertex_index = vertex_array_offset;
                polygon_array_offset += state->polygon_count;
                vertex_array_offset += state->vertex_count;
            }
        }

        Sort(index_array, &index_array[g_overlappingPolygonCount]);
        unsigned int polygonAllocCount = g_overlappingPolygonCount;

        if (DynamicIBAccessClass::Get_Default_Index_Count() / 3u < g_defaultSortingPolyCount) {
            polygonAllocCount = g_defaultSortingPolyCount;
        }

        if (g_overlappingPolygonCount > polygonAllocCount) {
            polygonAllocCount = g_overlappingPolygonCount;
        }

        captainslog_assert(g_defaultSortingPolyCount <= 1 || polygonAllocCount <= g_defaultSortingPolyCount);
        DynamicIBAccessClass dyn_ib_access(IndexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, 3 * polygonAllocCount);

        {
            DynamicIBAccessClass::WriteLockClass lock(&dyn_ib_access);
            unsigned short *dest_indices = lock.Get_Index_Array();
            for (unsigned int i = 0; i < g_overlappingPolygonCount; i++) {
                dest_indices[3 * i] = index_array[i].tri.i;
                dest_indices[3 * i + 1] = index_array[i].tri.j;
                dest_indices[3 * i + 2] = index_array[i].tri.k;
            }
        }

        DX8Wrapper::Set_Index_Buffer(dyn_ib_access, 0);
        DX8Wrapper::Set_Vertex_Buffer(dyn_vb_access);
        DX8Wrapper::Apply_Render_State_Changes();
        int polygon_count = 1;
        unsigned short start_index = 0;
        unsigned int idx = index_array->idx;

        for (unsigned int i = 1; i < g_overlappingPolygonCount; i++) {
            if (idx != index_array[i].idx) {
                Apply_Render_State(g_overlappingNodes[idx]->sorting_state);
                DX8Wrapper::Draw_Triangles(3 * start_index,
                    polygon_count,
                    g_overlappingNodes[idx]->min_vertex_index,
                    g_overlappingNodes[idx]->vertex_count);
                start_index = i;
                polygon_count = 0;
                idx = index_array[i].idx;
            }

            polygon_count++;
        }

        if (polygon_count) {
            Apply_Render_State(g_overlappingNodes[idx]->sorting_state);
            DX8Wrapper::Draw_Triangles(3 * start_index,
                polygon_count,
                g_overlappingNodes[idx]->min_vertex_index,
                g_overlappingNodes[idx]->vertex_count);
        }

        for (unsigned int i = 0; i < g_overlappingNodeCount; i++) {
            Release_Refs(g_overlappingNodes[i]);
            g_cleanList.Add_Head(g_overlappingNodes[i]);
        }

        g_overlappingNodeCount = 0;
        g_overlappingPolygonCount = 0;
        g_overlappingVertexCount = 0;
    }
#endif
}

void SortingRendererClass::Flush()
{
#ifdef BUILD_WITH_D3D8
    Matrix4 old_view;
    Matrix4 old_world;
    DX8Wrapper::Get_Transform(D3DTS_VIEW, old_view);
    DX8Wrapper::Get_Transform(D3DTS_WORLD, old_world);

    for (;;) {
        SortingNodeStruct *state = g_sortedList.Head();

        if (!state) {
            break;
        }

        state->Remove();

        if ((state->sorting_state.index_buffer_type == IndexBufferClass::BUFFER_TYPE_SORTING
                || state->sorting_state.index_buffer_type == IndexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING)
            && (state->sorting_state.vertex_buffer_types[0] == VertexBufferClass::BUFFER_TYPE_SORTING
                || state->sorting_state.vertex_buffer_types[0] == VertexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING)) {
            Insert_To_Sorting_Pool(state);
        } else {
            DX8Wrapper::Set_Render_State(state->sorting_state);
            DX8Wrapper::Draw_Triangles(
                state->start_index, state->polygon_count, state->min_vertex_index, state->vertex_count);
            DX8Wrapper::Release_Render_State();

            Release_Refs(state);
            g_cleanList.Add_Head(state);
        }
    }

    bool enable_triangle_draw = DX8Wrapper::Is_Triangle_Draw_Enabled();
    DX8Wrapper::Enable_Triangle_Draw(s_EnableTriangleDraw);
    Flush_Sorting_Pool();

    DX8Wrapper::Enable_Triangle_Draw(enable_triangle_draw);
    DX8Wrapper::Set_Index_Buffer(nullptr, 0);
    DX8Wrapper::Set_Vertex_Buffer(nullptr, 0);
    g_totalSortingVertices = 0;

    DynamicIBAccessClass::Reset(false);
    DynamicVBAccessClass::Reset(false);
    DX8Wrapper::Set_Transform(D3DTS_VIEW, old_view);
    DX8Wrapper::Set_Transform(D3DTS_WORLD, old_world);
#endif
}

void SortingRendererClass::Deinit()
{
    while (g_sortedList.Head() != nullptr) {
        auto head = g_sortedList.Head();
        g_sortedList.Remove_Head();
        delete head;
    }

    while (g_cleanList.Head() != nullptr) {
        auto head = g_cleanList.Head();
        g_cleanList.Remove_Head();
        delete head;
    }

    delete[] g_tempIndexArray;
    g_tempIndexArray = nullptr;
    g_tempIndexArrayCount = 0;
}
