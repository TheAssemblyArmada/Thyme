/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Raw Animation Class
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

struct NodeMotionStruct
{
    MotionChannelClass *X;
    MotionChannelClass *Y;
    MotionChannelClass *Z;
    MotionChannelClass *XR;
    MotionChannelClass *YR;
    MotionChannelClass *ZR;
    MotionChannelClass *Q;
    BitChannelClass *Vis;

    NodeMotionStruct() : X(nullptr), Y(nullptr), Z(nullptr), XR(nullptr), YR(nullptr), ZR(nullptr), Q(nullptr), Vis(nullptr)
    {
    }

    ~NodeMotionStruct()
    {
        delete X;
        delete Y;
        delete Z;
        delete XR;
        delete YR;
        delete ZR;
        delete Q;
        delete Vis;
    }
};

class HRawAnimClass : public HAnimClass
{
public:
    ~HRawAnimClass() override;
    const char *Get_Name() const override { return m_name; }
    const char *Get_HName() const override { return m_hierarchyName; }
    int Get_Num_Frames() override { return m_numFrames; }
    float Get_Frame_Rate() override { return m_frameRate; }
    float Get_Total_Time() override { return (float)m_numFrames / m_frameRate; }
    void Get_Translation(Vector3 &trans, int pividx, float frame) const override;
    void Get_Orientation(Quaternion &q, int pividx, float frame) const override;
    void Get_Transform(Matrix3D &mtx, int pividx, float frame) const override;
    bool Get_Visibility(int pividx, float frame) override;
    int Get_Num_Pivots() const override { return m_numNodes; }
    bool Is_Node_Motion_Present(int pividx) override;
    bool Has_X_Translation(int pividx) override;
    bool Has_Y_Translation(int pividx) override;
    bool Has_Z_Translation(int pividx) override;
    bool Has_Rotation(int pividx) override;
    bool Has_Visibility(int pividx) override;
    int Class_ID() const override { return CLASSID_RAW; }
    HRawAnimClass();
    W3DErrorType Load_W3D(ChunkLoadClass &cload);
    void Free();
    bool Read_Channel(ChunkLoadClass &cload, MotionChannelClass **newchan, bool pre30);
    void Add_Channel(MotionChannelClass *newchan);
    bool Read_Bit_Channel(ChunkLoadClass &cload, BitChannelClass **newchan, bool pre30);
    void Add_Bit_Channel(BitChannelClass *newchan);

    NodeMotionStruct *Get_Node_Motion() { return m_nodeMotion; }

private:
    char m_name[32];
    char m_hierarchyName[16];
    int m_numFrames;
    int m_numNodes;
    float m_frameRate;
    NodeMotionStruct *m_nodeMotion;
};
