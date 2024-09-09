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
#include "motchan.h"
#include "gamemath.h"
#include <cstring>

using std::memcpy;

static float g_filterTable[256] = { 1.0e-08f,
    1.0e-07f,
    1.0e-06f,
    1.0e-05f,
    0.0001f,
    0.001f,
    0.01f,
    0.1f,
    1.0f,
    10.0f,
    100.0f,
    1000.0f,
    10000.0f,
    100000.0f,
    1000000.0f,
    10000000.0f };

bool g_tableValid;

// Helper function for getting the Frame (timecode) out of m_data/m_bits
constexpr uint32_t Get_Frame_From_Data(uint32_t data)
{
    return data & 0x7FFFFFFF;
}
// Helper function for getting the Flag (???Can be interpolated???) out of m_data/m_bits
constexpr bool Get_Flag_From_Data(uint32_t data)
{
    return data & (1ULL << 31);
}

MotionChannelClass::MotionChannelClass() :
    m_pivotIdx(0),
    m_type(0),
    m_vectorLen(0),
    m_unusedFloat1(0),
    m_unusedFloat2(0),
    m_unusedBuffer(nullptr),
    m_data(nullptr),
    m_firstFrame(-1),
    m_lastFrame(-1)
{
}

MotionChannelClass::~MotionChannelClass()
{
    Free();
}

void MotionChannelClass::Free()
{
    if (m_unusedBuffer) {
        delete[] m_unusedBuffer;
        m_unusedBuffer = nullptr;
    }

    if (m_data) {
        delete[] m_data;
        m_data = nullptr;
    }
}

bool MotionChannelClass::Load_W3D(ChunkLoadClass &cload)
{
    W3dAnimChannelStruct chan;
    unsigned int chunk_size = cload.Cur_Chunk_Length() - sizeof(chan);

    if (cload.Read(&chan, sizeof(chan)) != sizeof(chan)) {
        return false;
    }

    m_firstFrame = chan.FirstFrame;
    m_lastFrame = chan.LastFrame;
    m_vectorLen = chan.VectorLen;
    m_type = chan.Flags;
    m_pivotIdx = chan.Pivot;
    unsigned int bytes = 4 * m_vectorLen * (m_lastFrame - m_firstFrame + 1);
    unsigned int bytesleft = bytes - 4;
    m_data = new float[bytes];
    m_data[0] = chan.Data[0];

    if (cload.Read(&m_data[1], bytesleft) != bytesleft) {
        Free();
        return false;
    }

    if (chunk_size != bytesleft) {
        cload.Seek(chunk_size - bytesleft);
    }

    return true;
}

BitChannelClass::BitChannelClass() :
    m_pivotIdx(0), m_type(0), m_defaultVal(0), m_firstFrame(-1), m_lastFrame(-1), m_bits(nullptr)
{
}

BitChannelClass::~BitChannelClass()
{
    Free();
}

void BitChannelClass::Free()
{
    if (m_bits) {
        delete[] m_bits;
        m_bits = nullptr;
    }
}

bool BitChannelClass::Load_W3D(ChunkLoadClass &cload)
{
    Free();
    W3dBitChannelStruct chan;

    unsigned int chunk_size = cload.Cur_Chunk_Length();
    if (cload.Read(&chan, sizeof(chan)) != sizeof(chan)) {
        return false;
    }

    m_firstFrame = chan.FirstFrame;
    m_lastFrame = chan.LastFrame;
    m_type = chan.Flags;
    m_pivotIdx = chan.Pivot;
    m_defaultVal = chan.DefaultVal;
    unsigned int bytes = (m_lastFrame - m_firstFrame + 8) >> 3;
    unsigned int bytesleft = bytes - 1;

    captainslog_assert((sizeof(W3dBitChannelStruct) + bytesleft) == chunk_size);

    m_bits = new unsigned char[bytes];

    captainslog_assert(m_bits);

    m_bits[0] = chan.Data[0];

    if (bytesleft && cload.Read(&m_bits[1], bytesleft) != bytesleft) {
        Free();
        return false;
    }

    return true;
}

