/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief HLod render object
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "hlod.h"
#include "assetmgr.h"
#include "boxrobj.h"
#include "nstrdup.h"
#include "rinfo.h"

W3DErrorType SnapPointsClass::Load_W3D(ChunkLoadClass &cload)
{
    int count = cload.Cur_Chunk_Length() / sizeof(Vector3);
    Resize(count);
    for (int i = 0; i < count; i++) {
        Vector3 vec;

        if (cload.Read(&vec, sizeof(vec)) == sizeof(vec)) {
            Add(vec);
        } else {
            return W3D_ERROR_LOAD_FAILED;
        }
    }
    return W3D_ERROR_OK;
}

void HLodDefClass::Free()
{
    if (m_name) {
        free(m_name);
        m_name = nullptr;
    }
    if (m_hierarchyTreeName) {
        free(m_hierarchyTreeName);
        m_hierarchyTreeName = nullptr;
    }
    if (m_lod) {
        delete[] m_lod;
        m_lod = nullptr;
    }
    if (m_proxyArray) {
        m_proxyArray->Release_Ref();
        m_proxyArray = nullptr;
    }
    m_lodCount = 0;
}

bool HLodDefClass::SubObjectArrayClass::Load_W3D(ChunkLoadClass &cload)
{
    if (!cload.Open_Chunk()) {
        return false;
    }

    if (cload.Cur_Chunk_ID() != W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER) {
        return false;
    }

    W3dHLodArrayHeaderStruct header;

    if (cload.Read(&header, sizeof(header)) != sizeof(header)) {
        return false;
    }

    if (!cload.Close_Chunk()) {
        return false;
    }

    m_modelCount = header.ModelCount;
    m_maxScreenSize = header.MaxScreenSize;
    m_modelName = new char *[m_modelCount];
    m_boneIndex = new int[m_modelCount];

    for (int i = 0; i < m_modelCount; i++) {
        if (!cload.Open_Chunk()) {
            return false;
        }

        if (cload.Cur_Chunk_ID() != W3D_CHUNK_HLOD_SUB_OBJECT) {
            return false;
        }

        W3dHLodSubObjectStruct subobjdef;

        if (cload.Read(&subobjdef, sizeof(subobjdef)) != sizeof(subobjdef)) {
            return false;
        }

        if (!cload.Close_Chunk()) {
            return false;
        }

        m_modelName[i] = strdup(subobjdef.Name);
        m_boneIndex[i] = subobjdef.BoneIndex;
    }

    return true;
}

W3DErrorType HLodDefClass::Load_W3D(ChunkLoadClass &cload)
{
    Free();

    if (!Read_Header(cload)) {
        return W3D_ERROR_LOAD_FAILED;
    }

    for (int i = 0; i < m_lodCount; i++) {
        if (!cload.Open_Chunk()) {
            return W3D_ERROR_LOAD_FAILED;
        }

        if (cload.Cur_Chunk_ID() != W3D_CHUNK_HLOD_LOD_ARRAY) {
            return W3D_ERROR_LOAD_FAILED;
        }

        m_lod[i].Load_W3D(cload);
        cload.Close_Chunk();
    }

    while (cload.Open_Chunk()) {
        switch (cload.Cur_Chunk_ID()) {
            case W3D_CHUNK_HLOD_AGGREGATE_ARRAY:
                m_aggregates.Load_W3D(cload);
                break;
            case W3D_CHUNK_HLOD_PROXY_ARRAY:
                Read_Proxy_Array(cload);
                break;
        }

        cload.Close_Chunk();
    }

    return W3D_ERROR_OK;
}

bool HLodDefClass::SubObjectArrayClass::Save_W3D(ChunkSaveClass &csave)
{
    bool b = false;

    if (csave.Begin_Chunk(W3D_CHUNK_HLOD_LOD_ARRAY)) {
        if (csave.Begin_Chunk(W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER)) {
            W3dHLodArrayHeaderStruct header;
            header.ModelCount = m_modelCount;
            header.MaxScreenSize = m_maxScreenSize;
            b = csave.Write(&header, sizeof(W3dHLodArrayHeaderStruct)) == sizeof(W3dHLodArrayHeaderStruct);
            csave.End_Chunk();

            if (b) {
                for (int i = 0; i < m_modelCount; i++) {

                    if (!b) {
                        break;
                    }

                    b &= csave.Begin_Chunk(W3D_CHUNK_HLOD_SUB_OBJECT);

                    if (b) {
                        W3dHLodSubObjectStruct subobjheader;
                        memset(&subobjheader, 0, sizeof(subobjheader));
                        subobjheader.BoneIndex = m_boneIndex[i];
                        strncpy(subobjheader.Name, m_modelName[i], ARRAY_SIZE(subobjheader.Name));
                        subobjheader.Name[ARRAY_SIZE(subobjheader.Name) - 1] = 0;
                        b = csave.Write(&subobjheader, sizeof(W3dHLodSubObjectStruct)) == sizeof(W3dHLodSubObjectStruct);
                        csave.End_Chunk();
                    }
                }
            }
        }

        csave.End_Chunk();
    }

    return b;
}

HLodDefClass::HLodDefClass(HLodClass &hlod) :
    m_name(nullptr), m_hierarchyTreeName(nullptr), m_lodCount(0), m_lod(nullptr), m_proxyArray(nullptr)
{
    Initialize(hlod);
}

void HLodDefClass::Initialize(HLodClass &hlod)
{
    Free();
    m_name = strdup(hlod.Get_Name());
    const HTreeClass *tree = hlod.Get_HTree();

    if (tree) {
        m_hierarchyTreeName = strdup(tree->Get_Name());
    }

    m_lodCount = hlod.Get_LOD_Count();

    if (m_lodCount > 0) {
        m_lod = new SubObjectArrayClass[m_lodCount];
        int curlod = 0;

        for (int i = 0; i < m_lodCount; i++) {
            m_lod[curlod].m_maxScreenSize = hlod.Get_Max_Screen_Size(i);
            m_lod[curlod].m_modelCount = hlod.Get_Lod_Model_Count(i);
            m_lod[curlod].m_modelName = new char *[m_lod[curlod].m_modelCount];
            m_lod[curlod].m_boneIndex = new int[m_lod[curlod].m_modelCount];

            for (int j = 0; j < m_lod[curlod].m_modelCount; j++) {
                RenderObjClass *robj = hlod.Peek_Lod_Model(i, j);

                if (robj) {
                    m_lod[curlod].m_modelName[j] = strdup(robj->Get_Name());
                    m_lod[curlod].m_boneIndex[j] = hlod.Get_Lod_Model_Bone(i, j);
                }
            }

            curlod++;
        }
    }
}

