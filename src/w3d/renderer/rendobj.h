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
#pragma once

#include "always.h"
#include "aabox.h"
#include "coltype.h"
#include "multilist.h"
#include "persist.h"
#include "refcount.h"
#include "sphere.h"

class SceneClass;
class HAnimClass;
class HTreeClass;
class MaterialInfoClass;
class HAnimComboClass;
class RayCollisionTestClass;
class AABoxCollisionTestClass;
class OBBoxCollisionTestClass;
class AABoxIntersectionTestClass;
class OBBoxIntersectionTestClass;
class RenderInfoClass;
class SpecialRenderInfoClass;
class IntersectionClass;
class IntersectionResultClass;
class CameraClass;
class DecalGeneratorClass;
class StringClass;
template<typename T> class DynamicVectorClass;
class RenderObjClass;

class RenderObjUnk
{
public:
    virtual ~RenderObjUnk() = 0;
    virtual bool Unk1(RenderObjClass *robj, RenderInfoClass &rinfo) = 0;
    virtual void Unk2(RenderObjClass *robj, RenderInfoClass &rinfo) = 0;
};

class RenderObjClass : public RefCountClass, public PersistClass, public MultiListObjectClass
{
public:
    enum
    {
        CLASSID_MESH = 0x0,
        CLASSID_HMODEL = 0x1,
        CLASSID_DISTLOD = 0x2,
        CLASSID_PREDLODGROUP = 0x3,
        CLASSID_HEIGHTMAP = 0x4,
        CLASSID_TERRAINTRACKS = 0x5,
        CLASSID_LINE3D = 0x6,
        CLASSID_BITMAP2D = 0x7,
        CLASSID_CAMERA = 0x8,
        CLASSID_DYNAMESH = 0x9,
        CLASSID_DYNASCREENMESH = 0xA,
        CLASSID_TEXTDRAW = 0xB,
        CLASSID_FOG = 0xC,
        CLASSID_LAYERFOG = 0xD,
        CLASSID_LIGHT = 0xE,
        CLASSID_PARTICLEEMITTER = 0xF,
        CLASSID_PARTICLEBUFFER = 0x10,
        CLASSID_SCREENPOINTGROUP = 0x11,
        CLASSID_VIEWPOINTGROUP = 0x12,
        CLASSID_WORLDPOINTGROUP = 0x13,
        CLASSID_TEXT2D = 0x14,
        CLASSID_TEXT3D = 0x15,
        CLASSID_NULL = 0x16,
        CLASSID_COLLECTION = 0x17,
        CLASSID_FLARE = 0x18,
        CLASSID_HLOD = 0x19,
        CLASSID_AABOX = 0x1A,
        CLASSID_OBBOX = 0x1B,
        CLASSID_SEGLINE = 0x1C,
        CLASSID_SPHERE = 0x1D,
        CLASSID_RING = 0x1E,
        CLASSID_BOUNDFOG = 0x1F,
        CLASSID_DAZZLE = 0x20,
        CLASSID_SOUND = 0x21,
        CLASSID_SEGLINETRAIL = 0x22,
        CLASSID_LAND = 0x23,
        CLASSID_SHDMESH = 0x24,
        CLASSID_LAST = 0xFFFF,
        CLASSID_UNKNOWN = 0xFFFFFFFF,
    };

    enum
    {
        ANIM_MODE_MANUAL = 0,
        ANIM_MODE_LOOP,
        ANIM_MODE_ONCE,
        ANIM_MODE_LOOP_PINGPONG,
        ANIM_MODE_LOOP_BACKWARDS,
        ANIM_MODE_ONCE_BACKWARDS,
    };

    enum
    {
        USER_DATA_MATERIAL_OVERRIDE = 0x1234567,
    };

    struct Material_Override
    {
        int m_structID;
        Vector2 m_customUVOffset;
        Material_Override() : m_structID(USER_DATA_MATERIAL_OVERRIDE), m_customUVOffset(0, 0) {}
    };

