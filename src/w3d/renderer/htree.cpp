/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief HTree
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "htree.h"
#include "chunkio.h"
#include "hanim.h"
#include "hrawanim.h"
#include "quat.h"
#include "w3d_file.h"
#include <cstring>
#include <strings.h>

using std::memcpy;
using std::strcpy;

HTreeClass::HTreeClass() : m_numPivots(0), m_pivot(nullptr), m_scaleFactor(1.0f)
{
    // #BUGFIX Initialize all members
    m_name[0] = '\0';
}

void HTreeClass::Init_Default()
{
    Free();

    m_name[0] = '\0';
    m_numPivots = 1;
    m_pivot = new PivotClass[1];
    strcpy(m_pivot->name, "RootTransform");
}

HTreeClass::~HTreeClass()
{
    Free();
}

HTreeClass::HTreeClass(HTreeClass const &src)
{
    memcpy(m_name, src.m_name, sizeof(m_name));
    m_numPivots = src.m_numPivots;

    m_pivot = new PivotClass[m_numPivots];

    for (int i = 0; i < m_numPivots; ++i) {
        PivotClass &pivot = m_pivot[i];
        pivot = src.m_pivot[i];

        if (pivot.parent) {
            m_pivot[i].parent = &m_pivot[pivot.parent->index];
        }
    }

    m_scaleFactor = src.m_scaleFactor;
}

int HTreeClass::Load_W3D(ChunkLoadClass &cload)
{
    Free();

    W3dHierarchyStruct header;

    if (!cload.Open_Chunk()) {
        return 1;
    }

    if (cload.Cur_Chunk_ID() != W3D_CHUNK_HIERARCHY_HEADER) {
        return 1;
    }

    if (cload.Read(&header, sizeof(header)) != sizeof(header)) {
        return 1;
    }

    cload.Close_Chunk();

    bool create_root = false;

    if (header.Version < 0x30000) {
        create_root = true;
        ++header.NumPivots;
    }

    static_assert(sizeof(m_name) >= sizeof(header.Name));
    strlcpy_tpl(m_name, header.Name);
    m_numPivots = header.NumPivots;

    if (m_numPivots > 0) {
        m_pivot = new PivotClass[m_numPivots];
    }

    if (!cload.Open_Chunk()) {
        return 0;
    }

    while (cload.Cur_Chunk_ID() != W3D_CHUNK_PIVOTS || Read_Pivots(cload, create_root)) {
        cload.Close_Chunk();

        if (!cload.Open_Chunk()) {
            return 0;
        }
    }

    Free();
    return 1;
}

bool HTreeClass::Read_Pivots(ChunkLoadClass &cload, bool pre30)
{
    int pidx = 0;

    if (pre30) {
        m_pivot[0].index = 0;
        m_pivot[0].parent = nullptr;
        m_pivot[0].base_transform.Make_Identity();
        m_pivot[0].transform.Make_Identity();
        m_pivot[0].is_visible = true;
        strcpy(m_pivot->name, "RootTransform");
        pidx = 1;
    }

    for (int i = pidx; i < m_numPivots; i++) {
        W3dPivotStruct pivot_data;

        if (cload.Read(&pivot_data, sizeof(pivot_data)) != sizeof(pivot_data)) {
            return 0;
        }

        PivotClass &pivot = m_pivot[i];
        memcpy(pivot.name, pivot_data.Name, sizeof(m_name));
        pivot.index = i;
        pivot.base_transform.Make_Identity();

        Vector3 translation;
        translation.X = pivot_data.Translation.x;
        translation.Y = pivot_data.Translation.y;
        translation.Z = pivot_data.Translation.z;
        pivot.base_transform.Translate(translation);

        Quaternion rotation;
        rotation.X = pivot_data.Rotation.q[0];
        rotation.Y = pivot_data.Rotation.q[1];
        rotation.Z = pivot_data.Rotation.q[2];
        rotation.W = pivot_data.Rotation.q[3];

        Matrix3D rotation_matrix = Build_Matrix3D(rotation);
        pivot.base_transform.Post_Mul(rotation_matrix);

        if (pre30) {
            ++pivot_data.ParentIdx;
        }

        if (pivot_data.ParentIdx == -1) {
            pivot.parent = nullptr;
        } else {
            pivot.parent = &m_pivot[pivot_data.ParentIdx];
        }
    }

    m_pivot[0].transform.Make_Identity();
    m_pivot[0].is_visible = true;
    return 1;
}

