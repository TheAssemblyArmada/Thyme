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
#include "matrixmapper.h"
#include "dx8wrapper.h"

MatrixMapperClass::MatrixMapperClass(int stage) :
    TextureMapperClass(stage),
    m_flags(0),
    m_type(ORTHO_PROJECTION),
    m_viewToTexture(true),
    m_viewToPixel(true),
    m_viewSpaceProjectionNormal(0.0f, 0.0f, 0.0f),
    m_gradientUCoord(0.5f)
{
}

void MatrixMapperClass::Update_View_To_Pixel_Transform(float texsize)
{
    float f1 = (texsize - 2.0f) * 0.5f / texsize;

    m_viewToPixel[0] = (m_viewToTexture[3] + m_viewToTexture[0]) * f1;
    m_viewToPixel[1] = (m_viewToTexture[3] - m_viewToTexture[1]) * f1;

    if (m_flags & INVERT_DEPTH_GRADIENT) {
        m_viewToPixel[2] = (m_viewToTexture[3] - m_viewToTexture[2]) * 0.5f;
    } else {
        m_viewToPixel[2] = (m_viewToTexture[3] + m_viewToTexture[2]) * 0.5f;
    }

    m_viewToPixel[3] = m_viewToTexture[3];
    m_viewSpaceProjectionNormal = Normalize(-(Vector3 &)m_viewToTexture[2]);
}

void MatrixMapperClass::Set_Texture_Transform(const Matrix4 &view_to_texture, float texsize)
{
    m_viewToTexture = view_to_texture;
    Update_View_To_Pixel_Transform(texsize);
}

TextureMapperClass *MatrixMapperClass::Clone() const
{
    return 0;
}

void MatrixMapperClass::Apply(int uv_array_index)
{
#ifdef BUILD_WITH_D3D8
    Matrix4 matrix;
    switch (m_type) {
        case ORTHO_PROJECTION:
            DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE)(m_stage + D3DTS_TEXTURE0), m_viewToPixel);
            DX8Wrapper::Set_DX8_Texture_Stage_State(uv_array_index, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
            DX8Wrapper::Set_DX8_Texture_Stage_State(uv_array_index, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
            break;
        case PERSPECTIVE_PROJECTION:
            matrix[0] = m_viewToPixel[0];
            matrix[1] = m_viewToPixel[1];
            matrix[2] = m_viewToPixel[3];
            DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE)(m_stage + D3DTS_TEXTURE0), matrix);
            DX8Wrapper::Set_DX8_Texture_Stage_State(uv_array_index, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
            DX8Wrapper::Set_DX8_Texture_Stage_State(
                uv_array_index, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3 | D3DTTFF_PROJECTED);
            break;
        case DEPTH_GRADIENT:
            matrix[1].X = m_viewToPixel[2].X;
            matrix[1].Y = m_viewToPixel[2].Y;
            matrix[1].Z = m_viewToPixel[2].Z;
            matrix[1].W = m_viewToPixel[2].W;
            matrix[0].W = m_gradientUCoord;
            matrix[0].X = 0;
            matrix[0].Y = 0;
            matrix[0].Z = 0;
            DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE)(m_stage + D3DTS_TEXTURE0), matrix);
            DX8Wrapper::Set_DX8_Texture_Stage_State(uv_array_index, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
            DX8Wrapper::Set_DX8_Texture_Stage_State(uv_array_index, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
            break;
        case NORMAL_GRADIENT:
            matrix[0].W = m_gradientUCoord;
            matrix[1].X = m_viewSpaceProjectionNormal.X;
            matrix[1].Y = m_viewSpaceProjectionNormal.Y;
            matrix[1].Z = m_viewSpaceProjectionNormal.Z;
            matrix[0].X = 0;
            matrix[0].Y = 0;
            matrix[0].Z = 0;
            matrix[1].W = 0;
            DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE)(m_stage + D3DTS_TEXTURE0), matrix);
            DX8Wrapper::Set_DX8_Texture_Stage_State(uv_array_index, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL);
            DX8Wrapper::Set_DX8_Texture_Stage_State(uv_array_index, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
            break;
    }
#endif
}

void MatrixMapperClass::Compute_Texture_Coordinate(const Vector3 &point, Vector3 *set_stq)
{
    Matrix4::Transform_Vector(m_viewToPixel, point, set_stq);
}

void MatrixMapperClass::Calculate_Texture_Matrix(Matrix4 &matrix)
{
    matrix = m_viewToPixel;
}