    RenderObjClass();
    RenderObjClass(const RenderObjClass &src);
    RenderObjClass &operator=(const RenderObjClass &that);

    virtual ~RenderObjClass();

    virtual RenderObjClass *Clone() const = 0;
    virtual int Class_ID() const { return CLASSID_UNKNOWN; }
    virtual const char *Get_Name() const { return "UNNAMED"; }
    virtual void Set_Name(const char *name) {}
    virtual const char *Get_Base_Model_Name() const { return nullptr; }
    virtual void Set_Base_Model_Name(const char *name) {}
    virtual int Get_Num_Polys() const { return 0; }
    virtual void Render(RenderInfoClass &rinfo) = 0;
    virtual void Special_Render(SpecialRenderInfoClass &rinfo) {}
    virtual void On_Frame_Update() {}
    virtual void Restart() {}
    virtual void Add(SceneClass *scene);
    virtual void Remove();
    virtual SceneClass *Get_Scene();
    virtual SceneClass *Peek_Scene() { return m_scene; }
    virtual void Set_Container(RenderObjClass *con);
    virtual void Validate_Transform() const;
    virtual void Set_Transform(const Matrix3D &m);
    virtual void Set_Position(const Vector3 &v);
    virtual void Notify_Added(SceneClass *scene);
    virtual void Notify_Removed(SceneClass *scene);
    virtual int Get_Num_Sub_Objects() const { return 0; }
    virtual RenderObjClass *Get_Sub_Object(int index) const { return nullptr; }
    virtual int Add_Sub_Object(RenderObjClass *subobj) { return 0; }
    virtual int Remove_Sub_Object(RenderObjClass *subobj) { return 0; }
    virtual RenderObjClass *Get_Sub_Object_By_Name(const char *name, int *index) const;
    virtual int Get_Num_Sub_Objects_On_Bone(int bone_index) const { return 0; }
    virtual RenderObjClass *Get_Sub_Object_On_Bone(int index, int bone_index) const { return nullptr; }
    virtual int Get_Sub_Object_Bone_Index(RenderObjClass *subobj) const { return 0; }
    virtual int Get_Sub_Object_Bone_Index(int lodindex, int modelindex) const { return 0; }
    virtual int Add_Sub_Object_To_Bone(RenderObjClass *subobj, int bone_index) { return 0; }
    virtual int Add_Sub_Object_To_Bone(RenderObjClass *subobj, const char *bone_name);
    virtual int Remove_Sub_Objects_From_Bone(int bone_index);
    virtual int Remove_Sub_Objects_From_Bone(const char *bone_name);
    virtual void Update_Sub_Object_Transforms();
    virtual void Set_Animation() {}
    virtual void Set_Animation(HAnimClass *motion, float frame, int anim_mode = ANIM_MODE_MANUAL) {}
    virtual void Set_Animation(HAnimClass *motion0, float frame0, HAnimClass *motion1, float frame1, float percentage) {}
    virtual void Set_Animation(HAnimComboClass *anim_combo) {}
    virtual HAnimClass *Peek_Animation() { return nullptr; }
    virtual int Get_Num_Bones() { return 0; }
    virtual const char *Get_Bone_Name(int bone_index) { return nullptr; }
    virtual int Get_Bone_Index(const char *bone_name) { return 0; }
    virtual const Matrix3D &Get_Bone_Transform(const char *bone_name) { return Get_Transform(); }
    virtual const Matrix3D &Get_Bone_Transform(int bone_index) { return Get_Transform(); }
    virtual void Capture_Bone(int bone_index) {}
    virtual void Release_Bone(int bone_index) {}
    virtual bool Is_Bone_Captured(int bone_index) const { return false; }
    virtual void Control_Bone(int bone_index, const Matrix3D &obj_tm, bool world_space_translation = false) {}
    virtual const HTreeClass *Get_HTree() const { return nullptr; }
    virtual bool Cast_Ray(RayCollisionTestClass &raytest) { return false; }
    virtual bool Cast_AABox(AABoxCollisionTestClass &boxtest) { return false; }
    virtual bool Cast_OBBox(OBBoxCollisionTestClass &boxtest) { return false; }
    virtual bool Intersect_AABox(AABoxIntersectionTestClass &boxtest) { return false; }
    virtual bool Intersect_OBBox(OBBoxIntersectionTestClass &boxtest) { return false; }
    virtual bool Intersect(IntersectionClass *intersect, IntersectionResultClass *res);
    virtual bool Intersect_Sphere(IntersectionClass *intersect, IntersectionResultClass *res);
    virtual bool Intersect_Sphere_Quick(IntersectionClass *intersect, IntersectionResultClass *res);
    virtual const SphereClass &Get_Bounding_Sphere() const;
    virtual const AABoxClass &Get_Bounding_Box() const;
    virtual void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const;
    virtual void Get_Obj_Space_Bounding_Box(AABoxClass &box) const;
    virtual void Update_Obj_Space_Bounding_Volumes() {}
    virtual void Prepare_LOD(CameraClass &camera);
    virtual void Recalculate_Static_LOD_Factors() {}
    virtual void Increment_LOD() {}
    virtual void Decrement_LOD() {}
    virtual float Get_Cost() const;
    virtual float Get_Value() const { return AT_MIN_LOD; }
    virtual float Get_Post_Increment_Value() const { return AT_MAX_LOD; }
    virtual void Set_LOD_Level(int lod) {}
    virtual int Get_LOD_Level() const { return 0; }
    virtual int Get_LOD_Count() const { return 1; }
    virtual void Set_LOD_Bias(float bias) {}
    virtual int Calculate_Cost_Value_Arrays(float screen_area, float *values, float *costs) const;

