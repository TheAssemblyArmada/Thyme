/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Keeps track of textures allocated from the graphics library.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "dx8texman.h"
#include "dx8wrapper.h"
#include <captainslog.h>

DX8TextureTrackerList DX8TextureManagerClass::s_managedTextures;

void DX8TextureTrackerClass::Recreate()
{
#ifdef BUILD_WITH_D3D8
    captainslog_assert(m_texture->Peek_Platform_Base_Texture() == nullptr);
    m_texture->Set_Platform_Base_Texture(
        DX8Wrapper::Create_Texture(m_width, m_height, m_format, m_mipLevelCount, D3DPOOL_DEFAULT, m_renderTarget));
#endif
}

void DX8TextureManagerClass::Add(DX8TextureTrackerClass *tracker)
{
    s_managedTextures.Add(tracker);
}

void DX8TextureManagerClass::Remove(TextureBaseClass *texture)
{
    MultiListIterator<DX8TextureTrackerClass> it(&s_managedTextures);

    for (it.First(); !it.Is_Done(); it.Next()) {
        DX8TextureTrackerClass *tracker = it.Peek_Obj();
        captainslog_assert(tracker != nullptr);

        if (tracker->m_texture == texture) {
            it.Remove_Current_Object();
            delete tracker;
        }
    }
}

void DX8TextureManagerClass::Release_Textures()
{
    MultiListIterator<DX8TextureTrackerClass> it(&s_managedTextures);

    for (it.First(); !it.Is_Done(); it.Next()) {
        it.Peek_Obj()->m_texture->Set_Platform_Base_Texture(W3D_TYPE_INVALID_TEXTURE);
    }
}

void DX8TextureManagerClass::Recreate_Textures()
{
    MultiListIterator<DX8TextureTrackerClass> it(&s_managedTextures);

    for (it.First(); !it.Is_Done(); it.Next()) {
        DX8TextureTrackerClass *ttc = it.Peek_Obj();
        ttc->Recreate();
        ttc->m_texture->Set_Dirty(true);
    }
}

void DX8TextureManagerClass::Shutdown()
{
    while (!s_managedTextures.Is_Empty()) {
        delete s_managedTextures.Remove_Head();
    }
}
