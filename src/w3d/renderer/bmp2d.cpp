/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Bitmap 2D
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "bmp2d.h"
#include "pot.h"
Bitmap2D::Bitmap2D(
    TextureClass *texture, float x, float y, bool adjust_xy, bool additive, bool create_material, bool opaque) :
    DynamicScreenMeshClass(2, 4, 1.0f)
{
    int width;
    int height;
    int bit_depth;
    bool windowed;
    W3D::Get_Device_Resolution(width, height, bit_depth, windowed);
    float w = (float)texture->Get_Width() / (float)width;
    float h = (float)texture->Get_Height() / (float)height;

    if (adjust_xy) {
        x = x - w * 0.5f;
        y = y - h * 0.5f;
    }

    ShaderClass shader;

    if (additive) {
        shader = ShaderClass::s_presetAdditive2DShader;
    } else if (opaque) {
        shader = ShaderClass::s_presetOpaque2DShader;
    } else if (Has_Alpha(texture->Get_Texture_Format())) {
        shader = ShaderClass::s_presetAlpha2DShader;
    } else {
        shader = ShaderClass::s_presetOpaque2DShader;
    }

    Disable_Sort();

    if (create_material) {
        shader.Set_Primary_Gradient(ShaderClass::GRADIENT_BUMPENVMAPLUMINANCE);
        VertexMaterialClass *mat = new VertexMaterialClass;
        mat->Set_Ambient(0.0f, 0.0f, 0.0f);
        mat->Set_Diffuse(0.0f, 0.0f, 0.0f);
        mat->Set_Specular(0.0f, 0.0f, 0.0f);
        mat->Set_Emissive(1.0f, 1.0f, 1.0f);
        Set_Vertex_Material(mat, true, 0);
        mat->Release_Ref();
    }

    m_model->Set_Single_Shader(shader, 0);
    m_model->Set_Single_Texture(texture, 0, 0);

    if (!texture->Is_Initialized()) {
        texture->Init();
    }

    Begin_Tri_Strip();
    Vertex(x, y, 0.0f, 0.0f, 0.0f);
    Vertex(x + w, y, 0.0f, 1.0f, 0.0f);
    Vertex(x, y + h, 0.0f, 0.0f, 1.0f);
    Vertex(x + w, y + h, 0.0f, 1.0f, 1.0f);
    End_Tri_Strip();
    Reset_Flags();
}

Bitmap2D::Bitmap2D(const char *texture,
    float x,
    float y,
    bool adjust_xy,
    bool additive,
    bool create_material,
    int tw,
    int th,
    bool opaque) :
    DynamicScreenMeshClass(2, 4, 1.0f)
{
    int width;
    int height;
    int bit_depth;
    bool windowed;
    W3D::Get_Device_Resolution(width, height, bit_depth, windowed);

    SurfaceClass *surface = new SurfaceClass(texture);
    SurfaceClass::SurfaceDescription desc;
    surface->Get_Description(desc);

    if (tw == -1) {
        tw = desc.width;
    }

    if (th == -1) {
        th = desc.height;
    }

    if (adjust_xy) {
        x = x - (float)tw / (float)width * 0.5f;
        y = y - (float)th / (float)height * 0.5f;
    }

    if (tw < th) {
        th = tw;
    }

    int pot = Find_POT(th);

    if (pot > 256) {
        pot = 256;
    }

    int w;
    int h;

    if (((pot - 1) & tw) != 0) {
        w = tw / pot + 1;
    } else {
        w = tw / pot;
    }

    if (((pot - 1) & th) != 0) {
        h = th / pot + 1;
    } else {
        h = th / pot;
    }

    Resize(2 * w * h, 4 * w * h);

    ShaderClass shader;

    if (additive) {
        shader = ShaderClass::s_presetAdditive2DShader;
    } else if (opaque) {
        shader = ShaderClass::s_presetOpaque2DShader;
    } else if (Has_Alpha(desc.format)) {
        shader = ShaderClass::s_presetAlpha2DShader;
    } else {
        shader = ShaderClass::s_presetOpaque2DShader;
    }

    Disable_Sort();

    if (create_material) {
        shader.Set_Primary_Gradient(ShaderClass::GRADIENT_BUMPENVMAPLUMINANCE);
        VertexMaterialClass *mat = new VertexMaterialClass;
        mat->Set_Ambient(0.0f, 0.0f, 0.0f);
        mat->Set_Diffuse(0.0f, 0.0f, 0.0f);
        mat->Set_Specular(0.0f, 0.0f, 0.0f);
        mat->Set_Emissive(1.0f, 1.0f, 1.0f);
        Set_Vertex_Material(mat, true, 0);
        mat->Release_Ref();
    }

    m_model->Set_Single_Shader(shader, 0);
    int i1 = 0;
    int i2 = th;

    for (int i = h; i > 0; i--) {
        int i3 = 0;
        int i4 = tw;

        for (int j = w; j > 0; j--) {
            int i5;

            if (pot >= i4) {
                i5 = i4;
            } else {
                i5 = pot;
            }

            int i6 = pot;

            if (pot >= i2) {
                i6 = i2;
            }

            int i7;

            if (Find_POT(i5) <= Find_POT(i6)) {
                i7 = Find_POT(i6);
            } else {
                i7 = Find_POT(i5);
            }

            int i8 = Find_POT(i7);

            SurfaceClass *surface2 = new SurfaceClass(i8, i8, desc.format);
            surface2->Copy(0, 0, i3, i1, i8, i8, surface);
            TextureClass *texture = new TextureClass(surface, MIP_LEVELS_1);
            texture->Get_Texture_Filter()->Set_U_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
            texture->Get_Texture_Filter()->Set_V_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
            Ref_Ptr_Release(surface2);

            float f1 = (float)i5 / (float)i7;
            float f2 = (float)i5 / (float)width;
            float f3 = (float)i6 / (float)height;
            float f4 = (float)((float)i3 / (float)width) + x;
            float f5 = (float)i6 / (float)i7;
            float f6 = (float)((float)i1 / (float)height) + y;
            Set_Texture(texture, false, 0);

            Begin_Tri_Strip();
            Vertex(f4, f6, 0.0f, 0.0f, 0.0f);
            Vertex(f4 + f2, f6, 0.0f, f1, 0.0f);
            Vertex(f4, f6 + f3, 0.0f, 0.0f, f5);
            Vertex(f4 + f2, f6 + f3, 0.0, f1, f5);
            End_Tri_Strip();

            i3 += pot;
            i4 += -pot;
        }

        i1 += pot;
        i2 += -pot;
    }

    Ref_Ptr_Release(surface);
    Reset_Flags();
}

Bitmap2D::Bitmap2D(const Bitmap2D &src) : DynamicScreenMeshClass(src) {}

RenderObjClass *Bitmap2D::Clone() const
{
    return new Bitmap2D(*this);
}
