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
#include "hrawanim.h"
#include "assetmgr.h"
#include <cstring>

using std::memset;
using std::strcat;
using std::strcpy;

HRawAnimClass::HRawAnimClass() : m_numFrames(0), m_numNodes(0), m_frameRate(0), m_nodeMotion(nullptr)
{
    memset(m_name, 0, sizeof(m_name));
    memset(m_hierarchyName, 0, sizeof(m_hierarchyName));
}

HRawAnimClass::~HRawAnimClass()
{
    Free();
}

void HRawAnimClass::Free()
{
    delete[] m_nodeMotion;
}

W3DErrorType HRawAnimClass::Load_W3D(ChunkLoadClass &cload)
{
    bool pre30 = false;
    Free();

    if (!cload.Open_Chunk()) {
        return W3D_ERROR_GENERIC;
    }

    if (cload.Cur_Chunk_ID() != W3D_CHUNK_ANIMATION_HEADER) {
        return W3D_ERROR_GENERIC;
    }

    W3dAnimHeaderStruct header;

    if (cload.Read(&header, sizeof(header)) != sizeof(header)) {
        return W3D_ERROR_GENERIC;
    }

    cload.Close_Chunk();

    if (header.Version < 0x30000) {
        pre30 = true;
    }

    static_assert(sizeof(m_name) >= sizeof(header.HierarchyName));
    strlcpy_tpl(m_name, header.HierarchyName);
    strlcat_tpl(m_name, ".");
    strlcat_tpl(m_name, header.Name);

    static_assert(sizeof(m_hierarchyName) >= sizeof(header.HierarchyName));
    strlcpy_tpl(m_hierarchyName, header.HierarchyName);
    HTreeClass *tree = W3DAssetManager::Get_Instance()->Get_HTree(m_hierarchyName);

    if (!tree) {
        Free();
        return W3D_ERROR_GENERIC;
    }

    m_frameRate = header.FrameRate;
    m_numNodes = tree->Num_Pivots();
    m_numFrames = header.NumFrames;
    m_nodeMotion = new NodeMotionStruct[m_numNodes];

    while (cload.Open_Chunk()) {
        const auto chunk_id = cload.Cur_Chunk_ID();

        if (chunk_id == W3D_CHUNK_BIT_CHANNEL) {
            BitChannelClass *channel;

            if (!Read_Bit_Channel(cload, &channel, pre30)) {
                Free();
                return W3D_ERROR_GENERIC;
            }

            if (channel->Get_Pivot() >= m_numNodes) {
                captainslog_error("Animation %s referring to missing Bone! Please re-export.", m_name);
                delete channel;
            } else {
                Add_Bit_Channel(channel);
            }
        } else if (chunk_id == W3D_CHUNK_ANIMATION_CHANNEL) {

            MotionChannelClass *channel;

            if (!Read_Channel(cload, &channel, pre30)) {
                Free();
                return W3D_ERROR_GENERIC;
            }

            if (channel->Get_Pivot() >= m_numNodes) {
                captainslog_error("Animation %s referring to missing Bone! Please re-export.", m_name);
                delete channel;
            } else {
                Add_Channel(channel);
            }
        }

        cload.Close_Chunk();
    }

    return W3D_ERROR_OK;
}

bool HRawAnimClass::Read_Channel(ChunkLoadClass &cload, MotionChannelClass **newchan, bool pre30)
{
    auto *p = new MotionChannelClass();

    if (p->Load_W3D(cload)) {
        if (pre30) {
            p->m_pivotIdx++;
        }
        *newchan = p;
        return true;
    }
    // #BUGFIX Do not leak memory when loading failed.
    delete p;
    return false;
}

void HRawAnimClass::Add_Channel(MotionChannelClass *newchan)
{
    const int pivot = newchan->Get_Pivot();

    switch (newchan->m_type) {
        case ANIM_CHANNEL_X:
            m_nodeMotion[pivot].X = newchan;
            break;
        case ANIM_CHANNEL_Y:
            m_nodeMotion[pivot].Y = newchan;
            break;
        case ANIM_CHANNEL_Z:
            m_nodeMotion[pivot].Z = newchan;
            break;
        case ANIM_CHANNEL_XR:
            m_nodeMotion[pivot].XR = newchan;
            break;
        case ANIM_CHANNEL_YR:
            m_nodeMotion[pivot].YR = newchan;
            break;
        case ANIM_CHANNEL_ZR:
            m_nodeMotion[pivot].ZR = newchan;
            break;
        case ANIM_CHANNEL_Q:
            m_nodeMotion[pivot].Q = newchan;
            break;
    }
}

bool HRawAnimClass::Read_Bit_Channel(ChunkLoadClass &cload, BitChannelClass **newchan, bool pre30)
{
    auto *p = new BitChannelClass();

    if (p->Load_W3D(cload)) {
        if (pre30) {
            p->m_pivotIdx++;
        }
        *newchan = p;
        return true;
    }
    // #BUGFIX Do not leak memory when loading failed.
    delete p;
    return false;
}

void HRawAnimClass::Add_Bit_Channel(BitChannelClass *newchan)
{
    if (newchan->Get_Type() == BIT_CHANNEL_VIS) {
        m_nodeMotion[newchan->Get_Pivot()].Vis = newchan;
    }
}