bool HLodDefClass::Read_Header(ChunkLoadClass &cload)
{
    if (cload.Open_Chunk()) {
        if (cload.Cur_Chunk_ID() == W3D_CHUNK_HLOD_HEADER) {
            W3dHLodHeaderStruct header;

            if (cload.Read(&header, sizeof(header)) == sizeof(header)) {
                cload.Close_Chunk();
                m_name = strdup(header.Name);
                m_hierarchyTreeName = strdup(header.HierarchyName);
                m_lodCount = header.LodCount;
                m_lod = new SubObjectArrayClass[m_lodCount];
                return true;
            }
        }
    }

    return false;
}

bool HLodDefClass::Read_Proxy_Array(ChunkLoadClass &cload)
{
    Ref_Ptr_Release(m_proxyArray);

    if (!cload.Open_Chunk()) {
        return false;
    }

    if (cload.Cur_Chunk_ID() != W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER) {
        return false;
    }

    W3dHLodArrayHeaderStruct header;

    if (cload.Read(&header, sizeof(header)) != sizeof(header)) {
        return false;
    }

    if (!cload.Close_Chunk()) {
        return false;
    }

    m_proxyArray = new ProxyArrayClass(header.ModelCount);

    for (int i = 0; i < m_proxyArray->Length(); i++) {
        if (!cload.Open_Chunk()) {
            return false;
        }

        if (cload.Cur_Chunk_ID() != W3D_CHUNK_HLOD_SUB_OBJECT) {
            return false;
        }

        W3dHLodSubObjectStruct subobjdef;

        if (cload.Read(&subobjdef, sizeof(subobjdef)) != sizeof(subobjdef)) {
            return false;
        }

        if (!cload.Close_Chunk()) {
            return false;
        }

        (*m_proxyArray)[i].Init(subobjdef);
    }

    return true;
}

PrototypeClass *HLodLoaderClass::Load_W3D(ChunkLoadClass &cload)
{
    HLodDefClass *def = new HLodDefClass();

    if (def) {
        if (def->Load_W3D(cload) != W3D_ERROR_OK) {
            delete def;
            return nullptr;
        } else {
            HLodPrototypeClass *proto = new HLodPrototypeClass(def);
            return proto;
        }
    }

    return nullptr;
}

W3DErrorType HLodDefClass::Save(ChunkSaveClass &csave)
{
    W3DErrorType error = W3D_ERROR_SAVE_FAILED;

    if (csave.Begin_Chunk(W3D_CHUNK_HLOD) == 1) {
        if (Save_Header(csave) == W3D_ERROR_OK && Save_Lod_Array(csave) == 0) {
            error = W3D_ERROR_OK;
        }

        csave.End_Chunk();
    }

    return error;
}

W3DErrorType HLodDefClass::Save_Header(ChunkSaveClass &csave)
{
    if (!csave.Begin_Chunk(W3D_CHUNK_HLOD_HEADER)) {
        return W3D_ERROR_SAVE_FAILED;
    }

    W3dHLodHeaderStruct header;
    memset(&header, 0, sizeof(header));
    header.Version = 0x10000;
    header.LodCount = m_lodCount;
    strncpy(header.Name, m_name, ARRAY_SIZE(header.Name));
    header.Name[ARRAY_SIZE(header.Name) - 1] = 0;
    strncpy(header.HierarchyName, m_hierarchyTreeName, ARRAY_SIZE(header.HierarchyName));
    header.HierarchyName[ARRAY_SIZE(header.HierarchyName) - 1] = 0;

    W3DErrorType error = W3D_ERROR_OK;

    if (csave.Write(&header, sizeof(W3dHLodHeaderStruct)) != sizeof(W3dHLodHeaderStruct)) {
        error = W3D_ERROR_SAVE_FAILED;
    }

    csave.End_Chunk();
    return error;
}

W3DErrorType HLodDefClass::Save_Lod_Array(ChunkSaveClass &csave)
{
    bool b = true;

    for (int i = 0; i < m_lodCount; i++) {
        if (!b) {
            break;
        }

        b = m_lod[i].Save_W3D(csave);
    }

    return b ? W3D_ERROR_OK : W3D_ERROR_SAVE_FAILED;
}

W3DErrorType HLodDefClass::Save_Aggregate_Array(ChunkSaveClass &csave)
{
    if (m_aggregates.m_modelCount > 0) {
        csave.Begin_Chunk(W3D_CHUNK_HLOD_AGGREGATE_ARRAY);
        m_aggregates.Save_W3D(csave);
        csave.End_Chunk();
    }

    return W3D_ERROR_OK;
}

RenderObjClass *HLodPrototypeClass::Create()
{
    return new HLodClass(*m_definition);
}

HLodPrototypeClass::~HLodPrototypeClass()
{
    if (m_definition) {
        delete m_definition;
    }
}

const char *HLodPrototypeClass::Get_Name() const
{
    return m_definition->Get_Name();
}

int32_t HLodPrototypeClass::Get_Class_ID() const
{
    return RenderObjClass::CLASSID_HLOD;
}

void HLodPrototypeClass::Delete_Self()
{
    delete this;
}

HLodClass::HLodClass(const HLodClass &src) :
    Animatable3DObjClass(src),
    m_lodCount(0),
    m_curLod(0),
    m_lod(nullptr),
    m_boundingBoxIndex(-1),
    m_cost(nullptr),
    m_value(nullptr),
    m_snapPoints(nullptr),
    m_proxyArray(nullptr),
    m_lodBias(1.0f)
{
    *this = src;
}

HLodClass::HLodClass(const HLodDefClass &src) :
    Animatable3DObjClass(src.m_hierarchyTreeName),
    m_lodCount(0),
    m_curLod(0),
    m_lod(nullptr),
    m_boundingBoxIndex(-1),
    m_cost(nullptr),
    m_value(nullptr),
    m_snapPoints(nullptr),
    m_proxyArray(nullptr),
    m_lodBias(1.0f)
{
    Set_Name(src.m_name);
    m_lodCount = src.m_lodCount;

    captainslog_assert(m_lodCount >= 1);

    m_lod = new ModelArrayClass[m_lodCount];
    m_cost = new float[m_lodCount];
    m_value = new float[m_lodCount + 1];

    for (int i = 0; i < m_lodCount; i++) {
        m_lod[i].m_maxScreenSize = src.m_lod[i].m_maxScreenSize;

        for (int j = 0; j < src.m_lod[i].m_modelCount; j++) {
            RenderObjClass *robj = W3DAssetManager::Get_Instance()->Create_Render_Obj(src.m_lod[i].m_modelName[j]);

            if (robj) {
                Add_Lod_Model(i, robj, src.m_lod[i].m_boneIndex[j]);
                robj->Release_Ref();
            }
        }
    }

    Recalculate_Static_LOD_Factors();

    for (int i = 0; i < src.m_aggregates.m_modelCount; i++) {
        RenderObjClass *robj = W3DAssetManager::Get_Instance()->Create_Render_Obj(src.m_aggregates.m_modelName[i]);

        if (robj) {
            Add_Sub_Object_To_Bone(robj, src.m_aggregates.m_boneIndex[i]);
            robj->Release_Ref();
        }
    }

    ProxyArrayClass *array = const_cast<ProxyArrayClass *>(src.m_proxyArray);
    Ref_Ptr_Set(m_proxyArray, array);

    int cost = Calculate_Cost_Value_Arrays(1.0f, m_value, m_cost);

    if (m_curLod < cost) {
        Set_LOD_Level(cost);
    }

    Set_Sub_Object_Transforms_Dirty(true);
    Update_Sub_Object_Bits();
    Update_Obj_Space_Bounding_Volumes();
}

