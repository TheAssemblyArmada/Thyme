/**
 * @file
 *
 * @author tomsons26
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "bwrender.h"
#include "vp.h"
#include <algorithm>
#include <cstring>

using GameMath::Float_To_Int_Floor;
using std::memset;

BWRenderClass::Buffer::Buffer(unsigned char *buffer, int scale) :
    m_buffer(buffer), m_scale(scale), m_minv(3), m_maxv(scale - m_minv)
{
}

void BWRenderClass::Buffer::Set_H_Line(int start_x, int end_x, int y)
{
    if (y >= m_minv && y < m_maxv && end_x >= m_minv && start_x < m_maxv) {
        if (start_x < m_minv) {
            start_x = m_minv;
        }
        if (end_x >= m_maxv) {
            end_x = m_maxv - 1;
        }
        if (end_x != start_x) {
            memset(&m_buffer[(m_scale * y) + start_x], TRI_COLOR, end_x - start_x);
        }
    }
}
void BWRenderClass::Buffer::Fill(unsigned char color)
{
    memset(m_buffer, color, m_scale * 2);
}

BWRenderClass::BWRenderClass(unsigned char *buffer, int scale) : m_pixelBuffer(buffer, scale), m_vertices(nullptr) {}

BWRenderClass::~BWRenderClass() {}

void BWRenderClass::Fill(unsigned char color)
{
    m_pixelBuffer.Fill(color);
}

void BWRenderClass::Set_Vertex_Locations(Vector2 *vertices, int count)
{
    m_vertices = vertices;
    float half_scale = (float)m_pixelBuffer.Scale() / 2.0f;
    VectorProcessorClass::MulAdd(&vertices[0][0], half_scale, half_scale, 2 * count);
}

bool Edge_Function(const Vector2 &corner_1, const Vector2 &corner_2, const Vector2 &corner_3)
{
    Vector2 a = corner_2 - corner_1;
    Vector2 b = corner_3 - corner_1;

    // 2D cross product
    float cross = a.X * b.Y - a.Y * b.X;

    return cross >= 0.0f;
}

void BWRenderClass::Render_Triangle(const Vector2 &c1, const Vector2 &c2, const Vector2 &c3)
{
    Vector2 corner_1 = c1;
    Vector2 corner_2 = c2;
    Vector2 corner_3 = c3;

    /**
     * Sort the three corners as ascending by Y coordinate.
     * After sorting following applies: corner_1.Y <= corner_2.Y <= corner_3.Y
     */
    if (corner_2[1] < corner_1[1]) {
        Swap(corner_1, corner_2);
    }

    if (corner_3[1] < corner_1[1]) {
        Swap(corner_1, corner_3);
    }

    if (corner_3[1] < corner_2[1]) {
        Swap(corner_2, corner_3);
    }

    Vector3i y_corners(Float_To_Int_Floor(corner_1.Y), Float_To_Int_Floor(corner_2.Y), Float_To_Int_Floor(corner_3.Y));

    Vector3 x_corners(corner_1.X, corner_2.X, corner_3.X);

    Render_Preprocessed_Triangle(x_corners, y_corners);
}

void BWRenderClass::Render_Triangles(const unsigned int *indices, int index_count)
{
    int count = index_count / 3;

    for (int i = 0; i < count; ++i) {

        unsigned int idx_0 = indices[0];
        ++indices;
        unsigned int idx_1 = indices[0];
        ++indices;
        unsigned int idx_2 = indices[0];
        ++indices;

        if (Edge_Function(m_vertices[idx_0], m_vertices[idx_1], m_vertices[idx_2]) == false) {
            Render_Triangle(m_vertices[idx_0], m_vertices[idx_1], m_vertices[idx_2]);
        }
    }
}

void BWRenderClass::Render_Triangle_Strip(const unsigned int *indices, int index_count)
{
    int count = index_count - 2;

    bool bool1 = false;

    for (int i = 0; i < count; ++i) {

        bool1 = bool1 == false;

        unsigned int idx_0 = indices[0];
        unsigned int idx_1 = indices[1];
        unsigned int idx_2 = indices[2];

        ++indices;

        if (Edge_Function(m_vertices[idx_0], m_vertices[idx_1], m_vertices[idx_2]) != bool1) {
            Render_Triangle(m_vertices[idx_0], m_vertices[idx_1], m_vertices[idx_2]);
        }
    }
}

void BWRenderClass::Render_Preprocessed_Triangle(Vector3 &x_corners, Vector3i &y_corners)
{
    // To draw the triangle we first split it into two triangles to simplify rendering:
    // - Flat Top (corner 1, corner 2, between corner 1 and 3)
    // - Flat Bottom (corner 2, corner 3, between corner 1 and 3)

    // setup x values
    float slope_left;
    float slope_right;

    float left = x_corners[0];
    float right = left;

    // left_height represents the height of the flat top triangle
    int left_height = y_corners[1] - y_corners[0];
    int right_height = y_corners[2] - y_corners[0];

    int y = y_corners[0];

    // Flat Top Triangle Draw
    if (left_height) {
        // calculate slopes
        slope_left = (x_corners[1] - x_corners[0]) / (float)left_height;
        slope_right = (x_corners[2] - x_corners[0]) / (float)right_height;

        if (slope_left > slope_right) {
            std::swap(slope_left, slope_right);
        }

        while (left_height > 0) {
            m_pixelBuffer.Set_H_Line(Float_To_Int_Floor(left), Float_To_Int_Floor(right), y);
            left += slope_left;
            right += slope_right;
            --left_height;
            ++y;
        }

    } else {

        if (x_corners[0] < x_corners[1]) {
            left = x_corners[0];
            right = x_corners[1];
        } else {
            right = x_corners[0];
            left = x_corners[1];
        }
    }

    // This represents the height of the flat bottom triangle
    int height = y_corners[2] - y_corners[1];
    y = y_corners[1];

    // Flat bottom Triangle Draw
    if (height) {
        float s = 1.0f / (float)height;
        // calculate slopes (gradient of side)
        // Note: slope_right should be equal to slope_right from Flat Top
        slope_left = (x_corners[2] - left) * s;
        slope_right = (x_corners[2] - right) * s;
        while (height > 0) {
            m_pixelBuffer.Set_H_Line(Float_To_Int_Floor(left), Float_To_Int_Floor(right), y);
            left += slope_left;
            right += slope_right;
            --height;
            ++y;
        }
    }
}
