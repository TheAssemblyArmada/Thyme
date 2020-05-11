/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief 4x4 Matrix class.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "matrix4.h"
#include "plane.h"

const Matrix4 Matrix4::IDENTITY(Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), Vector4(0, 0, 0, 1));

void Matrix4::Multiply(const Matrix4 &a, const Matrix4 &b, Matrix4 *res)
{
    captainslog_assert(res != &a);
    captainslog_assert(res != &b);
#define ROWCOL(i, j) a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j] + a[i][3] * b[3][j]
    (*res)[0][0] = ROWCOL(0, 0);
    (*res)[0][1] = ROWCOL(0, 1);
    (*res)[0][2] = ROWCOL(0, 2);
    (*res)[0][3] = ROWCOL(0, 3);
    (*res)[1][0] = ROWCOL(1, 0);
    (*res)[1][1] = ROWCOL(1, 1);
    (*res)[1][2] = ROWCOL(1, 2);
    (*res)[1][3] = ROWCOL(1, 3);
    (*res)[2][0] = ROWCOL(2, 0);
    (*res)[2][1] = ROWCOL(2, 1);
    (*res)[2][2] = ROWCOL(2, 2);
    (*res)[2][3] = ROWCOL(2, 3);
    (*res)[3][0] = ROWCOL(3, 0);
    (*res)[3][1] = ROWCOL(3, 1);
    (*res)[3][2] = ROWCOL(3, 2);
    (*res)[3][3] = ROWCOL(3, 3);
#undef ROWCOL
}

void Matrix4::Multiply(const Matrix3D &a, const Matrix4 &b, Matrix4 *res)
{
    captainslog_assert(res != &b);
#define ROWCOL(i, j) a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j] + a[i][3] * b[3][j]
    (*res)[0][0] = ROWCOL(0, 0);
    (*res)[0][1] = ROWCOL(0, 1);
    (*res)[0][2] = ROWCOL(0, 2);
    (*res)[0][3] = ROWCOL(0, 3);
    (*res)[1][0] = ROWCOL(1, 0);
    (*res)[1][1] = ROWCOL(1, 1);
    (*res)[1][2] = ROWCOL(1, 2);
    (*res)[1][3] = ROWCOL(1, 3);
    (*res)[2][0] = ROWCOL(2, 0);
    (*res)[2][1] = ROWCOL(2, 1);
    (*res)[2][2] = ROWCOL(2, 2);
    (*res)[2][3] = ROWCOL(2, 3);
    (*res)[3][0] = b[3][0];
    (*res)[3][1] = b[3][1];
    (*res)[3][2] = b[3][2];
    (*res)[3][3] = b[3][3];

#undef ROWCOL
}

void Matrix4::Multiply(const Matrix4 &a, const Matrix3D &b, Matrix4 *res)
{
    captainslog_assert(res != &a);
#define ROWCOL(i, j) a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j]
#define ROWCOL4(i, j) a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j] + a[i][3]
    (*res)[0][0] = ROWCOL(0, 0);
    (*res)[0][1] = ROWCOL(0, 1);
    (*res)[0][2] = ROWCOL(0, 2);
    (*res)[0][3] = ROWCOL4(0, 3);
    (*res)[1][0] = ROWCOL(1, 0);
    (*res)[1][1] = ROWCOL(1, 1);
    (*res)[1][2] = ROWCOL(1, 2);
    (*res)[1][3] = ROWCOL4(1, 3);
    (*res)[2][0] = ROWCOL(2, 0);
    (*res)[2][1] = ROWCOL(2, 1);
    (*res)[2][2] = ROWCOL(2, 2);
    (*res)[2][3] = ROWCOL4(2, 3);
    (*res)[3][0] = ROWCOL(3, 0);
    (*res)[3][1] = ROWCOL(3, 1);
    (*res)[3][2] = ROWCOL(3, 2);
    (*res)[3][3] = ROWCOL4(3, 3);
#undef ROWCOL
#undef ROWCOL4
}