HLodClass::HLodClass(const char *name, RenderObjClass **lods, int count) :
    Animatable3DObjClass(nullptr),
    m_lodCount(0),
    m_curLod(0),
    m_lod(nullptr),
    m_boundingBoxIndex(-1),
    m_cost(nullptr),
    m_value(nullptr),
    m_snapPoints(nullptr),
    m_proxyArray(nullptr),
    m_lodBias(1.0f)
{
    captainslog_assert(name != nullptr);
    captainslog_assert(lods != nullptr);
    captainslog_assert((count > 0) && (count < 256));

    Set_Name(name);
    m_lodCount = count;
    captainslog_assert(m_lodCount >= 1);

    m_lod = new ModelArrayClass[m_lodCount];
    captainslog_assert(m_lod);

    m_cost = new float[m_lodCount];
    captainslog_assert(m_cost);

    m_value = new float[m_lodCount];
    captainslog_assert(m_value);

    const HTreeClass *tree = lods[count - 1]->Get_HTree();

    if (tree != nullptr) {
        Set_HTree(new HTreeClass(*tree));
    } else {
        Set_HTree(new HTreeClass());
        m_htree->Init_Default();
    }

    for (int i = 0; i < m_lodCount; i++) {
        RenderObjClass *lod_obj = lods[i];
        captainslog_assert(lod_obj != nullptr);

        if (lod_obj->Class_ID() == CLASSID_HMODEL || lod_obj->Class_ID() == CLASSID_HLOD
            || lod_obj->Get_Num_Sub_Objects() > 1) {
            while (lod_obj->Get_Num_Sub_Objects() > 0) {
                RenderObjClass *o2 = lod_obj->Get_Sub_Object(0);
                int bone = lod_obj->Get_Sub_Object_Bone_Index(o2);
                lod_obj->Remove_Sub_Object(o2);
                Add_Lod_Model(i, o2, bone);
                o2->Release_Ref();
            }
        } else {
            Add_Lod_Model(i, lod_obj, 0);
        }
    }

    Recalculate_Static_LOD_Factors();
    int lod = Calculate_Cost_Value_Arrays(1.0f, m_value, m_cost);

    if (m_curLod < lod) {
        Set_LOD_Level(lod);
    }

    Set_Sub_Object_Transforms_Dirty(true);
    Update_Sub_Object_Bits();
    Update_Obj_Space_Bounding_Volumes();
}

HLodClass &HLodClass::operator=(const HLodClass &that)
{
    if (this != &that) {
        Free();
        Animatable3DObjClass::operator=(that);
        m_boundingBoxIndex = that.m_boundingBoxIndex;
        m_lodCount = that.m_lodCount;

        captainslog_assert(m_lodCount >= 1);

        m_lod = new ModelArrayClass[m_lodCount];
        m_cost = new float[m_lodCount];
        m_value = new float[m_lodCount + 1];

        for (int i = 0; i < m_lodCount; i++) {
            m_lod[i].Resize(that.m_lod[i].Count());
            m_lod[i].m_maxScreenSize = that.m_lod[i].m_maxScreenSize;

            for (int j = 0; j < that.m_lod[i].Count(); j++) {
                ModelNodeClass node;
                node.m_boneIndex = that.m_lod[i][j].m_boneIndex;
                node.m_model = that.m_lod[i][j].m_model->Clone();
                node.m_model->Set_Container(this);

                if (Is_In_Scene()) {
                    node.m_model->Notify_Added(m_scene);
                }

                m_lod[i].Add(node);
            }
        }

        m_additionalModels.Resize(that.m_additionalModels.Count());

        for (int j = 0; j < that.m_additionalModels.Count(); j++) {
            ModelNodeClass node;
            node.m_boneIndex = that.m_additionalModels[j].m_boneIndex;
            node.m_model = that.m_additionalModels[j].m_model->Clone();
            node.m_model->Set_Container(this);

            if (Is_In_Scene()) {
                node.m_model->Notify_Added(m_scene);
            }

            m_additionalModels.Add(node);
        }

        m_lodBias = that.m_lodBias;
    }

    Recalculate_Static_LOD_Factors();
    int cost = Calculate_Cost_Value_Arrays(1.0f, m_value, m_cost);

    if (m_curLod < cost) {
        Set_LOD_Level(cost);
    }

    Set_Sub_Object_Transforms_Dirty(true);
    Update_Sub_Object_Bits();
    Update_Obj_Space_Bounding_Volumes();
    return *this;
}

HLodClass::~HLodClass()
{
    Free();
}

RenderObjClass *HLodClass::Clone() const
{
    return new HLodClass(*this);
}

int HLodClass::Class_ID() const
{
    return CLASSID_HLOD;
}

int HLodClass::Get_Num_Polys() const
{
    int polys = 0;

    for (int i = 0; i < m_lod[m_curLod].Count(); i++) {
        if (m_lod[m_curLod][i].m_model->Is_Not_Hidden_At_All()) {
            polys += m_lod[m_curLod][i].m_model->Get_Num_Polys();
        }
    }

    for (int i = 0; i < m_additionalModels.Count(); i++) {
        if (m_additionalModels[i].m_model->Is_Not_Hidden_At_All()) {
            polys += m_additionalModels[i].m_model->Get_Num_Polys();
        }
    }

    return polys;
}

void HLodClass::Set_Transform(const Matrix3D &m)
{
    Animatable3DObjClass::Set_Transform(m);
    Set_Sub_Object_Transforms_Dirty(true);
}

void HLodClass::Set_Position(const Vector3 &v)
{
    Animatable3DObjClass::Set_Position(v);
    Set_Sub_Object_Transforms_Dirty(true);
}

