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
#include "texturebase.h"
#include "vector3.h"
#include "w3dformat.h"
#include "w3dtypes.h"

class TextureLoadTaskListClass;

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
        STATE_LOADED,
        STATE_LOAD_ENDED,
        STATE_4,
    };

    TextureLoadTaskClass();
    ~TextureLoadTaskClass(); // Original does not have the destructor virtual, so we need to match it.

    virtual void Destroy();
    virtual void Init(TextureBaseClass *texture, TaskType type, PriorityType priority);
    virtual void Deinit();
    virtual bool Begin_Compressed_Load();
    virtual bool Begin_Uncompressed_Load();
    virtual bool Load_Compressed_Mipmap();
    virtual bool Load_Uncompressed_Mipmap();
    virtual void Lock_Surfaces();
    virtual void Unlock_Surfaces();

    void Apply(bool initialized);
    void Apply_Missing_Texture();
    bool Begin_Load();
    bool Load();
    void End_Load();
    void Finish_Load();
    uint8_t *Get_Locked_Surface_Pointer(unsigned level);
    int Get_Locked_Surface_Pitch(unsigned level);
    StateType Get_Load_State() const { return m_loadState; }
    TaskType Get_Task_Type() const { return m_type; }
    void Set_Load_State(StateType state) { m_loadState = state; }
    void Set_Priority(PriorityType priority) { m_priority = priority; }
    PriorityType Get_Priority() const { return m_priority; }
    TextureLoadTaskListClass *Get_Parent() { return m_parent; }
    TextureBaseClass *Get_Texture() { return m_texture; }
    bool Allow_Compression() const { return m_texture->m_compressionAllowed; }

    static void Delete_Free_Pool();
    static TextureLoadTaskClass *Create(TextureBaseClass *texture, TaskType type, PriorityType priority);
#ifdef GAME_DLL
    static void Hook_Me();
    TextureLoadTaskClass *Hook_Ctor() { return new (this) TextureLoadTaskClass; }
    void Hook_Dtor() { TextureLoadTaskClass::~TextureLoadTaskClass(); }
    void Hook_Destroy() { TextureLoadTaskClass::Destroy(); }
    void Hook_Init(TextureBaseClass *texture, TaskType type, PriorityType priority)
    {
        TextureLoadTaskClass::Init(texture, type, priority);
    }
    void Hook_Deinit() { TextureLoadTaskClass::Deinit(); }
    bool Hook_Begin_Compressed_Load() { return TextureLoadTaskClass::Begin_Compressed_Load(); }
    bool Hook_Begin_Uncompressed_Load() { return TextureLoadTaskClass::Begin_Uncompressed_Load(); }
    bool Hook_Load_Compressed_Mipmap() { return TextureLoadTaskClass::Load_Compressed_Mipmap(); }
    bool Hook_Load_Uncompressed_Mipmap() { return TextureLoadTaskClass::Load_Uncompressed_Mipmap(); }
    void Hook_Lock_Surfaces() { TextureLoadTaskClass::Lock_Surfaces(); }
    void Hook_Unlock_Surfaces() { TextureLoadTaskClass::Unlock_Surfaces(); }
#endif

private:
    static bool Get_Texture_Information(const char *name, unsigned &reduction, unsigned &width, unsigned &height,
        unsigned &depth, WW3DFormat &format, unsigned &levels, bool use_dds);

protected:
    TextureLoadTaskListNode m_listNode;
    TextureLoadTaskListClass *m_parent;
    TextureBaseClass *m_texture;
    w3dtexture_t m_d3dTexture;
    WW3DFormat m_format;
    unsigned m_width;
    unsigned m_height;
    unsigned m_mipLevelCount;
    unsigned m_reduction;
    Vector3 m_hsvAdjust;
    uint8_t *m_lockedSurfacePtr[MAX_SURFACES];
    int m_lockedSurfacePitch[MAX_SURFACES];
    TaskType m_type;
    PriorityType m_priority;
    StateType m_loadState;
};

#ifdef GAME_DLL
#include "hooker.h"

inline void TextureLoadTaskClass::Hook_Me()
{
    Hook_Method(0x0082FF20, &Hook_Ctor);
    Hook_Method(0x0082FF80, &Hook_Dtor);
    Hook_Method(0x008301D0, &Hook_Destroy);
    Hook_Method(0x00830210, &Hook_Init);
    Hook_Method(0x00831160, &Hook_Deinit);
    Hook_Method(0x008303B0, &Hook_Begin_Compressed_Load);
    Hook_Method(0x00830950, &Hook_Begin_Uncompressed_Load);
    Hook_Method(0x00830B30, &Hook_Load_Compressed_Mipmap);
    Hook_Method(0x00830C40, &Hook_Load_Uncompressed_Mipmap);
    Hook_Method(0x00830A80, &Hook_Lock_Surfaces);
    Hook_Method(0x00830AE0, &Hook_Unlock_Surfaces);
    Hook_Method(0x00830380, &Apply);
    Hook_Method(0x008302D0, &Begin_Load);
    Hook_Method(0x00830310, &Load);
    Hook_Method(0x00830340, &End_Load);
    Hook_Function(0x0082FFD0, &Create);
    Hook_Function(0x008305F0, &Get_Texture_Information);
}
#endif
