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
#include "hcanim.h"
#include "assetmgr.h"
#include <cstring>

using std::memset;
using std::strcat;
using std::strcpy;

NodeCompressedMotionStruct::~NodeCompressedMotionStruct()
{
    if (Flavor == ANIM_FLAVOR_TIMECODED) {
        delete tc.X;
        delete tc.Y;
        delete tc.Z;
        delete tc.Q;
    } else if (Flavor == ANIM_FLAVOR_ADAPTIVE_DELTA) {
        delete ad.X;
        delete ad.Y;
        delete ad.Z;
        delete ad.Q;
    } else {
        captainslog_assert(0);
    }
    delete Vis;
}

HCompressedAnimClass::HCompressedAnimClass() :
    m_numFrames(0), m_numNodes(0), m_flavor(ANIM_FLAVOR_TIMECODED), m_frameRate(0), m_nodeMotion(nullptr)
{
    memset(m_name, 0, sizeof(m_name));
    memset(m_hierarchyName, 0, sizeof(m_hierarchyName));
}

HCompressedAnimClass::~HCompressedAnimClass()
{
    Free();
}

void HCompressedAnimClass::Free()
{
    delete[] m_nodeMotion;
}

W3DErrorType HCompressedAnimClass::Load_W3D(ChunkLoadClass &cload)
{
    Free();

    if (!cload.Open_Chunk()) {
        return W3D_ERROR_GENERIC;
    }

    if (cload.Cur_Chunk_ID() != W3D_CHUNK_COMPRESSED_ANIMATION_HEADER) {
        return W3D_ERROR_GENERIC;
    }

    W3dCompressedAnimHeaderStruct header;

    if (cload.Read(&header, sizeof(header)) != sizeof(header)) {
        return W3D_ERROR_GENERIC;
    }

    cload.Close_Chunk();

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

    m_numNodes = tree->Num_Pivots();
    m_numFrames = header.NumFrames;
    m_flavor = header.Flavor;
    m_frameRate = header.FrameRate;

    captainslog_assert((m_flavor == ANIM_FLAVOR_TIMECODED) || (m_flavor == ANIM_FLAVOR_ADAPTIVE_DELTA));

    m_nodeMotion = new NodeCompressedMotionStruct[m_numNodes];

    if (!m_nodeMotion) {
        Free();
        return W3D_ERROR_GENERIC;
    }

    for (int i = 0; i < m_numNodes; i++) {
        m_nodeMotion[i].Set_Flavor(m_flavor);
    }

    while (cload.Open_Chunk()) {
        const auto chunk_id = cload.Cur_Chunk_ID();

        if (chunk_id == W3D_CHUNK_COMPRESSED_BIT_CHANNEL) {
            TimeCodedBitChannelClass *channel;

            if (!read_bit_channel(cload, &channel)) {
                Free();
                return W3D_ERROR_GENERIC;
            }

            if (channel->Get_Pivot() >= m_numNodes) {
                delete channel;
                captainslog_error("ERROR! animation %s indexes a bone not present in the model. Please re-export!", m_name);
            } else {
                add_bit_channel(channel);
            }
        } else if (chunk_id == W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL) {

            if (m_flavor == ANIM_FLAVOR_TIMECODED) {
                TimeCodedMotionChannelClass *channel;

                if (!read_channel(cload, &channel)) {
                    Free();
                    return W3D_ERROR_GENERIC;
                }

                if (channel->Get_Pivot() >= m_numNodes) {
                    delete channel;
                    captainslog_error(
                        "ERROR! animation %s indexes a bone not present in the model. Please re-export!", m_name);
                } else {
                    add_channel(channel);
                }
            } else if (m_flavor == ANIM_FLAVOR_ADAPTIVE_DELTA) {
                AdaptiveDeltaMotionChannelClass *channel;

                if (!read_channel(cload, &channel)) {
                    Free();
                    return W3D_ERROR_GENERIC;
                }

                if (channel->Get_Pivot() >= m_numNodes) {
                    delete channel;
                    captainslog_error(
                        "ERROR! animation %s indexes a bone not present in the model. Please re-export!", m_name);
                } else {
                    add_channel(channel);
                }
            }
        }

        cload.Close_Chunk();
    }

    return W3D_ERROR_OK;
}

