/**
 * @file
 *
 * @author feliwir
 *
 * @brief Set of tests to validate w3d math components
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include <gtest/gtest.h>
#include <vector2.h>
#include <vector3.h>
#include <vector4.h>

template<class V> inline void TestLength(const V &v)
{
    EXPECT_FLOAT_EQ(v.Length(), 2.0f);
    EXPECT_FLOAT_EQ(v.Length2(), 4.0f);
}

template<class V> inline void TestNormalize(V v)
{
    v.Normalize();
    EXPECT_FLOAT_EQ(v.Length(), 1.0f);
}

template<class V> inline void TestDistance(const V &a, const V &b)
{
    EXPECT_FLOAT_EQ(a.Distance(a, b), (float)GameMath::Sqrt(8));
}

template<class V> inline void TestDotProduct(const V &a, const V &b)
{
    EXPECT_FLOAT_EQ(a.Dot_Product(a, a), 4.0f);
    EXPECT_FLOAT_EQ(a.Dot_Product(a, b), 0.0f);
}

TEST(w3d_math, vector2)
{
    Vector2 a(2.0f, 0.0f);

    // Test length
    TestLength(a);

    // Test normalize
    TestNormalize(a);

    Vector2 b(0.0f, 2.0f);

    // Test dot product
    TestDotProduct(a, b);

    // Test distance
    TestDistance(a, b);
}

TEST(w3d_math, vector3)
{
    Vector3 a(2.0f, 0.0f, 0.0f);

    // Test length
    TestLength(a);

    // Test normalize
    TestNormalize(a);

    Vector3 b(0.0f, 2.0f, 0.0f);

    // Test dot product
    TestDotProduct(a, b);

    // Test distance
    TestDistance(a, b);
}

TEST(w3d_math, vector4)
{
    Vector4 a(2.0f, 0.0f, 0.0f, 0.0f);

    // Test length
    TestLength(a);

    // Test normalize
    TestNormalize(a);

    Vector4 b(0.0f, 2.0f, 0.0f, 0.0f);

    // Test dot product
    TestDotProduct(a, b);
}