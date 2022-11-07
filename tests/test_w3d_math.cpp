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

TEST(w3d_math, vector2)
{
    Vector2 a(2.0f, 1.0f);

    // Test length
    EXPECT_FLOAT_EQ(a.Length(), GameMath::Sqrt(5.0f));
    EXPECT_FLOAT_EQ(a.Length2(), 5.0f);

    // Test normalize
    auto tmp = a;
    tmp.Normalize();
    EXPECT_FLOAT_EQ(tmp.Length(), 1.0f);

    Vector2 b(0.0f, 2.0f);

    // Two notations for dot product
    EXPECT_FLOAT_EQ(Vector2::Dot_Product(a, a), 5.0f);
    EXPECT_FLOAT_EQ(a * a, 5.0f);

    EXPECT_FLOAT_EQ(Vector2::Dot_Product(a, b), 2.0f);
    EXPECT_FLOAT_EQ(a * b, 2.0f);

    // Test distance
    EXPECT_FLOAT_EQ(Vector2::Distance(a, b), GameMath::Sqrt(5.0f));

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
    Vector3 a(2.0f, 1.0f, 1.0f);

    // Test length
    EXPECT_FLOAT_EQ(a.Length(), GameMath::Sqrt(6.0f));
    EXPECT_FLOAT_EQ(a.Length2(), 6.0f);

    // Test normalize
    auto tmp = a;
    tmp.Normalize();
    EXPECT_FLOAT_EQ(tmp.Length(), 1.0f);

    Vector3 b(0.0f, 2.0f, 0.0f);

    // Two notations for dot product
    EXPECT_FLOAT_EQ(Vector3::Dot_Product(a, a), 6.0f);
    EXPECT_FLOAT_EQ(a * a, 6.0f);

    EXPECT_FLOAT_EQ(Vector3::Dot_Product(a, b), 2.0f);
    EXPECT_FLOAT_EQ(a * b, 2.0f);

    // Test distance
    EXPECT_FLOAT_EQ(Vector3::Distance(a, b), GameMath::Sqrt(6.0f));

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
    Vector4 a(2.0f, 1.0f, 1.0f, 1.0f);

    // Test length
    EXPECT_FLOAT_EQ(a.Length(), GameMath::Sqrt(7.0f));
    EXPECT_FLOAT_EQ(a.Length2(), 7.0f);

    // Test normalize
    auto tmp = a;
    tmp.Normalize();
    EXPECT_FLOAT_EQ(tmp.Length(), 1.0f);

    Vector4 b(0.0f, 2.0f, 0.0f, 0.0f);

    // Two notations for dot product
    EXPECT_FLOAT_EQ(Vector4::Dot_Product(a, a), 7.0f);
    EXPECT_FLOAT_EQ(a * a, 7.0f);

    EXPECT_FLOAT_EQ(Vector4::Dot_Product(a, b), 2.0f);
    EXPECT_FLOAT_EQ(a * b, 2.0f);

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
