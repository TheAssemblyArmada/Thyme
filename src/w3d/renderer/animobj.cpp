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
#include "animobj.h"
#include "assetmgr.h"
#include "hrawanim.h"
#include "w3d.h"

Animatable3DObjClass::Animatable3DObjClass(const char *tree) : m_isTreeValid(false), m_curMotionMode(BASE_POSE)
{
    m_modeInterp.m_motion1 = 0;
    m_modeInterp.m_frame0 = 0;
    m_modeInterp.m_prevFrame0 = 0;
    m_modeInterp.m_prevFrame1 = 0;
    m_modeInterp.m_frame1 = 0;
    m_modeInterp.m_percentage = 0;
    m_modeInterp.m_motion0 = 0;

    if (!tree) {
        m_htree = nullptr;
        return;
    }

    HTreeClass *tr;

    if (*tree) {
        tr = W3DAssetManager::Get_Instance()->Get_HTree(tree);

        if (tr) {
            m_htree = new HTreeClass(*tr);
            return;
        }
    }

    m_htree = new HTreeClass();
    m_htree->Init_Default();
}

Animatable3DObjClass::Animatable3DObjClass(const Animatable3DObjClass &src) :
    CompositeRenderObjClass(src), m_isTreeValid(false), m_htree(0), m_curMotionMode(BASE_POSE)
{
    m_modeInterp.m_motion1 = 0;
    m_modeInterp.m_frame0 = 0;
    m_modeInterp.m_prevFrame0 = 0;
    m_modeInterp.m_prevFrame1 = 0;
    m_modeInterp.m_frame1 = 0;
    m_modeInterp.m_percentage = 0;
    m_modeInterp.m_motion0 = 0;

    *this = src;
}

Animatable3DObjClass::~Animatable3DObjClass()
{
    Release();

    if (m_htree) {
        delete m_htree;
    }
}

Animatable3DObjClass &Animatable3DObjClass::operator=(const Animatable3DObjClass &that)
{
    if (this != &that) {

        Release();

        if (m_htree) {
            delete m_htree;
        }

        CompositeRenderObjClass::operator=(that);

        m_isTreeValid = false;
        m_curMotionMode = BASE_POSE;
        m_modeInterp.m_motion1 = 0;
        m_modeInterp.m_frame0 = 0;
        m_modeInterp.m_prevFrame0 = 0;
        m_modeInterp.m_prevFrame1 = 0;
        m_modeInterp.m_frame1 = 0;
        m_modeInterp.m_percentage = 0;
        m_modeInterp.m_motion0 = 0;

        m_htree = new HTreeClass(*that.m_htree);
    }

    return *this;
}

void Animatable3DObjClass::Release()
{
    if (m_curMotionMode == SINGLE_ANIM) {
        m_modeAnim.m_motion->Release_Ref();
        m_modeAnim.m_motion = nullptr;
    } else if (m_curMotionMode == DOUBLE_ANIM) {
        m_modeInterp.m_motion0->Release_Ref();
        m_modeInterp.m_motion0 = nullptr;
        m_modeInterp.m_motion1->Release_Ref();
        m_modeInterp.m_motion1 = nullptr;
    }
}

void Animatable3DObjClass::Render(RenderInfoClass &rinfo)
{
    if (m_htree && Is_Not_Hidden_At_All()) {
        if (m_curMotionMode == SINGLE_ANIM && m_modeAnim.m_animMode != ANIM_MODE_MANUAL) {
            Single_Anim_Progress();
        }

        if (!m_isTreeValid || Are_Sub_Object_Transforms_Dirty()) {
            Update_Sub_Object_Transforms();
        }
    }
}

void Animatable3DObjClass::Special_Render(SpecialRenderInfoClass &rinfo)
{
    if (m_htree) {
        if (m_curMotionMode == SINGLE_ANIM && m_modeAnim.m_animMode != ANIM_MODE_MANUAL) {
            Single_Anim_Progress();
        }

        if (!m_isTreeValid) {
            Update_Sub_Object_Transforms();
        }
    }
}

