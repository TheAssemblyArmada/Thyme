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
#pragma once

#include "always.h"
#include "critsection.h"
#include "textureloadtasklist.h"

class SynchronizedTextureLoadTaskListClass : public TextureLoadTaskListClass
{
public:
    void Push_Front(TextureLoadTaskClass *task);
    void Push_Back(TextureLoadTaskClass *task);
    TextureLoadTaskClass *Pop_Front();
    TextureLoadTaskClass *Pop_Back();
    void Remove(TextureLoadTaskClass *task);

private:
    bool m_unused;
    FastCriticalSectionClass m_mutex;
};

#ifdef GAME_DLL
extern SynchronizedTextureLoadTaskListClass &g_foregroundQueue;
extern SynchronizedTextureLoadTaskListClass &g_backgroundQueue;
#else
extern SynchronizedTextureLoadTaskListClass g_foregroundQueue;
extern SynchronizedTextureLoadTaskListClass g_backgroundQueue;
#endif