void HLodClass::Notify_Added(SceneClass *scene)
{
    RenderObjClass::Notify_Added(scene);

    for (int i = 0; i < m_lod[m_curLod].Count(); i++) {
        m_lod[m_curLod][i].m_model->Notify_Added(scene);
    }

    for (int i = 0; i < m_additionalModels.Count(); i++) {
        m_additionalModels[i].m_model->Notify_Added(scene);
    }
}

void HLodClass::Notify_Removed(SceneClass *scene)
{
    for (int i = 0; i < m_lod[m_curLod].Count(); i++) {
        m_lod[m_curLod][i].m_model->Notify_Removed(scene);
    }

    for (int i = 0; i < m_additionalModels.Count(); i++) {
        m_additionalModels[i].m_model->Notify_Removed(scene);
    }

    RenderObjClass::Notify_Removed(scene);
}

int HLodClass::Get_Num_Sub_Objects() const
{
    int count = 0;

    for (int i = 0; i < m_lodCount; i++) {
        count += m_lod[i].Count();
    }

    return count;
}

void HLodClass::Set_Animation()
{
    Animatable3DObjClass::Set_Animation();
    Set_Sub_Object_Transforms_Dirty(true);
}

void HLodClass::Set_Animation(HAnimClass *anim, float frame, int anim_mode)
{
    Animatable3DObjClass::Set_Animation(anim, frame, anim_mode);
    Set_Sub_Object_Transforms_Dirty(true);
}

void HLodClass::Set_Animation(HAnimClass *motion0, float frame, HAnimClass *motion1, float frame1, float percentage)
{
    Animatable3DObjClass::Set_Animation(motion0, frame, motion1, frame1, percentage);
    Set_Sub_Object_Transforms_Dirty(true);
}

void HLodClass::Set_Animation(HAnimComboClass *combo)
{
    Animatable3DObjClass::Set_Animation(combo);
    Set_Sub_Object_Transforms_Dirty(true);
}

bool HLodClass::Cast_AABox(AABoxCollisionTestClass &boxtest)
{
    if (Are_Sub_Object_Transforms_Dirty()) {
        Update_Sub_Object_Transforms();
    }

    ModelArrayClass *lod = &m_lod[m_lodCount - 1];
    bool result = false;

    for (int i = 0; i < lod->Count(); i++) {
        result |= (*lod)[i].m_model->Cast_AABox(boxtest);
    }

    for (int i = 0; i < m_additionalModels.Count(); i++) {
        result |= m_additionalModels[i].m_model->Cast_AABox(boxtest);
    }

    return result;
}

bool HLodClass::Cast_OBBox(OBBoxCollisionTestClass &boxtest)
{
    if (Are_Sub_Object_Transforms_Dirty()) {
        Update_Sub_Object_Transforms();
    }

    ModelArrayClass *lod = &m_lod[m_lodCount - 1];
    bool result = false;

    for (int i = 0; i < lod->Count(); i++) {
        result |= (*lod)[i].m_model->Cast_OBBox(boxtest);
    }

    for (int i = 0; i < m_additionalModels.Count(); i++) {
        result |= m_additionalModels[i].m_model->Cast_OBBox(boxtest);
    }

    return result;
}

bool HLodClass::Intersect_AABox(AABoxIntersectionTestClass &boxtest)
{
    if (Are_Sub_Object_Transforms_Dirty()) {
        Update_Sub_Object_Transforms();
    }

    ModelArrayClass *lod = &m_lod[m_lodCount - 1];
    bool result = false;

    for (int i = 0; i < lod->Count(); i++) {
        result |= (*lod)[i].m_model->Intersect_AABox(boxtest);
    }

    for (int i = 0; i < m_additionalModels.Count(); i++) {
        result |= m_additionalModels[i].m_model->Intersect_AABox(boxtest);
    }

    return result;
}

bool HLodClass::Intersect_OBBox(OBBoxIntersectionTestClass &boxtest)
{
    if (Are_Sub_Object_Transforms_Dirty()) {
        Update_Sub_Object_Transforms();
    }

    ModelArrayClass *lod = &m_lod[m_lodCount - 1];
    bool result = false;

    for (int i = 0; i < lod->Count(); i++) {
        result |= (*lod)[i].m_model->Intersect_OBBox(boxtest);
    }

    for (int i = 0; i < m_additionalModels.Count(); i++) {
        result |= m_additionalModels[i].m_model->Intersect_OBBox(boxtest);
    }

    return result;
}

void HLodClass::Create_Decal(DecalGeneratorClass *generator)
{
    captainslog_dbgassert(0, "decals not supported");
}

void HLodClass::Delete_Decal(unsigned long decal_id)
{
    captainslog_dbgassert(0, "decals not supported");
}

int HLodClass::Get_Num_Snap_Points()
{
    if (m_snapPoints) {
        return m_snapPoints->Count();
    } else {
        return 0;
    }
}

void HLodClass::Get_Snap_Point(int point, Vector3 *set)
{
    captainslog_assert(set != nullptr);

    if (m_snapPoints) {
        *set = (*m_snapPoints)[point];
    } else {
        *set = Vector3(0, 0, 0);
    }
}

void HLodClass::Scale(float scalex, float scaley, float scalez) {}

void HLodClass::Set_HTree(HTreeClass *htree)
{
    Animatable3DObjClass::Set_HTree(htree);
}

RenderObjClass *HLodClass::Get_Sub_Object(int index) const
{
    captainslog_assert(index >= 0);

    for (int i = 0; i < m_lodCount; i++) {
        if (index < m_lod[i].Count()) {
            m_lod[i][index].m_model->Add_Ref();
            return m_lod[i][index].m_model;
        }

        index -= m_lod[i].Count();
    }

    captainslog_assert(index < m_additionalModels.Count());
    m_additionalModels[index].m_model->Add_Ref();
    return m_additionalModels[index].m_model;
}

int HLodClass::Add_Sub_Object(RenderObjClass *subobj)
{
    return Add_Sub_Object_To_Bone(subobj, 0);
}

int HLodClass::Remove_Sub_Object(RenderObjClass *subobj)
{
    if (!subobj) {
        return 0;
    }

    bool deleted = false;
    bool iscurrent = false;

    for (int i = 0; i < m_lodCount && !deleted; i++) {
        for (int j = 0; j < m_lod[i].Count() && !deleted; j++) {
            if (m_lod[i][j].m_model == subobj) {
                m_lod[i].Delete(j);
                deleted = true;

                if (i == m_curLod) {
                    iscurrent = true;
                }
            }
        }
    }

    for (int i = 0; i < m_additionalModels.Count() && !deleted; i++) {
        if (m_additionalModels[i].m_model == subobj) {
            m_additionalModels.Delete(i);
            deleted = true;
            iscurrent = true;
        }
    }

    if (!deleted) {
        return 0;
    }

    subobj->Set_Container(nullptr);

    if (iscurrent) {
        if (Is_In_Scene()) {
            subobj->Notify_Removed(m_scene);
        }
    }

    subobj->Release_Ref();
    Update_Sub_Object_Bits();
    Update_Obj_Space_Bounding_Volumes();
    return 1;
}

