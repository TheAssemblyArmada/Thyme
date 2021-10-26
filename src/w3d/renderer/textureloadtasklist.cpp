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
#include "textureloadtasklist.h"
#include "textureloadtask.h"

#ifndef GAME_DLL
TextureLoadTaskListClass g_freeList;
TextureLoadTaskListClass g_cubeFreeList;
TextureLoadTaskListClass g_volFreeList;
#endif

TextureLoadTaskListClass::TextureLoadTaskListClass()
{
    m_tail = reinterpret_cast<__mayalias TextureLoadTaskListNode *>(this);
    m_head = reinterpret_cast<__mayalias TextureLoadTaskListNode *>(this);
}

/**
 * Pushes a task to the front of the list.
 *
 * 0x0082E970
 */
void TextureLoadTaskListClass::Push_Front(TextureLoadTaskClass *task)
{
    task->m_listNode.succ = m_head;
    task->m_listNode.prev = reinterpret_cast<__mayalias TextureLoadTaskListNode *>(this);
    task->m_parent = this;
    m_head->prev = &task->m_listNode;
    m_head = &task->m_listNode;
}

/**
 * Pushes a task to the back of the list.
 */
void TextureLoadTaskListClass::Push_Back(TextureLoadTaskClass *task)
{
    task->m_listNode.succ = reinterpret_cast<__mayalias TextureLoadTaskListNode *>(this);
    task->m_listNode.prev = m_tail;
    task->m_parent = this;
    m_tail->succ = &task->m_listNode;
    m_tail = &task->m_listNode;
}

/**
 * Pops a task from the front of the list.
 */
TextureLoadTaskClass *TextureLoadTaskListClass::Pop_Front()
{
    if (m_head == reinterpret_cast<__mayalias TextureLoadTaskListNode *>(this)) {
        return nullptr;
    } else {
        TextureLoadTaskClass *task;

        if (m_head != nullptr) {
            task = reinterpret_cast<__mayalias TextureLoadTaskClass *>(
                reinterpret_cast<char *>(m_head) - offsetof(TextureLoadTaskClass, m_listNode));
        } else {
            task = nullptr;
        }

        Remove(task);

        return task;
    }
}

/**
 * Pops a task from the back of the list.
 */
TextureLoadTaskClass *TextureLoadTaskListClass::Pop_Back()
{
    if (m_head == reinterpret_cast<__mayalias TextureLoadTaskListNode *>(this)) {
        return nullptr;
    } else {
        TextureLoadTaskClass *task;

        if (m_tail != nullptr) {
            task = reinterpret_cast<__mayalias TextureLoadTaskClass *>(
                reinterpret_cast<char *>(m_tail) - offsetof(TextureLoadTaskClass, m_listNode));
        } else {
            task = nullptr;
        }

        Remove(task);

        return task;
    }
}

/**
 * Removes a task from the list.
 *
 * 0x0082E9A0
 */
void TextureLoadTaskListClass::Remove(TextureLoadTaskClass *task)
{
    // #BUGFIX Avoid task null dereference.

    if (task != nullptr && task->m_parent == this) {
        task->m_listNode.prev->succ = task->m_listNode.succ;
        task->m_listNode.succ->prev = task->m_listNode.prev;
        task->m_listNode.succ = nullptr;
        task->m_listNode.prev = nullptr;
        task->m_parent = nullptr;
    }
}
