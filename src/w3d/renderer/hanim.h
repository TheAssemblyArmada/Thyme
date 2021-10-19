/**
 * @file
 *
 * @author OmniBlade
 * @author tomsons26
 *
 * @brief Hashed animation base class.
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
#include "hash.h"
#include "refcount.h"

class Vector3;
class Quaternion;
class Matrix3D;

class HAnimClass : public RefCountClass, public HashableClass
{
public:
    enum
    {
        CLASSID_UNKNOWN = 0xFFFFFFFF,
        CLASSID_RAW = 0,
    };

    HAnimClass() : m_embeddedSoundBoneIndex(-1) {}
    virtual ~HAnimClass(){};
    virtual const char *Get_Name() const = 0;
    virtual const char *Get_HName() const = 0;
    virtual const char *Get_Key() { return Get_Name(); }
    virtual int Get_Num_Frames() = 0;
    virtual float Get_Frame_Rate() = 0;
    virtual float Get_Total_Time() = 0;
    virtual void Get_Translation(int pividx, float frame){};
    virtual void Get_Orientation(int pividx, float frame){};
    virtual void Get_Translation(Vector3 &trans, int pividx, float frame) const = 0;
    virtual void Get_Orientation(Quaternion &q, int pividx, float frame) const = 0;
    virtual void Get_Transform(Matrix3D &mtx, int pividx, float frame) const = 0;
    virtual bool Get_Visibility(int pividx, float frame) = 0;
    virtual int Get_Num_Pivots() const = 0;
    virtual bool Is_Node_Motion_Present(int pividx) = 0;
    virtual bool Has_X_Translation(int pividx) { return true; }
    virtual bool Has_Y_Translation(int pividx) { return true; }
    virtual bool Has_Z_Translation(int pividx) { return true; }
    virtual bool Has_Rotation(int pividx) { return true; }
    virtual bool Has_Visibility(int pividx) { return true; }
    virtual int Class_ID() const { return CLASSID_UNKNOWN; }
    virtual bool Has_Embedded_Sounds() const { return m_embeddedSoundBoneIndex >= 0; }
    virtual void Set_Embedded_Sound_Bone_Index(int index) { m_embeddedSoundBoneIndex = index; };
    virtual int Get_Embedded_Sound_Bone_Index() { return m_embeddedSoundBoneIndex; }

protected:
    int m_embeddedSoundBoneIndex;
};
