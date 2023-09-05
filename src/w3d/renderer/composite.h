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

    ~CompositeRenderObjClass() override;
    void Restart() override;

    const char *Get_Name() const override;
    void Set_Name(const char *name) override;
    const char *Get_Base_Model_Name() const override;
    void Set_Base_Model_Name(const char *name) override;
    int Get_Num_Polys() const override;

    void Notify_Added(SceneClass *scene) override;
    void Notify_Removed(SceneClass *scene) override;

    bool Cast_Ray(RayCollisionTestClass &raytest) override;
    bool Cast_AABox(AABoxCollisionTestClass &boxtest) override;
    bool Cast_OBBox(OBBoxCollisionTestClass &boxtest) override;
    bool Intersect_AABox(AABoxIntersectionTestClass &boxtest) override;
    bool Intersect_OBBox(OBBoxIntersectionTestClass &boxtest) override;

    void Create_Decal(DecalGeneratorClass *generator) override;
    void Delete_Decal(unsigned long decal_id) override;

    void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
    void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;
    void Update_Obj_Space_Bounding_Volumes() override;

    void Set_User_Data(void *value, bool recursive) override;
};
