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
#pragma once
#include "always.h"
#include "mempoolobj.h"
#include "partitionmanager.h"
class Object;

class ObjectIterator : public MemoryPoolObject
{
public:
    ObjectIterator() {}
    virtual ~ObjectIterator() override {}
    virtual Object *First() = 0;
    virtual Object *Next() = 0;
};

class SimpleObjectIterator : public ObjectIterator
{
    IMPLEMENT_NAMED_POOL(SimpleObjectIterator, SimpleObjectIteratorPool);

    struct Clump : public MemoryPoolObject
    {
        Clump();
        ~Clump() {}
        IMPLEMENT_NAMED_POOL(Clump, SimpleObjectIteratorClumpPool);
        Clump *m_nextClump;
        Object *m_obj;
        float m_numeric;
    };

public:
    SimpleObjectIterator();

    virtual ~SimpleObjectIterator() override;
    virtual Object *First() override { return First_With_Numeric(nullptr); }
    virtual Object *Next() override { return Next_With_Numeric(nullptr); }

    void Insert(Object *obj, float numeric);
    void Make_Empty();
    void Sort(IterOrderType iter);
    static float Sort_Near_To_Far(Clump *a, Clump *b);
    static float Sort_Far_To_Near(Clump *a, Clump *b);
    static float Sort_Cheap_To_Expensive(Clump *a, Clump *b);
    static float Sort_Expensive_To_Cheap(Clump *a, Clump *b);

    void Reset() { m_curClump = m_firstClump; }

    Object *First_With_Numeric(float *numeric)
    {
        Reset();
        return Next_With_Numeric(numeric);
    }

    Object *Next_With_Numeric(float *numeric)
    {
        Object *obj = nullptr;

        if (numeric != nullptr) {
            *numeric = 0.0f;
        }

        if (m_curClump) {
            obj = m_curClump->m_obj;

            if (numeric != nullptr) {
                *numeric = m_curClump->m_numeric;
            }

            m_curClump = m_curClump->m_nextClump;
        }

        return obj;
    }

private:
    Clump *m_firstClump;
    Clump *m_curClump;
    int m_clumpCount;
    static float (*s_theClumpCompareProcs[5])(Clump *, Clump *);
};
