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
#include "dazzle.h"
#include "assetmgr.h"
#include "camera.h"
#include "chunkio.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include "iniclass.h"
#include "quaternion.h"
#include "rinfo.h"
#include "scene.h"
#include "simplevec.h"
#include "sortingrenderer.h"
#include "texture.h"
#include "vector2.h"
#include "vector3i.h"
#include "vertmaterial.h"
#include "w3d.h"
#include "wwstring.h"

const char *DAZZLE_LIST_STRING = "Dazzles_List";

const char *DAZZLE_INTENSITY_POW_STRING = "DazzleIntensityPow";
const char *DAZZLE_SIZE_POW_STRING = "DazzleSizePow";
const char *DAZZLE_AREA_STRING = "DazzleArea";

const char *HALO_SCALE_X_STRING = "HaloScaleX";
const char *HALO_SCALE_Y_STRING = "HaloScaleY";

const char *HALO_INTENSITY_POW = "HaloIntensityPow";

const char *DAZZLE_SCALE_X_STRING = "DazzleScaleX";
const char *DAZZLE_SCALE_Y_STRING = "DazzleScaleY";

const char *HALO_INTENSITY_STRING = "HaloIntensity";
const char *DAZZLE_INTENSITY_STRING = "DazzleIntensity";

const char *DAZZLE_DIRECTION_AREA_STRING = "DazzleDirectionArea";
const char *DAZZLE_DIRECTION_STRING = "DazzleDirection";

const char *FADEOUT_START_STRING = "FadeoutStart";
const char *FADEOUT_END_STRING = "FadeoutEnd";

const char *SIZE_OPTIMIZATION_LIMIT_STRING = "SizeOptimizationLimit";

const char *HISTORY_WEIGHT_STRING = "HistoryWeight";

const char *DAZZLE_TEXTURE_STRING = "DazzleTextureName";
const char *HALO_TEXTURE_STRING = "HaloTextureName";

const char *USE_CAMERA_TRANSLATION = "UseCameraTranslation";

const char *HALO_COLOR_STRING = "HaloColor";
const char *DAZZLE_COLOR_STRING = "DazzleColor";

const char *DAZZLE_TEST_COLOR_STRING = "DazzleTestColor";

const char *BLINK_PERIOD_STRING = "BlinkPeriod";
const char *BLINK_ON_TIME_STRING = "BlinkOnTime";

const char *DAZZLE_LENSFLARE_STRING = "LensflareName";

const char *LENSFLARE_LIST_STRING = "Lensflares_List";

const char *LENSFLARE_TEXTURE_STRING = "TextureName";

const char *FLARE_COUNT_STRING = "FlareCount";

const char *FLARE_LOCATION_STRING = "FlareLocation";

const char *FLARE_SIZE_STRING = "FlareSize";

const char *FLARE_COLOR_STRING = "FlareColor";

const char *FLARE_UV_STRING = "FlareUV";

const char *RADIUS_STRING = "Radius";

static SimpleVecClass<DazzleRenderObjClass *> g_tempPtrs;

static DazzleTypeClass **g_Types;
static unsigned int g_typeCount;

static DazzleLayerClass *g_currentDazzleLayer = nullptr;

static LensflareTypeClass **g_lensFlares;
static unsigned int g_lensflareCount;

static ShaderClass g_defaultDazzleShader;
static ShaderClass g_defaultHaloShader;

static DazzleVisibilityClass _DefaultVisibilityHandler;
static const DazzleVisibilityClass *_VisibilityHandler = &_DefaultVisibilityHandler;

bool DazzleRenderObjClass::s_dazzleRenderingEnabled = true;

static void Init_Shaders()
{
    g_defaultDazzleShader.Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
    g_defaultDazzleShader.Set_Depth_Mask(ShaderClass::DEPTH_WRITE_DISABLE);
    g_defaultDazzleShader.Set_Depth_Compare(ShaderClass::PASS_ALWAYS);
    g_defaultDazzleShader.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_ONE);
    g_defaultDazzleShader.Set_Src_Blend_Func(ShaderClass::SRCBLEND_ONE);
    g_defaultDazzleShader.Set_Fog_Func(ShaderClass::FOG_DISABLE);
    g_defaultDazzleShader.Set_Primary_Gradient(ShaderClass::GRADIENT_MODULATE);
    g_defaultDazzleShader.Set_Texturing(ShaderClass::TEXTURING_ENABLE);

    g_defaultHaloShader.Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
    g_defaultHaloShader.Set_Depth_Mask(ShaderClass::DEPTH_WRITE_DISABLE);
    g_defaultHaloShader.Set_Depth_Compare(ShaderClass::PASS_LEQUAL);
    g_defaultHaloShader.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_ONE);
    g_defaultHaloShader.Set_Src_Blend_Func(ShaderClass::SRCBLEND_ONE);
    g_defaultHaloShader.Set_Fog_Func(ShaderClass::FOG_DISABLE);
    g_defaultHaloShader.Set_Primary_Gradient(ShaderClass::GRADIENT_MODULATE);
    g_defaultHaloShader.Set_Texturing(ShaderClass::TEXTURING_ENABLE);
}

class DazzleINIClass : public INIClass
{
public:
    DazzleINIClass(FileClass &file) : INIClass(file) {}
    const Vector2 Get_Vector2(char const *section, char const *entry, const Vector2 &defvalue = Vector2(0, 0)) const;
    const Vector3 Get_Vector3(char const *section, char const *entry, const Vector3 &defvalue = Vector3(0, 0, 0)) const;
    const Vector4 Get_Vector4(char const *section, char const *entry, const Vector4 &defvalue = Vector4(0, 0, 0, 0)) const;
};

const Vector2 DazzleINIClass::Get_Vector2(char const *section, char const *entry, const Vector2 &defvalue) const
{
    if (section != nullptr && entry != nullptr) {
        INIEntry *entryptr = Find_Entry(section, entry);

        if (entryptr && entryptr->m_value != nullptr) {
            Vector2 ret;

            if (sscanf(entryptr->m_value, "%f,%f", &ret[0], &ret[1]) == 2) {
                return ret;
            }
        }
    }

    return defvalue;
}