int HLodClass::Get_Num_Sub_Objects_On_Bone(int bone_index) const
{
    int count = 0;

    for (int i = 0; i < m_lodCount; i++) {
        for (int j = 0; j < m_lod[i].Count(); j++) {
            if (m_lod[i][j].m_boneIndex == bone_index) {
                count++;
            }
        }
    }

    for (int k = 0; k < m_additionalModels.Count(); k++) {
        if (m_additionalModels[k].m_boneIndex == bone_index) {
            count++;
        }
    }

    return count;
}

RenderObjClass *HLodClass::Get_Sub_Object_On_Bone(int index, int bone_index) const
{
    int count = 0;
    for (int i = 0; i < m_lodCount; i++) {
        for (int j = 0; j < m_lod[i].Count(); j++) {
            if (m_lod[i][j].m_boneIndex == bone_index) {
                if (count == index) {
                    m_lod[i][j].m_model->Add_Ref();
                    return m_lod[i][j].m_model;
                }

                count++;
            }
        }
    }

    for (int k = 0; k < m_additionalModels.Count(); k++) {
        if (m_additionalModels[k].m_boneIndex == bone_index) {
            if (count == index) {
                m_additionalModels[k].m_model->Add_Ref();
                return m_additionalModels[k].m_model;
            }

            count++;
        }
    }

    return nullptr;
}

int HLodClass::Get_Sub_Object_Bone_Index(RenderObjClass *object) const
{
    for (int i = 0; i < m_lodCount; i++) {
        for (int j = 0; j < m_lod[i].Count(); j++) {
            if (m_lod[i][j].m_model == object) {
                return m_lod[i][j].m_boneIndex;
            }
        }
    }

    for (int k = 0; k < m_additionalModels.Count(); k++) {
        if (m_additionalModels[k].m_model == object) {
            return m_additionalModels[k].m_boneIndex;
        }
    }

    return 0;
}

int HLodClass::Get_Sub_Object_Bone_Index(int lodindex, int modelindex) const
{
    return m_lod[lodindex][modelindex].m_boneIndex;
}

int HLodClass::Add_Sub_Object_To_Bone(RenderObjClass *subobj, int bone_index)
{
    captainslog_assert(subobj);

    if (bone_index >= 0 && bone_index < m_htree->Num_Pivots()) {
        subobj->Set_LOD_Bias(m_lodBias);
        subobj->Add_Ref();
        subobj->Set_Container(this);
        subobj->Set_Animation_Hidden(!m_htree->Get_Visibility(bone_index));

        ModelNodeClass node;
        node.m_model = subobj;
        node.m_boneIndex = bone_index;
        int res = m_additionalModels.Add(node);

        Update_Sub_Object_Bits();
        Update_Obj_Space_Bounding_Volumes();
        Set_Hierarchy_Valid(false);
        Set_Sub_Object_Transforms_Dirty(true);

        if (Is_In_Scene()) {
            subobj->Notify_Added(m_scene);
        }

        return res;
    } else {
        return 0;
    }
}

void HLodClass::Update_Sub_Object_Transforms()
{
    Animatable3DObjClass::Update_Sub_Object_Transforms();

    for (int i = 0; i < m_lodCount; i++) {
        ModelArrayClass &lod = m_lod[i];
        for (int m = 0; m < lod.Count(); ++m) {
            RenderObjClass *model = lod[m].m_model;
            int bone_index = lod[m].m_boneIndex;

            model->Set_Transform(m_htree->Get_Transform(bone_index));
            model->Set_Animation_Hidden(!m_htree->Get_Visibility(bone_index));
            model->Update_Sub_Object_Transforms();
        }
    }

    for (int m = 0; m < m_additionalModels.Count(); ++m) {
        RenderObjClass *model = m_additionalModels[m].m_model;
        int bone_index = m_additionalModels[m].m_boneIndex;

        model->Set_Transform(m_htree->Get_Transform(bone_index));
        model->Set_Animation_Hidden(!m_htree->Get_Visibility(bone_index));
        model->Update_Sub_Object_Transforms();
    }

    Set_Sub_Object_Transforms_Dirty(false);
}

const SphereClass &HLodClass::Get_Bounding_Sphere() const
{
    if (m_boundingBoxIndex < 0) {
        return RenderObjClass::Get_Bounding_Sphere();
    } else {
        Get_Obj_Space_Bounding_Sphere(m_cachedBoundingSphere);
        m_cachedBoundingSphere.Transform(Get_Transform());
        return m_cachedBoundingSphere;
    }
}

const AABoxClass &HLodClass::Get_Bounding_Box() const
{
    if (m_boundingBoxIndex < 0) {
        return RenderObjClass::Get_Bounding_Box();
    } else {
        AABoxClass box;
        Get_Obj_Space_Bounding_Box(box);
        Get_Transform().Transform_Center_Extent_AABox(
            box.m_center, box.m_extent, &m_cachedBoundingBox.m_center, &m_cachedBoundingBox.m_extent);
        return m_cachedBoundingBox;
    }
}

void HLodClass::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    AABoxClass box;
    Get_Obj_Space_Bounding_Box(box);
    sphere.Center = box.m_center;
    sphere.Radius = box.m_extent.Length();
}

void HLodClass::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    if (m_boundingBoxIndex < 0 || m_boundingBoxIndex >= m_lod[m_lodCount - 1].Count()) {
        CompositeRenderObjClass::Get_Obj_Space_Bounding_Box(box);
    } else {
        RenderObjClass *robj = m_lod[m_lodCount - 1][m_boundingBoxIndex].m_model;

        if (robj && robj->Class_ID() == CLASSID_OBBOX) {
            Matrix3D tm1;
            Simple_Evaluate_Bone(m_lod[m_lodCount - 1][m_boundingBoxIndex].m_boneIndex, &tm1);
            Matrix3D tm2;
            Get_Transform().Get_Orthogonal_Inverse(tm2);

            Matrix3D tm3;
            Matrix3D::Multiply(tm2, tm1, &tm3);
            tm3.Transform_Center_Extent_AABox(static_cast<OBBoxRenderObjClass *>(robj)->Get_Local_Center(),
                static_cast<OBBoxRenderObjClass *>(robj)->Get_Local_Extent(),
                &box.m_center,
                &box.m_extent);
        }
    }
}

