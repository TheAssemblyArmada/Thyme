/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Object Iterator
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "simpleobjectiterator.h"
#include "object.h"

float (*SimpleObjectIterator::s_theClumpCompareProcs[5])(
    Clump *, Clump *) = { nullptr, Sort_Near_To_Far, Sort_Far_To_Near, Sort_Cheap_To_Expensive, Sort_Expensive_To_Cheap };

SimpleObjectIterator::Clump::Clump() : m_nextClump(nullptr) {}

SimpleObjectIterator::SimpleObjectIterator() : m_firstClump(nullptr), m_curClump(nullptr), m_clumpCount(0) {}

SimpleObjectIterator::~SimpleObjectIterator()
{
    Make_Empty();
}

void SimpleObjectIterator::Insert(Object *obj, float numeric)
{
    captainslog_dbgassert(obj != nullptr, "sorry, no nulls allowed here");
    Clump *clump = new Clump;
    clump->m_nextClump = m_firstClump;
    m_firstClump = clump;
    clump->m_obj = obj;
    clump->m_numeric = numeric;
    m_clumpCount++;
}

void SimpleObjectIterator::Make_Empty()
{
    while (m_firstClump != nullptr) {
        Clump *next = m_firstClump->m_nextClump;
        m_firstClump->Delete_Instance();
        m_firstClump = next;
        m_clumpCount--;
    }

    captainslog_dbgassert(m_clumpCount == 0, "hmm");
    m_firstClump = nullptr;
    m_curClump = nullptr;
    m_clumpCount = 0;
}

void SimpleObjectIterator::Sort(IterOrderType iter)
{
    if (m_clumpCount != 0) {
        auto sort = s_theClumpCompareProcs[iter];

        if (sort != nullptr) {
            for (int i = 1;; i *= 2) {
                Clump *clump = m_firstClump;
                Clump *clump2 = nullptr;
                m_firstClump = nullptr;
                int count = 0;

                while (clump != nullptr) {
                    count++;
                    int to_do_count = 0;
                    Clump *clump3 = clump;

                    for (int j = 0; j < i; j++) {
                        to_do_count++;
                        clump3 = clump3->m_nextClump;

                        if (clump3 == nullptr) {
                            break;
                        }
                    }

                    int sub_count = clump3 != nullptr ? i : 0;
                    captainslog_dbgassert(sub_count + to_do_count >= 0, "uhoh");

                    while (sub_count + to_do_count > 0) {
                        Clump *clump4;

                        captainslog_dbgassert(sub_count + to_do_count >= 0, "uhoh");

                        if (sub_count == 0) {
                            captainslog_dbgassert(to_do_count > 0, "hmm, expected nonzero to_do_count");
                            clump4 = clump;
                            clump = clump->m_nextClump;
                            to_do_count--;
                        } else if (to_do_count == 0) {
                            captainslog_dbgassert(sub_count > 0, "hmm, expected nonzero subCount");
                            clump4 = clump3;
                            clump3 = clump3->m_nextClump;
                            sub_count--;
                        } else if (sort(clump, clump3) <= 0.0f) {
                            captainslog_dbgassert(to_do_count > 0, "hmm, expected nonzero to_do_count");
                            clump4 = clump;
                            clump = clump->m_nextClump;
                            to_do_count--;
                        } else {
                            captainslog_dbgassert(sub_count > 0, "hmm, expected nonzero subCount");
                            clump4 = clump3;
                            clump3 = clump3->m_nextClump;
                            sub_count--;
                        }

                        if (clump3 == nullptr) {
                            sub_count = 0;
                        }

                        if (clump == nullptr) {
                            to_do_count = 0;
                        }

                        if (clump2 != nullptr) {
                            clump2->m_nextClump = clump4;
                        } else {
                            m_firstClump = clump4;
                        }

                        clump2 = clump4;
                    }

                    clump = clump3;
                }

                if (clump2 != nullptr) {
                    clump2->m_nextClump = nullptr;
                }

                if (count <= 1) {
                    break;
                }
            }

            Reset();
        }
    }
}

float SimpleObjectIterator::Sort_Near_To_Far(Clump *a, Clump *b)
{
    return a->m_numeric - b->m_numeric;
}

float SimpleObjectIterator::Sort_Far_To_Near(Clump *a, Clump *b)
{
    return b->m_numeric - a->m_numeric;
}

float SimpleObjectIterator::Sort_Cheap_To_Expensive(Clump *a, Clump *b)
{
    return a->m_obj->Get_Template()->Get_Build_Cost() - b->m_obj->Get_Template()->Get_Build_Cost();
}

float SimpleObjectIterator::Sort_Expensive_To_Cheap(Clump *a, Clump *b)
{
    return b->m_obj->Get_Template()->Get_Build_Cost() - a->m_obj->Get_Template()->Get_Build_Cost();
}
