/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Motion Channel classes
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
#include "quaternion.h"
#include "w3d_file.h"
#include "w3dmpo.h"
class ChunkLoadClass;

class MotionChannelClass : public W3DMPO
{
    IMPLEMENT_W3D_POOL(MotionChannelClass)
public:
    MotionChannelClass();
    virtual ~MotionChannelClass() override;
    void Free();
    bool Load_W3D(ChunkLoadClass &cload);

    int Get_Type() { return m_type; }
    int Get_Pivot() { return m_pivotIdx; }

    void Get_Vector(int frame, float *vector)
    {
        if (!m_data || frame < m_firstFrame || frame > m_lastFrame) {
            Set_Identity(vector);
        } else if (m_data) {
            frame -= m_firstFrame;

            for (int i = 0; i < m_vectorLen; ++i) {
                vector[i] = m_data[i + m_vectorLen * frame];
            }
        }
    }

    void Get_Vector_As_Quat(int frame, Quaternion &quat)
    {
        if (frame >= m_firstFrame && frame <= m_lastFrame) {
            float *f = &m_data[m_vectorLen * (frame - m_firstFrame)];
            quat.Set(*f, f[1], f[2], f[3]);
        } else {
            quat.Set(0.0f, 0.0f, 0.0f, 1.0f);
        }
    }

    // #TODO Get_Vector function above is called for single floats, but this function writes to a
    // sequence of floats. This will likely corrupt memory!
    void Set_Identity(float *setvec)
    {
        if (m_type == ANIM_CHANNEL_Q) {
            setvec[0] = 0;
            setvec[1] = 0;
            setvec[2] = 0;
            setvec[3] = 1.0;
        } else {
            for (int i = 0; i < m_vectorLen; ++i) {
                setvec[i] = 0;
            }
        }
    }

private:
    unsigned long m_pivotIdx;
    unsigned long m_type;
    int m_vectorLen;
    int m_unusedFloat1;
    int m_unusedFloat2;
    short *m_unusedBuffer;
    float *m_data;
    int m_firstFrame;
    int m_lastFrame;
    friend class HRawAnimClass;
};

class BitChannelClass : public W3DMPO
{
    IMPLEMENT_W3D_POOL(BitChannelClass)
public:
    BitChannelClass();
    virtual ~BitChannelClass() override;
    void Free();
    bool Load_W3D(ChunkLoadClass &cload);

    int Get_Type() { return m_type; }
    int Get_Pivot() { return m_pivotIdx; }

    int Get_Bit(int frame)
    {
        if (frame < m_firstFrame || frame > m_lastFrame) {
            return m_defaultVal != 0;
        }

        frame -= m_firstFrame;
        return (m_bits[frame / 8] & (1 << frame % 8)) != 0;
    }

private:
    unsigned long m_pivotIdx;
    unsigned long m_type;
    int m_defaultVal;
    int m_firstFrame;
    int m_lastFrame;
    unsigned char *m_bits;
    friend class HRawAnimClass;
};

class TimeCodedMotionChannelClass : W3DMPO
{
    IMPLEMENT_W3D_POOL(TimeCodedMotionChannelClass)
public:
    TimeCodedMotionChannelClass();
    virtual ~TimeCodedMotionChannelClass() override;
    void Free();
    bool Load_W3D(ChunkLoadClass &cload);
    int Get_Type() { return m_type; }
    int Get_Pivot() { return m_pivotIdx; }
    void Get_Vector(float frame, float *setvec);
    Quaternion Get_Quat_Vector(float frame_idx);
    void Set_Identity(float *setvec);
    unsigned long Get_Index(unsigned int timecode);
    unsigned long Binary_Search_Index(unsigned int timecode);

private:
    unsigned long m_pivotIdx;
    unsigned long m_type;
    int m_vectorLen;
    unsigned int m_packetSize;
    unsigned int m_numTimeCodes;
    unsigned int m_lastTimeCodeIdx;
    unsigned int m_cachedIdx;
    unsigned int *m_data;
};

class TimeCodedBitChannelClass : W3DMPO
{
    IMPLEMENT_W3D_POOL(TimeCodedBitChannelClass)
public:
    TimeCodedBitChannelClass();
    virtual ~TimeCodedBitChannelClass() override;
    void Free();
    bool Load_W3D(ChunkLoadClass &cload);
    int Get_Type() { return m_type; }
    int Get_Pivot() { return m_pivotIdx; }
    int Get_Bit(int frame);

private:
    unsigned long m_pivotIdx;
    unsigned long m_type;
    int m_defaultVal;
    unsigned long m_numTimeCodes;
    unsigned long m_cachedIdx;
    unsigned long *m_bits;
};

class AdaptiveDeltaMotionChannelClass : W3DMPO
{
    IMPLEMENT_W3D_POOL(AdaptiveDeltaMotionChannelClass)
public:
    AdaptiveDeltaMotionChannelClass();
    virtual ~AdaptiveDeltaMotionChannelClass() override;
    void Free();
    bool Load_W3D(ChunkLoadClass &cload);
    int Get_Type() { return m_type; }
    int Get_Pivot() { return m_pivotIdx; }
    void Get_Vector(float frame, float *setvec);
    Quaternion Get_Quat_Vector(float frame_idx);
    float Get_Frame(unsigned int frame_idx, unsigned int vector_idx);
    void Decompress(unsigned int src_idx, float *srcdata, unsigned int frame_idx, float *outdata);
    void Decompress(unsigned int frame_idx, float *outdata);

private:
    unsigned int m_pivotIdx;
    unsigned int m_type;
    int m_vectorLen;
    unsigned int m_numFrames;
    float m_scale;
    unsigned int *m_data;
    unsigned int m_cacheFrame;
    float *m_cacheData;
};