void HLodClass::Update_Obj_Space_Bounding_Volumes()
{
    ModelArrayClass &lod = m_lod[m_lodCount - 1];

    if (m_boundingBoxIndex < 0 || m_boundingBoxIndex >= lod.Count()
        || lod[m_boundingBoxIndex].m_model->Class_ID() != CLASSID_OBBOX) {
        m_boundingBoxIndex = -1;
    }

    for (int i = lod.Count() - 1; i >= 0 && m_boundingBoxIndex == -1; i--) {
        RenderObjClass *model = lod[i].m_model;

        if (model->Class_ID() == CLASSID_OBBOX) {
            const char *mesh_name = model->Get_Name();
            const char *name_suffix = strchr(mesh_name, '.');

            if (name_suffix) {
                mesh_name = name_suffix + 1;
            }

            if (strcasecmp(mesh_name, "BOUNDINGBOX") == 0) {
                m_boundingBoxIndex = i;
            }
        }
    }

    if (Get_Num_Sub_Objects() > 0) {
        SphereClass obj_sphere;
        AABoxClass obj_box;
        MinMaxAABoxClass new_box;
        m_htree->Base_Update(Matrix3D(true));
        RenderObjClass *robj = Get_Sub_Object(0);

        captainslog_assert(robj);

        Matrix3D transform = m_htree->Get_Transform(Get_Sub_Object_Bone_Index(robj));
        robj->Get_Obj_Space_Bounding_Sphere(obj_sphere);
        obj_sphere.Transform(transform);
        robj->Get_Obj_Space_Bounding_Box(obj_box);
        new_box.Init(obj_box);
        new_box.Transform(transform);
        robj->Release_Ref();

        for (int i = 1; i < Get_Num_Sub_Objects(); i++) {
            RenderObjClass *robj2 = Get_Sub_Object(i);

            captainslog_assert(robj2);

            Matrix3D transform2 = m_htree->Get_Transform(Get_Sub_Object_Bone_Index(robj2));
            SphereClass sphere;
            robj2->Get_Obj_Space_Bounding_Sphere(sphere);
            sphere.Transform(transform2);
            obj_sphere.Add_Sphere(sphere);
            AABoxClass box;
            robj2->Get_Obj_Space_Bounding_Box(box);
            box.Transform(transform2);
            new_box.Add_Box(box);
            robj2->Release_Ref();
        }

        m_objSphere = obj_sphere;
        m_objBox = new_box;

        Invalidate_Cached_Bounding_Volumes();
        Set_Hierarchy_Valid(false);

        if (m_container) {
            m_container->Update_Obj_Space_Bounding_Volumes();
        }
    } else {
        m_objSphere.Init(Vector3(0, 0, 0), 0);
        m_objBox.Init(Vector3(0, 0, 0), Vector3(0, 0, 0));
    }
}

void HLodClass::Scale(float scale)
{
    if (scale != 1.0f) {
        for (int i = 0; i < m_lodCount; i++) {
            for (int j = 0; j < m_lod[i].Count(); j++) {
                m_lod[i][j].m_model->Scale(scale);
            }
        }

        for (int k = 0; k < m_additionalModels.Count(); k++) {
            m_additionalModels[k].m_model->Scale(scale);
        }

        m_htree->Scale(scale);
        Set_Hierarchy_Valid(false);

        if (Get_Container()) {
            Get_Container()->Update_Obj_Space_Bounding_Volumes();
        }
    }
}

void HLodClass::Set_Hidden(int hidden)
{
    for (int k = 0; k < m_additionalModels.Count(); k++) {
        RenderObjClass *model = m_additionalModels[k].m_model;
        if (model->Class_ID() == CLASSID_PARTICLEEMITTER) {
            model->Set_Hidden(hidden);
        }
    }

    RenderObjClass::Set_Hidden(hidden);
}

void HLodClass::Include_NULL_Lod(bool include)
{
    if (!include) {
        if (Is_NULL_Lod_Included()) {
            for (int i = 0; i < m_lod[0].Count(); i++) {
                RenderObjClass *robj = m_lod[0][i].m_model;
                m_lod[0][i].m_model = nullptr;

                captainslog_assert(robj);

                robj->Set_Container(nullptr);
                robj->Release_Ref();
            }

            m_lodCount--;
            m_lod[0].Delete_All();

            ModelArrayClass *lods = new ModelArrayClass[m_lodCount];

            for (int j = 0; j < m_lodCount; j++) {
                lods[j] = m_lod[j + 1];
            }

            float *costs = new float[m_lodCount];
            float *values = new float[m_lodCount + 1];
            memcpy(costs, &m_cost + 1, m_lodCount * sizeof(float));
            memcpy(values, &m_value + 1, (m_lodCount * sizeof(float)) + sizeof(float));

            delete[] m_lod;
            delete[] m_cost;
            delete[] m_value;
            m_lod = lods;
            m_cost = costs;
            m_value = values;

            if (m_curLod >= m_lodCount) {
                m_curLod = m_lodCount - 1;
            }
        }
    } else if (!Is_NULL_Lod_Included()) {
        RenderObjClass *null_object = W3DAssetManager::Get_Instance()->Create_Render_Obj("NULL");
        captainslog_assert(null_object);

        if (null_object) {
            ModelArrayClass *lods = new ModelArrayClass[m_lodCount + 1];

            for (int m = 0; m < m_lodCount; m++) {
                lods[m + 1] = m_lod[m];
            }

            float *costs = new float[m_lodCount];
            float *values = new float[m_lodCount + 1];
            memcpy(costs + 1, &m_cost, m_lodCount * sizeof(float));
            memcpy(values + 1, &m_value, (m_lodCount * sizeof(float)) + sizeof(float));

            delete[] m_lod;
            delete[] m_cost;
            delete[] m_value;
            m_lod = lods;
            m_cost = costs;
            m_value = values;
            m_lodCount++;

            Add_Lod_Model(0, null_object, 0);
            null_object->Release_Ref();
        }
    }

    int lod = Calculate_Cost_Value_Arrays(1.0f, m_value, m_cost);

    if (m_curLod < lod) {
        Set_LOD_Level(lod);
    }
}

int HLodClass::Get_Proxy_Count()
{
    if (m_proxyArray) {
        return m_proxyArray->Length();
    }
    return 0;
}

bool HLodClass::Get_Proxy(int index, ProxyClass &proxy)
{
    if (m_proxyArray) {
        m_htree->Base_Update(Get_Transform());
        Set_Hierarchy_Valid(false);
        proxy.Set_Transform(m_htree->Get_Transform((*m_proxyArray)[index].Get_Bone_Index()));
        proxy.Set_Name((*m_proxyArray)[index].Get_Name());
        return true;
    } else {
        proxy.Set_Name("");
        proxy.Set_Transform(Matrix3D(true));
    }

    return false;
}

