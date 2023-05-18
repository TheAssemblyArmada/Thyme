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
#include "smudge.h"
DLListClass<Smudge> SmudgeSet::s_freeSmudgeList;

SmudgeSet::SmudgeSet() : m_smudgeCount(0) {}

SmudgeSet::~SmudgeSet()
{
    Reset();
}

Smudge *SmudgeSet::Add_Smudge_To_Set()
{
    Smudge *smudge = s_freeSmudgeList.Head();

    if (smudge != nullptr) {
        smudge->Remove();
        m_smudgeList.Add_Tail(smudge);
        m_smudgeCount++;
        return smudge;
    } else {
        smudge = new Smudge();
        m_smudgeList.Add_Tail(smudge);
        m_smudgeCount++;
        return smudge;
    }
}

void SmudgeSet::Remove_Smudge_From_Set(Smudge &smudge)
{
    smudge.Remove();
    s_freeSmudgeList.Add_Head(&smudge);
    m_smudgeCount--;
}

void SmudgeSet::Reset()
{
    for (;;) {
        Smudge *smudge = m_smudgeList.Head();

        if (smudge == nullptr) {
            break;
        }

        m_smudgeList.Remove_Head();
        s_freeSmudgeList.Add_Head(smudge);
    }
}

SmudgeManager::SmudgeManager() : m_hardwareTestResult(HARDWARE_TEST_NOT_RUN), m_heatHazeCount(0) {}

SmudgeManager::~SmudgeManager()
{
    Reset();

    for (;;) {
        SmudgeSet *set = m_smudgeList.Head();

        if (set == nullptr) {
            break;
        }

        m_smudgeList.Remove_Head();
        delete set;
    }

    for (;;) {
        Smudge *smudge = SmudgeSet::s_freeSmudgeList.Head();

        if (smudge == nullptr) {
            break;
        }

        // BUGFIX this was calling m_freeSmudgeList.Remove_Head not SmudgeSet::m_freeSmudgeList.Remove_Head
        SmudgeSet::s_freeSmudgeList.Remove_Head();
        delete smudge;
    }
}

void SmudgeManager::Reset()
{
    for (;;) {
        SmudgeSet *set = m_smudgeList.Head();

        if (set == nullptr) {
            break;
        }

        m_smudgeList.Remove_Head();
        set->Reset();
        m_freeSmudgeList.Add_Tail(set);
    }
}

SmudgeSet *SmudgeManager::Add_Smudge_Set()
{
    SmudgeSet *set = m_freeSmudgeList.Head();

    if (set != nullptr) {
        set->Remove();
        m_smudgeList.Add_Tail(set);
        return set;
    } else {
        set = new SmudgeSet();
        m_smudgeList.Add_Tail(set);
        return set;
    }
}

void SmudgeManager::Remove_Smudge_Set(SmudgeSet &set)
{
    set.Remove();
    m_freeSmudgeList.Add_Head(&set);
}