const Vector3 DazzleINIClass::Get_Vector3(char const *section, char const *entry, const Vector3 &defvalue) const
{
    if (section != nullptr && entry != nullptr) {
        INIEntry *entryptr = Find_Entry(section, entry);

        if (entryptr && entryptr->m_value != nullptr) {
            Vector3 ret;

            if (sscanf(entryptr->m_value, "%f,%f,%f", &ret[0], &ret[1], &ret[2]) == 3) {
                return ret;
            }
        }
    }

    return defvalue;
}

const Vector4 DazzleINIClass::Get_Vector4(char const *section, char const *entry, const Vector4 &defvalue) const
{
    if (section != nullptr && entry != nullptr) {
        INIEntry *entryptr = Find_Entry(section, entry);

        if (entryptr && entryptr->m_value != nullptr) {
            Vector4 ret;

            if (sscanf(entryptr->m_value, "%f,%f,%f,%f", &ret[0], &ret[1], &ret[2], &ret[3]) == 4) {
                return ret;
            }
        }
    }

    return defvalue;
}

LensflareTypeClass::LensflareTypeClass(const LensflareInitClass &is) : m_lic(is), m_texture(nullptr) {}

LensflareTypeClass::~LensflareTypeClass()
{
    Ref_Ptr_Release(m_texture);
}

TextureClass *LensflareTypeClass::Get_Texture()
{
    if (!m_texture) {
        m_texture = W3DAssetManager::Get_Instance()->Get_Texture(m_lic.m_textureName);
    }

    return m_texture;
}

void LensflareTypeClass::Generate_Vertex_Buffers(VertexFormatXYZNDUV2 *vertex,
    int &vertex_count,
    float screen_x_scale,
    float screen_y_scale,
    float dazzle_intensity,
    const Vector4 &transformed_location)
{
#ifdef BUILD_WITH_D3D8
    float z = transformed_location[2];

    float distance_multiplier =
        GameMath::Sqrt(transformed_location[0] * transformed_location[0] + transformed_location[1] * transformed_location[1])
        + 1.0f;

    for (int a = 0; a < m_lic.m_flareCount; ++a) {
        float x = m_lic.m_flareLocations[a] * transformed_location[0];
        float y = m_lic.m_flareLocations[a] * transformed_location[1];
        float size = m_lic.m_flareSizes[a] * distance_multiplier;
        float ix = size * screen_x_scale;
        float iy = size * screen_y_scale;

        Vector3 col = m_lic.m_flareColors[a] * dazzle_intensity;

        if (col[0] > 1.0f) {
            col[0] = 1.0f;
        }

        if (col[1] > 1.0f) {
            col[1] = 1.0f;
        }

        if (col[2] > 1.0f) {
            col[2] = 1.0f;
        }

        unsigned int color = DX8Wrapper::Convert_Color(col, 1.0f);

        vertex->x = x + ix;
        vertex->y = y - iy;
        vertex->z = z;
        vertex->u1 = m_lic.m_flareUV[a][0];
        vertex->v1 = m_lic.m_flareUV[a][1];
        vertex->diffuse = color;
        vertex++;

        vertex->x = x + ix;
        vertex->y = y + iy;
        vertex->z = z;
        vertex->u1 = m_lic.m_flareUV[a][2];
        vertex->v1 = m_lic.m_flareUV[a][1];
        vertex->diffuse = color;
        vertex++;

        vertex->x = x - ix;
        vertex->y = y + iy;
        vertex->z = z;
        vertex->u1 = m_lic.m_flareUV[a][2];
        vertex->v1 = m_lic.m_flareUV[a][3];
        vertex->diffuse = color;
        vertex++;

        vertex->x = x - ix;
        vertex->y = y - iy;
        vertex->z = z;
        vertex->u1 = m_lic.m_flareUV[a][0];
        vertex->v1 = m_lic.m_flareUV[a][3];
        vertex->diffuse = color;
        vertex++;

        vertex_count += 4;
    }
#endif
}

DazzleTypeClass::DazzleTypeClass(const DazzleInitClass &is) :
    m_ic(is),
    m_dazzleShader(g_defaultDazzleShader),
    m_haloShader(g_defaultHaloShader),
    m_primaryTexture(nullptr),
    m_secondaryTexture(nullptr),
    m_lensflareId(DazzleRenderObjClass::Get_Lensflare_ID(is.m_lensflareName)),
    m_radius(is.m_radius)
{
    m_fadeoutEndSqr = m_ic.m_fadeoutEnd * m_ic.m_fadeoutEnd;
    m_fadeoutStartSqr = m_ic.m_fadeoutStart * m_ic.m_fadeoutStart;
    m_dazzleTestColorInteger = 0xff000000 | (unsigned(255.0f * m_ic.m_dazzleTestColor[2]) << 16)
        | (unsigned(255.0f * m_ic.m_dazzleTestColor[1]) << 8) | (unsigned(255.0f * m_ic.m_dazzleTestColor[0]));
    m_dazzleTestMaskInteger = m_dazzleTestColorInteger & 0xf8f8f8f8;
}

DazzleTypeClass::~DazzleTypeClass()
{
    Ref_Ptr_Release(m_primaryTexture);
    Ref_Ptr_Release(m_secondaryTexture);
}

void DazzleTypeClass::Set_Dazzle_Shader(const ShaderClass &s)
{
    m_dazzleShader = s;
}

void DazzleTypeClass::Set_Halo_Shader(const ShaderClass &s)
{
    m_haloShader = s;
}

TextureClass *DazzleTypeClass::Get_Dazzle_Texture()
{
    if (!m_primaryTexture) {
        m_primaryTexture = W3DAssetManager::Get_Instance()->Get_Texture(m_ic.m_primaryTextureName);
    }

    return m_primaryTexture;
}