void HTreeClass::Free()
{
    if (m_pivot) {
        delete[] m_pivot;
        m_pivot = nullptr;
    }

    m_numPivots = 0;
    m_scaleFactor = 1.0f;
}

bool HTreeClass::Simple_Evaluate_Pivot(
    HAnimClass *motion, int pivot_index, float frame, Matrix3D const &obj_tm, Matrix3D *end_tm)
{
    end_tm->Make_Identity();

    if (!motion || !end_tm || pivot_index < 0 || pivot_index >= m_numPivots) {
        return false;
    }

    for (PivotClass *i = &m_pivot[pivot_index]; i; i = i->parent) {
        if (!i->parent) {
            break;
        }

        Matrix3D anim_tm;
        motion->Get_Transform(anim_tm, i->index, frame);
        anim_tm.Set_Translation(anim_tm.Get_Translation() * m_scaleFactor);

        Matrix3D curr_tm;
        Matrix3D::Multiply(i->base_transform, anim_tm, &curr_tm);
        end_tm->Mul(curr_tm, *end_tm);
    }

    end_tm->Mul(obj_tm, *end_tm);
    return true;
}

bool HTreeClass::Simple_Evaluate_Pivot(int pivot_index, Matrix3D const &obj_tm, Matrix3D *end_tm)
{
    end_tm->Make_Identity();

    if (!end_tm || pivot_index < 0 || pivot_index >= m_numPivots) {
        return false;
    }

    for (PivotClass *i = &m_pivot[pivot_index]; i; i = i->parent) {
        if (!i->parent) {
            break;
        }

        Matrix3D anim_tm(true);
        Matrix3D base_tm;

        Matrix3D::Multiply(i->base_transform, anim_tm, &base_tm);
        Matrix3D::Multiply(base_tm, *end_tm, end_tm);
    }

    Matrix3D::Multiply(obj_tm, *end_tm, end_tm);
    return true;
}

void HTreeClass::Base_Update(Matrix3D const &root)
{
    m_pivot->transform = root;
    m_pivot->is_visible = true;

    for (int i = 1; i < m_numPivots; i++) {
        captainslog_assert(m_pivot[i].parent != nullptr);
        Matrix3D::Multiply(m_pivot[i].parent->transform, m_pivot[i].base_transform, &m_pivot[i].transform);
        m_pivot[i].is_visible = true;

        if (m_pivot[i].is_captured) {
            m_pivot[i].Capture_Update();
        }
    }
}

void HTreeClass::Anim_Update(Matrix3D const &root, HAnimClass *motion, float frame)
{
    m_pivot[0].transform = root;
    m_pivot[0].is_visible = true;
    int num_anim_pivots = motion->Get_Num_Pivots();

    for (int i = 1; i < m_numPivots; i++) {
        PivotClass *pivot = &m_pivot[i];
        Matrix3D::Multiply(pivot->parent->transform, pivot->base_transform, &pivot->transform);

        if (i < num_anim_pivots) {
            Vector3 trans;
            motion->Get_Translation(trans, i, frame);
            pivot->transform.Translate(trans);
            Quaternion q;
            motion->Get_Orientation(q, i, frame);
            Matrix3D mtx = Build_Matrix3D(q);
            pivot->transform.Post_Mul(mtx);
            pivot->is_visible = motion->Get_Visibility(i, frame);
        }

        if (pivot->is_captured) {
            pivot->Capture_Update();
            pivot->is_visible = true;
        }
    }
}

void HTreeClass::Anim_Update(Matrix3D const &root, HRawAnimClass *motion, float frame)
{
    m_pivot[0].transform = root;
    m_pivot[0].is_visible = true;
    int num_anim_pivots = motion->Get_Num_Pivots();

    int fr = frame;

    if (frame >= motion->Get_Num_Frames()) {
        fr = 0;
    }

    PivotClass *pivot = m_pivot;
    NodeMotionStruct *node = &motion->Get_Node_Motion()[1];
    PivotClass *pivot2 = &pivot[1];
    PivotClass *pivot3 = &pivot[num_anim_pivots];
    PivotClass *pivot4 = &pivot[m_numPivots];

    while (pivot2 < pivot4) {
        Matrix3D::Multiply(pivot2->parent->transform, pivot2->base_transform, &pivot2->transform);

        if (pivot2 < pivot3) {
            float x = 0;
            float y = 0;
            float z = 0;

            if (node->X) {
                node->X->Get_Vector(fr, &x);
            }

            if (node->Y) {
                node->Y->Get_Vector(fr, &y);
            }

            if (node->Z) {
                node->Z->Get_Vector(fr, &z);
            }

            if (m_scaleFactor == 1.0f) {
                pivot2->transform.Translate(x, y, z);
            } else {
                pivot2->transform.Translate(x * m_scaleFactor, y * m_scaleFactor, z * m_scaleFactor);
            }

            if (node->Q) {
                Quaternion q;
                node->Q->Get_Vector_As_Quat(fr, q);
                Matrix3D mtx = Build_Matrix3D(q);
                pivot2->transform.Post_Mul(mtx);
            }

            if (node->Vis) {
                pivot2->is_visible = node->Vis->Get_Bit(fr) != 0;
            }
        }

        if (pivot2->is_captured) {
            pivot2->Capture_Update();
            pivot2->is_visible = true;
        }
        pivot2++;
        node++;
    }
}

