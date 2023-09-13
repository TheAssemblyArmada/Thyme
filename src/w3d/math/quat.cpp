/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Quaternion class.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "quat.h"
#include "gamemath.h"
#include "matrix3.h"
#include "matrix3d.h"
#include "matrix4.h"
#include "vector2.h"
#include "vector3.h"

namespace
{
// Utility function used in Trackball
float Project_To_Sphere(float r, float x, float y)
{
    float t, z;
    float d = GameMath::Sqrt(x * x + y * y);

    if (d < r * (GAMEMATH_SQRT2 / (2.0f))) { // inside sphere
        z = GameMath::Sqrt(r * r - d * d);
    } else { // on hyperbola
        t = r / GAMEMATH_SQRT2;
        z = t * t / d;
    }

    return z;
}

} // namespace

Quaternion::Quaternion(const Vector3 &axis, float angle)
{
    float s = GameMath::Sin(angle / 2);
    float c = GameMath::Cos(angle / 2);
    X = s * axis.X;
    Y = s * axis.Y;
    Z = s * axis.Z;
    W = c;
}

void Quaternion::Rotate_X(float theta)
{
    *this = (*this) * Quaternion(Vector3(1.0f, 0.0f, 0.0f), theta);
}

void Quaternion::Rotate_Y(float theta)
{
    *this = (*this) * Quaternion(Vector3(0.0f, 1.0f, 0.0f), theta);
}

void Quaternion::Rotate_Z(float theta)
{
    *this = (*this) * Quaternion(Vector3(0.0f, 0.0f, 1.0f), theta);
}

void Quaternion::Normalize()
{
    float len2 = X * X + Y * Y + Z * Z + W * W;

    if (0.0f == len2) {
        return;
    } else {
        float inv_mag = GameMath::Inv_Sqrt(len2);

        X *= inv_mag;
        Y *= inv_mag;
        Z *= inv_mag;
        W *= inv_mag;
    }
}

void Quaternion::Randomize()
{
    X = ((float)(rand() & 0xFFFF)) / 65536.0f;
    Y = ((float)(rand() & 0xFFFF)) / 65536.0f;
    Z = ((float)(rand() & 0xFFFF)) / 65536.0f;
    W = ((float)(rand() & 0xFFFF)) / 65536.0f;

    Normalize();
}

Quaternion &Quaternion::Make_Closest(const Quaternion &qto)
{
    float cos_t = qto.X * X + qto.Y * Y + qto.Z * Z + qto.W * W;

    // if we are on opposite hemisphere from qto, negate ourselves
    if (cos_t < 0.0) {
        X = -X;
        Y = -Y;
        Z = -Z;
        W = -W;
    }

    return *this;
}

Quaternion Axis_To_Quat(const Vector3 &a, float phi)
{
    Quaternion q;
    Vector3 tmp = a;

    tmp.Normalize();
    q[0] = tmp[0];
    q[1] = tmp[1];
    q[2] = tmp[2];

    q.Scale(GameMath::Sin(phi / 2.0f));
    q[3] = GameMath::Cos(phi / 2.0f);

    return q;
}

Quaternion Trackball(float x0, float y0, float x1, float y1, float sphsize)
{
    Vector3 a;
    Vector3 p1;
    Vector3 p2;
    Vector3 d;

    float phi, t;

    if ((x0 == x1) && (y0 == y1)) {
        return Quaternion(0.0f, 0.0f, 0.0f, 1.0f); // Zero rotation
    }

    // Compute z coordinates for projection of p1 and p2 to
    // deformed sphere
    p1[0] = x0;
    p1[1] = y0;
    p1[2] = Project_To_Sphere(sphsize, x0, y0);

    p2[0] = x1;
    p2[1] = y1;
    p2[2] = Project_To_Sphere(sphsize, x1, y1);

    // Find their cross product
    Vector3::Cross_Product(p2, p1, &a);

    // Compute how much to rotate
    d = p1 - p2;
    t = d.Length() / (2.0f * sphsize);

    // Avoid problems with out of control values
    if (t > 1.0f) {
        t = 1.0f;
    }

    if (t < -1.0f) {
        t = -1.0f;
    }

    phi = 2.0f * GameMath::Asin(t);

    return Axis_To_Quat(a, phi);
}

