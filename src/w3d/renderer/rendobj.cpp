/**
 * @file
 *
 * @author Jonathan Wilson
 * @author tomsons26
 *
 * @brief base render object class
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "rendobj.h"
#include "camera.h"
#include "coltest.h"
#include "htree.h"
#include "intersec.h"
#include "scene.h"
#include "vector.h"
#include "w3d.h"
#include "wwstring.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

const float RenderObjClass::AT_MIN_LOD = FLT_MAX;
const float RenderObjClass::AT_MAX_LOD = -1.0f;

StringClass Filename_From_Asset_Name(const char *asset_name)
{
    StringClass filename;

    if (asset_name != NULL) {
        strcpy(filename.Get_Buffer(strlen(asset_name) + 5), asset_name);
        char *suffix = strchr(filename.Peek_Buffer(), '.');

        if (suffix != NULL) {
            suffix[0] = 0;
        }

        filename += ".w3d";
    }

    return filename;
}

static inline bool Check_Is_Transform_Identity(const Matrix3D &m)
{
    const float zero = 0.0f;
    const float one = 1.0f;

    unsigned d = ((unsigned &)m[0][0] ^ (unsigned &)one) | ((unsigned &)m[0][1] ^ (unsigned &)zero)
        | ((unsigned &)m[0][2] ^ (unsigned &)zero) | ((unsigned &)m[0][3] ^ (unsigned &)zero)
        | ((unsigned &)m[1][0] ^ (unsigned &)zero) | ((unsigned &)m[1][1] ^ (unsigned &)one)
        | ((unsigned &)m[1][2] ^ (unsigned &)zero) | ((unsigned &)m[1][3] ^ (unsigned &)zero)
        | ((unsigned &)m[2][0] ^ (unsigned &)zero) | ((unsigned &)m[2][1] ^ (unsigned &)zero)
        | ((unsigned &)m[2][2] ^ (unsigned &)one) | ((unsigned &)m[2][3] ^ (unsigned &)zero);
    return !d;
}

RenderObjClass::RenderObjClass() :
    m_bits(DEFAULT_BITS),
    m_transform(true),
    m_objectScale(1),
    m_houseColor(0),
    m_cachedBoundingSphere(Vector3(0, 0, 0), 1),
    m_cachedBoundingBox(Vector3(0, 0, 0), Vector3(1, 1, 1)),
    m_nativeScreenSize(W3D::Get_Default_Native_Screen_Size()),
    m_isTransformIdentity(false),
    m_scene(nullptr),
    m_container(nullptr),
    m_userData(nullptr),
    m_unknown(nullptr)
{
}

RenderObjClass::RenderObjClass(const RenderObjClass &src) :
    m_bits(src.m_bits),
    m_transform(src.m_transform),
    m_objectScale(1),
    m_houseColor(0),
    m_cachedBoundingSphere(src.m_cachedBoundingSphere),
    m_cachedBoundingBox(src.m_cachedBoundingBox),
    m_nativeScreenSize(src.m_nativeScreenSize),
    m_isTransformIdentity(src.m_isTransformIdentity),
    m_scene(nullptr),
    m_container(nullptr),
    m_userData(nullptr),
    m_unknown(nullptr)
{
}

RenderObjClass &RenderObjClass::RenderObjClass::operator=(const RenderObjClass &that)
{
    if (this != &that) {
        Set_Hidden(that.Is_Hidden());
        Set_Animation_Hidden(that.Is_Animation_Hidden());
        Set_Force_Visible(that.Is_Force_Visible());
        Set_Collision_Type(that.Get_Collision_Type());
        Set_Native_Screen_Size(that.Get_Native_Screen_Size());
        m_isTransformIdentity = false;
    }

    return *this;
}

RenderObjClass::~RenderObjClass()
{
    if (m_unknown) {
        delete m_unknown;
    }

    // BUGFIX Cleanup always to avoid dangling pointers.
    RenderObjClass::Remove();
}

float RenderObjClass::Get_Screen_Size(CameraClass &camera)
{
    Vector3 cam = camera.Get_Position();
    ViewportClass viewport = camera.Get_Viewport();
    Vector2 vpr_min;
    Vector2 vpr_max;
    camera.Get_View_Plane(vpr_min, vpr_max);

    float width_factor = (viewport.m_max.X - viewport.m_min.X) / (vpr_max.X - vpr_min.X);
    float height_factor = (viewport.m_max.Y - viewport.m_min.Y) / (vpr_max.Y - vpr_min.Y);
    float dist = (Get_Bounding_Sphere().Center - cam).Length();
    float radius = 0.0f;

    if (dist != 0.0f) { // todo FPU compare stuff
        radius = Get_Bounding_Sphere().Radius / dist;
    }

    return GAMEMATH_PI * radius * radius * width_factor * height_factor;
}

SceneClass *RenderObjClass::Get_Scene()
{
    if (m_scene) {
        m_scene->Add_Ref();
    }

    return m_scene;
}

void RenderObjClass::Set_Container(RenderObjClass *con)
{
    captainslog_assert((con == nullptr) || (m_container == nullptr));
    m_container = con;
}

void RenderObjClass::Set_Transform(const Matrix3D &m)
{
    m_transform = m;
    m_isTransformIdentity = Check_Is_Transform_Identity(m);
    Invalidate_Cached_Bounding_Volumes();
}

void RenderObjClass::Set_Position(const Vector3 &v)
{
    m_transform.Set_Translation(v);
    m_isTransformIdentity = Check_Is_Transform_Identity(m_transform);
    Invalidate_Cached_Bounding_Volumes();
}

void RenderObjClass::Validate_Transform() const
{
    RenderObjClass *con = Get_Container();
    bool dirty = false;

    if (con != NULL) {
        dirty = con->Are_Sub_Object_Transforms_Dirty();

        while (con->Get_Container() != nullptr) {
            dirty |= con->Are_Sub_Object_Transforms_Dirty();
            con = con->Get_Container();
        }

        if (dirty) {
            con->Update_Sub_Object_Transforms();
            m_isTransformIdentity = Check_Is_Transform_Identity(m_transform);
        }
    }
}

Vector3 RenderObjClass::Get_Position() const
{
    Validate_Transform();
    return m_transform.Get_Translation();
}

RenderObjClass *RenderObjClass::Get_Sub_Object_By_Name(const char *name, int *index) const
{
    int i;

    for (i = 0; i < Get_Num_Sub_Objects(); i++) {
        RenderObjClass *robj = Get_Sub_Object(i);

        if (robj) {
            if (strcasecmp(robj->Get_Name(), name) == 0) {
                if (index) {
                    *index = i;
                }

                return robj;
            } else {
                robj->Release_Ref();
            }
        }
    }

    for (i = 0; i < Get_Num_Sub_Objects(); i++) {
        RenderObjClass *robj = Get_Sub_Object(i);

        if (robj) {
            const char *subobjname = strchr(robj->Get_Name(), '.');
            if (subobjname == nullptr) {
                subobjname = robj->Get_Name();
            } else {
                subobjname = subobjname + 1;
            }

            if (strcasecmp(subobjname, name) == 0) {
                if (index) {
                    *index = i;
                }

                return robj;
            } else {
                robj->Release_Ref();
            }
        }
    }

    return nullptr;
}

int RenderObjClass::Add_Sub_Object_To_Bone(RenderObjClass *subobj, const char *bone_name)
{
    int bindex = Get_Bone_Index(bone_name);
    return Add_Sub_Object_To_Bone(subobj, bindex);
}

int RenderObjClass::Remove_Sub_Objects_From_Bone(int bone_index)
{
    int count = Get_Num_Sub_Objects_On_Bone(bone_index);
    int remove_count = 0;

    for (int i = count - 1; i >= 0; i--) {
        RenderObjClass *robj = Get_Sub_Object_On_Bone(i, bone_index);

        if (robj) {
            remove_count += Remove_Sub_Object(robj);
            robj->Release_Ref();
        }
    }

    return remove_count;
}

int RenderObjClass::Remove_Sub_Objects_From_Bone(const char *bone_name)
{
    int bindex = Get_Bone_Index(bone_name);
    return Remove_Sub_Objects_From_Bone(bindex);
}

void RenderObjClass::Prepare_LOD(CameraClass &camera) {}

float RenderObjClass::Get_Cost() const
{
    int polycount = Get_Num_Polys();
    float cost = (polycount != 0) ? polycount : 0.000001f;
    return cost;
}

int RenderObjClass::Calculate_Cost_Value_Arrays(float screen_area, float *values, float *costs) const
{
    values[0] = AT_MIN_LOD;
    values[1] = AT_MAX_LOD;
    costs[0] = Get_Cost();
    return 0;
}

void RenderObjClass::Update_Sub_Object_Bits()
{
    if (Get_Num_Sub_Objects() == 0) {
        return;
    }

    int coltype = 0;
    int istrans = 0;
    int isalpha = 0;
    int isadd = 0;

    for (int ni = 0; ni < Get_Num_Sub_Objects(); ni++) {
        RenderObjClass *robj = Get_Sub_Object(ni);
        coltype |= robj->Get_Collision_Type();
        istrans |= robj->Is_Translucent();
        isalpha |= robj->Is_Alpha();
        isadd |= robj->Is_Additive();
        robj->Release_Ref();
    }

    Set_Collision_Type(coltype);
    Set_Translucent(istrans);
    Set_Alpha(isalpha);
    Set_Additive(isadd);

    if (m_container) {
        m_container->Update_Sub_Object_Bits();
    }
}

void RenderObjClass::Update_Sub_Object_Transforms() {}

void RenderObjClass::Add(SceneClass *scene)
{
    captainslog_assert(scene);
    captainslog_assert(m_container == nullptr);
    m_scene = scene;
    scene->Add_Render_Object(this);
}

void RenderObjClass::Remove()
{
    if (m_container) {
        m_container->Remove_Sub_Object(this);
    } else if (m_scene) {
        m_scene->Remove_Render_Object(this);
    }
}

void RenderObjClass::Notify_Added(SceneClass *scene)
{
    m_scene = scene;
}

void RenderObjClass::Notify_Removed(SceneClass *scene)
{
    m_scene = nullptr;
}

void RenderObjClass::Update_Cached_Bounding_Volumes() const
{
    Get_Obj_Space_Bounding_Box(m_cachedBoundingBox);
    Get_Obj_Space_Bounding_Sphere(m_cachedBoundingSphere);
    m_cachedBoundingSphere.Center = Get_Transform() * m_cachedBoundingSphere.Center;
    m_cachedBoundingBox.Transform(Get_Transform());
    Validate_Cached_Bounding_Volumes();
}

void RenderObjClass::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    sphere.Init(Vector3(0, 0, 0), 1);
}

void RenderObjClass::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    box.Init(Vector3(0, 0, 0), Vector3(0, 0, 0));
}

bool RenderObjClass::Intersect(IntersectionClass *intersect, IntersectionResultClass *res)
{
    if (Intersect_Sphere_Quick(intersect, res)) {
        CastResultStruct castresult;
        LineSegClass lineseg;
        Vector3 end = *intersect->m_rayLocation + *intersect->m_rayDirection * intersect->m_maxDistance;
        lineseg.Set(*intersect->m_rayLocation, end);
        RayCollisionTestClass ray(lineseg, &castresult);
        ray.m_collisionType = COLLISION_TYPE_ALL;

        if (Cast_Ray(ray)) {
            lineseg.Compute_Point(ray.m_result->fraction, &(res->m_intersection));
            res->m_intersects = true;
            res->m_intersectionType = IntersectionResultClass::GENERIC;
            if (intersect->m_intersectionNormal) {
                *intersect->m_intersectionNormal = castresult.normal;
            }
            res->m_intersectedRenderObject = this;
            res->m_modelMatrix = m_transform;
            return true;
        }
    }
    res->m_intersects = false;
    return false;
}

bool RenderObjClass::Intersect_Sphere(IntersectionClass *intersect, IntersectionResultClass *res)
{
    SphereClass sphere = Get_Bounding_Sphere();
    return intersect->Intersect_Sphere(sphere, res);
}

bool RenderObjClass::Intersect_Sphere_Quick(IntersectionClass *intersect, IntersectionResultClass *res)
{
    SphereClass sphere = Get_Bounding_Sphere();
    return intersect->Intersect_Sphere_Quick(sphere, res);
}

bool RenderObjClass::Build_Dependency_List(DynamicVectorClass<StringClass> &file_list, bool recursive)
{
    if (recursive) {
        int subobj_count = Get_Num_Sub_Objects();

        for (int index = 0; index < subobj_count; index++) {
            RenderObjClass *psub_obj = Get_Sub_Object(index);

            if (psub_obj != NULL) {
                psub_obj->Build_Dependency_List(file_list);
                psub_obj->Release_Ref();
            }
        }
    }

    Add_Dependencies_To_List(file_list);
    return (file_list.Count() > 0);
}

bool RenderObjClass::Build_Texture_List(DynamicVectorClass<StringClass> &texture_file_list, bool recursive)
{
    if (recursive) {
        int subobj_count = Get_Num_Sub_Objects();

        for (int index = 0; index < subobj_count; index++) {
            RenderObjClass *sub_obj = Get_Sub_Object(index);

            if (sub_obj != NULL) {
                sub_obj->Build_Texture_List(texture_file_list);
                sub_obj->Release_Ref();
            }
        }
    }

    Add_Dependencies_To_List(texture_file_list, true);
    return (texture_file_list.Count() > 0);
}

void RenderObjClass::Add_Dependencies_To_List(DynamicVectorClass<StringClass> &file_list, bool textures_only)
{
    if (textures_only == false) {
        const char *model_name = Get_Name();
        file_list.Add(::Filename_From_Asset_Name(model_name));
        const HTreeClass *phtree = Get_HTree();

        if (phtree != NULL) {
            const char *htree_name = phtree->Get_Name();

            if (strcasecmp(htree_name, model_name) != 0) {
                file_list.Add(::Filename_From_Asset_Name(htree_name));
            }
        }

        const char *base_model_name = Get_Base_Model_Name();

        if (base_model_name != NULL) {
            file_list.Add(::Filename_From_Asset_Name(base_model_name));
        }
    }
}

#ifdef GAME_DLL
const PersistFactoryClass &RenderObjClass::Get_Factory() const
{
    // return _RenderObjFactory;
    captainslog_dbgassert(false, "Unimplemented code called!");

    // temp
    static class PersistFactoryClass *p;
    return *p;
}
#endif

bool RenderObjClass::Save(ChunkSaveClass &csave)
{
    captainslog_assert(false);
    return true;
}

bool RenderObjClass::Load(ChunkLoadClass &cload)
{
    captainslog_assert(false);
    return true;
}
