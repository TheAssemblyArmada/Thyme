/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief dazzle
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
#include "matrix3d.h"
#include "matrix4.h"
#include "proto.h"
#include "rendobj.h"
#include "shader.h"
#include "vector3.h"
#include "w3d_file.h"
#include "w3derr.h"
#include "w3dmpo.h"
#include "wwstring.h"

class DazzleRenderObjClass;
class DazzleLayerClass;
class INIClass;
class CameraClass;
class DazzleVisibilityClass;
class TextureClass;
struct VertexFormatXYZNDUV2;

class DazzleInitClass
{
public:
    unsigned int m_type;
    bool m_useCameraTranslation;
    StringClass m_primaryTextureName;
    StringClass m_secondaryTextureName;
    StringClass m_lensflareName;
    float m_haloIntensity;
    float m_haloIntensityPow;
    float m_haloScaleX;
    float m_haloScaleY;
    float m_dazzleSizePow;
    float m_dazzleIntensityPow;
    float m_dazzleIntensity;
    float m_dazzleArea;
    float m_dazzleDirectionArea;
    Vector3 m_dazzleDirection;
    Vector3 m_dazzleTestColor;
    Vector3 m_dazzleColor;
    Vector3 m_haloColor;
    float m_dazzleScaleX;
    float m_dazzleScaleY;
    float m_fadeoutStart;
    float m_fadeoutEnd;
    float m_sizeOptimizationLimit;
    float m_historyWeight;
    float m_radius;
    float m_blinkPeriod;
    float m_blinkOnTime;
};

class LensflareInitClass
{
public:
    LensflareInitClass() : m_flareLocations(0), m_flareSizes(0), m_flareColors(0), m_flareUV(0) {}

    LensflareInitClass(const LensflareInitClass &lic) :
        m_type(lic.m_type),
        m_textureName(lic.m_textureName),
        m_flareCount(lic.m_flareCount),
        m_flareLocations(0),
        m_flareSizes(0),
        m_flareColors(0),
        m_flareUV(0)
    {
        if (m_flareCount) {
            m_flareLocations = new float[m_flareCount];
            memcpy(m_flareLocations, lic.m_flareLocations, sizeof(float) * m_flareCount);
            m_flareSizes = new float[m_flareCount];
            memcpy(m_flareSizes, lic.m_flareSizes, sizeof(float) * m_flareCount);
            m_flareColors = new Vector3[m_flareCount];
            memcpy(m_flareColors, lic.m_flareColors, sizeof(Vector3) * m_flareCount);
            m_flareUV = new Vector4[m_flareCount];
            memcpy(m_flareUV, lic.m_flareUV, sizeof(Vector4) * m_flareCount);
        }
    }

    ~LensflareInitClass()
    {
        delete[] m_flareLocations;
        delete[] m_flareSizes;
        delete[] m_flareColors;
        delete[] m_flareUV;
    }

    unsigned int m_type;
    StringClass m_textureName;
    int m_flareCount;
    float *m_flareLocations;
    float *m_flareSizes;
    Vector3 *m_flareColors;
    Vector4 *m_flareUV;
};

class DazzleTypeClass
{
    friend DazzleRenderObjClass;
    friend DazzleLayerClass;

    TextureClass *m_primaryTexture;
    TextureClass *m_secondaryTexture;
    DazzleInitClass m_ic;
    float m_fadeoutEndSqr;
    float m_fadeoutStartSqr;
    StringClass m_name;
    unsigned int m_dazzleTestColorInteger;
    unsigned int m_dazzleTestMaskInteger;
    unsigned int m_lensflareId;

    ShaderClass m_dazzleShader;
    ShaderClass m_haloShader;

    float m_radius;

    DazzleTypeClass(const DazzleInitClass &is);
    virtual ~DazzleTypeClass();

public:
    virtual void Calculate_Intensities(float &dazzle_intensity,
        float &dazzle_size,
        float &halo_intensity,
        const Vector3 &camera_dir,
        const Vector3 &dazzle_dir,
        const Vector3 &dir,
        float distance) const;

    void Set_Dazzle_Shader(const ShaderClass &s);
    void Set_Halo_Shader(const ShaderClass &s);

    TextureClass *Get_Dazzle_Texture();
    TextureClass *Get_Halo_Texture();
};

class DazzleLayerClass
{
    friend DazzleRenderObjClass;

public:
    DazzleLayerClass();
    ~DazzleLayerClass();
    void Render(CameraClass *camera);

private:
    virtual int Get_Visible_Item_Count(unsigned int type) const;
    virtual void Clear_Visible_List(unsigned int type);

    DazzleRenderObjClass **m_visibleLists;
};

class LensflareTypeClass
{
    friend DazzleLayerClass;
    friend DazzleRenderObjClass;

    TextureClass *m_texture;
    LensflareInitClass m_lic;
    StringClass m_name;