void HTreeClass::Blend_Update(
    Matrix3D const &root, HAnimClass *motion0, float frame0, HAnimClass *motion1, float frame1, float percentage)
{
    m_pivot->transform = root;
    m_pivot->is_visible = true;
    int num_anim_pivots = motion0->Get_Num_Pivots();

    if (num_anim_pivots >= motion1->Get_Num_Pivots()) {
        num_anim_pivots = motion1->Get_Num_Pivots();
    }

    for (int i = 1; i < m_numPivots; i++) {
        PivotClass *pivot = &m_pivot[i];
        Matrix3D::Multiply(pivot->parent->transform, pivot->base_transform, &pivot->transform);

        if (i < num_anim_pivots) {
            Vector3 translation0, translation1;
            motion0->Get_Translation(translation0, i, frame0);
            motion1->Get_Translation(translation1, i, frame1);

            Vector3 translation;
            Vector3::Lerp(translation0, translation1, percentage, &translation);

            pivot->transform.Translate(translation);

            Quaternion rotation0, rotation1;
            motion0->Get_Orientation(rotation0, i, frame0);
            motion1->Get_Orientation(rotation1, i, frame1);

            Quaternion rotation;
            Fast_Slerp(rotation, rotation0, rotation1, percentage);

            Matrix3D rotation_matrix = Build_Matrix3D(rotation);
            pivot->transform.Post_Mul(rotation_matrix);

            pivot->is_visible = motion0->Get_Visibility(i, frame0) || motion1->Get_Visibility(i, frame1);
        }

        if (pivot->is_captured) {
            pivot->Capture_Update();
            pivot->is_visible = true;
        }
    }
}

int HTreeClass::Get_Bone_Index(char const *name)
{
    if (m_numPivots <= 0) {
        return 0;
    } else {
        for (int i = 0; i < m_numPivots; i++) {
            if (!strcasecmp(m_pivot[i].name, name)) {
                return i;
            }
        }
    }

    return 0;
}

char *HTreeClass::Get_Bone_Name(int boneidx)
{
    return m_pivot[boneidx].name;
}

int HTreeClass::Get_Parent_Index(int boneidx) const
{
    if (m_pivot[boneidx].parent) {
        return m_pivot[boneidx].parent->index;
    } else {
        return 0;
    }
}

void HTreeClass::Scale(float factor)
{
    if (factor != 1.0f) {
        int count = 0;

        if (m_numPivots > 0) {
            do {
                m_pivot[count].base_transform.Set_Translation(m_pivot[count].base_transform.Get_Translation() * factor);
                count++;
            } while (count < m_numPivots);
        }

        m_scaleFactor = factor * m_scaleFactor;
    }
}

void HTreeClass::Capture_Bone(int boneindex)
{
    m_pivot[boneindex].is_captured = true;
}

void HTreeClass::Release_Bone(int boneindex)
{
    m_pivot[boneindex].is_captured = false;
}

bool HTreeClass::Is_Bone_Captured(int boneindex)
{
    return m_pivot[boneindex].is_captured;
}

void HTreeClass::Control_Bone(int boneindex, Matrix3D const &relative_tm, bool world_space_translation)
{
    m_pivot[boneindex].world_space_translation = world_space_translation;
    m_pivot[boneindex].cap_transform = relative_tm;
}

void HTreeClass::Get_Bone_Control(int boneindex, Matrix3D &relative_tm)
{
    if (m_pivot[boneindex].is_captured) {
        relative_tm = m_pivot[boneindex].cap_transform;
    } else {
        relative_tm.Make_Identity();
    }
}
