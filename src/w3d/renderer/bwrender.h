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
#pragma once

#include "always.h"
#include "vector2.h"
#include "vector3.h"
#include "vector3i.h"

class BWRenderClass
{
public:
    class Buffer
    {
    public:
        Buffer(unsigned char *buffer, int32_t scale);
        ~Buffer() {}

        void Set_H_Line(int32_t start_x, int32_t end_x, int32_t y);
        void Fill(unsigned char color);
        int32_t Scale() const { return m_scale; }

    private:
        unsigned char *m_buffer;
        int32_t m_scale;
        int32_t m_minv;
        int32_t m_maxv;
    };

    enum
    {
        TRI_COLOR = 0,
    };

    BWRenderClass(unsigned char *buffer, int32_t scale);
    ~BWRenderClass();

    void Fill(unsigned char color);

    void Set_Vertex_Locations(Vector2 *vertices, int32_t count);

    void Render_Triangle(const Vector2 &c1, const Vector2 &c2, const Vector2 &c3);
    void Render_Triangles(const uint32_t *indices, int32_t index_count);
    void Render_Triangle_Strip(const uint32_t *indices, int32_t index_count);
    void Render_Preprocessed_Triangle(Vector3 &x_corners, Vector3i &y_corners);

protected:
    Buffer m_pixelBuffer;
    Vector2 *m_vertices;
};