Quaternion Build_Quaternion(const Matrix3D &mat)
{
    static int _nxt[3] = { 1, 2, 0 };
    float tr, s;
    int i, j, k;
    Quaternion q;
    tr = mat[0][0] + mat[1][1] + mat[2][2];

    if (tr > 0.0f) {
        s = GameMath::Sqrt(tr + 1.0);
        q[3] = s * 0.5;
        s = 0.5 / s;
        q[0] = (mat[2][1] - mat[1][2]) * s;
        q[1] = (mat[0][2] - mat[2][0]) * s;
        q[2] = (mat[1][0] - mat[0][1]) * s;
    } else {
        i = 0;

        if (mat[1][1] > mat[0][0]) {
            i = 1;
        }

        if (mat[2][2] > mat[i][i]) {
            i = 2;
        }

        j = _nxt[i];
        k = _nxt[j];
        s = GameMath::Sqrt((mat[i][i] - (mat[j][j] + mat[k][k])) + 1.0);
        q[i] = s * 0.5;

        if (s != 0.0) {
            s = 0.5 / s;
        }

        q[3] = (mat[k][j] - mat[j][k]) * s;
        q[j] = (mat[j][i] + mat[i][j]) * s;
        q[k] = (mat[k][i] + mat[i][k]) * s;
    }

    return q;
}

Quaternion Build_Quaternion(const Matrix3 &mat)
{
    static int _nxt[3] = { 1, 2, 0 };
    float tr, s;
    int i, j, k;
    Quaternion q;
    tr = mat[0][0] + mat[1][1] + mat[2][2];

    if (tr > 0.0) {
        s = GameMath::Sqrt(tr + 1.0);
        q[3] = s * 0.5;
        s = 0.5 / s;

        q[0] = (mat[2][1] - mat[1][2]) * s;
        q[1] = (mat[0][2] - mat[2][0]) * s;
        q[2] = (mat[1][0] - mat[0][1]) * s;

    } else {
        i = 0;

        if (mat[1][1] > mat[0][0]) {
            i = 1;
        }

        if (mat[2][2] > mat[i][i]) {
            i = 2;
        }

        j = _nxt[i];
        k = _nxt[j];
        s = GameMath::Sqrt((mat[i][i] - (mat[j][j] + mat[k][k])) + 1.0);
        q[i] = s * 0.5;

        if (s != 0.0) {
            s = 0.5 / s;
        }

        q[3] = (mat[k][j] - mat[j][k]) * s;
        q[j] = (mat[j][i] + mat[i][j]) * s;
        q[k] = (mat[k][i] + mat[i][k]) * s;
    }

    return q;
}

Quaternion Build_Quaternion(const Matrix4 &mat)
{
    static int _nxt[3] = { 1, 2, 0 };
    float tr, s;
    int i, j, k;
    Quaternion q;
    tr = mat[0][0] + mat[1][1] + mat[2][2];

    if (tr > 0.0) {
        s = GameMath::Sqrt(tr + 1.0);
        q[3] = s * 0.5;
        s = 0.5 / s;
        q[0] = (mat[2][1] - mat[1][2]) * s;
        q[1] = (mat[0][2] - mat[2][0]) * s;
        q[2] = (mat[1][0] - mat[0][1]) * s;
    } else {
        i = 0;

        if (mat[1][1] > mat[0][0]) {
            i = 1;
        }

        if (mat[2][2] > mat[i][i]) {
            i = 2;
        }

        j = _nxt[i];
        k = _nxt[j];
        s = GameMath::Sqrt((mat[i][i] - (mat[j][j] + mat[k][k])) + 1.0);
        q[i] = s * 0.5;

        if (s != 0.0) {
            s = 0.5 / s;
        }

        q[3] = (mat[k][j] - mat[j][k]) * s;
        q[j] = (mat[j][i] + mat[i][j]) * s;
        q[k] = (mat[k][i] + mat[i][k]) * s;
    }

    return q;
}