TimeCodedMotionChannelClass::TimeCodedMotionChannelClass() :
    m_pivotIdx(0),
    m_type(0),
    m_vectorLen(0),
    m_packetSize(0),
    m_numTimeCodes(0),
    m_lastTimeCodeIdx(0),
    m_cachedIdx(0),
    m_data(nullptr)
{
}

TimeCodedMotionChannelClass::~TimeCodedMotionChannelClass()
{
    Free();
}

void TimeCodedMotionChannelClass::Free()
{
    if (m_data) {
        delete[] m_data;
        m_data = nullptr;
    }
}

bool TimeCodedMotionChannelClass::Load_W3D(ChunkLoadClass &cload)
{
    W3dTimeCodedAnimChannelStruct chan;
    unsigned int chunk_size = cload.Cur_Chunk_Length() - sizeof(chan);

    if (cload.Read(&chan, sizeof(chan)) != sizeof(chan)) {
        return false;
    }

    m_type = chan.Flags;
    m_vectorLen = chan.VectorLen;
    m_numTimeCodes = chan.NumTimeCodes;
    m_packetSize = chan.VectorLen + 1;
    m_lastTimeCodeIdx = (chan.VectorLen + 1) * (m_numTimeCodes - 1);
    m_pivotIdx = chan.Pivot;
    m_cachedIdx = 0;
    m_data = new unsigned int[((chunk_size / sizeof(unsigned int)) + 1)];
    m_data[0] = chan.Data[0];

    if (cload.Read(&m_data[1], chunk_size) != chunk_size) {
        Free();
        return false;
    }

    return true;
}

void TimeCodedMotionChannelClass::Get_Vector(float frame, float *setvec)
{
    int index = Get_Index(frame);

    if (index == m_packetSize * (m_numTimeCodes - 1)) {
        float *data = (float *)&m_data[index + 1];

        for (int i = 0; i < m_vectorLen; ++i) {
            setvec[i] = data[i];
        }
    } else {
        int index2 = m_packetSize + index;
        unsigned int val = m_data[index2];

        if (Get_Flag_From_Data(val)) {
            float frame_1 = Get_Frame_From_Data(m_data[index]);
            float *data1 = (float *)&m_data[index + 1];
            float *data2 = (float *)&m_data[index2 + 1];
            float frame_2 = Get_Frame_From_Data(val);
            float t = (frame - frame_1) / (frame_2 - frame_1);

            for (int k = 0; k < m_vectorLen; ++k) {
                setvec[k] = GameMath::Lerp(data1[k], data2[k], t);
            }
        } else {
            float *data3 = (float *)&m_data[index + 1];

            for (int j = 0; j < m_vectorLen; ++j) {
                setvec[j] = data3[j];
            }
        }
    }
}

Quaternion TimeCodedMotionChannelClass::Get_Quat_Vector(float frame_idx)
{
    captainslog_assert(m_vectorLen == 4);

    Quaternion q1(true);
    int index = Get_Index(frame_idx);

    if (index == m_packetSize * (m_numTimeCodes - 1)) {
        Quaternion *dq1 = (Quaternion *)&m_data[index + 1];
        q1.Set(dq1->X, dq1->Y, dq1->Z, dq1->W);
        return q1;
    } else {
        int index2 = m_packetSize + index;
        unsigned int val = m_data[index2];

        if (Get_Flag_From_Data(val)) {
            float frame_1 = Get_Frame_From_Data(m_data[index]);
            float frame_2 = Get_Frame_From_Data(val);
            float alpha = (frame_idx - frame_1) / (frame_2 - frame_1);

            Quaternion *dq3 = (Quaternion *)&m_data[index + 1];
            Quaternion *dq4 = (Quaternion *)&m_data[index2 + 1];
            Quaternion q2(true);
            Quaternion q3(true);

            q2.Set(dq3->X, dq3->Y, dq3->Z, dq3->W);
            q3.Set(dq4->X, dq4->Y, dq4->Z, dq4->W);
            Fast_Slerp(q1, q2, q3, alpha);
            return q1;
        } else {
            Quaternion *dq2 = (Quaternion *)&m_data[index + 1];
            q1.Set(dq2->X, dq2->Y, dq2->Z, dq2->W);
            return q1;
        }
    }
}