TextureClass *DazzleTypeClass::Get_Halo_Texture()
{
    if (!m_secondaryTexture) {
        m_secondaryTexture = W3DAssetManager::Get_Instance()->Get_Texture(m_ic.m_secondaryTextureName);
    }

    return m_secondaryTexture;
}

void DazzleTypeClass::Calculate_Intensities(float &dazzle_intensity,
    float &dazzle_size,
    float &halo_intensity,
    const Vector3 &camera_dir,
    const Vector3 &dazzle_dir,
    const Vector3 &dir,
    float distance) const
{
    float f = -Vector3::Dot_Product(dir, camera_dir);
    dazzle_intensity = f;

    if (m_ic.m_useCameraTranslation && distance > (m_fadeoutEndSqr)) {
        dazzle_intensity = 0.0f;
        return;
    }

    dazzle_intensity -= 1.0f - m_ic.m_dazzleArea;
    dazzle_intensity /= m_ic.m_dazzleArea;
    dazzle_intensity = std::clamp(dazzle_intensity, 0.0f, 1.0f);

    if (m_ic.m_dazzleDirectionArea > 0.0f) {
        float angle = -Vector3::Dot_Product(camera_dir, dazzle_dir);
        angle -= 1.0f - m_ic.m_dazzleDirectionArea;
        angle /= m_ic.m_dazzleDirectionArea;
        angle = std::clamp(angle, 0.0f, 1.0f);
        dazzle_intensity *= angle;
    }

    if (dazzle_intensity > 0.0f) {
        dazzle_size = GameMath::Pow(dazzle_intensity, m_ic.m_dazzleSizePow);
        dazzle_intensity = GameMath::Pow(dazzle_intensity, m_ic.m_dazzleIntensityPow);
    } else {
        dazzle_intensity = 0.0f;
    }

    if (m_ic.m_haloIntensityPow > GAMEMATH_EPSILON) {
        if (f <= 0.0f) {
            halo_intensity = 0.0f;
        } else {
            halo_intensity = GameMath::Pow(f, m_ic.m_haloIntensityPow) * halo_intensity;
        }
    }

    dazzle_intensity *= m_ic.m_dazzleIntensity;
    halo_intensity *= m_ic.m_haloIntensity;

    if (m_ic.m_useCameraTranslation) {
        if (distance > (m_fadeoutStartSqr)) {
            distance = GameMath::Sqrt(distance);
            distance -= m_ic.m_fadeoutStart;
            distance /= m_ic.m_fadeoutEnd - m_ic.m_fadeoutStart;
            dazzle_intensity *= 1.0f - distance;
            halo_intensity *= 1.0f - distance;
        }
    }
}

void DazzleRenderObjClass::Init_From_INI(const INIClass *ini)
{
    if (!ini) {
        return;
    }

    if (ini->Section_Count() == 0) {
        return;
    }

    Init_Shaders();

    unsigned int count = ini->Entry_Count(LENSFLARE_LIST_STRING);
    unsigned int entry;

    for (entry = 0; entry < count; entry++) {
        char section_name[80];
        ini->Get_String(
            LENSFLARE_LIST_STRING, ini->Get_Entry(LENSFLARE_LIST_STRING, entry), "", section_name, sizeof(section_name));

        LensflareInitClass lic;

        ini->Get_String(lic.m_textureName, section_name, LENSFLARE_TEXTURE_STRING);
        lic.m_flareCount = ini->Get_Int(section_name, FLARE_COUNT_STRING, 0);

        if (lic.m_flareCount) {
            lic.m_flareLocations = new float[lic.m_flareCount];
            lic.m_flareSizes = new float[lic.m_flareCount];
            lic.m_flareColors = new Vector3[lic.m_flareCount];
            lic.m_flareUV = new Vector4[lic.m_flareCount];
        } else {
            lic.m_flareLocations = nullptr;
            lic.m_flareSizes = nullptr;
            lic.m_flareColors = nullptr;
        }

        for (int flare = 0; flare < lic.m_flareCount; ++flare) {
            StringClass tmp_name(true);
            tmp_name.Format("%s%d", FLARE_LOCATION_STRING, flare + 1);
            lic.m_flareLocations[flare] = ini->Get_Float(section_name, tmp_name, 0.0f);
            tmp_name.Format("%s%d", FLARE_SIZE_STRING, flare + 1);
            lic.m_flareSizes[flare] = ini->Get_Float(section_name, tmp_name, 1.0f);
            const DazzleINIClass *dini = reinterpret_cast<const DazzleINIClass *>(ini);
            tmp_name.Format("%s%d", FLARE_COLOR_STRING, flare + 1);
            lic.m_flareColors[flare] = dini->Get_Vector3(section_name, tmp_name, Vector3(1.0f, 1.0f, 1.0f));
            tmp_name.Format("%s%d", FLARE_UV_STRING, flare + 1);
            lic.m_flareUV[flare] = dini->Get_Vector4(section_name, tmp_name, Vector4(0.0f, 0.0f, 1.0f, 1.0f));
        }

        lic.m_type = entry;

        Init_Lensflare(lic);
        g_lensFlares[entry]->m_name = section_name;
    }

    count = ini->Entry_Count(DAZZLE_LIST_STRING);
    for (entry = 0; entry < count; entry++) {
        char section_name[80];
        ini->Get_String(
            DAZZLE_LIST_STRING, ini->Get_Entry(DAZZLE_LIST_STRING, entry), "", section_name, sizeof(section_name));

        DazzleInitClass dic;

        ini->Get_String(dic.m_primaryTextureName, section_name, DAZZLE_TEXTURE_STRING);
        ini->Get_String(dic.m_secondaryTextureName, section_name, HALO_TEXTURE_STRING);
        dic.m_haloIntensity = ini->Get_Float(section_name, HALO_INTENSITY_STRING, 0.95f);
        dic.m_haloIntensityPow = ini->Get_Float(section_name, HALO_INTENSITY_POW, 0.0f);
        dic.m_haloScaleX = ini->Get_Float(section_name, HALO_SCALE_X_STRING, 2.0f);
        dic.m_haloScaleY = ini->Get_Float(section_name, HALO_SCALE_Y_STRING, 2.0f);
        dic.m_dazzleArea = ini->Get_Float(section_name, DAZZLE_AREA_STRING, 0.05f);
        dic.m_dazzleDirectionArea = ini->Get_Float(section_name, DAZZLE_DIRECTION_AREA_STRING, 0.50f);
        dic.m_dazzleIntensity = ini->Get_Float(section_name, DAZZLE_INTENSITY_STRING, 0.90f);
        dic.m_dazzleIntensityPow = ini->Get_Float(section_name, DAZZLE_INTENSITY_POW_STRING, 0.90f);
        dic.m_dazzleSizePow = ini->Get_Float(section_name, DAZZLE_SIZE_POW_STRING, 0.90f);
        dic.m_dazzleScaleX = ini->Get_Float(section_name, DAZZLE_SCALE_X_STRING, 100.0f);
        dic.m_dazzleScaleY = ini->Get_Float(section_name, DAZZLE_SCALE_Y_STRING, 25.0f);
        dic.m_fadeoutStart = ini->Get_Float(section_name, FADEOUT_START_STRING, 25.0f);
        dic.m_fadeoutEnd = ini->Get_Float(section_name, FADEOUT_END_STRING, 50.0f);
        dic.m_sizeOptimizationLimit = ini->Get_Float(section_name, SIZE_OPTIMIZATION_LIMIT_STRING, 0.05f);
        dic.m_historyWeight = ini->Get_Float(section_name, HISTORY_WEIGHT_STRING, 0.5f);
        dic.m_useCameraTranslation = !!ini->Get_Int(section_name, USE_CAMERA_TRANSLATION, 1);
        ini->Get_String(dic.m_lensflareName, section_name, DAZZLE_LENSFLARE_STRING);
        dic.m_type = entry;

        const DazzleINIClass *dini = reinterpret_cast<const DazzleINIClass *>(ini);
        Vector3 tp = dini->Get_Vector3(section_name, DAZZLE_DIRECTION_STRING, Vector3(0.0f, 0.0f, 0.0f));
        tp.Normalize();
        dic.m_dazzleDirection = tp;

        dic.m_dazzleTestColor = dini->Get_Vector3(section_name, DAZZLE_TEST_COLOR_STRING, Vector3(1.0f, 1.0f, 1.0f));

        dic.m_dazzleColor = dini->Get_Vector3(section_name, DAZZLE_COLOR_STRING, Vector3(1.0f, 1.0f, 1.0f));

        dic.m_haloColor = dini->Get_Vector3(section_name, HALO_COLOR_STRING, Vector3(1.0f, 1.0f, 1.0f));

        dic.m_radius = ini->Get_Float(section_name, RADIUS_STRING, 1.0f);

        dic.m_blinkPeriod = ini->Get_Float(section_name, BLINK_PERIOD_STRING, 0.0f);
        dic.m_blinkOnTime = ini->Get_Float(section_name, BLINK_ON_TIME_STRING, 0.0f);

        Init_Type(dic);
        g_Types[entry]->m_name = section_name;
    }
}