void HLodClass::Add_Lod_Model(int lod, RenderObjClass *robj, int bone_index)
{
    captainslog_assert(robj);

    ModelNodeClass node;
    node.m_model = robj;
    node.m_boneIndex = bone_index;
    robj->Add_Ref();
    robj->Set_Container(this);
    robj->Set_Transform(m_htree->Get_Transform(bone_index));

    if (Is_In_Scene() && lod == m_curLod) {
        robj->Notify_Added(m_scene);
    }

    m_lod[lod].Add(node);
}

void HLodClass::Special_Render(SpecialRenderInfoClass &rinfo)
{
    if (Is_Not_Hidden_At_All()) {
        Animatable3DObjClass::Special_Render(rinfo);
        int lod = m_curLod;

        if (rinfo.m_renderType == SpecialRenderInfoClass::RENDER_SHADOW) {
            lod = m_lodCount - 1;
        }

        ModelArrayClass *mlod = &m_lod[lod];

        for (int pos = 0; pos < mlod->Count(); pos++) {
            (*mlod)[pos].m_model->Special_Render(rinfo);
        }

        for (int pos = 0; pos < m_additionalModels.Count(); pos++) {
            m_additionalModels[pos].m_model->Special_Render(rinfo);
        }
    }
}

void HLodClass::Render(RenderInfoClass &rinfo)
{
    if (Is_Not_Hidden_At_All()) {
        Animatable3DObjClass::Render(rinfo);
        ModelArrayClass *mlod = &m_lod[m_curLod];

        for (int pos = 0; pos < mlod->Count(); pos++) {
            if ((*mlod)[pos].m_model->Class_ID() != CLASSID_OBBOX) {
                (*mlod)[pos].m_model->Render(rinfo);
            }
        }

        if (!Is_Sub_Objects_Match_LOD_Enabled()) {
            for (int pos = 0; pos < m_additionalModels.Count(); pos++) {
                m_additionalModels[pos].m_model->Render(rinfo);
            }
        } else {
            for (int pos = 0; pos < m_additionalModels.Count(); pos++) {
                m_additionalModels[pos].m_model->Set_LOD_Level(Get_LOD_Level());
                m_additionalModels[pos].m_model->Render(rinfo);
            }
        }
    }
}

void HLodClass::Set_Max_Screen_Size(int lod_index, float size)
{
    captainslog_assert(lod_index >= 0);
    captainslog_assert(lod_index < m_lodCount);

    if (lod_index >= 0) {
        if (lod_index < m_lodCount) {
            m_lod[lod_index].m_maxScreenSize = size;
            Recalculate_Static_LOD_Factors();
            int lod = Calculate_Cost_Value_Arrays(1.0f, m_value, m_cost);

            if (m_curLod < lod) {
                Set_LOD_Level(lod);
            }
        }
    }
}

float HLodClass::Get_Max_Screen_Size(int lod_index)
{
    captainslog_assert(lod_index >= 0);
    captainslog_assert(lod_index < m_lodCount);

    if (lod_index >= 0) {
        if (lod_index < m_lodCount) {
            return m_lod[lod_index].m_maxScreenSize;
        }
    }
    return NO_MAX_SCREEN_SIZE;
}

int HLodClass::Get_Lod_Count()
{
    return m_lodCount;
}

void HLodClass::Set_LOD_Bias(float bias)
{
    captainslog_assert(bias > 0.0f);

    float newbias;

    if (bias <= 0.0f) {
        newbias = 0.0f;
    } else {
        newbias = bias;
    }

    m_lodBias = newbias;

    for (int i = 0; i < m_additionalModels.Count(); i++) {
        m_additionalModels[i].m_model->Set_LOD_Bias(newbias);
    }
}

int HLodClass::Get_Lod_Model_Count(int lod_index)
{
    captainslog_assert(lod_index >= 0);
    captainslog_assert(lod_index < m_lodCount);

    if (lod_index >= 0) {
        if (lod_index < m_lodCount) {
            return m_lod[lod_index].Count();
        }
    }
    return 0;
}

RenderObjClass *HLodClass::Peek_Lod_Model(int lod_index, int model_index)
{
    captainslog_assert(lod_index >= 0);
    captainslog_assert(lod_index < m_lodCount);

    if (lod_index >= 0) {
        if (lod_index < m_lodCount) {
            ModelArrayClass &model = m_lod[lod_index];
            if (model_index < model.Count()) {
                return model[model_index].m_model;
            }
        }
    }
    return nullptr;
}

RenderObjClass *HLodClass::Get_Lod_Model(int lod_index, int model_index)
{
    captainslog_assert(lod_index >= 0);
    captainslog_assert(lod_index < m_lodCount);

    if (lod_index >= 0) {
        if (lod_index < m_lodCount) {
            ModelArrayClass &model = m_lod[lod_index];
            if (model_index < model.Count()) {
                model[model_index].m_model->Add_Ref();
                return model[model_index].m_model;
            }
        }
    }
    return nullptr;
}

int HLodClass::Get_Lod_Model_Bone(int lod_index, int model_index)
{
    captainslog_assert(lod_index >= 0);
    captainslog_assert(lod_index < m_lodCount);

    if (lod_index >= 0) {
        if (lod_index < m_lodCount) {
            ModelArrayClass &model = m_lod[lod_index];
            if (model_index < model.Count()) {
                return model[model_index].m_boneIndex;
            }
        }
    }
    return 0;
}

int HLodClass::Get_Additional_Model_Count()
{
    return m_additionalModels.Count();
}

RenderObjClass *HLodClass::Peek_Additional_Model(int model_index)
{
    if (model_index >= 0) {
        if (model_index < m_additionalModels.Count()) {
            return m_additionalModels[model_index].m_model;
        }
    }
    return nullptr;
}

RenderObjClass *HLodClass::Get_Additional_Model(int model_index)
{
    if (model_index >= 0) {
        if (model_index < m_additionalModels.Count()) {
            m_additionalModels[model_index].m_model->Add_Ref();
            return m_additionalModels[model_index].m_model;
        }
    }
    return nullptr;
}

int HLodClass::Get_Additional_Model_Bone(int model_index)
{
    if (model_index >= 0) {
        if (model_index < m_additionalModels.Count()) {
            return m_additionalModels[model_index].m_boneIndex;
        }
    }
    return 0;
}

bool HLodClass::Is_NULL_Lod_Included()
{
    if (m_lodCount > 0) {
        RenderObjClass *obj = (*m_lod)[0].m_model;
        if (obj) {
            return (obj->Class_ID() == RenderObjClass::CLASSID_NULL);
        }
    }
    return false;
}