void TimeCodedMotionChannelClass::Set_Identity(float *setvec) const
{
    if (m_type == ANIM_CHANNEL_Q) {
        setvec[0] = 0.0f;
        setvec[1] = 0.0f;
        setvec[2] = 0.0f;
        setvec[3] = 1.0f;
    } else {
        setvec[0] = 0.0f;
    }
}

unsigned int TimeCodedMotionChannelClass::Get_Index(unsigned int timecode)
{
    captainslog_assert(m_cachedIdx <= m_lastTimeCodeIdx);

    if (timecode < Get_Frame_From_Data(m_data[m_cachedIdx])) {
        unsigned int result = TimeCodedMotionChannelClass::Binary_Search_Index(timecode);
        m_cachedIdx = result;
        return result;
    }

    if (m_cachedIdx == m_lastTimeCodeIdx) {
        return m_cachedIdx;
    }

    unsigned int result = m_packetSize + m_cachedIdx;

    if (timecode < Get_Frame_From_Data(m_data[m_packetSize + m_cachedIdx])) {
        return m_cachedIdx;
    }

    m_cachedIdx = result;

    if (result != m_lastTimeCodeIdx) {
        int index = result + m_packetSize;

        if (timecode >= Get_Frame_From_Data(m_data[index])) {
            result = TimeCodedMotionChannelClass::Binary_Search_Index(timecode);
            m_cachedIdx = result;
        }
    }

    return result;
}

unsigned int TimeCodedMotionChannelClass::Binary_Search_Index(unsigned int timecode) const
{
    int count2;
    int count = 0;
    int rightIdx = m_numTimeCodes - 2;
    unsigned int result = m_lastTimeCodeIdx;

    if (timecode < Get_Frame_From_Data(m_data[m_lastTimeCodeIdx])) {
        while (true) {
            while (true) {
                count2 = count + ((rightIdx - count) >> 1);
                result = count2 * m_packetSize;

                if (timecode >= Get_Frame_From_Data(m_data[result])) {
                    break;
                }

                rightIdx = count + ((rightIdx - count) >> 1);
            }

            if (timecode < Get_Frame_From_Data(m_data[result + m_packetSize])) {
                break;
            }

            if (count == count2) {
                ++count;
            } else {
                count += (rightIdx - count) >> 1;
            }
        }
    }

    return result;
}

TimeCodedBitChannelClass::TimeCodedBitChannelClass() :
    m_pivotIdx(0), m_type(0), m_defaultVal(0), m_numTimeCodes(0), m_cachedIdx(0), m_bits(nullptr)
{
}

TimeCodedBitChannelClass::~TimeCodedBitChannelClass()
{
    Free();
}

void TimeCodedBitChannelClass::Free()
{
    if (m_bits) {
        delete[] m_bits;
    }
}

bool TimeCodedBitChannelClass::Load_W3D(ChunkLoadClass &cload)
{
    W3dTimeCodedBitChannelStruct chan;
    Free();
    const unsigned int chunk_size = cload.Cur_Chunk_Length();

    if (cload.Read(&chan, sizeof(chan)) != sizeof(chan)) {
        return false;
    }

    m_numTimeCodes = chan.NumTimeCodes;
    m_type = chan.Flags;
    m_pivotIdx = chan.Pivot;
    m_defaultVal = chan.DefaultVal;
    m_cachedIdx = 0;
    const int bytesleft = 4 * m_numTimeCodes - 4;

    captainslog_assert((sizeof(chan) + bytesleft) == chunk_size);

    m_bits = new unsigned int[m_numTimeCodes];

    captainslog_assert(m_bits);

    m_bits[0] = chan.Data[0];

    if (bytesleft && cload.Read(&m_bits[1], bytesleft) != bytesleft) {
        Free();
        return false;
    }

    return true;
}