void Slerp_Setup(const Quaternion &p, const Quaternion &q, SlerpInfoStruct *slerpinfo)
{
    float cos_t;
    captainslog_assert(slerpinfo != nullptr);
    cos_t = p.X * q.X + p.Y * q.Y + p.Z * q.Z + p.W * q.W;

    if (cos_t < 0.0f) {
        cos_t = -cos_t;
        slerpinfo->flip = true;
    } else {
        slerpinfo->flip = false;
    }

    constexpr double slerp_epsilon = 0.001;

    if (1.0f - cos_t < slerp_epsilon) {
        slerpinfo->linear = true;
        slerpinfo->theta = 0.0f;
        slerpinfo->sin_theta = 0.0f;
    } else {
        slerpinfo->linear = false;
        slerpinfo->theta = GameMath::Acos(cos_t);
        slerpinfo->sin_theta = GameMath::Sin(slerpinfo->theta);
    }
}

Quaternion Cached_Slerp(const Quaternion &p, const Quaternion &q, float alpha, SlerpInfoStruct *slerpinfo)
{
    float beta;
    float oo_sin_t;

    if (slerpinfo->linear) {
        beta = 1.0f - alpha;
    } else {
        oo_sin_t = 1.0f / slerpinfo->theta;
        beta = GameMath::Sin(slerpinfo->theta - alpha * slerpinfo->theta) * oo_sin_t;
        alpha = GameMath::Sin(alpha * slerpinfo->theta) * oo_sin_t;
    }

    if (slerpinfo->flip) {
        alpha = -alpha;
    }

    Quaternion res;
    res.X = beta * p.X + alpha * q.X;
    res.Y = beta * p.Y + alpha * q.Y;
    res.Z = beta * p.Z + alpha * q.Z;
    res.W = beta * p.W + alpha * q.W;

    return res;
}

void Cached_Slerp(const Quaternion &p, const Quaternion &q, float alpha, SlerpInfoStruct *slerpinfo, Quaternion *set_q)
{
    float beta;
    float oo_sin_t;

    if (slerpinfo->linear) {
        beta = 1.0f - alpha;
    } else {
        oo_sin_t = 1.0f / slerpinfo->theta;
        beta = GameMath::Sin(slerpinfo->theta - alpha * slerpinfo->theta) * oo_sin_t;
        alpha = GameMath::Sin(alpha * slerpinfo->theta) * oo_sin_t;
    }

    if (slerpinfo->flip) {
        alpha = -alpha;
    }

    set_q->X = beta * p.X + alpha * q.X;
    set_q->Y = beta * p.Y + alpha * q.Y;
    set_q->Z = beta * p.Z + alpha * q.Z;
    set_q->W = beta * p.W + alpha * q.W;
}

void Fast_Slerp(Quaternion &res, const Quaternion &p, const Quaternion &q, float alpha)
{
    float beta; // complementary interpolation parameter
    float theta; // angle between p and q
    float cos_t; // sine, cosine of theta
    float oo_sin_t;
    int qflip; // use flip of q?

    // cos theta = dot product of p and q
    cos_t = p.X * q.X + p.Y * q.Y + p.Z * q.Z + p.W * q.W;

    // if q is on opposite hemisphere from A, use -B instead
    if (cos_t < 0.0f) {
        cos_t = -cos_t;
        qflip = true;
    } else {
        qflip = false;
    }

    if (1.0f - cos_t < GAMEMATH_EPSILON * GAMEMATH_EPSILON) {
        // if q is very close to p, just linearly interpolate
        // between the two.
        beta = 1.0f - alpha;

    } else {
        theta = GameMath::Fast_Acos(cos_t);
        float sin_t = GameMath::Fast_Sin(theta);
        oo_sin_t = 1.0f / sin_t;
        beta = GameMath::Fast_Sin(theta - alpha * theta) * oo_sin_t;
        alpha = GameMath::Fast_Sin(alpha * theta) * oo_sin_t;
    }

    if (qflip) {
        alpha = -alpha;
    }

    res.X = beta * p.X + alpha * q.X;
    res.Y = beta * p.Y + alpha * q.Y;
    res.Z = beta * p.Z + alpha * q.Z;
    res.W = beta * p.W + alpha * q.W;
}