bool HCompressedAnimClass::read_channel(ChunkLoadClass &cload, TimeCodedMotionChannelClass **newchan)
{
    auto *p = new TimeCodedMotionChannelClass();
    if (p->Load_W3D(cload)) {
        *newchan = p;
        return true;
    }
    // #BUGFIX Do not leak memory when loading failed.
    delete p;
    return false;
}

bool HCompressedAnimClass::read_channel(ChunkLoadClass &cload, AdaptiveDeltaMotionChannelClass **newchan)
{
    auto *p = new AdaptiveDeltaMotionChannelClass();
    if (p->Load_W3D(cload)) {
        *newchan = p;
        return true;
    }
    // #BUGFIX Do not leak memory when loading failed.
    delete p;
    return false;
}

void HCompressedAnimClass::add_channel(TimeCodedMotionChannelClass *newchan)
{
    switch (newchan->Get_Type()) {
        case ANIM_CHANNEL_X:
            m_nodeMotion[newchan->Get_Pivot()].tc.X = newchan;
            break;
        case ANIM_CHANNEL_Y:
            m_nodeMotion[newchan->Get_Pivot()].tc.Y = newchan;
            break;
        case ANIM_CHANNEL_Z:
            m_nodeMotion[newchan->Get_Pivot()].tc.Z = newchan;
            break;
        case ANIM_CHANNEL_Q:
            m_nodeMotion[newchan->Get_Pivot()].tc.Q = newchan;
            break;
    }
}

void HCompressedAnimClass::add_channel(AdaptiveDeltaMotionChannelClass *newchan)
{
    switch (newchan->Get_Type()) {
        case ANIM_CHANNEL_X:
            m_nodeMotion[newchan->Get_Pivot()].ad.X = newchan;
            break;
        case ANIM_CHANNEL_Y:
            m_nodeMotion[newchan->Get_Pivot()].ad.Y = newchan;
            break;
        case ANIM_CHANNEL_Z:
            m_nodeMotion[newchan->Get_Pivot()].ad.Z = newchan;
            break;
        case ANIM_CHANNEL_Q:
            m_nodeMotion[newchan->Get_Pivot()].ad.Q = newchan;
            break;
    }
}

bool HCompressedAnimClass::read_bit_channel(ChunkLoadClass &cload, TimeCodedBitChannelClass **newchan)
{
    auto *p = new TimeCodedBitChannelClass();
    if (p->Load_W3D(cload)) {
        *newchan = p;
        return true;
    }
    // #BUGFIX Do not leak memory when loading failed.
    delete p;
    return false;
}

void HCompressedAnimClass::add_bit_channel(TimeCodedBitChannelClass *newchan)
{
    if (newchan->Get_Type() == BIT_CHANNEL_VIS) {
        m_nodeMotion[newchan->Get_Pivot()].Vis = newchan;
    }
}

void HCompressedAnimClass::Get_Translation(Vector3 &trans, int pividx, float frame) const
{
    NodeCompressedMotionStruct *mot = &m_nodeMotion[pividx];
    trans.Set(0, 0, 0);

    if (m_flavor == ANIM_FLAVOR_TIMECODED) {
        if (mot->tc.X) {
            mot->tc.X->Get_Vector(frame, &trans.X);
        }

        if (mot->tc.Y) {
            mot->tc.Y->Get_Vector(frame, &trans.Y);
        }

        if (mot->tc.Z) {
            mot->tc.Z->Get_Vector(frame, &trans.Z);
        }
    } else if (m_flavor == ANIM_FLAVOR_ADAPTIVE_DELTA) {
        if (mot->ad.X) {
            mot->ad.X->Get_Vector(frame, &trans.X);
        }

        if (mot->ad.Y) {
            mot->ad.Y->Get_Vector(frame, &trans.Y);
        }

        if (mot->ad.Z) {
            mot->ad.Z->Get_Vector(frame, &trans.Z);
        }
    } else {
        captainslog_assert(0);
    }
}