int TimeCodedBitChannelClass::Get_Bit(int frame)
{
    captainslog_assert(frame >= 0);
    captainslog_assert(m_cachedIdx < m_numTimeCodes);

    unsigned int count = 0;

    if (frame >= static_cast<int>(Get_Frame_From_Data(m_bits[m_cachedIdx]))) {
        count = m_cachedIdx + 1;
    }

    while (count < m_numTimeCodes && frame >= static_cast<int>(Get_Frame_From_Data(m_bits[count]))) {
        ++count;
    }

    int index = count - 1;
    if (index < 0) {
        index = 0;
    }

    m_cachedIdx = index;
    return Get_Flag_From_Data(m_bits[index]);
}

AdaptiveDeltaMotionChannelClass::AdaptiveDeltaMotionChannelClass() :
    m_pivotIdx(0),
    m_type(0),
    m_vectorLen(0),
    m_numFrames(0),
    m_scale(0.0f),
    m_data(nullptr),
    m_cacheFrame(0),
    m_cacheData(nullptr)
{
    if (!g_tableValid) {
        const double quarter_rot = DEG_TO_RAD(90.0);
        for (int i = 0; i < 240; ++i) {
            g_filterTable[i + 16] = 1.0f - GameMath::Sin((i / 240.0f) * quarter_rot);
        }
        g_tableValid = true;
    }
}

AdaptiveDeltaMotionChannelClass::~AdaptiveDeltaMotionChannelClass()
{
    Free();
}

void AdaptiveDeltaMotionChannelClass::Free()
{
    if (m_data) {
        delete[] m_data;
        m_data = nullptr;
    }
    if (m_cacheData) {
        delete m_cacheData;
        m_cacheData = nullptr;
    }
}

bool AdaptiveDeltaMotionChannelClass::Load_W3D(ChunkLoadClass &cload)
{
    W3dAdaptiveDeltaAnimChannelStruct chan;
    unsigned int chunk_size = cload.Cur_Chunk_Length() - sizeof(chan);

    if (cload.Read(&chan, sizeof(chan)) != sizeof(chan)) {
        return false;
    }

    m_vectorLen = chan.VectorLen;
    m_type = chan.Flags;
    m_pivotIdx = chan.Pivot;
    m_numFrames = chan.NumFrames;
    m_scale = chan.Scale;
    m_cacheFrame = 0x7FFFFFFF;
    m_cacheData = new float[2 * m_vectorLen];
    m_data = new unsigned int[((chunk_size / sizeof(unsigned int)) + 1)];
    m_data[0] = chan.Data[0];

    if (cload.Read(&m_data[1], chunk_size) != chunk_size) {
        Free();
        return false;
    }

    return true;
}

void AdaptiveDeltaMotionChannelClass::Get_Vector(float frame, float *setvec)
{
    float value1 = AdaptiveDeltaMotionChannelClass::Get_Frame(frame, 0);
    float value2 = AdaptiveDeltaMotionChannelClass::Get_Frame(frame + 1, 0);
    *setvec = GameMath::Lerp(value1, value2, frame - (unsigned int)frame);
}

Quaternion AdaptiveDeltaMotionChannelClass::Get_Quat_Vector(float frame_idx)
{
    unsigned int frame = frame_idx;
    unsigned int next_frame = frame + 1;
    float alpha = frame_idx - frame;
    Quaternion q2;
    q2.Set(Get_Frame(frame, 0), Get_Frame(frame, 1), Get_Frame(frame, 2), Get_Frame(frame, 3));
    Quaternion q3;
    q3.Set(Get_Frame(next_frame, 0), Get_Frame(next_frame, 1), Get_Frame(next_frame, 2), Get_Frame(next_frame, 3));
    Quaternion q(true);
    Fast_Slerp(q, q2, q3, alpha);
    return q;
}