void DazzleRenderObjClass::Init_Type(const DazzleInitClass &i)
{
    Init_Shaders();

    if (i.m_type >= g_typeCount) {
        unsigned int new_count = i.m_type + 1;
        DazzleTypeClass **new_types = new DazzleTypeClass *[new_count];
        unsigned int a;

        for (a = 0; a < g_typeCount; ++a) {
            new_types[a] = g_Types[a];
        }

        for (; a < new_count; ++a) {
            new_types[a] = 0;
        }

        delete[] g_Types;
        g_Types = new_types;
        g_typeCount = new_count;
    }

    delete g_Types[i.m_type];
    g_Types[i.m_type] = new DazzleTypeClass(i);
}

void DazzleRenderObjClass::Init_Lensflare(const LensflareInitClass &i)
{
    Init_Shaders();

    if (i.m_type >= g_lensflareCount) {
        unsigned int new_count = i.m_type + 1;
        LensflareTypeClass **new_lensflares = new LensflareTypeClass *[new_count];
        unsigned int a;

        for (a = 0; a < g_lensflareCount; ++a) {
            new_lensflares[a] = g_lensFlares[a];
        }

        for (; a < new_count; ++a) {
            new_lensflares[a] = 0;
        }

        delete[] g_lensFlares;
        g_lensFlares = new_lensflares;
        g_lensflareCount = new_count;
    }

    delete g_lensFlares[i.m_type];
    g_lensFlares[i.m_type] = new LensflareTypeClass(i);
}

void DazzleRenderObjClass::Deinit()
{
    if (g_Types) {
        for (unsigned int a = 0; a < g_typeCount; ++a) {
            delete g_Types[a];
        }
        delete[] g_Types;
    }

    g_Types = nullptr;
    g_typeCount = 0;

    if (g_lensFlares) {
        for (unsigned int a = 0; a < g_lensflareCount; ++a) {
            delete g_lensFlares[a];
        }
        delete[] g_lensFlares;
    }

    g_lensFlares = nullptr;
    g_lensflareCount = 0;
}

void DazzleRenderObjClass::Install_Dazzle_Visibility_Handler(const DazzleVisibilityClass *visibility_handler)
{
    if (visibility_handler == nullptr) {
        _VisibilityHandler = &_DefaultVisibilityHandler;
    } else {
        _VisibilityHandler = visibility_handler;
    }
}

DazzleRenderObjClass::DazzleRenderObjClass(unsigned int t) :
    m_succ(nullptr),
    m_type(t),
    m_currentDazzleIntensity(0.0f),
    m_currentDazzleSize(0.0f),
    m_dazzleColor(1.0f, 1.0f, 1.0f),
    m_haloColor(1.0f, 1.0f, 1.0f),
    m_lensflareIntensity(1.0f),
    m_scale(1.0f),
    m_onList(false),
    m_visibility(0.0f)
{
    if (g_Types && g_Types[t]) {
        m_radius = g_Types[t]->m_radius;
    } else {
        m_radius = 0.0f;
    }

    m_creationTime = W3D::Get_Sync_Time();
}

