/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Animatable 3D Render Object
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
#include "composite.h"
#include "htree.h"

class Animatable3DObjClass : public CompositeRenderObjClass
{
public:
    Animatable3DObjClass(const char *htree_name);
    Animatable3DObjClass(const Animatable3DObjClass &src);
    Animatable3DObjClass &operator=(const Animatable3DObjClass &);

    virtual ~Animatable3DObjClass() override;
    virtual void Render(RenderInfoClass &rinfo) override;
    virtual void Special_Render(SpecialRenderInfoClass &rinfo) override;

    virtual void Set_Transform(const Matrix3D &m) override;
    virtual void Set_Position(const Vector3 &v) override;

    virtual void Set_Animation() override;
    virtual void Set_Animation(HAnimClass *motion, float frame, int anim_mode = ANIM_MODE_MANUAL) override;
    virtual void Set_Animation(
        HAnimClass *motion0, float frame0, HAnimClass *motion1, float frame1, float percentage) override;
    virtual void Set_Animation(HAnimComboClass *anim_combo) override;
    virtual HAnimClass *Peek_Animation() override;

    virtual int Get_Num_Bones() override;
    virtual const char *Get_Bone_Name(int bone_index) override;
    virtual int Get_Bone_Index(const char *bonename) override;
    virtual const Matrix3D &Get_Bone_Transform(const char *bonename) override;
    virtual const Matrix3D &Get_Bone_Transform(int boneindex) override;
    virtual void Capture_Bone(int boneindex) override;
    virtual void Release_Bone(int boneindex) override;
    virtual bool Is_Bone_Captured(int boneindex) const override;
    virtual void Control_Bone(int bindex, const Matrix3D &objtm, bool world_space_translation = false) override;

    virtual const HTreeClass *Get_HTree() const override { return m_htree; }
    virtual void Set_Animation_Frame_Rate_Multiplier(float multiplier) { m_modeAnim.m_frameRateMultiplier = multiplier; }
    virtual HAnimClass *Peek_Animation_And_Info(float &frame, int &frames, int &mode, float &multiplier);
    virtual bool Is_Animation_Complete() const;
    virtual bool Simple_Evaluate_Bone(int boneindex, Matrix3D *tm) const;
    virtual bool Simple_Evaluate_Bone(int boneindex, float frame, Matrix3D *tm) const;
    virtual void Set_HTree(HTreeClass *new_htree);

protected:
    float Compute_Current_Frame(float *newDirection) const;
    void Update_Sub_Object_Transforms() override;

    void Base_Update(const Matrix3D &root);
    void Anim_Update(const Matrix3D &root, HAnimClass *motion, float frame);
    void Blend_Update(
        const Matrix3D &root, HAnimClass *motion0, float frame0, HAnimClass *motion1, float frame1, float percentage);
    void Combo_Update(const Matrix3D &root, HAnimComboClass *anim);

    bool Is_Hierarchy_Valid() const { return m_isTreeValid; }
    void Set_Hierarchy_Valid(bool onoff) { m_isTreeValid = onoff; }
    void Single_Anim_Progress();
    void Release();

    bool m_isTreeValid;
    HTreeClass *m_htree;

    enum
    {
        NONE = 0,
        BASE_POSE,
        SINGLE_ANIM,
        DOUBLE_ANIM,
        MULTIPLE_ANIM,
    };

    int m_curMotionMode;

    union
    {
        // m_curMotionMode == SINGLE_ANIM
        struct
        {
            HAnimClass *m_motion;
            float m_frame;
            float m_prevFrame;
            int m_animMode;
            int m_lastSyncTime;
            float m_animDirection;
            float m_frameRateMultiplier;
        } m_modeAnim;

        // m_curMotionMode == DOUBLE_ANIM
        struct
        {
            HAnimClass *m_motion0;
            HAnimClass *m_motion1;
            float m_frame0;
            float m_frame1;
            float m_prevFrame0;
            float m_prevFrame1;
            float m_percentage;
        } m_modeInterp;

        // m_curMotionMode == MULTIPLE_ANIM
        struct
        {
            HAnimComboClass *AnimCombo;
        } m_modeCombo;
    };
};

inline void Animatable3DObjClass::Base_Update(const Matrix3D &root)
{
    if (m_htree) {
        m_htree->Base_Update(root);
    }
    Set_Hierarchy_Valid(true);
}

inline void Animatable3DObjClass::Blend_Update(
    const Matrix3D &root, HAnimClass *motion0, float frame0, HAnimClass *motion1, float frame1, float percentage)
{
    if (m_htree) {
        m_htree->Blend_Update(root, motion0, frame0, motion1, frame1, percentage);
    }
    Set_Hierarchy_Valid(true);
}

inline void Animatable3DObjClass::Combo_Update(const Matrix3D &root, HAnimComboClass *anim)
{
    captainslog_dbgassert(0, "HanimComboClass not supported");
}
