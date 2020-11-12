/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Decal Mesh Class
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
#include "refcount.h"
#include "simplevec.h"
#include "vector.h"
#include "vector3.h"

class MeshClass;
class DecalSystemClass;
class DecalGeneratorClass;
class OBBoxClass;

class DecalMeshClass : public RefCountClass
{
public:
    DecalMeshClass(MeshClass *parent, DecalSystemClass *system);
    virtual ~DecalMeshClass();

    MeshClass *Peek_Parent() { return m_parent; }
    DecalSystemClass *Peek_System() { return m_decalSystem; }
    DecalMeshClass *Peek_Next_Visible() { return m_nextVisible; }

    void Set_Next_Visible(DecalMeshClass *next) { m_nextVisible = next; }

    virtual void Render() = 0;

    virtual bool Create_Decal(DecalGeneratorClass *generator,
        const OBBoxClass &box,
        SimpleDynVecClass<uint32_t> &apt,
        const DynamicVectorClass<Vector3> *world_positions) = 0;
    virtual bool Delete_Decal(unsigned long decal_id) = 0;
    virtual int Decal_Count() = 0;
    virtual unsigned long Get_Decal_ID(int index) = 0;

protected:
    MeshClass *m_parent;
    DecalSystemClass *m_decalSystem;
    DecalMeshClass *m_nextVisible;
};
