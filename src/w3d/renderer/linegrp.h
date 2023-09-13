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
#include "shader.h"
#include "sharebuf.h"
#include "vector3.h"
#include "vector4.h"

class RenderInfoClass;
class TextureClass;

class LineGroupClass
{
public:
    enum
    {
        TETRAHEADRON_NUM_POLYGONS = 4,
        TETRAHEADRON_NUM_VERTEXES = 4,
        TETRAHEADRON_NUM_INDEXES = 3,

        PRISM_NUM_POLYGONS = 8,
        PRISM_NUM_VERTEXES = 6,
        PRISM_NUM_INDEXES = 3,
    };

    LineGroupClass();
    virtual ~LineGroupClass();

    void Set_Arrays(ShareBufferClass<Vector3> *start_locs,
        ShareBufferClass<Vector3> *end_locs,
        ShareBufferClass<Vector4> *diffuse,
        ShareBufferClass<Vector4> *tail_diffuse,
        ShareBufferClass<unsigned int> *alt,
        ShareBufferClass<float> *sizes,
        ShareBufferClass<float> *u_coords,
        int count);

    enum FlagsType
    {
        TRANSFORM
    };

    enum LineModeType
    {
        TETRAHEDRON,
        PRISM,
    };

    void Set_Line_Size(float line_size) { m_defaultSize = line_size; }
    float Get_Line_Size() const { return m_defaultSize; }

    void Set_Line_Color(const Vector3 &color) { m_defaultColor = color; }
    Vector3 Get_Line_Color() const { return m_defaultColor; }

    void Set_Tail_Diffuse(const Vector4 &tail_diffuse) { m_defaultTailDiffuse = tail_diffuse; }
    Vector4 Get_Tail_Diffuse() const { return m_defaultTailDiffuse; }

    void Set_Line_Alpha(float alpha) { m_defaultAlpha = alpha; }
    float Get_Line_Alpha() const { return m_defaultAlpha; }

    void Set_Line_UCoord(float u_coord) { m_defaultUCoord = u_coord; }
    float Get_Line_UCoord() const { return m_defaultUCoord; }

    void Set_Flag(FlagsType flag, bool on)
    {
        if (on) {
            m_flags |= 1 << flag;
        } else {
            m_flags &= ~(1 << flag);
        }
    }
    bool Get_Flag(FlagsType flag) const { return (m_flags >> flag) & 1; }

    void Set_Texture(TextureClass *texture);
    TextureClass *Get_Texture() { return m_texture; }
    TextureClass *Peek_Texture() { return m_texture; }

    void Set_Shader(const ShaderClass &shader) { m_shader = shader; }
    ShaderClass Get_Shader() const { return m_shader; }

    void Set_Line_Mode(LineModeType mode) { m_mode = mode; }
    LineModeType Get_Line_Mode() const { return m_mode; }

    void Render(RenderInfoClass &rinfo);

    int Get_Polygon_Count() const;

protected:
    ShareBufferClass<Vector3> *m_startLoc;
    ShareBufferClass<Vector3> *m_endLoc;
    ShareBufferClass<Vector4> *m_diffuseBuffer;
    ShareBufferClass<Vector4> *m_tailDiffuseBuffer;
    ShareBufferClass<unsigned int> *m_altBuffer;
    ShareBufferClass<float> *m_sizeBuffer;
    ShareBufferClass<float> *m_UCoordsBuffer;

    int m_lineCount;

    TextureClass *m_texture;
    ShaderClass m_shader;

    unsigned int m_flags;

    float m_defaultSize;
    Vector3 m_defaultColor;
    float m_defaultAlpha;
    float m_defaultUCoord;
    Vector4 m_defaultTailDiffuse;

    LineModeType m_mode;
};