DazzleRenderObjClass::DazzleRenderObjClass(const char *type_name) :
    m_succ(nullptr),
    m_type(Get_Type_ID(type_name)),
    m_currentDazzleIntensity(0.0f),
    m_currentDazzleSize(0.0f),
    m_dazzleColor(1.0f, 1.0f, 1.0f),
    m_haloColor(1.0f, 1.0f, 1.0f),
    m_lensflareIntensity(1.0f),
    m_scale(1.0f),
    m_onList(false),
    m_visibility(0.0f)
{
    int id = Get_Type_ID(type_name);

    if (g_Types && g_Types[id]) {
        m_radius = g_Types[id]->m_radius;
    } else {
        m_radius = 0.0f;
    }

    m_creationTime = W3D::Get_Sync_Time();
}

DazzleRenderObjClass::DazzleRenderObjClass(const DazzleRenderObjClass &src) :
    m_succ(nullptr),
    m_type(src.m_type),
    m_currentDazzleIntensity(src.m_currentDazzleIntensity),
    m_currentDazzleSize(src.m_currentDazzleSize),
    m_currentDir(src.m_currentDir),
    m_dazzleColor(src.m_dazzleColor),
    m_haloColor(src.m_haloColor),
    m_lensflareIntensity(src.m_lensflareIntensity),
    m_scale(src.m_scale),
    m_onList(false),
    m_visibility(src.m_visibility),
    m_radius(src.m_radius)
{
    m_creationTime = W3D::Get_Sync_Time();
}

DazzleRenderObjClass &DazzleRenderObjClass::operator=(const DazzleRenderObjClass &src)
{
    m_type = src.m_type;
    m_currentDir = src.m_currentDir;
    m_currentDazzleIntensity = src.m_currentDazzleIntensity;
    m_currentDazzleSize = src.m_currentDazzleSize;
    m_dazzleColor = src.m_dazzleColor;
    m_haloColor = src.m_haloColor;
    m_lensflareIntensity = src.m_lensflareIntensity;
    m_visibility = src.m_visibility;
    m_radius = src.m_radius;
    m_creationTime = W3D::Get_Sync_Time();
    m_scale = src.m_scale;
    return *this;
}

void DazzleRenderObjClass::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    sphere.Init(Vector3(0, 0, 0), m_radius * m_scale);
}

void DazzleRenderObjClass::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    box.m_center.Set(0, 0, 0);
    box.m_extent.Set(m_radius, m_radius, m_radius);
    box.m_extent *= m_scale;
}

void DazzleRenderObjClass::Set_Layer(DazzleLayerClass *layer)
{
    if (m_onList) {
        return;
    }

    if (!layer) {
        captainslog_assert(0);
        return;
    }

    if (m_type >= g_typeCount) {
        return;
    }

    m_succ = layer->m_visibleLists[m_type];
    layer->m_visibleLists[m_type] = this;
    m_onList = true;
    Add_Ref();
}

void DazzleRenderObjClass::Set_Current_Dazzle_Layer(DazzleLayerClass *layer)
{
    g_currentDazzleLayer = layer;
}

RenderObjClass *DazzleRenderObjClass::Clone() const
{
    return new DazzleRenderObjClass(*this);
}

void DazzleRenderObjClass::Render(RenderInfoClass &rinfo)
{
#ifdef BUILD_WITH_D3D8
    if (Is_Not_Hidden_At_All() && s_dazzleRenderingEnabled && !DX8Wrapper::Is_Render_To_Texture()) {
        bool is_on = true;
        DazzleInitClass &ic = g_Types[m_type]->m_ic;

        if (ic.m_blinkPeriod > 0.0f) {
            float elapsed_time = ((float)(W3D::Get_Sync_Time() - m_creationTime)) / 1000.0f;
            float wrapped_time = fmodf(elapsed_time, ic.m_blinkPeriod);
            if (wrapped_time > ic.m_blinkOnTime) {
                is_on = false;
            }
        }

        m_visibility = 1.0f;

        if (is_on == false) {
            m_visibility = 0.0f;
        } else {
            Matrix4 view_transform, projection_transform;
            DX8Wrapper::Get_Transform(D3DTS_VIEW, view_transform);
            DX8Wrapper::Get_Transform(D3DTS_PROJECTION, projection_transform);
            Vector3 camera_loc(rinfo.m_camera.Get_Position());
            Vector3 camera_dir(-view_transform[2][0], -view_transform[2][1], -view_transform[2][2]);

            Vector3 loc = Get_Position();
            m_transformedLoc = view_transform * loc;
            m_transformedLoc = projection_transform * m_transformedLoc;
            m_transformedLoc[0] /= m_transformedLoc[3];
            m_transformedLoc[1] /= m_transformedLoc[3];
            m_transformedLoc[2] /= m_transformedLoc[3];
            m_transformedLoc[3] = 1.0f;
            m_currentVloc = Vector3(m_transformedLoc[0], m_transformedLoc[1], m_transformedLoc[2]);

            float dazzle_intensity = 1.0f;
            Vector3 dir;
            dir = camera_loc - loc;
            m_currentDistance = dir.Length2();
            dir.Normalize();

            float dazzle_size;
            m_currentHaloIntensity = 1.0f;
            const DazzleTypeClass *params = g_Types[m_type];
            params->Calculate_Intensities(
                dazzle_intensity, dazzle_size, m_currentHaloIntensity, camera_dir, m_currentDir, dir, m_currentDistance);

            unsigned int time_ms = W3D::Get_Frame_Time();

            if (time_ms == 0) {
                time_ms = 1;
            }

            float weight = pow(params->m_ic.m_historyWeight, time_ms);

            if (dazzle_intensity > 0.0f) {
                m_visibility = _VisibilityHandler->Compute_Dazzle_Visibility(rinfo, this, loc);
                dazzle_intensity *= m_visibility;
            } else {
                m_visibility = 0.0f;
            }

            if (m_visibility == 0.0f) {
                float i = dazzle_intensity * (1.0f - weight) + m_currentDazzleIntensity * weight;
                m_currentDazzleIntensity = i;

                if (m_currentDazzleIntensity < 0.05f) {
                    m_currentDazzleIntensity = 0.0f;
                }

                dazzle_intensity = i;
                float s = dazzle_size * (1.0f - weight) + m_currentDazzleSize * weight;
                m_currentDazzleSize = s;
                dazzle_size = s;
            } else {
                m_currentDazzleIntensity = dazzle_intensity;
                m_currentDazzleSize = dazzle_size;
            }
        }

        if (m_currentDazzleIntensity > 0.0f || m_currentHaloIntensity > 0.0f) {
            captainslog_assert(g_Types[m_type]);
            Set_Layer(g_currentDazzleLayer);
        }
    } else {
        m_visibility = 0.0f;
    }
#endif
}