float AdaptiveDeltaMotionChannelClass::Get_Frame(unsigned int frame_idx, unsigned int vector_idx)
{
    float dst[4];

    if (frame_idx >= m_numFrames) {
        frame_idx = m_numFrames - 1;
    }
    if (m_cacheFrame == frame_idx) {
        return m_cacheData[vector_idx];
    } else if (m_cacheFrame + 1 == frame_idx) {
        return m_cacheData[m_vectorLen + vector_idx];
    } else if (frame_idx < m_cacheFrame) {
        Decompress(frame_idx, m_cacheData);

        if (frame_idx != m_numFrames - 1) {
            Decompress(frame_idx, m_cacheData, frame_idx + 1, &m_cacheData[m_vectorLen]);
        }

        m_cacheFrame = frame_idx;
        return m_cacheData[vector_idx];
    } else if (frame_idx == m_cacheFrame + 2) {
        memcpy(m_cacheData, &m_cacheData[m_vectorLen], sizeof(float) * m_vectorLen);
        Decompress(++m_cacheFrame, m_cacheData, frame_idx, &m_cacheData[m_vectorLen]);
        return m_cacheData[vector_idx + m_vectorLen];
    } else {

        captainslog_assert(m_vectorLen < 4);

        memcpy(dst, &m_cacheData[m_vectorLen], sizeof(float) * m_vectorLen);
        Decompress(m_cacheFrame, dst, frame_idx, m_cacheData);
        m_cacheFrame = frame_idx;

        if (frame_idx != m_numFrames - 1) {
            Decompress(m_cacheFrame, m_cacheData, frame_idx + 1, &m_cacheData[m_vectorLen]);
        }

        return m_cacheData[vector_idx];
    }
}

void AdaptiveDeltaMotionChannelClass::Decompress(
    unsigned int src_idx, float *srcdata, unsigned int frame_idx, float *outdata)
{
    char dst[4];

    captainslog_assert(src_idx < frame_idx);

    unsigned int src = src_idx + 1;
    float *base = (float *)&m_data[m_vectorLen];
    bool done = false;

    for (int i = 0; i < m_vectorLen; ++i) {
        float *f1 = (float *)((char *)base + 9 * i + ((src - 1) >> 4) * 9 * m_vectorLen);
        int i1 = ((char)src - 1) & 0xF;
        float f2 = srcdata[i];
        unsigned int i2 = src;

        while (i2 <= frame_idx) {
            int i3 = *(char *)f1;
            float *f3 = (float *)((char *)f1 + 1);

            while (i1 < 0x10) {
                int v9 = i1 >> 1;
                if (i1 & 1) {
                    *(int *)dst = (int)*((char *)f3 + v9) >> 4;
                } else {
                    dst[0] = *((char *)f3 + v9);
                }

                int i4 = dst[0] & 0xF;
                if (i4 & 8) {
                    i4 |= 0xFFFFFFF0;
                }

                float scale = g_filterTable[i3] * m_scale;
                float f4 = (double)i4 * scale;
                f2 = f2 + f4;

                if (i2 == frame_idx) {
                    done = 1;
                    break;
                }

                ++i2;
                ++i1;
            }

            i1 = 0;

            if (done) {
                break;
            }

            f1 = (float *)((char *)f3 + 9 * m_vectorLen - 1);
        }

        outdata[i] = f2;
    }
}

void AdaptiveDeltaMotionChannelClass::Decompress(unsigned int frame_idx, float *outdata)
{
    char dst[4];

    float *srcdata = (float *)m_data;
    bool done = 0;

    for (int i = 0; i < m_vectorLen; ++i) {
        float *f1 = (float *)((char *)m_data + 9 * i + 4 * m_vectorLen);
        float f2 = srcdata[i];
        unsigned int i1 = 1;
        while (i1 <= frame_idx) {
            int i2 = *(char *)f1;
            float *f3 = (float *)((char *)f1 + 1);

            for (int j = 0; j < 16; ++j) {
                int i3 = j >> 1;

                if (j & 1) {
                    *(int *)dst = (int)*((char *)f3 + i3) >> 4;
                } else {
                    dst[0] = *((char *)f3 + i3);
                }

                int i4 = dst[0] & 0xF;
                if (i4 & 8) {
                    i4 |= 0xFFFFFFF0;
                }

                float scale = g_filterTable[i2] * m_scale;
                float f4 = (double)i4 * scale;
                f2 = f2 + f4;

                if (i1 == frame_idx) {
                    done = 1;
                    break;
                }

                ++i1;
            }

            if (done) {
                break;
            }

            f1 = (float *)((char *)f3 + 9 * m_vectorLen - 1);
        }

        outdata[i] = f2;
    }
}