void HLodClass::Prepare_LOD(CameraClass &camera)
{
    if (Is_Not_Hidden_At_All()) {
        float sz = Get_Screen_Size(camera);

        if (m_lodCount > 1) {
            int lod = Calculate_Cost_Value_Arrays(sz, m_value, m_cost);

            if (m_curLod < lod) {
                Set_LOD_Level(lod);
            }
        }

        for (int i = 0; i < m_additionalModels.Count(); i++) {
            if (m_additionalModels[i].m_model->Is_Not_Hidden_At_All()) {
                m_additionalModels[i].m_model->Prepare_LOD(camera);
            }
        }
    }
}

void HLodClass::Recalculate_Static_LOD_Factors()
{
    for (int i = 0; i < m_lodCount; i++) {
        m_lod[i].m_pixelCostPerArea = 0;
        int polycount = 0;

        for (int j = 0; j < m_lod[i].Count(); j++) {
            if (m_lod[i][j].m_model->Is_Not_Hidden_At_All()) {
                polycount += m_lod[i][j].m_model->Get_Num_Polys();
            }
        }

        if (polycount) {
            m_lod[i].m_nonPixelCost = (float)polycount;
        } else {
            m_lod[i].m_nonPixelCost = 0.000001f;
        }

        if (polycount) {
            m_lod[i].m_benefitFactor = 1.0f - 0.5f / (float)(polycount * polycount);
        } else {
            m_lod[i].m_benefitFactor = 0.0f;
        }
    }
}

void HLodClass::Increment_LOD()
{
    if (m_curLod < m_lodCount - 1) {
        if (Is_In_Scene()) {
            for (int i = 0; i < m_lod[m_curLod].Count(); i++) {
                m_lod[m_curLod][i].m_model->Notify_Removed(m_scene);
            }
        }

        m_curLod++;

        if (Is_In_Scene()) {
            for (int i = 0; i < m_lod[m_curLod].Count(); i++) {
                m_lod[m_curLod][i].m_model->Notify_Added(m_scene);
            }
        }
    }
}

void HLodClass::Decrement_LOD()
{
    if (m_curLod > 0) {
        if (Is_In_Scene()) {
            for (int i = 0; i < m_lod[m_curLod].Count(); i++) {
                m_lod[m_curLod][i].m_model->Notify_Removed(m_scene);
            }
        }

        m_curLod--;

        if (Is_In_Scene()) {
            for (int i = 0; i < m_lod[m_curLod].Count(); i++) {
                m_lod[m_curLod][i].m_model->Notify_Added(m_scene);
            }
        }
    }
}

float HLodClass::Get_Cost() const
{
    return m_cost[m_curLod];
}

float HLodClass::Get_Value() const
{
    return m_value[m_curLod];
}

float HLodClass::Get_Post_Increment_Value() const
{
    return m_value[m_curLod + 1];
}

void HLodClass::Set_LOD_Level(int lod)
{
    int newlod = lod < 0 ? 0 : lod;

    if (newlod >= m_lodCount - 1) {
        newlod = m_lodCount - 1;
    }

    if (newlod != m_curLod) {
        if (Is_In_Scene()) {
            for (int i = 0; i < m_lod[m_curLod].Count(); i++) {
                m_lod[m_curLod][i].m_model->Notify_Removed(m_scene);
            }
        }

        m_curLod = newlod;

        if (Is_In_Scene()) {
            for (int i = 0; i < m_lod[m_curLod].Count(); i++) {
                m_lod[m_curLod][i].m_model->Notify_Added(m_scene);
            }
        }
    }
}

int HLodClass::Get_LOD_Level() const
{
    return m_curLod;
}

int HLodClass::Get_LOD_Count() const
{
    return m_lodCount;
}

int HLodClass::Calculate_Cost_Value_Arrays(float screen_area, float *values, float *costs) const
{
    for (int i = 0; i < m_lodCount; i++) {
        costs[i] = screen_area * m_lod[i].m_pixelCostPerArea + m_lod[i].m_nonPixelCost;
    }

    int lod = 0;

    for (lod = 0; lod < m_lodCount; lod++) {
        if (m_lod[lod].m_maxScreenSize >= screen_area) {
            break;
        }
        values[lod] = AT_MIN_LOD;
    }

    if (lod < m_lodCount) {
        values[lod] = AT_MIN_LOD;
    } else {
        lod = m_lodCount - 1;
    }

    for (int i = lod + 1; i < m_lodCount; i++) {
        values[i] = screen_area * m_lod[i].m_benefitFactor * m_lodBias / costs[i];
    }

    values[m_lodCount] = AT_MAX_LOD;

    return lod;
}

RenderObjClass *HLodClass::Get_Current_LOD()
{
    if (Get_Lod_Model_Count(m_curLod)) {
        return Get_Lod_Model(m_curLod, 0);
    }

    return nullptr;
}

bool HLodClass::Cast_Ray(RayCollisionTestClass &raytest)
{
    if (Are_Sub_Object_Transforms_Dirty()) {
        Update_Sub_Object_Transforms();
    }

    ModelArrayClass *lod = &m_lod[m_lodCount - 1];
    bool result = false;

    for (int i = 0; i < lod->Count(); i++) {
        result |= (*lod)[i].m_model->Cast_Ray(raytest);
    }

    for (int i = 0; i < m_additionalModels.Count(); i++) {
        result |= m_additionalModels[i].m_model->Cast_Ray(raytest);
    }

    return result;
}

void HLodClass::Free()
{
    for (int i = 0; i < m_lodCount; i++) {
        for (int j = 0; j < m_lod[i].Count(); j++) {
            RenderObjClass *robj = m_lod[i][j].m_model;
            m_lod[i][j].m_model = nullptr;

            captainslog_assert(robj);

            robj->Set_Container(nullptr);
            robj->Release_Ref();
        }
        m_lod[i].Delete_All();
    }

    delete[] m_lod;
    m_lodCount = 0;
    delete[] m_cost;
    delete[] m_value;
    m_lod = nullptr;
    m_cost = nullptr;
    m_value = nullptr;

    for (int i = 0; i < m_additionalModels.Count(); i++) {
        RenderObjClass *robj = m_additionalModels[i].m_model;
        m_additionalModels[i].m_model = nullptr;

        captainslog_assert(robj);

        robj->Set_Container(nullptr);
        robj->Release_Ref();
    }

    m_additionalModels.Delete_All();
    Ref_Ptr_Release(m_snapPoints);
    Ref_Ptr_Release(m_proxyArray);
}

PrototypeClass *DistLODLoaderClass::Load_W3D(ChunkLoadClass &cload)
{
    captainslog_assert("DistLOD not suported");
    return nullptr;
}

PrototypeClass *HModelLoaderClass::Load_W3D(ChunkLoadClass &cload)
{
    captainslog_assert("HModel not suported");
    return nullptr;
}

PrototypeClass *CollectionLoaderClass::Load_W3D(ChunkLoadClass &cload)
{
    captainslog_assert("Collection not suported");
    return nullptr;
}