    virtual RenderObjClass *Get_Current_LOD()
    {
        Add_Ref();
        return this;
    }

    virtual bool Build_Dependency_List(DynamicVectorClass<StringClass> &file_list, bool recursive = true);
    virtual bool Build_Texture_List(DynamicVectorClass<StringClass> &texture_file_list, bool recursive = true);
    virtual void Create_Decal(DecalGeneratorClass *generator) { captainslog_dbgassert(0, "decals not supported"); }
    virtual void Delete_Decal(unsigned long decal_id) { captainslog_dbgassert(0, "decals not supported"); }
    virtual MaterialInfoClass *Get_Material_Info() { return nullptr; }
    virtual void Set_User_Data(void *value, bool recursive = false) { m_userData = value; }
    virtual void *Get_User_Data() { return m_userData; }
    virtual int Get_Num_Snap_Points() { return 0; }
    virtual void Get_Snap_Point(int index, Vector3 *set) {}
    virtual float Get_Screen_Size(CameraClass &camera);
    virtual void Scale(float scale) {}
    virtual void Scale(float scalex, float scaley, float scalez) {}
    virtual void Set_ObjectScale(float scale) { m_objectScale = scale; }
    float Get_ObjectScale() { return m_objectScale; }
    virtual int Get_Sort_Level() const { return 0; }
    virtual void Set_Sort_Level(int level) {}
    virtual int Is_Really_Visible() { return ((m_bits & IS_REALLY_VISIBLE) == IS_REALLY_VISIBLE); }
    virtual int Is_Not_Hidden_At_All() { return ((m_bits & IS_NOT_HIDDEN_AT_ALL) == IS_NOT_HIDDEN_AT_ALL); }
    virtual int Is_Visible() const { return (m_bits & IS_VISIBLE); }

    virtual void Set_Visible(int onoff)
    {
        if (onoff) {
            m_bits |= IS_VISIBLE;
        } else {
            m_bits &= ~IS_VISIBLE;
        }
    }

    virtual int Is_Hidden() const { return !(m_bits & IS_NOT_HIDDEN); }

    virtual void Set_Hidden(int onoff)
    {
        if (onoff) {
            m_bits &= ~IS_NOT_HIDDEN;
        } else {
            m_bits |= IS_NOT_HIDDEN;
        }
    }

    virtual int Is_Animation_Hidden() const { return !(m_bits & IS_NOT_ANIMATION_HIDDEN); }

