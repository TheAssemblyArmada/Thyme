/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Composite Render Object
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
#include "rendobj.h"
#include "wwstring.h"

class CompositeRenderObjClass : public RenderObjClass
{
protected:
    StringClass m_name;
    StringClass m_baseModelName;
    SphereClass m_objSphere;
    AABoxClass m_objBox;

public:
    CompositeRenderObjClass();
    CompositeRenderObjClass(const CompositeRenderObjClass &src);
    CompositeRenderObjClass &operator=(const CompositeRenderObjClass &that);

    virtual ~CompositeRenderObjClass() override;
    virtual void Restart() override;

    virtual const char *Get_Name() const override;
    virtual void Set_Name(const char *name) override;
    virtual const char *Get_Base_Model_Name() const override;
    virtual void Set_Base_Model_Name(const char *name) override;
    virtual int Get_Num_Polys() const override;

    virtual void Notify_Added(SceneClass *scene) override;
    virtual void Notify_Removed(SceneClass *scene) override;

    virtual bool Cast_Ray(RayCollisionTestClass &raytest) override;
    virtual bool Cast_AABox(AABoxCollisionTestClass &boxtest) override;
    virtual bool Cast_OBBox(OBBoxCollisionTestClass &boxtest) override;
    virtual bool Intersect_AABox(AABoxIntersectionTestClass &boxtest) override;
    virtual bool Intersect_OBBox(OBBoxIntersectionTestClass &boxtest) override;

    virtual void Create_Decal(DecalGeneratorClass *generator) override;
    virtual void Delete_Decal(unsigned long decal_id) override;

    virtual void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
    virtual void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;
    virtual void Update_Obj_Space_Bounding_Volumes() override;

    virtual void Set_User_Data(void *value, bool recursive) override;
};
