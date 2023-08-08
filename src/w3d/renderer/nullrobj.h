/**
 * @file
 *
 * @author Duncans_pumpkin
 *
 * @brief Null Render Object
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
#include "proto.h"
#include "rendobj.h"
#include "w3d_file.h"
#include "w3dmpo.h"

class Null3DObjClass final : public RenderObjClass
{
public:
    Null3DObjClass(char *name);
    Null3DObjClass(const Null3DObjClass &src);
    Null3DObjClass &operator=(const Null3DObjClass &that);

    int Class_ID() const override { return CLASSID_NULL; }
    RenderObjClass *Clone() const override { return new Null3DObjClass(*this); }
    const char *Get_Name() const override { return m_Name; }
    void Render(RenderInfoClass &rinfo) override {}
    void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override { sphere.Init(Vector3(0, 0, 0), 0.1f); }
    void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override
    {
        box.Init(Vector3(0, 0, 0), Vector3(0.1f, 0.1f, 0.1f));
    }
    ~Null3DObjClass() override {}

private:
    char m_Name[32];
};

class NullPrototypeClass final : public W3DMPO, public PrototypeClass
{
    IMPLEMENT_W3D_POOL(NullPrototypeClass);

public:
    NullPrototypeClass();
    NullPrototypeClass(const W3dNullObjectStruct &def);
    const char *Get_Name() const override { return m_definition.name; }
    int32_t Get_Class_ID() const override { return RenderObjClass::CLASSID_NULL; }
    RenderObjClass *Create() override { return new Null3DObjClass(m_definition.name); };
    void Delete_Self() override { delete this; };
    ~NullPrototypeClass() override{};

private:
    W3dNullObjectStruct m_definition;
};

class NullLoaderClass : public PrototypeLoaderClass
{
public:
    int Chunk_Type() override { return W3D_CHUNK_NULL_OBJECT; }
    PrototypeClass *Load_W3D(ChunkLoadClass &cload) override;
};