    virtual void Set_Animation_Hidden(int onoff)
    {
        if (onoff) {
            m_bits &= ~IS_NOT_ANIMATION_HIDDEN;
        } else {
            m_bits |= IS_NOT_ANIMATION_HIDDEN;
        }
    }

    virtual int Is_Force_Visible() const { return m_bits & IS_FORCE_VISIBLE; }

    virtual void Set_Force_Visible(int onoff)
    {
        if (onoff) {
            m_bits |= IS_FORCE_VISIBLE;
        } else {
            m_bits &= ~IS_FORCE_VISIBLE;
        }
    }

    virtual int Is_Translucent() const { return m_bits & IS_TRANSLUCENT; }

    virtual void Set_Translucent(int onoff)
    {
        if (onoff) {
            m_bits |= IS_TRANSLUCENT;
        } else {
            m_bits &= ~IS_TRANSLUCENT;
        }
    }

    virtual int Is_Alpha() const { return m_bits & IS_ALPHA; }

    virtual void Set_Alpha(int onoff)
    {
        if (onoff) {
            m_bits |= IS_ALPHA;
        } else {
            m_bits &= ~IS_ALPHA;
        }
    }

    virtual int Is_Additive() const { return m_bits & IS_ADDITIVE; }

    virtual void Set_Additive(int onoff)
    {
        if (onoff) {
            m_bits |= IS_ADDITIVE;
        } else {
            m_bits &= ~IS_ADDITIVE;
        }
    }

    virtual int Get_Collision_Type() const { return (m_bits & COLLISION_TYPE_MASK); }

    virtual void Set_Collision_Type(int type)
    {
        m_bits &= ~COLLISION_TYPE_MASK;
        m_bits |= (type & COLLISION_TYPE_MASK) | COLLISION_TYPE_ALL;
    }

    virtual bool Is_Complete() { return false; }
    virtual bool Is_In_Scene() { return m_scene != nullptr; }
    virtual float Get_Native_Screen_Size() const { return m_nativeScreenSize; }
    virtual void Set_Native_Screen_Size(float screensize) { m_nativeScreenSize = screensize; }
#ifdef GAME_DLL
    virtual const PersistFactoryClass &Get_Factory() const;
#endif
    virtual bool Save(ChunkSaveClass &csave);
    virtual bool Load(ChunkLoadClass &cload);
    const Matrix3D &Get_Transform() const;
    const Matrix3D &Get_Transform(bool &is_transform_identity) const;
    const Matrix3D &Get_Transform_No_Validity_Check() const;
    const Matrix3D &Get_Transform_No_Validity_Check(bool &is_transform_identity) const;
    bool Is_Transform_Identity() const;
    bool Is_Transform_Identity_No_Validity_Check() const;
    RenderObjClass *Get_Container() const { return m_container; }
    void Set_House_Color(uint32_t color) { m_houseColor = color; }
    uint32_t Get_House_Color() const { return m_houseColor; }

    void Set_Sub_Object_Transforms_Dirty(bool onoff)
    {
        if (onoff) {
            m_bits |= SUBOBJ_TRANSFORMS_DIRTY;
        } else {
            m_bits &= ~SUBOBJ_TRANSFORMS_DIRTY;
        }
    }

    bool Are_Sub_Object_Transforms_Dirty() { return (m_bits & SUBOBJ_TRANSFORMS_DIRTY) != 0; }
    Vector3 Get_Position() const;
    static const float AT_MIN_LOD;
    static const float AT_MAX_LOD;
    RenderObjUnk *Get_Unknown() const { return m_unknown; }
    bool Is_Unk_15() const { return (m_bits & IS_UNK_15) != 0; }
    bool Bounding_Volumes_Valid(void) const { return (m_bits & BOUNDING_VOLUMES_VALID) != 0; }
    int Is_Sub_Objects_Match_LOD_Enabled() { return (m_bits & SUBOBJS_MATCH_LOD) != 0; }

protected:
    virtual void Add_Dependencies_To_List(DynamicVectorClass<StringClass> &file_list, bool textures_only = false);
    virtual void Update_Cached_Bounding_Volumes() const;
    virtual void Update_Sub_Object_Bits();
    void Invalidate_Cached_Bounding_Volumes() const { m_bits &= ~BOUNDING_VOLUMES_VALID; }
    void Validate_Cached_Bounding_Volumes() const { m_bits |= BOUNDING_VOLUMES_VALID; }