    LensflareTypeClass(const LensflareInitClass &is);
    virtual ~LensflareTypeClass();

public:
    TextureClass *Get_Texture();

    void Generate_Vertex_Buffers(VertexFormatXYZNDUV2 *vertex,
        int &vertex_count,
        float screen_x_scale,
        float screen_y_scale,
        float dazzle_intensity,
        const Vector4 &transformed_location);
};

class DazzleRenderObjClass : public RenderObjClass
{
    friend DazzleLayerClass;

public:
    DazzleRenderObjClass(unsigned int type);
    DazzleRenderObjClass(const char *type_name);
    DazzleRenderObjClass(const DazzleRenderObjClass &src);
    DazzleRenderObjClass &operator=(const DazzleRenderObjClass &);

    virtual ~DazzleRenderObjClass() override {}

    DazzleRenderObjClass *Succ() { return m_succ; }
    const DazzleRenderObjClass *Succ() const { return m_succ; }

    virtual RenderObjClass *Clone() const override;
    virtual int Class_ID() const override { return CLASSID_DAZZLE; }

    virtual void Render(RenderInfoClass &rinfo) override;
    virtual void Special_Render(SpecialRenderInfoClass &rinfo) override;
    virtual void Set_Transform(const Matrix3D &m) override;
    virtual void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
    virtual void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;
    virtual void Scale(float scale) override { m_scale *= scale; }

    void Set_Dazzle_Color(const Vector3 &col) { m_dazzleColor = col; }
    void Set_Halo_Color(const Vector3 &col) { m_haloColor = col; }
    void Set_Lensflare_Intensity(float intensity) { m_lensflareIntensity = intensity; }

    unsigned int Get_Dazzle_Type() { return m_type; }

    void Set_Layer(DazzleLayerClass *layer);

#ifdef GAME_DLL
    virtual const PersistFactoryClass &Get_Factory() const override;
#endif

    static void Set_Current_Dazzle_Layer(DazzleLayerClass *layer);

    static void Init_Type(const DazzleInitClass &i);
    static void Init_Lensflare(const LensflareInitClass &i);
    static void Init_From_INI(const INIClass *ini);
    static unsigned int Get_Type_ID(const char *name);
    static const char *Get_Type_Name(unsigned int id);
    static DazzleTypeClass *Get_Type_Class(unsigned int id);
    static unsigned int Get_Lensflare_ID(const char *name);
    static LensflareTypeClass *Get_Lensflare_Class(unsigned int id);

    static void Deinit();
    static void Install_Dazzle_Visibility_Handler(const DazzleVisibilityClass *visibility_handler);
    static void Enable_Dazzle_Rendering(bool onoff) { s_dazzleRenderingEnabled = onoff; }
    static bool Is_Dazzle_Rendering_Enabled() { return s_dazzleRenderingEnabled; }

private:
    void Vis_Render_Dazzle(SpecialRenderInfoClass &rinfo);
    void Render_Dazzle(CameraClass *camera);

    static bool s_dazzleRenderingEnabled;

    DazzleRenderObjClass *m_succ;
    unsigned int m_type;
    float m_currentDazzleIntensity;
    float m_currentDazzleSize;
    float m_currentHaloIntensity;
    float m_currentDistance;
    Vector4 m_transformedLoc;
    Vector3 m_currentVloc;
    Vector3 m_currentDir;
    Vector3 m_dazzleColor;
    Vector3 m_haloColor;
    float m_lensflareIntensity;
    float m_scale;
    float m_visibility;
    bool m_onList;
    float m_radius;
    unsigned int m_creationTime;
};

class DazzleVisibilityClass
{
public:
    virtual float Compute_Dazzle_Visibility(
        RenderInfoClass &rinfo, DazzleRenderObjClass *dazzle, const Vector3 &point) const;
};

class DazzlePrototypeClass : public W3DMPO, public PrototypeClass
{
    IMPLEMENT_W3D_POOL(W3DMPO);

public:
    DazzlePrototypeClass() : m_dazzleType(0) {}
    virtual ~DazzlePrototypeClass() override {}
    virtual const char *Get_Name() const override { return m_name; }
    virtual int Get_Class_ID() const override { return RenderObjClass::CLASSID_DAZZLE; }
    virtual RenderObjClass *Create() override;
    virtual void Delete_Self() override { delete this; };
    W3DErrorType Load_W3D(ChunkLoadClass &cload);

private:
    StringClass m_name;
    int m_dazzleType;
};

class DazzleLoaderClass : public PrototypeLoaderClass
{
public:
    DazzleLoaderClass() {}
    ~DazzleLoaderClass() {}
    virtual int Chunk_Type() override { return W3D_CHUNK_DAZZLE; }
    virtual PrototypeClass *Load_W3D(ChunkLoadClass &cload) override;
};