void Animatable3DObjClass::Set_Transform(const Matrix3D &m)
{
    RenderObjClass::Set_Transform(m);
    m_isTreeValid = false;
}

void Animatable3DObjClass::Set_Position(const Vector3 &v)
{
    RenderObjClass::Set_Position(v);
    m_isTreeValid = false;
}

int Animatable3DObjClass::Get_Num_Bones()
{
    if (m_htree) {
        return m_htree->Num_Pivots();
    } else {
        return 1;
    }
}

const char *Animatable3DObjClass::Get_Bone_Name(int bone_index)
{
    if (m_htree) {
        return m_htree->Get_Bone_Name(bone_index);
    }

    return "RootTransform";
}

int Animatable3DObjClass::Get_Bone_Index(const char *bonename)
{
    if (m_htree) {
        return m_htree->Get_Bone_Index(bonename);
    }

    return 0;
}

void Animatable3DObjClass::Set_Animation()
{
    Release();
    m_curMotionMode = BASE_POSE;
    m_isTreeValid = false;
}

void Animatable3DObjClass::Set_Animation(HAnimClass *motion, float frame, int anim_mode)
{
    if (motion) {
        motion->Add_Ref();
        Release();
        m_curMotionMode = SINGLE_ANIM;
        m_modeAnim.m_motion = motion;
        m_modeAnim.m_prevFrame = m_modeAnim.m_frame;
        m_modeAnim.m_frame = frame;
        m_modeAnim.m_lastSyncTime = W3D::Get_Sync_Time();
        m_modeAnim.m_frameRateMultiplier = 1.0f;
        m_modeAnim.m_animDirection = 1.0f;
        m_modeAnim.m_animMode = anim_mode;

        if (anim_mode >= ANIM_MODE_LOOP_BACKWARDS) {
            m_modeAnim.m_animDirection = -1.0f;
        } else {
            m_modeAnim.m_animDirection = 1.0f;
        }

    } else {
        m_curMotionMode = BASE_POSE;
        Release();
    }

    m_isTreeValid = false;
}

void Animatable3DObjClass::Set_Animation(
    HAnimClass *motion0, float frame0, HAnimClass *motion1, float frame1, float percentage)
{
    Release();
    m_curMotionMode = DOUBLE_ANIM;
    m_modeInterp.m_motion0 = motion0;
    m_modeInterp.m_motion1 = motion1;
    m_modeInterp.m_prevFrame0 = m_modeInterp.m_frame0;
    m_modeInterp.m_prevFrame1 = m_modeInterp.m_frame1;
    m_modeInterp.m_frame0 = frame0;
    m_modeInterp.m_frame1 = frame1;
    m_modeInterp.m_percentage = percentage;
    m_isTreeValid = false;

    if (motion0) {
        motion0->Add_Ref();
    }

    if (motion1) {
        motion1->Add_Ref();
    }
}

void Animatable3DObjClass::Set_Animation(HAnimComboClass *anim_combo)
{
    captainslog_dbgassert(0, "HanimComboClass not supported");
}

HAnimClass *Animatable3DObjClass::Peek_Animation()
{
    if (m_curMotionMode == SINGLE_ANIM) {
        return m_modeAnim.m_motion;
    }

    return nullptr;
}

const Matrix3D &Animatable3DObjClass::Get_Bone_Transform(const char *bonename)
{
    if (m_htree) {
        captainslog_assert(bonename);
        return Get_Bone_Transform(m_htree->Get_Bone_Index(bonename));
    } else {
        return Get_Transform();
    }
}

const Matrix3D &Animatable3DObjClass::Get_Bone_Transform(int boneindex)
{
    Validate_Transform();

    if (!m_htree) {
        return m_transform;
    }

    if (!m_isTreeValid) {
        Update_Sub_Object_Transforms();
    }

    return m_htree->Get_Transform(boneindex);
}

void Animatable3DObjClass::Capture_Bone(int boneindex)
{
    if (m_htree) {
        m_htree->Capture_Bone(boneindex);
    }
}