void DazzleRenderObjClass::Render_Dazzle(CameraClass *camera)
{
#ifdef BUILD_WITH_D3D8
    Matrix4 old_view_transform;
    Matrix4 old_world_transform;
    Matrix4 old_projection_transform;
    Matrix4 view_transform;
    Matrix4 world_transform;
    Matrix4 projection_transform;
    DX8Wrapper::Get_Transform(D3DTS_VIEW, view_transform);
    DX8Wrapper::Get_Transform(D3DTS_WORLD, world_transform);
    DX8Wrapper::Get_Transform(D3DTS_PROJECTION, projection_transform);
    old_view_transform = view_transform;
    old_world_transform = world_transform;
    old_projection_transform = projection_transform;
    Vector3 camera_loc(camera->Get_Position());
    Vector3 camera_dir(-view_transform[2][0], -view_transform[2][1], -view_transform[2][2]);

    int display_width, display_height, display_bits;
    bool windowed;
    W3D::Get_Device_Resolution(display_width, display_height, display_bits, windowed);
    float w = float(display_width);
    float h = float(display_height);
    float screen_x_scale = 1.0f;
    float screen_y_scale = 1.0f;

    if (w > h) {
        screen_y_scale = w / h;
    } else {
        screen_x_scale = h / w;
    }

    float halo_scale_x = g_Types[m_type]->m_ic.m_haloScaleX;
    float halo_scale_y = g_Types[m_type]->m_ic.m_haloScaleY;
    float dazzle_scale_x = m_scale * g_Types[m_type]->m_ic.m_dazzleScaleX;
    float dazzle_scale_y = m_scale * g_Types[m_type]->m_ic.m_dazzleScaleY;

    int vertex_count = 4;

    const DazzleTypeClass *params = g_Types[m_type];

    int halo_vertex_count = 0;
    int dazzle_vertex_count = 0;
    int lensflare_vertex_count = 0;

    Vector3 dl;

    int lens_max_verts = 0;
    LensflareTypeClass *lensflare = DazzleRenderObjClass::Get_Lensflare_Class(g_Types[m_type]->m_lensflareId);

    if (lensflare) {
        lens_max_verts = 4 * lensflare->m_lic.m_flareCount;
    }

    DynamicVBAccessClass vb_access(
        DX8VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, dynamic_fvf_type, vertex_count * 2 + lens_max_verts);
    {
        DynamicVBAccessClass::WriteLockClass lock(&vb_access);
        VertexFormatXYZNDUV2 *verts = lock.Get_Formatted_Vertex_Array();

        float halo_size = 1.0f;

        Vector3 dazzle_dxt(screen_x_scale, 0.0f, 0.0f);
        Vector3 halo_dxt = dazzle_dxt * halo_scale_x;
        dazzle_dxt *= dazzle_scale_x;

        Vector3 dazzle_dyt(0.0f, screen_y_scale, 0.0f);
        Vector3 halo_dyt = dazzle_dyt * halo_scale_y;
        dazzle_dyt *= dazzle_scale_y;

        if (m_currentDazzleIntensity > 0.0f) {
            VertexFormatXYZNDUV2 *vertex = verts;
            dazzle_vertex_count += 4;

            Vector3 col(m_dazzleColor[0] * params->m_ic.m_dazzleColor[0],
                m_dazzleColor[1] * params->m_ic.m_dazzleColor[1],
                m_dazzleColor[2] * params->m_ic.m_dazzleColor[2]);
            col *= m_currentDazzleIntensity;

            if (col[0] > 1.0f) {
                col[0] = 1.0f;
            }

            if (col[1] > 1.0f) {
                col[1] = 1.0f;
            }

            if (col[2] > 1.0f) {
                col[2] = 1.0f;
            }

            unsigned int color = DX8Wrapper::Convert_Color(col, 1.0f);

            dl = m_currentVloc + (dazzle_dxt - dazzle_dyt) * m_currentDazzleSize;
            reinterpret_cast<Vector3 &>(vertex->x) = dl;
            vertex->u1 = 0.0f;
            vertex->v1 = 0.0f;
            vertex->diffuse = color;
            vertex++;

            dl = m_currentVloc + (dazzle_dxt + dazzle_dyt) * m_currentDazzleSize;
            reinterpret_cast<Vector3 &>(vertex->x) = dl;
            vertex->u1 = 1.0f;
            vertex->v1 = 0.0f;
            vertex->diffuse = color;
            vertex++;

            dl = m_currentVloc - (dazzle_dxt - dazzle_dyt) * m_currentDazzleSize;
            reinterpret_cast<Vector3 &>(vertex->x) = dl;
            vertex->u1 = 1.0f;
            vertex->v1 = 1.0f;
            vertex->diffuse = color;
            vertex++;

            dl = m_currentVloc - (dazzle_dxt + dazzle_dyt) * m_currentDazzleSize;
            reinterpret_cast<Vector3 &>(vertex->x) = dl;
            vertex->u1 = 0.0f;
            vertex->v1 = 1.0f;
            vertex->diffuse = color;
        }

        if (m_currentHaloIntensity) {
            VertexFormatXYZNDUV2 *vertex = verts + dazzle_vertex_count;
            halo_vertex_count += 4;

            Vector3 col(m_haloColor[0] * params->m_ic.m_haloColor[0],
                m_haloColor[1] * params->m_ic.m_haloColor[1],
                m_haloColor[2] * params->m_ic.m_haloColor[2]);
            col *= m_currentHaloIntensity;

            if (col[0] > 1.0f) {
                col[0] = 1.0f;
            }

            if (col[1] > 1.0f) {
                col[1] = 1.0f;
            }

            if (col[2] > 1.0f) {
                col[2] = 1.0f;
            }

            unsigned int color = DX8Wrapper::Convert_Color(col, 1.0f);

            dl = m_currentVloc + (halo_dxt - halo_dyt) * halo_size;
            reinterpret_cast<Vector3 &>(vertex->x) = dl;
            vertex->u1 = 0.0f;
            vertex->v1 = 0.0f;
            vertex->diffuse = color;
            vertex++;

            dl = m_currentVloc + (halo_dxt + halo_dyt) * halo_size;
            reinterpret_cast<Vector3 &>(vertex->x) = dl;
            vertex->u1 = 1.0f;
            vertex->v1 = 0.0f;
            vertex->diffuse = color;
            vertex++;

            dl = m_currentVloc - (halo_dxt - halo_dyt) * halo_size;
            reinterpret_cast<Vector3 &>(vertex->x) = dl;
            vertex->u1 = 1.0f;
            vertex->v1 = 1.0f;
            vertex->diffuse = color;
            vertex++;

            dl = m_currentVloc - (halo_dxt + halo_dyt) * halo_size;
            reinterpret_cast<Vector3 &>(vertex->x) = dl;
            vertex->u1 = 0.0f;
            vertex->v1 = 1.0f;
            vertex->diffuse = color;
        }

        if (lensflare && m_currentDazzleIntensity > 0.0f) {
            VertexFormatXYZNDUV2 *vertex = verts + halo_vertex_count + dazzle_vertex_count;

            lensflare->Generate_Vertex_Buffers(vertex,
                lensflare_vertex_count,
                screen_x_scale,
                screen_y_scale,
                m_currentDazzleIntensity * m_lensflareIntensity,
                m_transformedLoc);
            vertex_count += lensflare_vertex_count;
        }
    }

    int dazzle_poly_count = dazzle_vertex_count >> 1;
    int halo_poly_count = halo_vertex_count >> 1;
    int lensflare_poly_count = lensflare_vertex_count >> 1;
    int poly_count = halo_poly_count > dazzle_poly_count ? halo_poly_count : dazzle_poly_count;

    if (lensflare_poly_count > poly_count) {
        poly_count = lensflare_poly_count;
    }

    if (!poly_count) {
        return;
    }

    DX8Wrapper::Set_Vertex_Buffer(vb_access);

    DynamicIBAccessClass ib_access(DX8IndexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, poly_count * 3);
    {
        DynamicIBAccessClass::WriteLockClass lock(&ib_access);
        unsigned short *inds = lock.Get_Index_Array();

        for (int a = 0; a < poly_count / 2; a++) {
            *inds++ = short(4 * a);
            *inds++ = short(4 * a + 1);
            *inds++ = short(4 * a + 2);
            *inds++ = short(4 * a);
            *inds++ = short(4 * a + 2);
            *inds++ = short(4 * a + 3);
        }
    }

    DX8Wrapper::Set_World_Identity();
    DX8Wrapper::Set_View_Identity();
    DX8Wrapper::Set_Transform(D3DTS_PROJECTION, Matrix4(true));

    if (halo_poly_count) {
        DX8Wrapper::Set_Index_Buffer(ib_access, dazzle_vertex_count);
        DX8Wrapper::Set_Shader(g_defaultHaloShader);
        DX8Wrapper::Set_Texture(0, g_Types[m_type]->Get_Halo_Texture());
        SphereClass sphere(Get_Position(), 0.1f);

        DX8Wrapper::Draw_Triangles(0, halo_poly_count, 0, vertex_count);
    }

    if (dazzle_poly_count) {
        DX8Wrapper::Set_Index_Buffer(ib_access, 0);
        DX8Wrapper::Set_Shader(g_defaultDazzleShader);
        DX8Wrapper::Set_Texture(0, g_Types[m_type]->Get_Dazzle_Texture());
        SphereClass sphere(Vector3(0.0f, 0.0f, 0.0f), 0.0f);
        DX8Wrapper::Draw_Triangles(0, dazzle_poly_count, 0, vertex_count);
    }

    if (lensflare_poly_count) {
        DX8Wrapper::Set_Index_Buffer(ib_access, dazzle_vertex_count + halo_vertex_count);
        DX8Wrapper::Set_Shader(g_defaultDazzleShader);
        DX8Wrapper::Set_Texture(0, lensflare->Get_Texture());
        SphereClass sphere(Vector3(0.0f, 0.0f, 0.0f), 0.0f);
        DX8Wrapper::Draw_Triangles(0, lensflare_poly_count, 0, vertex_count);
    }

    DX8Wrapper::Set_Transform(D3DTS_PROJECTION, old_projection_transform);
    DX8Wrapper::Set_Transform(D3DTS_VIEW, old_view_transform);
    DX8Wrapper::Set_Transform(D3DTS_WORLD, old_world_transform);
#endif
}

