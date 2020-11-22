/**
 * @file
 *
 * @author Duncans_pumpkin
 *
 * @brief Prototype Loader Interface
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once
#include "proto.h"
#include "rendobj.h"
#include "w3d_file.h"
#include "w3dmpo.h"

class Null3DObjClass final : public RenderObjClass
{
    char m_Name[32];

    virtual int Class_ID() const override;
    virtual RenderObjClass *Clone() const override;
    virtual const char *Get_Name() const override;
    virtual void Render(RenderInfoClass &rinfo) override;
    virtual void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
    virtual void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;
    virtual ~Null3DObjClass();
};

class NullPrototypeClass final : public W3DMPO, public PrototypeClass
{
    IMPLEMENT_W3D_POOL(NullPrototypeClass);

public:
    virtual const char *Get_Name() override { return &m_definition.name[0]; }
    virtual int32_t Get_Class_ID() override { return RenderObjClass::CLASSID_NULL; }
    virtual RenderObjClass *Create() override { return new Null3DObjClass; };
    virtual void Delete_Self() override { delete this; };
    virtual ~NullPrototypeClass() override{};

private:
    W3dNullObjectStruct m_definition;
};
