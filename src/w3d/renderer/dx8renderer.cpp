/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Mesh Renderer
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "dx8renderer.h"
#ifndef GAME_DLL
DX8MeshRendererClass g_theDX8MeshRenderer;
DynamicVectorClass<Vector3> g_tempVertexBuffer;
DynamicVectorClass<Vector3> g_tempNormalBuffer;
MultiListClass<DX8TextureCategoryClass> g_textureCategoryDeleteList;
MultiListClass<DX8FVFCategoryContainer> g_fvfCategoryContainerDeleteList;
#else
#include "hooker.h"
#endif

void DX8MeshRendererClass::Init()
{
    if (!m_textureCategoryContainerListSkin) {
        m_textureCategoryContainerListSkin = new MultiListClass<DX8FVFCategoryContainer>;
    }
}

void DX8MeshRendererClass::Shutdown()
{
    Invalidate(true);
    Clear_Pending_Delete_Lists();
    g_tempVertexBuffer.Clear();
    g_tempNormalBuffer.Clear();
}

void DX8MeshRendererClass::Invalidate(bool shutdown)
{
#ifdef GAME_DLL
    Call_Method<void, DX8MeshRendererClass, bool>(PICK_ADDRESS(0x00827390, 0x005171C0), this, shutdown);
#endif
}

void DX8MeshRendererClass::Clear_Pending_Delete_Lists()
{
    for (MultiListObjectClass *i = g_textureCategoryDeleteList.Remove_Head(); i;
         i = g_textureCategoryDeleteList.Remove_Head()) {
        delete i;
    }
    for (MultiListObjectClass *i = g_fvfCategoryContainerDeleteList.Remove_Head(); i;
         i = g_fvfCategoryContainerDeleteList.Remove_Head()) {
        delete i;
    }
}
