/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Class for holding a list of texture loading tasks with syncronization.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "synctextureloadtasklist.h"

#ifndef GAME_DLL
SynchronizedTextureLoadTaskListClass g_foregroundQueue;
SynchronizedTextureLoadTaskListClass g_backgroundQueue;
#endif

void SynchronizedTextureLoadTaskListClass::Push_Front(TextureLoadTaskClass *task)
{
    FastCriticalSectionClass::LockClass lock(m_mutex);
    TextureLoadTaskListClass::Push_Front(task);
}

void SynchronizedTextureLoadTaskListClass::Push_Back(TextureLoadTaskClass *task)
{
    FastCriticalSectionClass::LockClass lock(m_mutex);
    TextureLoadTaskListClass::Push_Back(task);
}

TextureLoadTaskClass *SynchronizedTextureLoadTaskListClass::Pop_Front()
{
    FastCriticalSectionClass::LockClass lock(m_mutex);
    return TextureLoadTaskListClass::Pop_Front();
}

TextureLoadTaskClass *SynchronizedTextureLoadTaskListClass::Pop_Back()
{
    FastCriticalSectionClass::LockClass lock(m_mutex);
    return TextureLoadTaskListClass::Pop_Back();
}

void SynchronizedTextureLoadTaskListClass::Remove(TextureLoadTaskClass *task)
{
    FastCriticalSectionClass::LockClass lock(m_mutex);
    TextureLoadTaskListClass::Remove(task);
}