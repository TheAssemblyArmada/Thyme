/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Class for holding a list of texture loading tasks.
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

class TextureLoadTaskClass;
struct TextureLoadTaskListNode;

class TextureLoadTaskListClass
{
public:
    TextureLoadTaskListClass();

    void Push_Front(TextureLoadTaskClass *task);
    void Push_Back(TextureLoadTaskClass *task);
    TextureLoadTaskClass *Pop_Front();
    TextureLoadTaskClass *Pop_Back();
    void Remove(TextureLoadTaskClass *task);
    bool Empty() { return m_head == reinterpret_cast<__mayalias TextureLoadTaskListNode *>(this); }

private:
    TextureLoadTaskListNode *m_head;
    TextureLoadTaskListNode *m_tail;
};

#ifdef GAME_DLL
extern TextureLoadTaskListClass &g_freeList;
extern TextureLoadTaskListClass &g_cubeFreeList;
extern TextureLoadTaskListClass &g_volFreeList;
#else
extern TextureLoadTaskListClass g_freeList;
extern TextureLoadTaskListClass g_cubeFreeList;
extern TextureLoadTaskListClass g_volFreeList;
#endif