void Animatable3DObjClass::Release_Bone(int boneindex)
{
    if (m_htree) {
        m_htree->Release_Bone(boneindex);
    }
}

bool Animatable3DObjClass::Is_Bone_Captured(int boneindex) const
{
    if (m_htree) {
        return m_htree->Is_Bone_Captured(boneindex);
    }

    return false;
}

void Animatable3DObjClass::Control_Bone(int bindex, const Matrix3D &objtm, bool world_space_translation)
{
    if (m_htree) {
        m_htree->Control_Bone(bindex, objtm, world_space_translation);
        m_isTreeValid = false;
    }
}

void Animatable3DObjClass::Update_Sub_Object_Transforms()
{
    RenderObjClass::Update_Sub_Object_Transforms();

    switch (m_curMotionMode) {
        case BASE_POSE:
            Base_Update(m_transform);
            m_isTreeValid = true;
            break;
        case SINGLE_ANIM:
            if (m_modeAnim.m_animMode != ANIM_MODE_MANUAL) {
                Single_Anim_Progress();
            }

            Anim_Update(m_transform, m_modeAnim.m_motion, m_modeAnim.m_frame);
            break;
        case DOUBLE_ANIM:
            Blend_Update(m_transform,
                m_modeInterp.m_motion0,
                m_modeInterp.m_frame0,
                m_modeInterp.m_motion1,
                m_modeInterp.m_frame1,
                m_modeInterp.m_percentage);
            break;
        case MULTIPLE_ANIM:
            captainslog_dbgassert(0, "HanimComboClass not supported");
            break;
    }

    m_isTreeValid = true;
}

bool Animatable3DObjClass::Simple_Evaluate_Bone(int boneindex, Matrix3D *tm) const
{
    if (m_curMotionMode != NONE && m_curMotionMode != BASE_POSE && m_curMotionMode != SINGLE_ANIM) {
        ((Animatable3DObjClass *)this)->Update_Sub_Object_Transforms();
        *tm = m_htree->Get_Transform(boneindex);
        return false;
    }

    return Simple_Evaluate_Bone(boneindex, Compute_Current_Frame(nullptr), tm);
}

bool Animatable3DObjClass::Simple_Evaluate_Bone(int boneindex, float frame, Matrix3D *tm) const
{
    if (m_htree) {
        if (m_curMotionMode != SINGLE_ANIM) {
            if (m_curMotionMode == NONE || m_curMotionMode == BASE_POSE) {
                return m_htree->Simple_Evaluate_Pivot(boneindex, Get_Transform(), tm);
            }
            *tm = Get_Transform();
            return false;
        }
        return m_htree->Simple_Evaluate_Pivot(m_modeAnim.m_motion, boneindex, frame, Get_Transform(), tm);
    }
    *tm = Get_Transform();
    return false;
}

