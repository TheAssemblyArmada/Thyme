#pragma once

class HAnimClass : class RefCountClass, class HashableClass
{
public:
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
    virtual int Class_ID() { return -1 /*CLASSID_UNKNOWN*/; }
    virtual bool Has_Embedded_Sounds() const { return (Bitfield >> 0x1F) ^ 1; }
    virtual void Set_Embedded_Sound_Bone_Index(int index) { Bitfield = index; };
    virtual int Get_Embedded_Sound_Bone_Index() { return Bitfield; }

protected:
    unsigned int Bitfield;
};
