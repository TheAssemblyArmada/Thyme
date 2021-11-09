/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Point Group
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
#include "vector.h"
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"

class VertexMaterialClass;
class RenderInfoClass;
class TextureClass;

class PointGroupClass
{
public:
    enum PointModeEnum
    {
        TRIS,
        QUADS,
        SCREENSPACE
    };

    enum FlagsType
    {
        TRANSFORM,
    };

#ifdef GAME_DLL
    PointGroupClass *Hook_Ctor() { return new (this) PointGroupClass(); }
#endif

    PointGroupClass();
    virtual ~PointGroupClass();
    PointGroupClass &operator=(const PointGroupClass &that);

    void Set_Arrays(ShareBufferClass<Vector3> *locs,
        ShareBufferClass<Vector4> *diffuse = nullptr,
        ShareBufferClass<unsigned int> *apt = nullptr,
        ShareBufferClass<float> *sizes = nullptr,
        ShareBufferClass<unsigned char> *orientations = nullptr,
        ShareBufferClass<unsigned char> *frames = nullptr,
        int active_point_count = -1,
        float vpxmin = 0.0f,
        float vpymin = 0.0f,
        float vpxmax = 0.0f,
        float vpymax = 0.0f);
    void Set_Point_Size(float size);
    float Get_Point_Size();
    void Set_Point_Color(Vector3 color);
    Vector3 Get_Point_Color();
    void Set_Point_Alpha(float alpha);
    float Get_Point_Alpha();
    void Set_Point_Orientation(unsigned char orientation);
    unsigned char Get_Point_Orientation();
    void Set_Point_Frame(unsigned char frame);
    unsigned char Get_Point_Frame();
    void Set_Point_Mode(PointModeEnum mode);
    PointModeEnum Get_Point_Mode();
    void Set_Flag(FlagsType flag, bool onoff);
    int Get_Flag(FlagsType flag);
    void Set_Texture(TextureClass *texture);
    TextureClass *Get_Texture();
    TextureClass *Peek_Texture();
    void Set_Shader(ShaderClass shader);
    ShaderClass Get_Shader();
    unsigned char Get_Frame_Row_Column_Count_Log2();
    void Set_Frame_Row_Column_Count_Log2(unsigned char frccl2);

    void Set_Billboard(bool onoff);
    bool Get_Billboard();

    int Get_Polygon_Count();

    void Render(RenderInfoClass &rinfo);
    void Render_Volume_Particle(RenderInfoClass &rinfo, int unk);

protected:
    void Update_Arrays(Vector3 *point_loc,
        Vector4 *point_diffuse,
        float *point_size,
        unsigned char *point_orientation,
        unsigned char *point_frame,
        int active_points,
        int total_points,
        int &vnum,
        int &pnum);

    ShareBufferClass<Vector3> *m_pointLoc;
    ShareBufferClass<Vector4> *m_pointDiffuse;
    ShareBufferClass<unsigned int> *m_APT;
    ShareBufferClass<float> *m_pointSize;
    ShareBufferClass<unsigned char> *m_pointOrientation;
    ShareBufferClass<unsigned char> *m_pointFrame;
    int m_pointCount;

    unsigned char m_frameRowColumnCountLog2;
    TextureClass *m_texture;
    ShaderClass m_shader;
    PointModeEnum m_pointMode;
    unsigned int m_flags;
    float m_defaultPointSize;
    Vector3 m_defaultPointColor;
    float m_defaultPointAlpha;
    unsigned char m_defaultPointOrientation;
    unsigned char m_defaultPointFrame;
    float m_VPXMin;
    float m_VPYMin;
    float m_VPXMax;
    float m_VPYMax;
    bool m_billboard;

public:
    static void Init();
    static void Shutdown();

private:
    static Vector3 s_triVertexLocationOrientationTable[256][3];
    static Vector3 s_quadVertexLocationOrientationTable[256][4];
    static Vector3 g_screenspaceVertexLocationSizeTable[2][3];
    static Vector2 *s_triVertexUVFrameTable[5];
    static Vector2 *s_quadVertexUVFrameTable[5];
    static VertexMaterialClass *s_pointMaterial;
    static VectorClass<Vector3> s_compressedLoc;
    static VectorClass<Vector4> s_compressedDiffuse;
    static VectorClass<float> s_compressedSize;
    static VectorClass<unsigned char> s_compressedOrient;
    static VectorClass<unsigned char> s_compressedFrame;
    static VectorClass<Vector3> s_transformedLoc;
};