float Animatable3DObjClass::Compute_Current_Frame(float *new_direction) const
{
    float direction = m_modeAnim.m_animDirection;
    float frame = 0;
    if (m_curMotionMode == SINGLE_ANIM) {
        frame = m_modeAnim.m_frame;

        if (m_modeAnim.m_animMode != ANIM_MODE_MANUAL) {
            int time = W3D::Get_Sync_Time() - m_modeAnim.m_lastSyncTime;
            frame = m_modeAnim.m_motion->Get_Frame_Rate() * m_modeAnim.m_frameRateMultiplier * time
                    * m_modeAnim.m_animDirection * 0.001f
                + frame;

            switch (m_modeAnim.m_animMode) {
                case ANIM_MODE_LOOP:
                    if ((float)(m_modeAnim.m_motion->Get_Num_Frames() - 1) <= frame) {
                        frame = frame - (float)(m_modeAnim.m_motion->Get_Num_Frames() - 1);
                    }

                    if ((float)(m_modeAnim.m_motion->Get_Num_Frames() - 1) <= frame) {
                        frame = 0.0f;
                    }
                    break;
                case ANIM_MODE_ONCE:
                    if ((float)(m_modeAnim.m_motion->Get_Num_Frames() - 1) <= frame) {
                        frame = (float)(m_modeAnim.m_motion->Get_Num_Frames() - 1);
                    }
                case ANIM_MODE_LOOP_PINGPONG:
                    if (m_modeAnim.m_animDirection < 1.0f) {
                        if (frame > 0.0f) {
                            break;
                        }

                        frame = -frame;

                        if ((float)(m_modeAnim.m_motion->Get_Num_Frames() - 1) <= frame) {
                            frame = 0.0f;
                        }
                    } else {
                        if ((float)(m_modeAnim.m_motion->Get_Num_Frames() - 1) > frame) {
                            break;
                        }

                        frame = (float)(2 * m_modeAnim.m_motion->Get_Num_Frames() - 2) - frame;

                        if ((float)(m_modeAnim.m_motion->Get_Num_Frames() - 1) <= frame) {
                            frame = (float)(m_modeAnim.m_motion->Get_Num_Frames() - 1);
                        }
                    }

                    direction = m_modeAnim.m_animDirection * -1.0f;
                    break;
                case ANIM_MODE_LOOP_BACKWARDS:
                    if (frame < 0.0f) {
                        frame = (float)(m_modeAnim.m_motion->Get_Num_Frames() - 1) + frame;

                        if (frame < 0.0f) {
                            frame = (float)(m_modeAnim.m_motion->Get_Num_Frames() - 1);
                        }
                    }
                    break;
                case ANIM_MODE_ONCE_BACKWARDS:
                    if (frame < 0.0f) {
                        frame = 0.0f;
                    }
                    break;
            }
        }
    }

    if (new_direction) {
        *new_direction = direction;
    }

    return frame;
}

void Animatable3DObjClass::Single_Anim_Progress()
{
    if (m_curMotionMode == SINGLE_ANIM) {
        int frame = m_modeAnim.m_prevFrame;
        m_modeAnim.m_prevFrame = m_modeAnim.m_frame;
        m_modeAnim.m_frame = Compute_Current_Frame(&m_modeAnim.m_animDirection);

        if (m_modeAnim.m_frame == m_modeAnim.m_prevFrame) {
            m_modeAnim.m_prevFrame = frame;
        }

        m_modeAnim.m_lastSyncTime = W3D::Get_Sync_Time();
        m_isTreeValid = false;
    }
}

bool Animatable3DObjClass::Is_Animation_Complete() const
{
    if (m_curMotionMode != SINGLE_ANIM) {
        return false;
    } else {
        if (m_modeAnim.m_animMode == ANIM_MODE_ONCE) {
            return (m_modeAnim.m_motion->Get_Num_Frames() - 1) == m_modeAnim.m_frame;
        }

        if (m_modeAnim.m_animMode == ANIM_MODE_ONCE_BACKWARDS) {
            return m_modeAnim.m_frame == 0.0f;
        }
    }

    return false;
}

void Animatable3DObjClass::Set_HTree(HTreeClass *new_htree)
{
    captainslog_assert(new_htree->Num_Pivots() == m_htree->Num_Pivots());

    if (m_htree) {
        delete m_htree;
    }

    m_htree = new HTreeClass(*new_htree);
}

HAnimClass *Animatable3DObjClass::Peek_Animation_And_Info(float &frame, int &frames, int &mode, float &multiplier)
{
    if (m_curMotionMode != SINGLE_ANIM) {
        return nullptr;
    }

    frame = m_modeAnim.m_frame;

    if (m_modeAnim.m_motion) {
        frames = m_modeAnim.m_motion->Get_Num_Frames();
    } else {
        frames = 0;
    }

    mode = m_modeAnim.m_animMode;
    multiplier = m_modeAnim.m_frameRateMultiplier;
    return m_modeAnim.m_motion;
}

void Animatable3DObjClass::Anim_Update(const Matrix3D &root, HAnimClass *motion, float frame)
{
    if (motion && m_htree) {
        if (motion->Class_ID()) {
            m_htree->Anim_Update(root, motion, frame);
        } else {
            m_htree->Anim_Update(root, (HRawAnimClass *)motion, frame);
        }
    }
    Set_Hierarchy_Valid(true);
}
