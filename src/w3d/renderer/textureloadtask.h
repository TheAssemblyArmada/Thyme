/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 * @author tomsons26
 *
 * @brief Class for holding a texture loading task for the loader.
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
#include "vector3.h"
#include "w3dformat.h"
#include "w3dtypes.h"

class TextureLoadTaskListClass;
class TextureBaseClass;

struct TextureLoadTaskListNode
{
    TextureLoadTaskListNode *succ;
    TextureLoadTaskListNode *prev;
};

class TextureLoadTaskClass
{
    friend class TextureLoadTaskListClass;

    enum
    {
        MAX_SURFACES = 12,
    };

public:
    enum TaskType
    {
        TASK_NONE,
        TASK_THUMBNAIL,
        TASK_LOAD,
    };

    enum PriorityType
    {
        PRIORITY_BACKGROUND,
        PRIORITY_FOREGROUND,
    };

    enum StateType
    {
        STATE_NONE,
        STATE_LOAD_BEGUN,
    };

    TextureLoadTaskClass();
    ~TextureLoadTaskClass(); // Original does not have the destructor virtual, so we need to match it.

    virtual void Destroy();
    virtual void Init(TextureBaseClass *texture, TaskType type, PriorityType priority);
    virtual void Deinit();
    virtual void Begin_Compressed_Load();
    virtual void Begin_Uncompressed_Load();
    virtual void Load_Compressed_Mipmap();
    virtual void Load_Uncompressed_Mipmap();
    virtual void Lock_Surfaces();
    virtual void Unlock_Surfaces();

    static void Delete_Free_Pool();

private:
    static void Get_Texture_Information(const char *name, unsigned &reduction, unsigned &width, unsigned &height, unsigned &depth, WW3DFormat &format, unsigned &levels, bool use_dds);

protected:
    TextureLoadTaskListNode m_listNode;
    TextureLoadTaskListClass *m_parent;
    TextureBaseClass *m_texture;
    w3dbasetexture_t m_d3dTexture;
    WW3DFormat m_format;
    unsigned m_width;
    unsigned m_height;
    unsigned m_mipLevelCount;
    unsigned m_reduction;
    Vector3 m_hsvAdjust;
    char *m_lockedSurfacePtr[MAX_SURFACES];
    int m_lockedSurfacePitch[MAX_SURFACES];
    TaskType m_type;
    PriorityType m_priority;
    StateType m_loadState;
};

#ifndef THYME_STANDALONE
#include "hooker.h"
#endif
