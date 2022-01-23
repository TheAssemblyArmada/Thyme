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
#pragma once

#include "always.h"
#include "matrix3d.h"
#include "pivot.h"
#include "w3dmpo.h"

class ChunkLoadClass;
class HAnimClass;
class HAnimComboClass;
class HRawAnimClass;

class HTreeClass : public W3DMPO
{
    IMPLEMENT_W3D_POOL(HTreeClass)
private:
    char m_name[16];
    int m_numPivots;
    PivotClass *m_pivot;
    float m_scaleFactor;

public:
    virtual ~HTreeClass();

    HTreeClass();
    HTreeClass(HTreeClass const &src);

    void Init_Default();
    void Free();

    int Load_W3D(ChunkLoadClass &cload);
    bool Read_Pivots(ChunkLoadClass &cload, bool pre30);

    bool Simple_Evaluate_Pivot(HAnimClass *motion, int pivot_index, float frame, Matrix3D const &obj_tm, Matrix3D *end_tm);
    bool Simple_Evaluate_Pivot(int pivot_index, Matrix3D const &obj_tm, Matrix3D *end_tm);

    void Base_Update(Matrix3D const &root);
    void Anim_Update(Matrix3D const &root, HAnimClass *motion, float frame);
    void Anim_Update(Matrix3D const &root, HRawAnimClass *motion, float frame);
    void Blend_Update(
        Matrix3D const &root, HAnimClass *motion0, float frame0, HAnimClass *motion1, float frame1, float percentage);

    int Get_Bone_Index(char const *name);
    char *Get_Bone_Name(int boneidx);
    int Get_Parent_Index(int boneidx) const;

    void Scale(float factor);

    void Capture_Bone(int boneindex);
    void Release_Bone(int boneindex);
    bool Is_Bone_Captured(int boneindex);

    void Control_Bone(int boneindex, Matrix3D const &relative_tm, bool world_space_translation);
    void Get_Bone_Control(int boneindex, Matrix3D &relative_tm);

    int Num_Pivots() const { return m_numPivots; }
    const Matrix3D &Get_Transform(int pivot) const
    {
        captainslog_assert(pivot >= 0);
        captainslog_assert(pivot < m_numPivots);
        return m_pivot[pivot].transform;
    }
    bool Get_Visibility(int pivot)
    {
        captainslog_assert(pivot >= 0);
        captainslog_assert(pivot < m_numPivots);
        return m_pivot[pivot].is_visible;
    }
    const Matrix3D &Get_Root_Transform() { return m_pivot[0].transform; }
    const char *Get_Name() const { return m_name; }

    HTreeClass *Hook_Ctor() { return new (this) HTreeClass; }
    HTreeClass *Hook_Ctor2(const HTreeClass &src) { return new (this) HTreeClass(src); }
};