void Slerp(Quaternion &res, const Quaternion &p, const Quaternion &q, float alpha)
{
    float beta; // complementary interploation parameter
    float theta; // angle between p and q
    // float sin_t
    float cos_t; // sine, cosine of theta
    float oo_sin_t;
    int qflip; // use flip of q?

    // cos theta = dot product of p and q
    cos_t = p.X * q.X + p.Y * q.Y + p.Z * q.Z + p.W * q.W;

    // if q is on opposite hemisphere from A, use -B instead
    if (cos_t < 0.0f) {
        cos_t = -cos_t;
        qflip = true;
    } else {
        qflip = false;
    }

    if (1.0f - cos_t < GAMEMATH_EPSILON * GAMEMATH_EPSILON) {
        // if q is very close to p, just linearly interpolate
        // between the two.
        beta = 1.0f - alpha;

    } else {
        // normal slerp!
        theta = GameMath::Acos(cos_t);
        float sin_t = GameMath::Sin(theta);
        oo_sin_t = 1.0f / sin_t;
        beta = GameMath::Sin(theta - alpha * theta) * oo_sin_t;
        alpha = GameMath::Sin(alpha * theta) * oo_sin_t;
    }

    if (qflip) {
        alpha = -alpha;
    }

    res.X = beta * p.X + alpha * q.X;
    res.Y = beta * p.Y + alpha * q.Y;
    res.Z = beta * p.Z + alpha * q.Z;
    res.W = beta * p.W + alpha * q.W;
}

Matrix3 Build_Matrix3(const Quaternion &q)
{
    Matrix3 m;

    m[0][0] = (float)(1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]));
    m[0][1] = (float)(2.0 * (q[0] * q[1] - q[2] * q[3]));
    m[0][2] = (float)(2.0 * (q[2] * q[0] + q[1] * q[3]));

    m[1][0] = (float)(2.0 * (q[0] * q[1] + q[2] * q[3]));
    m[1][1] = (float)(1.0 - 2.0f * (q[2] * q[2] + q[0] * q[0]));
    m[1][2] = (float)(2.0 * (q[1] * q[2] - q[0] * q[3]));

    m[2][0] = (float)(2.0 * (q[2] * q[0] - q[1] * q[3]));
    m[2][1] = (float)(2.0 * (q[1] * q[2] + q[0] * q[3]));
    m[2][2] = (float)(1.0 - 2.0 * (q[1] * q[1] + q[0] * q[0]));

    return m;
}

Matrix3D Build_Matrix3D(const Quaternion &q)
{
    Matrix3D m;

    // initialize the rotation sub-matrix
    m[0][0] = (float)(1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]));
    m[0][1] = (float)(2.0 * (q[0] * q[1] - q[2] * q[3]));
    m[0][2] = (float)(2.0 * (q[2] * q[0] + q[1] * q[3]));

    m[1][0] = (float)(2.0 * (q[0] * q[1] + q[2] * q[3]));
    m[1][1] = (float)(1.0 - 2.0f * (q[2] * q[2] + q[0] * q[0]));
    m[1][2] = (float)(2.0 * (q[1] * q[2] - q[0] * q[3]));

    m[2][0] = (float)(2.0 * (q[2] * q[0] - q[1] * q[3]));
    m[2][1] = (float)(2.0 * (q[1] * q[2] + q[0] * q[3]));
    m[2][2] = (float)(1.0 - 2.0 * (q[1] * q[1] + q[0] * q[0]));

    // no translation
    m[0][3] = m[1][3] = m[2][3] = 0.0f;

    return m;
}

Matrix4 Build_Matrix4(const Quaternion &q)
{
    Matrix4 m;

    // initialize the rotation sub-matrix
    m[0][0] = (float)(1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]));
    m[0][1] = (float)(2.0 * (q[0] * q[1] - q[2] * q[3]));
    m[0][2] = (float)(2.0 * (q[2] * q[0] + q[1] * q[3]));

    m[1][0] = (float)(2.0 * (q[0] * q[1] + q[2] * q[3]));
    m[1][1] = (float)(1.0 - 2.0f * (q[2] * q[2] + q[0] * q[0]));
    m[1][2] = (float)(2.0 * (q[1] * q[2] - q[0] * q[3]));

    m[2][0] = (float)(2.0 * (q[2] * q[0] - q[1] * q[3]));
    m[2][1] = (float)(2.0 * (q[1] * q[2] + q[0] * q[3]));
    m[2][2] = (float)(1.0 - 2.0 * (q[1] * q[1] + q[0] * q[0]));

    // no translation
    m[0][3] = m[1][3] = m[2][3] = 0.0f;

    // last row
    m[3][0] = m[3][1] = m[3][2] = 0.0f;
    m[3][3] = 1.0f;
    return m;
}