void DazzleRenderObjClass::Set_Transform(const Matrix3D &m)
{
    RenderObjClass::Set_Transform(m);

    if (m_type < g_typeCount) {
        Matrix3D::Rotate_Vector(m, g_Types[m_type]->m_ic.m_dazzleDirection, &m_currentDir);
    }
}

unsigned int DazzleRenderObjClass::Get_Type_ID(const char *name)
{
    for (unsigned int a = 0; a < g_typeCount; ++a) {
        if (g_Types[a] && g_Types[a]->m_name == name)
            return a;
    }

    return UINT_MAX;
}

const char *DazzleRenderObjClass::Get_Type_Name(unsigned int id)
{
    if ((id < g_typeCount) && (id >= 0)) {
        return g_Types[id]->m_name;
    } else {
        return "DEFAULT";
    }
}

DazzleTypeClass *DazzleRenderObjClass::Get_Type_Class(unsigned int id)
{
    if (id >= g_typeCount) {
        return nullptr;
    }

    return g_Types[id];
}

unsigned int DazzleRenderObjClass::Get_Lensflare_ID(const char *name)
{
    for (unsigned int a = 0; a < g_lensflareCount; ++a) {
        if (g_lensFlares[a] && g_lensFlares[a]->m_name == name)
            return a;
    }

    return UINT_MAX;
}

