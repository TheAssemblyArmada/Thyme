/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Static class for handling texture loading.
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
#include "thread.h"
#include "w3dformat.h"
#include "w3dtypes.h"

class StringClass;
class TextureBaseClass;
class TextureLoadTaskClass;
class Vector3;

class LoaderThreadClass : public ThreadClass
{
public:
    LoaderThreadClass(const char *thread_name) : ThreadClass(thread_name, nullptr) {}
    virtual ~LoaderThreadClass() {}

    virtual void Thread_Function() override;
};

class TextureLoader
{
public:
    static void Init();
    static void Deinit();
    static bool Is_DX8_Thread();
    static void Validate_Texture_Size(unsigned &width, unsigned &height, unsigned &volume);
    static w3dtexture_t Load_Thumbnail(const StringClass &texture, const Vector3 &adjust);
    static w3dsurface_t Load_Surface_Immediate(const StringClass &texture, WW3DFormat format, bool allow_compressed);
    static void Request_Thumbnail(TextureBaseClass *texture);
    static void Request_Background_Loading(TextureBaseClass *texture);
    static void Request_Foreground_Loading(TextureBaseClass *texture);
    static void Flush_Pending_Load_Tasks();
    static void Update(void (*update)(void));
    static void Suspend_Texture_Load() { s_textureLoadSuspended = true; }
    static void Continue_Texture_Load() { s_textureLoadSuspended = false; }
    static void Process_Foreground_Thumbnail(TextureLoadTaskClass *task);
    static void Process_Foreground_Load(TextureLoadTaskClass *task);
    static void Begin_Load_And_Queue(TextureLoadTaskClass *task);
    static void Load_Thumbnail(TextureBaseClass *texture);

private:
    static bool Queues_Not_Empty();
    static bool Is_Format_Compressed(WW3DFormat format, bool allow_compressed);
#ifdef GAME_DLL
    static unsigned &s_textureInactiveOverrideTime;
    static LoaderThreadClass &s_textureLoadThread;
    static bool &s_textureLoadSuspended;
#else
    static unsigned s_textureInactiveOverrideTime;
    static LoaderThreadClass s_textureLoadThread;
    static bool s_textureLoadSuspended;
#endif
};
