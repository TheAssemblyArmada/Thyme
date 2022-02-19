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

    int32_t Get_Type() const { return m_type; }
    int32_t Get_Pivot() const { return m_pivotIdx; }

    void Get_Vector(int32_t frame, float *vector) const
    {
        if (!m_data || frame < m_firstFrame || frame > m_lastFrame) {
            Set_Identity(vector);
        } else if (m_data) {
            frame -= m_firstFrame;

            for (int32_t i = 0; i < m_vectorLen; ++i) {
                vector[i] = m_data[i + m_vectorLen * frame];
            }
        }
    }

    void Get_Vector_As_Quat(int32_t frame, Quaternion &quat) const
    {
        if (frame >= m_firstFrame && frame <= m_lastFrame) {
            float *f = &m_data[m_vectorLen * (frame - m_firstFrame)];
            quat.Set(*f, f[1], f[2], f[3]);
        } else {
            quat.Set(0.0f, 0.0f, 0.0f, 1.0f);
        }
    }

    void Set_Identity(float *setvec) const
    {
        if (m_type == ANIM_CHANNEL_Q) {
            setvec[0] = 0.0f;
            setvec[1] = 0.0f;
            setvec[2] = 0.0f;
            setvec[3] = 1.0f;
        } else {
            for (int32_t i = 0; i < m_vectorLen; ++i) {
                setvec[i] = 0.0f;
            }
        }
    }

private:
    uint32_t m_pivotIdx; // Could be signed, but original uses unsigned, so we do same for now.
    uint32_t m_type; // Could be signed, but original uses unsigned, so we do same for now.
    int32_t m_vectorLen;
    int32_t m_unusedFloat1;
    int32_t m_unusedFloat2;
    short *m_unusedBuffer;
    float *m_data;
    int32_t m_firstFrame;
    int32_t m_lastFrame;
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

    int32_t Get_Type() const { return m_type; }
    int32_t Get_Pivot() const { return m_pivotIdx; }

    int32_t Get_Bit(int32_t frame) const
    {
        if (frame < m_firstFrame || frame > m_lastFrame) {
            return m_defaultVal != 0;
        }

        frame -= m_firstFrame;
        return (m_bits[frame / 8] & (1 << frame % 8)) != 0;
    }

private:
    uint32_t m_pivotIdx;
    uint32_t m_type;
    int32_t m_defaultVal;
    int32_t m_firstFrame;
    int32_t m_lastFrame;
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
    int32_t Get_Type() const { return m_type; }
    int32_t Get_Pivot() const { return m_pivotIdx; }
    void Get_Vector(float frame, float *setvec);
    Quaternion Get_Quat_Vector(float frame_idx);
    void Set_Identity(float *setvec) const;
    uint32_t Get_Index(uint32_t timecode);
    uint32_t Binary_Search_Index(uint32_t timecode) const;

private:
    uint32_t m_pivotIdx;
    uint32_t m_type;
    int32_t m_vectorLen;
    uint32_t m_packetSize;
    uint32_t m_numTimeCodes;
    uint32_t m_lastTimeCodeIdx;
    uint32_t m_cachedIdx;
    uint32_t *m_data;
};

class TimeCodedBitChannelClass : W3DMPO
{
    IMPLEMENT_W3D_POOL(TimeCodedBitChannelClass)
public:
    TimeCodedBitChannelClass();
    virtual ~TimeCodedBitChannelClass() override;
    void Free();
    bool Load_W3D(ChunkLoadClass &cload);
    int32_t Get_Type() const { return m_type; }
    int32_t Get_Pivot() const { return m_pivotIdx; }
    int32_t Get_Bit(int32_t frame);

private:
    uint32_t m_pivotIdx;
    uint32_t m_type;
    int32_t m_defaultVal;
    uint32_t m_numTimeCodes;
    uint32_t m_cachedIdx;
    uint32_t *m_bits;
};

class AdaptiveDeltaMotionChannelClass : W3DMPO
{
    IMPLEMENT_W3D_POOL(AdaptiveDeltaMotionChannelClass)
public:
    AdaptiveDeltaMotionChannelClass();
    virtual ~AdaptiveDeltaMotionChannelClass() override;
    void Free();
    bool Load_W3D(ChunkLoadClass &cload);
    int32_t Get_Type() const { return m_type; }
    int32_t Get_Pivot() const { return m_pivotIdx; }
    void Get_Vector(float frame, float *setvec);
    Quaternion Get_Quat_Vector(float frame_idx);
    float Get_Frame(uint32_t frame_idx, uint32_t vector_idx);
    void Decompress(uint32_t src_idx, float *srcdata, uint32_t frame_idx, float *outdata);
    void Decompress(uint32_t frame_idx, float *outdata);

private:
    uint32_t m_pivotIdx;
    uint32_t m_type;
    int32_t m_vectorLen;
    uint32_t m_numFrames;
    float m_scale;
    uint32_t *m_data;
    uint32_t m_cacheFrame;
    float *m_cacheData;
};
