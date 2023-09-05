/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief HLod Render Object
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
#include "animobj.h"
#include "proto.h"
#include "refcount.h"
#include "vector.h"
#include "vector3.h"
#include "w3d_file.h"
#include "w3derr.h"

class HLodClass;

class SnapPointsClass : public DynamicVectorClass<Vector3>, public RefCountClass
{
public:
    W3DErrorType Load_W3D(ChunkLoadClass &cload);
    ~SnapPointsClass() override {}
};

class ProxyRecordClass
{
private:
    int m_boneIndex;
    char m_name[32];

public:
    ProxyRecordClass() : m_boneIndex(0) { memset(m_name, 0, sizeof(m_name)); }

    bool operator==(const ProxyRecordClass &src) { return false; }
    bool operator!=(const ProxyRecordClass &src) { return true; }

    void Init(W3dHLodSubObjectStruct &str)
    {
        m_boneIndex = str.BoneIndex;
        strcpy(m_name, str.Name);
        m_name[31] = 0;
    }

    int Get_Bone_Index() { return m_boneIndex; }
    const char *Get_Name() { return m_name; }
};

class ProxyArrayClass : public W3DMPO, public VectorClass<ProxyRecordClass>, public RefCountClass
{
    IMPLEMENT_W3D_POOL(ProxyArrayClass);

public:
    ProxyArrayClass(int size) : VectorClass(size) {}
    ~ProxyArrayClass() override {}
};

class ProxyClass
{
private:
    StringClass m_name;
    Matrix3D m_transform;

public:
    ProxyClass(const char *name, class Matrix3D &tm) : m_name(name), m_transform(tm) {}
    ProxyClass() {}
    ~ProxyClass() {}

    const char *Get_Name() { return m_name; }
    void Set_Name(const char *name) { m_name = name; }
    const Matrix3D &Get_Transform() { return m_transform; }
    void Set_Transform(const Matrix3D &tm) { m_transform = tm; }
    bool operator==(const ProxyClass &src) { return (m_name == src.m_name && m_transform == src.m_transform); }
    bool operator!=(const ProxyClass &src) { return (m_name != src.m_name || m_transform != src.m_transform); }
};

class HLodDefClass : public W3DMPO
{
    IMPLEMENT_W3D_POOL(HLodDefClass);

public:
    class SubObjectArrayClass
    {
    private:
        float m_maxScreenSize;
        int m_modelCount;
        char **m_modelName;
        int *m_boneIndex;

    public:
        SubObjectArrayClass() : m_maxScreenSize(NO_MAX_SCREEN_SIZE), m_modelCount(0), m_modelName(0), m_boneIndex(0) {}
        ~SubObjectArrayClass() { Reset(); }

        void Reset()
        {
            m_maxScreenSize = NO_MAX_SCREEN_SIZE;
            if (m_modelName) {
                for (int i = 0; i < m_modelCount; i++) {
                    free(m_modelName[i]);
                }
                delete[] m_modelName;
                m_modelName = 0;
            }
            if (m_boneIndex) {
                delete[] m_boneIndex;
                m_boneIndex = 0;
            }
            m_modelCount = 0;
        }

        bool Load_W3D(ChunkLoadClass &cload);
        bool Save_W3D(ChunkSaveClass &csave);
        friend class HLodClass;
        friend class HLodDefClass;
    };

private:
    char *m_name;
    char *m_hierarchyTreeName;
    int m_lodCount;
    SubObjectArrayClass *m_lod;
    SubObjectArrayClass m_aggregates;
    ProxyArrayClass *m_proxyArray;

public:
    HLodDefClass() : m_name(nullptr), m_hierarchyTreeName(nullptr), m_lodCount(0), m_lod(nullptr), m_proxyArray(nullptr) {}
    HLodDefClass(HLodClass &hlod);
    void Initialize(HLodClass &hlod);
    ~HLodDefClass() override { Free(); }

    void Free();

