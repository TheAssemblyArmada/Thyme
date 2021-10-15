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
#pragma once
#include "always.h"
#include "w3dtypes.h"
struct SortingNodeStruct;
class SphereClass;

class SortingRendererClass
{
public:
    static void Flush_Sorting_Pool();
    static void Insert_To_Sorting_Pool(SortingNodeStruct *state);
    static void Insert_Triangles(unsigned short start_index,
        unsigned short polygon_count,
        unsigned short min_vertex_index,
        unsigned short vertex_count);
    static void Insert_Triangles(const SphereClass &bounding_sphere,
        unsigned short start_index,
        unsigned short polygon_count,
        unsigned short min_vertex_index,
        unsigned short vertex_count);
    static void Flush();
    static void Deinit();
    static void Set_Min_Vertex_Buffer_Size(unsigned int val);
    static bool s_EnableTriangleDraw;
};