LensflareTypeClass *DazzleRenderObjClass::Get_Lensflare_Class(unsigned int id)
{
    if (id >= g_lensflareCount) {
        return nullptr;
    }

    return g_lensFlares[id];
}

void DazzleRenderObjClass::Vis_Render_Dazzle(SpecialRenderInfoClass &rinfo)
{
    captainslog_dbgassert(0, "vis not supported");
}

void DazzleRenderObjClass::Special_Render(SpecialRenderInfoClass &rinfo)
{
    if (rinfo.m_renderType == SpecialRenderInfoClass::RENDER_VIS) {
        Vis_Render_Dazzle(rinfo);
    }
}

#ifdef GAME_DLL
const PersistFactoryClass &DazzleRenderObjClass::Get_Factory() const
{
    // return _DazzleRenderObjFactory;
    captainslog_dbgassert(false, "Unimplemented code called!");

    // temp
    static class PersistFactoryClass *p;
    return *p;
}
#endif

DazzleLayerClass::DazzleLayerClass() : m_visibleLists(nullptr)
{
    if (g_typeCount != 0) {
        captainslog_assert(g_typeCount);
        m_visibleLists = new DazzleRenderObjClass *[g_typeCount];

        for (unsigned int i = 0; i < g_typeCount; i++) {
            m_visibleLists[i] = nullptr;
        }
    }
}

DazzleLayerClass::~DazzleLayerClass()
{
    captainslog_assert(g_typeCount);

    for (unsigned int i = 0; i < g_typeCount; i++) {
        Clear_Visible_List(i);
    }

    delete[] m_visibleLists;
}

void DazzleLayerClass::Render(CameraClass *camera)
{
    if (!camera) {
        return;
    }

    camera->Apply();
    unsigned int time_ms = W3D::Get_Frame_Time();

    if (time_ms == 0) {
        time_ms = 1;
    }

    DX8Wrapper::Set_Material(nullptr);

    for (unsigned int type = 0; type < g_typeCount; ++type) {
        if (!g_Types[type]) {
            continue;
        }

        int count = Get_Visible_Item_Count(type);

        if (!count) {
            continue;
        }

        DazzleRenderObjClass *n = m_visibleLists[type];

        while (n) {
            n->Render_Dazzle(camera);
            n = n->Succ();
        }

        Clear_Visible_List(type);
    }
}

int DazzleLayerClass::Get_Visible_Item_Count(unsigned int type) const
{
    if (type >= g_typeCount) {
        captainslog_assert(0);
        return 0;
    }

    int count = 0;

    DazzleRenderObjClass *n = m_visibleLists[type];

    while (n) {
        count++;
        n = n->Succ();
    }

    return count;
}

void DazzleLayerClass::Clear_Visible_List(unsigned int type)
{
    if (type >= g_typeCount) {
        captainslog_assert(0);
        return;
    }

    DazzleRenderObjClass *n = m_visibleLists[type];

    while (n) {
        n->Release_Ref();
        n->m_onList = false;
        n = n->Succ();
    }

    m_visibleLists[type] = nullptr;
}

float DazzleVisibilityClass::Compute_Dazzle_Visibility(
    RenderInfoClass &rinfo, DazzleRenderObjClass *dazzle, const Vector3 &point) const
{
    SceneClass *scene = dazzle->Get_Scene();
    RenderObjClass *container = dazzle->Get_Container();

    while ((scene == nullptr) && (container != nullptr)) {
        scene = container->Get_Scene();
        container = container->Get_Container();
    }

    if (scene != nullptr) {
        float value = scene->Compute_Point_Visibility(rinfo, point);
        scene->Release_Ref();
        return value;
    } else {
        return 1.0f;
    }
}

RenderObjClass *DazzlePrototypeClass::Create()
{
    return new DazzleRenderObjClass(m_dazzleType);
}

W3DErrorType DazzlePrototypeClass::Load_W3D(ChunkLoadClass &cload)
{
    StringClass dazzle_type;

    while (cload.Open_Chunk()) {
        switch (cload.Cur_Chunk_ID()) {
            case W3D_CHUNK_DAZZLE_NAME:
                cload.Read(m_name.Get_Buffer(cload.Cur_Chunk_Length()), cload.Cur_Chunk_Length());
            case W3D_CHUNK_DAZZLE_TYPENAME:
                cload.Read(dazzle_type.Get_Buffer(cload.Cur_Chunk_Length()), cload.Cur_Chunk_Length());
            default:
                break;
        }
        cload.Close_Chunk();
    }

    m_dazzleType = DazzleRenderObjClass::Get_Type_ID(dazzle_type);

    if (m_dazzleType == UINT_MAX) {
        m_dazzleType = 0;
    }

    return W3D_ERROR_OK;
}

PrototypeClass *DazzleLoaderClass::Load_W3D(ChunkLoadClass &cload)
{
    DazzlePrototypeClass *new_proto = new DazzlePrototypeClass;
    new_proto->Load_W3D(cload);
    return new_proto;
}
