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
#ifdef GAME_DLL
#include "hooker.h"
#endif

void SortingRendererClass::Insert_Triangles(
    unsigned short start_index, unsigned short polygon_count, unsigned short min_vertex_index, unsigned short vertex_count)
{
#ifdef GAME_DLL
    Call_Function<void, unsigned short, unsigned short, unsigned short, unsigned short>(
        PICK_ADDRESS(0x0080CC40, 0x00569CD0), start_index, polygon_count, min_vertex_index, vertex_count);
#endif
}

void SortingRendererClass::Insert_Triangles(const SphereClass &bounding_sphere,
    unsigned short start_index,
    unsigned short polygon_count,
    unsigned short min_vertex_index,
    unsigned short vertex_count)
{
#ifdef GAME_DLL
    Call_Function<void, const SphereClass &, unsigned short, unsigned short, unsigned short, unsigned short>(
        PICK_ADDRESS(0x0080C610, 0x00569570), bounding_sphere, start_index, polygon_count, min_vertex_index, vertex_count);
#endif
}

void SortingRendererClass::Flush()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x0080DBB0, 0x0056B0D0));
#endif
}

void SortingRendererClass::Deinit()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x0080E1E0, 0x0056B6B0));
#endif
}

void SortingRendererClass::SetMinVertexBufferSize(unsigned int val)
{
#ifdef GAME_DLL
    Call_Function<void, unsigned int>(PICK_ADDRESS(0x0080C570, 0x00569410), val);
#endif
}
