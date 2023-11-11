/**
 * @file
 *
 * @author Tiberian Technologies
 * @author OmniBlade
 *
 * @brief Floating point math functions. Based on Tiberian Technologies Renegade Scripts wwmath class.
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
#include "array.h"
#include <captainslog.h>
#include <cfloat>
#include <cstdlib>

#ifdef PROCESSOR_X86
#include <xmmintrin.h>
#endif

#ifdef BUILD_WITH_GAMEMATH
#include <gmath.h>
#else
#include <cmath>
#endif

#define GAMEMATH_EPSILON 0.0001f
#define GAMEMATH_EPSILON2 GAMEMATH_EPSILON *GAMEMATH_EPSILON
#define GAMEMATH_PI 3.141592654f
#define GAMEMATH_PI2 6.2831855f
#define GAMEMATH_PI_DBL 3.141592741012573
#define GAMEMATH_FLOAT_MAX (FLT_MAX)
#define GAMEMATH_FLOAT_MIN (FLT_MIN)
#define GAMEMATH_FLOAT_TINY (1.0e-37f)
#define GAMEMATH_SQRT2 1.414213562f
#define GAMEMATH_SQRT3 1.732050808f
#define GAMEMATH_OOSQRT2 0.707106781f
#define GAMEMATH_OOSQRT3 0.577350269f
#define GAMEMATH_TIGHT_CORNER_RADIUS 0.5f

#ifndef RAD_TO_DEG
#define RAD_TO_DEG(x) (((double)x) * 180.0 / GAMEMATH_PI_DBL)
#endif
#ifndef DEG_TO_RAD
#define DEG_TO_RAD(x) (((double)x) * GAMEMATH_PI_DBL / 180.0)
#endif
#ifndef RAD_TO_DEGF
#define RAD_TO_DEGF(x) (((float)x) * 180.0f / GAMEMATH_PI)
#endif
#ifndef DEG_TO_RADF
#define DEG_TO_RADF(x) (((float)x) * GAMEMATH_PI / 180.0f)
#endif

const int ARC_TABLE_SIZE = 1024;
const int SIN_TABLE_SIZE = 1024;

extern const Array<float, ARC_TABLE_SIZE> _FastAcosTable;
extern const Array<float, ARC_TABLE_SIZE> _FastAsinTable;
extern const Array<float, SIN_TABLE_SIZE> _FastSinTable;
extern const Array<float, SIN_TABLE_SIZE> _FastInvSinTable;

namespace GameMath
{
void Init();
void Shutdown();

inline float Max(float v1, float v2)
{
#ifdef BUILD_WITH_GAMEMATH
    return gm_fmaxf(v1, v2);
#else
    return fmaxf(v1, v2);
#endif
}

inline float Min(float v1, float v2)
{
#ifdef BUILD_WITH_GAMEMATH
    return gm_fminf(v1, v2);
#else
    return fminf(v1, v2);
#endif
}

inline float Pow(float val, float power)
{
#ifdef BUILD_WITH_GAMEMATH
    return gm_powf(val, power);
#else
    return powf(val, power);
#endif
}

inline float Square(float val)
{
    return float(val * val);
}

inline float Fabs(float val)
{
    float tmp = val;
    uint32_t value = *reinterpret_cast<uint32_a *>(&tmp);
    value &= 0x7fffffff;
    return float(*reinterpret_cast<float_a *>(&value));
}

inline int Float_To_Int_Chop(const float &f)
{
    int32_t a = *reinterpret_cast<const int32_a *>(&f);
    int32_t sign = (a >> 31);
    int32_t mantissa = (a & ((1 << 23) - 1)) | (1 << 23);
    int32_t exponent = ((a & 0x7fffffff) >> 23) - 127;
    int32_t r = ((uint32_t)(mantissa) << 8) >> (31 - exponent);

    return ((r ^ (sign)) - sign) & ~(exponent >> 31);
}

inline int Float_To_Int_Floor(const float &f)
{
    int32_t a = *reinterpret_cast<const int32_a *>(&f);
    int32_t sign = (a >> 31);
    a &= 0x7fffffff;
    int32_t exponent = (a >> 23) - 127;
    int32_t expsign = ~(exponent >> 31);
    int32_t imask = ((1 << (31 - (exponent)))) - 1;
    int32_t mantissa = (a & ((1 << 23) - 1));
    int32_t r = ((uint32_t)(mantissa | (1 << 23)) << 8) >> (31 - exponent);
    r = ((r & expsign) ^ (sign)) + (((!((mantissa << 8) & imask)) & (expsign ^ ((a - 1) >> 31))) & sign);

    return r;
}

inline float Cos(float val)
{
#ifdef BUILD_WITH_GAMEMATH
    return gm_cosf(val);
#else
    return float(cosf(val)); // TODO reimplement based on fdlibm for cross platform reproducibility.
#endif
}

inline float Sin(float val)
{
#ifdef BUILD_WITH_GAMEMATH
    return gm_sinf(val);
#else
    return float(sinf(val)); // TODO reimplement based on fdlibm for cross platform reproducibility.
#endif
}

inline float Inv_Sin(float val)
{
    return (val > 0.0f) ? 1.0f / Sin(val) : GAMEMATH_FLOAT_MAX;
}

inline float Sqrt(float val)
{
#ifdef BUILD_WITH_GAMEMATH
    return gm_sqrtf(val);
#else
    return float(sqrtf(val)); // IEEE standard says this is predictable for all conforming implementations.
#endif
}

inline float Inv_Sqrt(float val)
{
    return float(1.0f / Sqrt(val));
}

inline float Acos(float val)
{
#ifdef BUILD_WITH_GAMEMATH
    return gm_acosf(val);
#else
    return float(acosf(val)); // TODO reimplement based on fdlibm for cross platform reproducibility.
#endif
}

inline float Asin(float val)
{
#ifdef BUILD_WITH_GAMEMATH
    return gm_asinf(val);
#else
    return float(asinf(val)); // TODO reimplement based on fdlibm for cross platform reproducibility.
#endif
}

inline float Atan(float val)
{
#ifdef BUILD_WITH_GAMEMATH
    return gm_atanf(val);
#else
    return float(atanf(val)); // TODO reimplement based on fdlibm for cross platform reproducibility.
#endif
}

inline float Log10(float val)
{
#ifdef BUILD_WITH_GAMEMATH
    return gm_log10f(val);
#else
    return float(log10f(val)); // TODO reimplement based on fdlibm for cross platform reproducibility.
#endif
}

inline float Atan2(float y, float x)
{
#ifdef BUILD_WITH_GAMEMATH
    return gm_atan2f(y, x);
#else
    return float(atan2f(y, x)); // TODO reimplement based on fdlibm for cross platform reproducibility.
#endif
}

inline float Tan(float val)
{
#ifdef BUILD_WITH_GAMEMATH
    return gm_tanf(val);
#else
    return float(tanf(val)); // TODO reimplement based on fdlibm for cross platform reproducibility.
#endif
}

inline float Fast_Sin(float val)
{
    val *= float(SIN_TABLE_SIZE) / (2.0f * GAMEMATH_PI);
    int idx0 = Float_To_Int_Floor(val);
    int idx1 = idx0 + 1;
    float frac = val - float(idx0);
    idx0 = (unsigned(idx0)) & (SIN_TABLE_SIZE - 1);
    idx1 = (unsigned(idx1)) & (SIN_TABLE_SIZE - 1);

    return (1.0f - frac) * _FastSinTable[idx0] + frac * _FastSinTable[idx1];
}

inline float Fast_Inv_Sin(float val)
{
    return float(1.0f / Fast_Sin(val));
}

inline float Fast_Cos(float val)
{
    val += (GAMEMATH_PI * 0.5f);
    val *= float(SIN_TABLE_SIZE) / (2.0f * GAMEMATH_PI);
    int idx0 = Float_To_Int_Floor(val);
    int idx1 = idx0 + 1;
    float frac = val - float(idx0);
    idx0 = (unsigned(idx0)) & (SIN_TABLE_SIZE - 1);
    idx1 = (unsigned(idx1)) & (SIN_TABLE_SIZE - 1);

    return (1.0f - frac) * _FastSinTable[idx0] + frac * _FastSinTable[idx1];
}

inline float Fast_Inv_Cos(float val)
{
    return float(1.0f / Fast_Cos(val));
}

inline float Fast_Acos(float val)
{
    // Presumably the table method isn't accurate enough in this range.
    if (Fabs(val) > 0.975f) {
        return Acos(val);
    }

    val *= float(ARC_TABLE_SIZE / 2);
    int idx0 = Float_To_Int_Floor(val);
    int idx1 = idx0 + 1;
    float frac = val - float(idx0);
    idx0 += ARC_TABLE_SIZE / 2;
    idx1 += ARC_TABLE_SIZE / 2;

    captainslog_dbgassert((idx0 >= 0) && (idx0 < ARC_TABLE_SIZE), "Index out of table range");
    captainslog_dbgassert((idx1 >= 0) && (idx1 < ARC_TABLE_SIZE), "Index out of table range");

    return (1.0f - frac) * _FastAcosTable[idx0] + frac * _FastAcosTable[idx1];
}

inline float Fast_Asin(float val)
{
    // Presumably the table method isn't accurate enough in this range.
    if (Fabs(val) > 0.975f) {
        return Asin(val);
    }

    val *= float(ARC_TABLE_SIZE / 2);
    int idx0 = Float_To_Int_Floor(val);
    int idx1 = idx0 + 1;
    float frac = val - float(idx0);
    idx0 += ARC_TABLE_SIZE / 2;
    idx1 += ARC_TABLE_SIZE / 2;

    captainslog_dbgassert((idx0 >= 0) && (idx0 < ARC_TABLE_SIZE), "Index out of table range");
    captainslog_dbgassert((idx1 >= 0) && (idx1 < ARC_TABLE_SIZE), "Index out of table range");

    return (1.0f - frac) * _FastAsinTable[idx0] + frac * _FastAsinTable[idx1];
}

inline float Sign(float val)
{
    if (val > 0.0f) {
        return +1.0f;
    }

    if (val < 0.0f) {
        return -1.0f;
    }

    return 0.0f;
}

inline float Ceil(float val)
{
#ifdef BUILD_WITH_GAMEMATH
    return gm_ceilf(val);
#else
    return float(ceilf(val)); // TODO reimplement based on fdlibm for cross platform reproducibility.
#endif
}

inline float Floor(float val)
{
#ifdef BUILD_WITH_GAMEMATH
    return gm_floorf(val);
#else
    return float(floorf(val)); // TODO reimplement based on fdlibm for cross platform reproducibility.
#endif
}

inline bool Fast_Is_Float_Positive(const float &val)
{
    return ((*reinterpret_cast<uint32_a const *>(&val)) & 0x80000000) == 0;
}

inline int Fast_To_Int_Floor(float val)
{
    static const float _almost_one = 0.99999994f;

    if (!Fast_Is_Float_Positive(val)) {
        val -= _almost_one;
    }

#ifdef BUILD_WITH_GAMEMATH
    return gm_lrintf(gm_truncf(val));
#else
    return lrintf(truncf(val)); // TODO reimplement based on fdlibm for cross platform reproducibility.
#endif
}

inline int Fast_To_Int_Ceil(float val)
{
    static const float _almost_one = 0.99999994f;

    if (Fast_Is_Float_Positive(val)) {
        val += _almost_one;
    }

#ifdef BUILD_WITH_GAMEMATH
    return gm_lrintf(gm_truncf(val));
#else
    return lrintf(truncf(val)); // TODO reimplement based on fdlibm for cross platform reproducibility.
#endif
}

inline int Fast_To_Int_Truncate(float val)
{
#ifdef BUILD_WITH_GAMEMATH
    return gm_lrintf(gm_truncf(val));
#else
    return lrintf(truncf(val)); // TODO reimplement based on fdlibm for cross platform reproducibility.
#endif
}

inline float Random_Float()
{
    return float((float(rand() & 0xFFF)) / float(0xFFF));
}

inline float Random_Float(float min, float max)
{
    return float(Random_Float() * (max - min) + min);
}

inline float Wrap(float val, float min = 0.0f, float max = 1.0f)
{
    if (val >= max) {
        val -= (max - min);
    }

    if (val < min) {
        val += (max - min);
    }

    if (val < min) {
        val = min;
    }

    if (val > max) {
        val = max;
    }

    return float(val);
}

// inline double Wrap(double val, double min = 0.0f, double max = 1.0f)
//{
//    if ( val >= max ) {
//        val -= (max - min);
//    }
//
//    if ( val < min ) {
//        val += (max - min);
//    }
//
//    if ( val < min ) {
//        val = min;
//    }
//
//    if ( val > max ) {
//        val = max;
//    }
//
//    return static_cast<double>(val);
//}

inline float Lerp(float a, float b, float lerp)
{
    return float(a + (b - a) * lerp);
}

// Do we want any double math?
// inline double Lerp(double a, double b, float lerp)
//{
//    return (a + (b - a) * lerp);
//}

inline int Lerp(int a, int b, float lerp)
{
    return (a + int((b - a) * lerp));
}

inline int Float_To_Long(float f)
{
#ifdef PROCESSOR_X86
    return _mm_cvtt_ss2si(_mm_load_ss(&f));
#else
    return int(f);
#endif
}

// Do we want any double math?
// inline long Float_To_Long(double f)
//{
//#ifdef PROCESSOR_X86
//    return _mm_cvttsd_si32(_mm_load_pd(&f));
//#else
//    return (int)(f);
//#endif
//}

inline uint8_t Unit_Float_To_Byte(float f)
{
    return uint8_t(f * 255.0f);
}

inline float Byte_To_Unit_Float(unsigned char byte)
{
    return float(byte) / 255.0f;
}

inline bool Is_Valid_Float(float x)
{
    uint32_a *plong = reinterpret_cast<uint32_a *>(&x);
    uint32_t exponent = ((*plong) & 0x7F800000) >> (32 - 9);

    if (exponent == 0xFF) {
        return false;
    }

    return true;
}

inline float Fast_Float_Floor(float val)
{
    static const float _almost_one = 0.99999994f;

    if (!Fast_Is_Float_Positive(val)) {
        val -= _almost_one;
    }

#ifdef BUILD_WITH_GAMEMATH
    return gm_truncf(val);
#else
    return truncf(val); // TODO reimplement based on fdlibm for cross platform reproducibility.
#endif
}

inline float Fast_Float_Ceil(float val)
{
    static const float _almost_one = 0.99999994f;

    if (Fast_Is_Float_Positive(val)) {
        val += _almost_one;
    }

#ifdef BUILD_WITH_GAMEMATH
    return gm_truncf(val);
#else
    return truncf(val); // TODO reimplement based on fdlibm for cross platform reproducibility.
#endif
}

inline int Lrintf(float val)
{
#ifdef BUILD_WITH_GAMEMATH
    return gm_lrintf(val);
#else
    return lrintf(val); // TODO reimplement based on fdlibm for cross platform reproducibility.
#endif
}

// inline bool Is_Valid_Double(double x)
//{
//    uint32_a *plong = reinterpret_cast<uint32_a*>(&x) + 1;
//    uint32_t exponent = ((*plong) & 0x7FF00000) >> (32 - 12);
//
//    if ( exponent == 0x7FF ) {
//        return false;
//    }
//
//    return true;
//}

inline bool Is_Nan(float val)
{
#ifdef BUILD_WITH_GAMEMATH
    return gm_isnanf(val) != 0;
#else
    return std::isnan(val);
#endif
}

} // namespace GameMath

inline float Normalize_Angle(float angle)
{
    captainslog_dbgassert(!GameMath::Is_Nan(angle), "Angle is NAN in normalizeAngle!\n");
    if (GameMath::Is_Nan(angle)) {
        return 0.0f;
    }

    while (angle > GAMEMATH_PI) {
        angle = angle - GAMEMATH_PI * 2;
    }

    while (angle <= -GAMEMATH_PI) {
        angle = angle + GAMEMATH_PI * 2;
    }

    return angle;
}
