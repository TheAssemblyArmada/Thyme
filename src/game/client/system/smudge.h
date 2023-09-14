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
#include "dllist.h"
#include "vector2.h"
#include "vector3.h"

struct SmudgeCoords
{
    Vector3 m_position;
    Vector2 m_uv;
};

class Smudge : public DLNodeClass<Smudge>
{
    IMPLEMENT_W3D_POOL(Smudge)
public:
    Smudge() {}
    virtual ~Smudge() override {}

private:
    Vector3 m_smudgePos;
    Vector2 m_smudgeRandom;
    float m_smudgeSize;
    float m_smudgeAlpha;
    SmudgeCoords m_smudgeStructs[5];
    friend class W3DSmudgeManager;
    friend class W3DParticleSystemManager;
};

class SmudgeSet : public DLNodeClass<SmudgeSet>
{
    IMPLEMENT_W3D_POOL(SmudgeSet)
public:
    SmudgeSet();
    virtual ~SmudgeSet() override;

    Smudge *Add_Smudge_To_Set();
    void Remove_Smudge_From_Set(Smudge &smudge);
    void Reset();

    DLListClass<Smudge> &Get_Smudge_List() { return m_smudgeList; }

private:
    DLListClass<Smudge> m_smudgeList;
    int m_smudgeCount;
    static DLListClass<Smudge> s_freeSmudgeList;
    friend class SmudgeManager;
    friend class W3DSmudgeManager;
};

class SmudgeManager
{
public:
    enum HardwareTestResult
    {
        HARDWARE_TEST_NOT_RUN,
        HARDWARE_TEST_FAIL,
        HARDWARE_TEST_SUCCEED,
    };

    SmudgeManager();
    virtual ~SmudgeManager();

    virtual void Init() {}
    virtual void Reset();
    virtual void Release_Resources() {}
    virtual void Re_Acquire_Resources() {}

    SmudgeSet *Add_Smudge_Set();
    void Remove_Smudge_Set(SmudgeSet &set);

    int Get_Heat_Haze_Count() { return m_heatHazeCount; }
    void Set_Heat_Haze_Count(int count) { m_heatHazeCount = count; }
    HardwareTestResult Get_Hardware_Test_Result() { return m_hardwareTestResult; }

protected:
    HardwareTestResult m_hardwareTestResult;
    DLListClass<SmudgeSet> m_smudgeList;
    DLListClass<SmudgeSet> m_freeSmudgeList;
    int m_heatHazeCount;
};