    const char *Get_Name() { return m_name; }
    W3DErrorType Load_W3D(ChunkLoadClass &cload);
    bool Read_Header(ChunkLoadClass &cload);
    bool Read_Proxy_Array(ChunkLoadClass &cload);
    W3DErrorType Save(ChunkSaveClass &csave);
    W3DErrorType Save_Header(ChunkSaveClass &csave);
    W3DErrorType Save_Lod_Array(ChunkSaveClass &csave);
    W3DErrorType Save_Aggregate_Array(ChunkSaveClass &csave);
    friend class HLodClass;
};

class HLodLoaderClass : public PrototypeLoaderClass
{
public:
    HLodLoaderClass() {}
    ~HLodLoaderClass() {}
    int32_t Chunk_Type() override { return W3D_CHUNK_HLOD; }
    PrototypeClass *Load_W3D(ChunkLoadClass &cload) override;
};

class DistLODLoaderClass : public PrototypeLoaderClass
{
public:
    DistLODLoaderClass() {}
    ~DistLODLoaderClass() {}
    int32_t Chunk_Type() override { return W3D_CHUNK_LODMODEL; }
    PrototypeClass *Load_W3D(ChunkLoadClass &cload) override;
};

class HModelLoaderClass : public PrototypeLoaderClass
{
public:
    HModelLoaderClass() {}
    ~HModelLoaderClass() {}
    int32_t Chunk_Type() override { return W3D_CHUNK_HMODEL; }
    PrototypeClass *Load_W3D(ChunkLoadClass &cload) override;
};

class CollectionLoaderClass : public PrototypeLoaderClass
{
public:
    CollectionLoaderClass() {}
    ~CollectionLoaderClass() {}
    int32_t Chunk_Type() override { return W3D_CHUNK_COLLECTION; }
    PrototypeClass *Load_W3D(ChunkLoadClass &cload) override;
};

class HLodPrototypeClass : public W3DMPO, public PrototypeClass
{
    IMPLEMENT_W3D_POOL(HLodPrototypeClass);

private:
    HLodDefClass *m_definition;

public:
    HLodPrototypeClass(HLodDefClass *def) : m_definition(def) {}
    ~HLodPrototypeClass() override;
    const char *Get_Name() const override;
    int32_t Get_Class_ID() const override;
    RenderObjClass *Create() override;
    void Delete_Self() override;

    HLodDefClass *Get_Definition() { return m_definition; }
};

class HLodClass : public W3DMPO, public Animatable3DObjClass
{
    IMPLEMENT_W3D_POOL(HLodClass);

private:
    class ModelNodeClass
    {
    public:
        RenderObjClass *m_model;
        int m_boneIndex;

        bool operator==(const ModelNodeClass &src) { return m_model == src.m_model && m_boneIndex == src.m_boneIndex; }
        bool operator!=(const ModelNodeClass &src) { return m_model != src.m_model || m_boneIndex != src.m_boneIndex; }
    };

    class ModelArrayClass : public DynamicVectorClass<ModelNodeClass>
    {
    public:
        float m_maxScreenSize;
        float m_nonPixelCost;
        float m_pixelCostPerArea;
        float m_benefitFactor;

        ModelArrayClass() : m_maxScreenSize(FLT_MAX), m_nonPixelCost(0), m_pixelCostPerArea(0), m_benefitFactor(0) {}
        ~ModelArrayClass() override {}
    };

    int m_lodCount;
    int m_curLod;
    ModelArrayClass *m_lod;
    int m_boundingBoxIndex;
    float *m_cost;
    float *m_value;
    ModelArrayClass m_additionalModels;
    SnapPointsClass *m_snapPoints;
    ProxyArrayClass *m_proxyArray;
    float m_lodBias;

public:
    HLodClass(const HLodClass &src);
    HLodClass(const HLodDefClass &src);
    HLodClass(const char *name, RenderObjClass **lods, int count);

    HLodClass &operator=(const HLodClass &that);

    ~HLodClass() override;

    RenderObjClass *Clone() const override;
    int Class_ID() const override;
    int Get_Num_Polys() const override;

    void Render(RenderInfoClass &rinfo) override;
    void Special_Render(SpecialRenderInfoClass &rinfo) override;

