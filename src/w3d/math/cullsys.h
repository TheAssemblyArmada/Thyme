/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Culling Systems
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
#include "aabox.h"
#include "refcount.h"

class CullableClass;
class CullSystemClass;
class FrustumClass;
class OBBoxClass;

class CullLinkClass
{
public:
    inline CullLinkClass(CullSystemClass *system)
    {
        m_system = system;
        captainslog_assert(m_system);
    }
    ~CullLinkClass() { captainslog_assert(m_system == nullptr); }

    inline void Set_Culling_System(CullSystemClass *sys) { m_system = sys; }
    inline CullSystemClass *Get_Culling_System() { return m_system; }

protected:
    CullSystemClass *m_system;
};

class CullableClass : public RefCountClass
{
public:
    CullableClass();
    virtual ~CullableClass();

    inline const AABoxClass &Get_Cull_Box() const { return m_cullBox; }
    void Set_Cull_Box(const AABoxClass &box, bool just_loaded = false);

    void Set_Culling_System(CullSystemClass *sys);
    CullSystemClass *Get_Culling_System() const;
    inline void Set_Cull_Link(CullLinkClass *c) { m_cullLink = c; }
    inline CullLinkClass *Get_Cull_Link() const { return m_cullLink; }

private:
    inline void Set_Next_Collected(CullableClass *c) { m_nextCollected = c; }
    inline CullableClass *Get_Next_Collected() { return m_nextCollected; }

    CullLinkClass *m_cullLink;
    AABoxClass m_cullBox;
    CullableClass *m_nextCollected;

    CullableClass(const CullableClass &src);
    CullableClass &operator=(const CullableClass &src);

    friend class CullSystemClass;
};

class CullSystemClass
{
public:
    CullSystemClass();
    virtual ~CullSystemClass();

    void Reset_Collection();
    virtual void Collect_Objects(const Vector3 &point) = 0;
    virtual void Collect_Objects(const AABoxClass &box) = 0;
    virtual void Collect_Objects(const OBBoxClass &box) = 0;
    virtual void Collect_Objects(const FrustumClass &frustum) = 0;
    virtual void Update_Culling(CullableClass *obj) = 0;

protected:
    CullableClass *Get_First_Collected_Object_Internal();
    CullableClass *Get_Next_Collected_Object_Internal(CullableClass *obj);
    CullableClass *Peek_First_Collected_Object_Internal();
    CullableClass *Peek_Next_Collected_Object_Internal(CullableClass *obj);

    void Add_To_Collection(CullableClass *obj);

    CullableClass *m_collectionHead;

    friend class CullableClass;
};