    void Set_Sub_Objects_Match_LOD(int onoff)
    {
        if (onoff) {
            m_bits |= SUBOBJS_MATCH_LOD;
        } else {
            m_bits &= ~SUBOBJS_MATCH_LOD;
        }
    }

    enum
    {
        COLLISION_TYPE_MASK = 0x000000FF,
        IS_VISIBLE = 0x00000100,
        IS_NOT_HIDDEN = 0x00000200,
        IS_NOT_ANIMATION_HIDDEN = 0x00000400,
        IS_FORCE_VISIBLE = 0x00000800,
        BOUNDING_VOLUMES_VALID = 0x00002000,
        IS_TRANSLUCENT = 0x00004000,
        IS_UNK_15 = 0x00008000,
        SUBOBJS_MATCH_LOD = 0x00010000,
        SUBOBJ_TRANSFORMS_DIRTY = 0x00020000,
        IS_ALPHA = 0x00040000,
        IS_ADDITIVE = 0x00100000,
        IS_REALLY_VISIBLE = IS_VISIBLE | IS_NOT_HIDDEN | IS_NOT_ANIMATION_HIDDEN,
        IS_NOT_HIDDEN_AT_ALL = IS_NOT_HIDDEN | IS_NOT_ANIMATION_HIDDEN,
        DEFAULT_BITS = COLLISION_TYPE_ALL | IS_NOT_HIDDEN | IS_NOT_ANIMATION_HIDDEN,
    };

    mutable unsigned long m_bits;
    Matrix3D m_transform;
    float m_objectScale;
    uint32_t m_houseColor;
    mutable SphereClass m_cachedBoundingSphere;
    mutable AABoxClass m_cachedBoundingBox;
    float m_nativeScreenSize;
    mutable bool m_isTransformIdentity;
    SceneClass *m_scene;
    RenderObjClass *m_container;
    void *m_userData;
    RenderObjUnk *m_unknown;
};

inline const SphereClass &RenderObjClass::Get_Bounding_Sphere() const
{
    if (!(m_bits & BOUNDING_VOLUMES_VALID)) {
        Update_Cached_Bounding_Volumes();
    }

    return m_cachedBoundingSphere;
}

inline const AABoxClass &RenderObjClass::Get_Bounding_Box() const
{
    if (!(m_bits & BOUNDING_VOLUMES_VALID)) {
        Update_Cached_Bounding_Volumes();
    }

    return m_cachedBoundingBox;
}

inline float Bound_Degrees(float angle)
{
    while (angle > 359) {
        angle -= 360;
    }

    while (angle < 0) {
        angle += 360;
    }

    return angle;
}

inline const Matrix3D &RenderObjClass::Get_Transform() const
{
    Validate_Transform();
    return m_transform;
}

inline const Matrix3D &RenderObjClass::Get_Transform(bool &is_transform_identity) const
{
    Validate_Transform();
    is_transform_identity = m_isTransformIdentity;
    return m_transform;
}

inline bool RenderObjClass::Is_Transform_Identity() const
{
    Validate_Transform();
    return m_isTransformIdentity;
}

inline const Matrix3D &RenderObjClass::Get_Transform_No_Validity_Check() const
{
    return m_transform;
}

inline const Matrix3D &RenderObjClass::Get_Transform_No_Validity_Check(bool &is_transform_identity) const
{
    is_transform_identity = m_isTransformIdentity;
    return m_transform;
}

inline bool RenderObjClass::Is_Transform_Identity_No_Validity_Check() const
{
    return m_isTransformIdentity;
}