    void Set_Transform(const Matrix3D &m) override;
    void Set_Position(const Vector3 &v) override;
    void Notify_Added(SceneClass *scene) override;
    void Notify_Removed(SceneClass *scene) override;

    int Get_Num_Sub_Objects() const override;
    RenderObjClass *Get_Sub_Object(int index) const override;
    int Add_Sub_Object(RenderObjClass *subobj) override;
    int Remove_Sub_Object(RenderObjClass *subobj) override;
    int Get_Num_Sub_Objects_On_Bone(int bone_index) const override;
    RenderObjClass *Get_Sub_Object_On_Bone(int index, int bone_index) const override;
    int Get_Sub_Object_Bone_Index(RenderObjClass *object) const override;
    int Get_Sub_Object_Bone_Index(int lodindex, int modelindex) const override;
    int Add_Sub_Object_To_Bone(RenderObjClass *subobj, int bone_index) override;
    void Update_Sub_Object_Transforms() override;

    void Set_Animation() override;
    void Set_Animation(HAnimClass *motion, float frame, int anim_mode) override;
    void Set_Animation(HAnimClass *motion0, float frame0, HAnimClass *motion1, float frame1, float percentage) override;
    void Set_Animation(HAnimComboClass *combo) override;

    bool Cast_Ray(RayCollisionTestClass &raytest) override;
    bool Cast_AABox(AABoxCollisionTestClass &boxtest) override;
    bool Cast_OBBox(OBBoxCollisionTestClass &boxtest) override;
    bool Intersect_AABox(AABoxIntersectionTestClass &boxtest) override;
    bool Intersect_OBBox(OBBoxIntersectionTestClass &boxtest) override;

    const SphereClass &Get_Bounding_Sphere() const override;
    const AABoxClass &Get_Bounding_Box() const override;
    void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
    void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;
    void Update_Obj_Space_Bounding_Volumes() override;

    void Prepare_LOD(CameraClass &camera) override;
    void Recalculate_Static_LOD_Factors() override;
    void Increment_LOD() override;
    void Decrement_LOD() override;
    float Get_Cost() const override;
    float Get_Value() const override;
    float Get_Post_Increment_Value() const override;
    void Set_LOD_Level(int lod) override;
    int Get_LOD_Level() const override;
    int Get_LOD_Count() const override;
    void Set_LOD_Bias(float bias) override;
    int Calculate_Cost_Value_Arrays(float screen_area, float *values, float *costs) const override;
    RenderObjClass *Get_Current_LOD() override;

    void Create_Decal(DecalGeneratorClass *generator) override;
    void Delete_Decal(unsigned long decal_id) override;

    int Get_Num_Snap_Points() override;
    void Get_Snap_Point(int index, Vector3 *set) override;

    void Scale(float scale) override;
    void Scale(float scalex, float scaley, float scalez) override;

    void Set_Hidden(int hidden) override;
    void Set_HTree(HTreeClass *htree) override;
    virtual void Set_Max_Screen_Size(int lod_index, float size);
    virtual float Get_Max_Screen_Size(int lod_index);

    virtual int Get_Lod_Count();
    virtual int Get_Lod_Model_Count(int lod_index);
    virtual RenderObjClass *Peek_Lod_Model(int lod_index, int model_index);
    virtual RenderObjClass *Get_Lod_Model(int lod_index, int model_index);
    virtual int Get_Lod_Model_Bone(int lod_index, int model_index);

    virtual int Get_Additional_Model_Count();
    virtual RenderObjClass *Peek_Additional_Model(int model_index);
    virtual RenderObjClass *Get_Additional_Model(int model_index);
    virtual int Get_Additional_Model_Bone(int model_index);

    virtual bool Is_NULL_Lod_Included();
    virtual void Include_NULL_Lod(bool include);

    virtual int Get_Proxy_Count();
    virtual bool Get_Proxy(int index, ProxyClass &proxy);

    void Free();
    void Add_Lod_Model(int lod, RenderObjClass *robj, int bone_index);
    friend class W3DRenderObjectSnapshot;
};
