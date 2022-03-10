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

#include <vector2i.h>
#include <vector3i.h>

#include <matrix3.h>
#include <matrix4.h>

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
    // Two notations for dot product
    EXPECT_FLOAT_EQ(a.Dot_Product(a, a), 4.0f);
    EXPECT_FLOAT_EQ(a * a, 4.0f);

    EXPECT_FLOAT_EQ(a.Dot_Product(a, b), 0.0f);
    EXPECT_FLOAT_EQ(a * b, 0.0f);
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

    // Test equality operator
    EXPECT_EQ(a, a);
    EXPECT_NE(a, b);

    // Test arithmetic
    EXPECT_EQ(Vector2(1.0f, 0.0f) + Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f));
    EXPECT_EQ(Vector2(1.0f, 0.0f) - Vector2(0.0f, 1.0f), Vector2(1.0f, -1.0f));
    EXPECT_EQ(Vector2(1.0f, 0.0f) * 2, Vector2(2.0f, 0.0f));
    EXPECT_EQ(Vector2(1.0f, 0.0f) / 2, Vector2(0.5f, 0.0f));
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

    // Test equality operator
    EXPECT_EQ(a, a);
    EXPECT_NE(a, b);

    // Test arithmetic
    EXPECT_EQ(Vector3(1.0f, 0.0f, 0.0f) + Vector3(0.0f, 1.0f, 0.0f), Vector3(1.0f, 1.0f, 0.0f));
    EXPECT_EQ(Vector3(1.0f, 0.0f, 0.0f) - Vector3(0.0f, 1.0f, 0.0f), Vector3(1.0f, -1.0f, 0.0f));
    EXPECT_EQ(Vector3(1.0f, 0.0f, 0.0f) * 2, Vector3(2.0f, 0.0f, 0.0f));
    EXPECT_EQ(Vector3(1.0f, 0.0f, 0.0f) / 2, Vector3(0.5f, 0.0f, 0.0f));
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

    // Test equality operator
    EXPECT_EQ(a, a);
    EXPECT_NE(a, b);

    // Test arithmetic
    EXPECT_EQ(Vector4(1.0f, 0.0f, 0.0f, 0.0f) + Vector4(0.0f, 1.0f, 0.0f, 0.0f), Vector4(1.0f, 1.0f, 0.0f, 0.0f));
    EXPECT_EQ(Vector4(1.0f, 0.0f, 0.0f, 0.0f) - Vector4(0.0f, 1.0f, 0.0f, 0.0f), Vector4(1.0f, -1.0f, 0.0f, 0.0f));
    EXPECT_EQ(Vector4(1.0f, 0.0f, 0.0f, 0.0f) * 2, Vector4(2.0f, 0.0f, 0.0f, 0.0f));
    EXPECT_EQ(Vector4(1.0f, 0.0f, 0.0f, 0.0f) / 2, Vector4(0.5f, 0.0f, 0.0f, 0.0f));
}

TEST(w3d_math, vector2i)
{
    Vector2i a(2, 0);

    // Test equality operator
    EXPECT_EQ(a, a);
    EXPECT_NE(a, Vector2i(0, 2));
}

TEST(w3d_math, vector3i)
{
    Vector3i a(2, 0, 0);

    // Test equality operator
    EXPECT_EQ(a, a);
    EXPECT_NE(a, Vector3i(0, 2, 0));
}

TEST(w3d_math, matrix3)
{
    Matrix3 mat(true);
    mat[0][0] = 2.0f;
    EXPECT_FLOAT_EQ(mat.Determinant(), 2.0f);
    EXPECT_EQ(mat.Get_X_Vector(), Vector3(2.0f, 0.0f, 0.0f));
    EXPECT_EQ(mat[0], Vector3(2.0f, 0.0f, 0.0f));

    EXPECT_EQ(mat.Get_Y_Vector(), Vector3(0.0f, 1.0f, 0.0f));
    EXPECT_EQ(mat[1], Vector3(0.0f, 1.0f, 0.0f));

    EXPECT_EQ(mat.Get_Z_Vector(), Vector3(0.0f, 0.0f, 1.0f));
    EXPECT_EQ(mat[2], Vector3(0.0f, 0.0f, 1.0f));

    Matrix3 trans = mat.Transpose();
    EXPECT_EQ(trans, mat);
    Matrix3 inv = mat.Inverse();
    EXPECT_FLOAT_EQ(inv[0][0], 0.5f);
}

TEST(w3d_math, matrix4)
{
    Matrix4 mat(true);
    mat[0][0] = 2.0f;
    EXPECT_FLOAT_EQ(mat.Determinant(), 2.0f);
    EXPECT_EQ(mat[0], Vector4(2.0f, 0.0f, 0.0f, 0.0f));
    EXPECT_EQ(mat[1], Vector4(0.0f, 1.0f, 0.0f, 0.0f));
    EXPECT_EQ(mat[2], Vector4(0.0f, 0.0f, 1.0f, 0.0f));
    EXPECT_EQ(mat[3], Vector4(0.0f, 0.0f, 0.0f, 1.0f));

    Matrix4 trans = mat.Transpose();
    EXPECT_EQ(trans, mat);
    Matrix4 inv = mat.Inverse();
    EXPECT_FLOAT_EQ(inv[0][0], 0.5f);
}
