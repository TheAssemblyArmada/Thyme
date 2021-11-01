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
#include "cullsys.h"

CullableClass::CullableClass() : m_cullLink(nullptr), m_nextCollected(nullptr)
{
    m_cullBox.Init(Vector3(0, 0, 0), Vector3(1, 1, 1));
}

CullableClass::~CullableClass()
{
    captainslog_assert(m_cullLink == nullptr);
}

void CullableClass::Set_Cull_Box(const AABoxClass &box, bool just_loaded)
{
    m_cullBox = box;

    if (!just_loaded) {
        CullSystemClass *sys = Get_Culling_System();

        if (sys != nullptr) {
            sys->Update_Culling(this);
        }
    }
}

void CullableClass::Set_Culling_System(CullSystemClass *sys)
{
    if (m_cullLink) {
        m_cullLink->Set_Culling_System(sys);
    }
}

CullSystemClass *CullableClass::Get_Culling_System() const
{
    if (m_cullLink) {
        return m_cullLink->Get_Culling_System();
    }

    return nullptr;
}

CullSystemClass::CullSystemClass() : m_collectionHead(nullptr) {}

CullSystemClass::~CullSystemClass() {}

CullableClass *CullSystemClass::Get_First_Collected_Object_Internal()
{
    return m_collectionHead;
}

CullableClass *CullSystemClass::Get_Next_Collected_Object_Internal(CullableClass *obj)
{
    if (obj != nullptr) {
        return obj->m_nextCollected;
    }

    return nullptr;
}

CullableClass *CullSystemClass::Peek_First_Collected_Object_Internal()
{
    return m_collectionHead;
}

CullableClass *CullSystemClass::Peek_Next_Collected_Object_Internal(CullableClass *obj)
{
    if (obj != nullptr) {
        return obj->m_nextCollected;
    }

    return nullptr;
}

void CullSystemClass::Reset_Collection()
{
    m_collectionHead = nullptr;
}

void CullSystemClass::Add_To_Collection(CullableClass *obj)
{
    captainslog_assert(obj != nullptr);
    obj->m_nextCollected = m_collectionHead;
    m_collectionHead = obj;
}
