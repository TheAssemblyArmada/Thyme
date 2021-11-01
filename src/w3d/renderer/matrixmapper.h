/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Matrix Mapper
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
#include "mapper.h"
#include "matrix4.h"

class MatrixMapperClass : public TextureMapperClass
{
    IMPLEMENT_W3D_POOL(MatrixMapperClass);

public:
    enum
    {
        INVERT_DEPTH_GRADIENT = 1,
    };

    enum MappingType
    {
        ORTHO_PROJECTION = 0,
        PERSPECTIVE_PROJECTION = 1,
        DEPTH_GRADIENT = 2,
        NORMAL_GRADIENT = 3,
    };

protected:
    unsigned long m_flags;
    MappingType m_type;
    Matrix4 m_viewToTexture;
    Matrix4 m_viewToPixel;
    Vector3 m_viewSpaceProjectionNormal;
    float m_gradientUCoord;

public:
    virtual ~MatrixMapperClass() override {}
    TextureMapperClass *Clone() const override;
    void Apply(int uv_array_index) override;
    virtual void Calculate_Texture_Matrix(Matrix4 &matrix) override;

    MatrixMapperClass(int stage = 0);
    void Update_View_To_Pixel_Transform(float texsize);
    void Set_Texture_Transform(const Matrix4 &view_to_texture, float texsize);
    void Compute_Texture_Coordinate(const Vector3 &point, Vector3 *set_stq);
    void Set_Type(MappingType type) { m_type = type; }
};