void HCompressedAnimClass::Get_Orientation(Quaternion &q, int pividx, float frame) const
{
    NodeCompressedMotionStruct *mot = &m_nodeMotion[pividx];
    q.Set();

    if (m_flavor == ANIM_FLAVOR_TIMECODED) {
        if (mot->tc.Q) {
            q = mot->tc.Q->Get_Quat_Vector(frame);
        }
    } else if (m_flavor == ANIM_FLAVOR_ADAPTIVE_DELTA) {
        if (mot->tc.Q) {
            q = mot->ad.Q->Get_Quat_Vector(frame);
        }
    } else {
        captainslog_assert(0);
    }
}

void HCompressedAnimClass::Get_Transform(Matrix3D &mtx, int pividx, float frame) const
{
    NodeCompressedMotionStruct *mot = &m_nodeMotion[pividx];
    mtx.Make_Identity();

    if (m_flavor == ANIM_FLAVOR_TIMECODED) {
        if (mot->tc.Q) {
            mtx = Build_Matrix3D(mot->tc.Q->Get_Quat_Vector(frame));
        }

        if (mot->tc.X) {
            mot->tc.X->Get_Vector(frame, &mtx[0].W);
        }

        if (mot->tc.Y) {
            mot->tc.Y->Get_Vector(frame, &mtx[1].W);
        }

        if (mot->tc.Z) {
            mot->tc.Z->Get_Vector(frame, &mtx[2].W);
        }
    } else if (m_flavor == ANIM_FLAVOR_ADAPTIVE_DELTA) {
        if (mot->ad.Q) {
            mtx = Build_Matrix3D(mot->ad.Q->Get_Quat_Vector(frame));
        }

        if (mot->ad.X) {
            mot->ad.X->Get_Vector(frame, &mtx[0].W);
        }

        if (mot->ad.Y) {
            mot->ad.Y->Get_Vector(frame, &mtx[1].W);
        }

        if (mot->ad.Z) {
            mot->ad.Z->Get_Vector(frame, &mtx[2].W);
        }
    } else {
        captainslog_assert(0);
    }
}

bool HCompressedAnimClass::Get_Visibility(int pividx, float frame)
{
    if (m_nodeMotion[pividx].Vis) {
        return m_nodeMotion[pividx].Vis->Get_Bit(frame) == 1;
    }
    return true;
}

bool HCompressedAnimClass::Is_Node_Motion_Present(int pividx)
{
    captainslog_assert((pividx >= 0) && (pividx < m_numNodes));
    NodeCompressedMotionStruct *mot = &m_nodeMotion[pividx];

    if (mot->vd.X) {
        return true;
    }

    if (mot->vd.Y) {
        return true;
    }

    if (mot->vd.Z) {
        return true;
    }

    if (mot->vd.Q) {
        return true;
    }

    return mot->Vis != nullptr;
}

bool HCompressedAnimClass::Has_X_Translation(int pividx)
{
    captainslog_assert((pividx >= 0) && (pividx < m_numNodes));
    NodeCompressedMotionStruct *mot = &m_nodeMotion[pividx];
    return mot->vd.X != nullptr;
}

bool HCompressedAnimClass::Has_Y_Translation(int pividx)
{
    captainslog_assert((pividx >= 0) && (pividx < m_numNodes));
    NodeCompressedMotionStruct *mot = &m_nodeMotion[pividx];
    return mot->vd.Y != nullptr;
}

bool HCompressedAnimClass::Has_Z_Translation(int pividx)
{
    captainslog_assert((pividx >= 0) && (pividx < m_numNodes));
    NodeCompressedMotionStruct *mot = &m_nodeMotion[pividx];
    return mot->vd.Z != nullptr;
}

bool HCompressedAnimClass::Has_Rotation(int pividx)
{
    captainslog_assert((pividx >= 0) && (pividx < m_numNodes));
    NodeCompressedMotionStruct *mot = &m_nodeMotion[pividx];
    return mot->vd.Q != nullptr;
}

bool HCompressedAnimClass::Has_Visibility(int pividx)
{
    captainslog_assert((pividx >= 0) && (pividx < m_numNodes));
    NodeCompressedMotionStruct *mot = &m_nodeMotion[pividx];
    return mot->Vis != nullptr;
}
