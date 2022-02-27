/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief
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
#include "smudge.h"

class IndexBufferClass;
class RenderInfoClass;

class W3DSmudgeManager : public SmudgeManager
{
public:
    enum
    {
        NUM_SMUDGES = 500,
        SMUDGE_INDEX_COUNT = 12,
        SMUDGE_VERTEX_COUNT = 5,
        SMUDGE_POLY_COUNT = 4,
    };

    W3DSmudgeManager()
        // #BUGFIX Initialize all members
        :
        unk1{}, unk2{}, unk3{}, unk4{}, m_indexBuffer(nullptr), m_surfaceWidth{}, m_surfaceHeight{}
    {
    }
    virtual ~W3DSmudgeManager() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Release_Resources() override;
    virtual void Re_Acquire_Resources() override;
    bool Test_Hardware_Support();
    void Render(RenderInfoClass &rinfo);
    W3DSmudgeManager *Hook_Ctor() { return new (this) W3DSmudgeManager; }

private:
    int unk1;
    int unk2;
    int unk3;
    int unk4;
    IndexBufferClass *m_indexBuffer;
    unsigned int m_surfaceWidth;
    unsigned int m_surfaceHeight;
};

#ifdef GAME_DLL
extern W3DSmudgeManager *&g_theSmudgeManager;
#else
extern W3DSmudgeManager *g_theSmudgeManager;
#endif