void HRawAnimClass::Get_Translation(Vector3 &trans, int pividx, float frame) const
{
    auto motion = &m_nodeMotion[pividx];

    if (motion->X || motion->Y || motion->Z) {
        int frame0 = frame - 0.49999899f;
        int frame1 = frame0 + 1;
        float delta = frame - (float)frame0;

        if (frame1 >= m_numFrames) {
            frame1 = 0;
        }

        Vector3 t0;

        if (motion->X) {
            motion->X->Get_Vector(frame0, &t0.X);
        }

        if (motion->Y) {
            motion->Y->Get_Vector(frame0, &t0.Y);
        }

        if (motion->Z) {
            motion->Z->Get_Vector(frame0, &t0.Z);
        }

        if (delta == 0) {
            trans = t0;
        } else {
            Vector3 t1;

            if (motion->X) {
                motion->X->Get_Vector(frame1, &t1.X);
            }

            if (motion->Y) {
                motion->Y->Get_Vector(frame1, &t1.Y);
            }

            if (motion->Z) {
                motion->Z->Get_Vector(frame1, &t1.Z);
            }

            Vector3::Lerp(t0, t1, delta, &trans);
        }
    } else {
        trans.Set(0, 0, 0);
    }
}

void HRawAnimClass::Get_Orientation(Quaternion &q, int pividx, float frame) const
{
    int frame0 = frame - 0.49999899f;
    int frame1 = frame0 + 1;
    float delta = frame - (float)frame0;

    if (frame1 >= m_numFrames) {
        frame1 = 0;
    }

    Quaternion q0;
    Quaternion q1;

    if (m_nodeMotion->Q) {
        m_nodeMotion->Q->Get_Vector_As_Quat(frame0, q0);
        m_nodeMotion->Q->Get_Vector_As_Quat(frame1, q1);
    } else {
        q0.Set();
        q1.Set();
    }

    if (delta == 0) {
        q = q0;
    } else if (delta == 1) {
        q = q1;
    } else {
        Fast_Slerp(q, q0, q1, delta);
    }
}

void HRawAnimClass::Get_Transform(Matrix3D &mtx, int pividx, float frame) const
{
    int frame0 = frame - 0.49999899f;
    int frame1 = frame0 + 1;
    float delta = frame - (float)frame0;
    NodeMotionStruct *mot = &m_nodeMotion[pividx];

    if (frame1 >= m_numFrames) {
        frame1 = 0;
    }

    Quaternion q;
    q.Set();

    if (mot->Q) {
        mot->Q->Get_Vector_As_Quat(frame0, q);
    }

    if (delta == 0) {
        mtx = Build_Matrix3D(q);

        if (mot->X) {
            mot->X->Get_Vector(frame0, &mtx[0].W);
        }

        if (mot->Y) {
            mot->Y->Get_Vector(frame0, &mtx[1].W);
        }

        if (mot->Z) {
            mot->Z->Get_Vector(frame0, &mtx[2].W);
        }
    } else {
        Quaternion q1;

        if (mot->Q) {
            mot->Q->Get_Vector_As_Quat(frame1, q1);
        } else {
            // #BUGFIX Set to identity when there is no data.
            q1.Make_Identity();
        }

        Quaternion q2;
        Fast_Slerp(q2, q, q1, delta);
        mtx = Build_Matrix3D(q2);

        Vector3 v1(0, 0, 0);

        if (mot->X) {
            mot->X->Get_Vector(frame0, &v1.X);
        }

        if (mot->Y) {
            mot->Y->Get_Vector(frame0, &v1.Y);
        }

        if (mot->Z) {
            mot->Z->Get_Vector(frame0, &v1.Z);
        }

        Vector3 v2(0, 0, 0);

        if (mot->X) {
            mot->X->Get_Vector(frame1, &v2.X);
        }

        if (mot->Y) {
            mot->Y->Get_Vector(frame1, &v2.Y);
        }

        if (mot->Z) {
            mot->Z->Get_Vector(frame1, &v2.Z);
        }

        Vector3 v3;
        Vector3::Lerp(v1, v2, delta, &v3);
        mtx.Set_Translation(v3);
    }
}

bool HRawAnimClass::Get_Visibility(int pividx, float frame)
{
    if (m_nodeMotion[pividx].Vis == nullptr) {
        return true;
    }

    return m_nodeMotion[pividx].Vis->Get_Bit(frame) == 1;
}

bool HRawAnimClass::Is_Node_Motion_Present(int pividx)
{
    NodeMotionStruct *mot = &m_nodeMotion[pividx];

    if (mot->X) {
        return true;
    }

    if (mot->Y) {
        return true;
    }

    if (mot->Z) {
        return true;
    }

    if (mot->XR) {
        return true;
    }

    if (mot->YR) {
        return true;
    }

    if (mot->ZR) {
        return true;
    }

    if (mot->Q) {
        return true;
    }

    return mot->Vis != nullptr;
}

bool HRawAnimClass::Has_X_Translation(int pividx)
{
    return m_nodeMotion[pividx].X != nullptr;
}

bool HRawAnimClass::Has_Y_Translation(int pividx)
{
    return m_nodeMotion[pividx].Y != nullptr;
}

bool HRawAnimClass::Has_Z_Translation(int pividx)
{
    return m_nodeMotion[pividx].Z != nullptr;
}

bool HRawAnimClass::Has_Rotation(int pividx)
{
    return m_nodeMotion[pividx].Q != nullptr;
}

bool HRawAnimClass::Has_Visibility(int pividx)
{
    return m_nodeMotion[pividx].Vis != nullptr;
}
