/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Compressed Animation Class
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
#include "hanim.h"
#include "motchan.h"
#include "w3d_file.h"
#include "w3derr.h"

struct NodeCompressedMotionStruct
{
    int Flavor;
    union
    {
        struct
        {
            TimeCodedMotionChannelClass *X;
            TimeCodedMotionChannelClass *Y;
            TimeCodedMotionChannelClass *Z;
            TimeCodedMotionChannelClass *Q;
        } tc;
        struct
        {
            AdaptiveDeltaMotionChannelClass *X;
            AdaptiveDeltaMotionChannelClass *Y;
            AdaptiveDeltaMotionChannelClass *Z;
            AdaptiveDeltaMotionChannelClass *Q;
        } ad;
        struct
        {
            void *X;
            void *Y;
            void *Z;
            void *Q;
        } vd;
    };
    TimeCodedBitChannelClass *Vis;

    // #BUGFIX: Initialize all members
    NodeCompressedMotionStruct() : Flavor(0), Vis(nullptr)
    {
        vd.X = nullptr;
        vd.Y = nullptr;
        vd.Z = nullptr;
        vd.Q = nullptr;
    }
    ~NodeCompressedMotionStruct();
    void Set_Flavor(int flavor) { Flavor = flavor; }
};

class HCompressedAnimClass : public HAnimClass
{
public:
    virtual ~HCompressedAnimClass() override;
    virtual const char *Get_Name() const override { return m_name; }
    virtual const char *Get_HName() const override { return m_hierarchyName; }
    virtual int Get_Num_Frames() override { return m_numFrames; }
    virtual float Get_Frame_Rate() override { return m_frameRate; }
    virtual float Get_Total_Time() override { return (float)m_numFrames / m_frameRate; }
    virtual void Get_Translation(Vector3 &trans, int pividx, float frame) const override;
    virtual void Get_Orientation(Quaternion &q, int pividx, float frame) const override;
    virtual void Get_Transform(Matrix3D &mtx, int pividx, float frame) const override;
    virtual bool Get_Visibility(int pividx, float frame) override;
    virtual int Get_Num_Pivots() const override { return m_numNodes; }
    virtual bool Is_Node_Motion_Present(int pividx) override;
    virtual bool Has_X_Translation(int pividx) override;
    virtual bool Has_Y_Translation(int pividx) override;
    virtual bool Has_Z_Translation(int pividx) override;
    virtual bool Has_Rotation(int pividx) override;
    virtual bool Has_Visibility(int pividx) override;

    HCompressedAnimClass();
    W3DErrorType Load_W3D(ChunkLoadClass &cload);
    void Free();
    bool read_channel(ChunkLoadClass &cload, AdaptiveDeltaMotionChannelClass **newchan);
    bool read_channel(ChunkLoadClass &cload, TimeCodedMotionChannelClass **newchan);
    void add_channel(AdaptiveDeltaMotionChannelClass *newchan);
    void add_channel(TimeCodedMotionChannelClass *newchan);
    bool read_bit_channel(ChunkLoadClass &cload, TimeCodedBitChannelClass **newchan);
    void add_bit_channel(TimeCodedBitChannelClass *newchan);

    int Get_Flavor() { return m_flavor; }

private:
    char m_name[32];
    char m_hierarchyName[16];
    int m_numFrames;
    int m_numNodes;
    int m_flavor;
    float m_frameRate;
    NodeCompressedMotionStruct *m_nodeMotion;
};
