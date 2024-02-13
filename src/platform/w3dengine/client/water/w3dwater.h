/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Water Render Object
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
#include "asciistring.h"
#include "gametype.h"
#include "rendobj.h"
#include "shader.h"
#include "snapshot.h"
#include "vector3.h"

class DX8IndexBufferClass;
class SceneClass;
class VertexMaterialClass;
class LightClass;
class TextureClass;
class WaterTracksRenderSystem;
class PolygonTrigger;

struct IDirect3DDevice8;
struct IDirect3DVertexBuffer8;
struct IDirect3DIndexBuffer8;
struct IDirect3DTexture8;
struct VertexFormatXYZDUV2;

class WaterRenderObjClass : public SnapShot, public RenderObjClass
{
    ALLOW_HOOKING

public:
    enum WaterType
    {
        WATER_TYPE_0_TRANSLUCENT = 0,
        WATER_TYPE_1_FB_REFLECTION,
        WATER_TYPE_2_PVSHADER,
        WATER_TYPE_3_GRIDMESH,
        WATER_TYPE_MAX,
    };

    struct WaterMeshData
    {
        float height;
        float velocity;
        unsigned char status;
        unsigned char preferred_height;
    };

    struct Setting
    {
        TextureClass *sky_texture;
        TextureClass *water_texture;
        int water_repeat_count;
        float sky_texels_per_unit;
        unsigned long vertex00_diffuse;
        unsigned long vertex10_diffuse;
        unsigned long vertex11_diffuse;
        unsigned long vertex01_diffuse;
        unsigned long water_diffuse;
        unsigned long transparent_water_diffuse;
        float u_scroll_per_ms;
        float v_scroll_per_ms;
    };

    struct SEA_PATCH_VERTEX
    {
        float x;
        float y;
        float z;
        unsigned int c;
        float tu;
        float tv;
    };

    WaterRenderObjClass();

#ifdef GAME_DLL
    WaterRenderObjClass *Hook_Ctor();
#endif

    virtual ~WaterRenderObjClass() override;
    virtual RenderObjClass *Clone() const override;
    virtual int Class_ID() const override;
    virtual void Render(RenderInfoClass &rinfo) override;
    virtual void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
    virtual void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;
    virtual int Get_Sort_Level() const override;
    virtual void Set_Sort_Level(int level) override;

    void Re_Acquire_Resources();
    void Release_Resources();
    void Add_Velocity(float world_x, float world_y, float z_velocity, float preferred_height);
    void Change_Grid_Height(float wx, float wy, float delta);
    void Enable_Water_Grid(bool state);
    void Get_Grid_Resolution(float *grid_cells_x, float *grid_cells_y, float *cell_size);
    void Get_Grid_Transform(Matrix3D *transform);
    float Get_Water_Height(float x, float y);
    int Init(float water_level, float dx, float dy, SceneClass *parent_scene, WaterType type);
    void Load();
    void Render_Water();
    void Replace_Skybox_Texture(Utf8String const &oldname, Utf8String const &newname);
    void Reset();
    void Set_Grid_Change_Attenuation_Factors(float att_0, float att_1, float att_2, float range);
    void Set_Grid_Height_Clamps(float min_z, float max_z);
    void Set_Grid_Resolution(float grid_cells_x, float grid_cells_y, float cell_size);
    void Set_Grid_Transform(Matrix3D const *transform);
    void Set_Grid_Transform(float angle, float x, float y, float z);
    void Set_Time_Of_Day(TimeOfDayType tod);
    void Update();
    void Update_Map_Overrides();
    void Update_Render_Target_Textures(CameraClass *cam);
    void Toggle_Cloud_Layer(bool toggle) { m_useCloudLayer = toggle; }
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}
    bool World_To_Grid_Space(float wx, float wy, float &gx, float &gy);
    void Get_Grid_Vertex_Height(int x, int y, float *height);

protected:
    void Draw_River_Water(PolygonTrigger *ptrig);
    void Draw_Sea(RenderInfoClass &rinfo);
    void Draw_Trapezoid_Water(Vector3 *const points);
#ifdef BUILD_WITH_D3D8
    HRESULT Generate_Index_Buffer(int size_x, int size_y);
    HRESULT Generate_Vertex_Buffer(int size_x, int size_y, int vertex_size, bool do_static);
#endif
    bool Get_Clipped_Water_Plane(CameraClass *cam, AABoxClass *box);
    long Init_Bump_Map(IDirect3DTexture8 **tex, TextureClass *bump_source);
    void Load_Setting(Setting *setting, TimeOfDayType tod);
    void Render_Mirror(CameraClass *cam);
    void Render_Sky();
    void Render_Sky_Body(Matrix3D *mat);
    void Render_Water_Mesh();
    void Setup_Flat_Water_Shader();
    void Setup_Jba_Water_Shader();

    DX8IndexBufferClass *m_indexBuffer;
    SceneClass *m_parentScene;
    ShaderClass m_shaderClass;
    VertexMaterialClass *m_vertexMaterialClass;
    VertexMaterialClass *m_meshVertexMaterialClass;
    LightClass *m_meshLight;
    TextureClass *m_alphaClippingTexture;
    float m_dx;
    float m_dy;
    Vector3 m_planeNormal;
    float m_planeDistance;
    float m_level;
    float m_uOffset;
    float m_vOffset;
    float m_uScrollPerMs;
    float m_vScrollPerMs;
    int m_lastUpdateTime;
    bool m_useCloudLayer;
    WaterType m_waterType;
    int m_sortLevel;
    IDirect3DDevice8 *m_pDev;
    IDirect3DVertexBuffer8 *m_vertexBufferD3D;
    IDirect3DIndexBuffer8 *m_indexBufferD3D;
    int m_vertexBufferD3DOffset;
    unsigned long m_dwWavePixelShader;
    unsigned long m_dwWaveVertexShader;
    int m_numVertices;
    int m_numIndices;
    IDirect3DTexture8 *m_pBumpTexture[32];
    IDirect3DTexture8 *m_pBumpTexture2[32];
    int m_iBumpFrame;
    float m_fBumpScale;
    TextureClass *m_pReflectionTexture;
    RenderObjClass *m_skyBox;
    WaterTracksRenderSystem *m_waterTrackSystem;
    WaterMeshData *m_meshData;
    int m_meshDataCount;
    bool m_meshInMotion;
    bool m_doWaterGrid;
    Vector2 m_gridDirectionX;
    Vector2 m_gridDirectionY;
    Vector2 m_gridOrigin;
    float m_gridWidth;
    float m_gridHeight;
    int m_minGridHeight;
    int m_maxGridHeight;
    float m_gridChangeMaxRange;
    int m_gridChangeAtt0;
    int m_gridChangeAtt1;
    int m_gridChangeAtt2;
    float m_gridCellSize;
    int m_gridCellsX;
    int m_gridCellsY;
    float m_riverVOrigin;
    TextureClass *m_riverTexture;
    TextureClass *m_unkTexture;
    TextureClass *m_waterNoiseTexture;
    unsigned long m_waterPixelShader;
    unsigned long m_riverWaterPixelShader;
    unsigned long m_trapezoidWaterPixelShader;
    TextureClass *m_waterSparklesTexture;
    float m_riverXOffset;
    float m_riverYOffset;
    bool m_drawingRiver;
    bool m_disableRiver;
    TextureClass *m_riverAlphaEdge;
    TimeOfDayType m_tod;
    Setting m_settings[TIME_OF_DAY_COUNT];
};

#ifdef GAME_DLL
extern WaterRenderObjClass *&g_theWaterRenderObj;
#else
extern WaterRenderObjClass *g_theWaterRenderObj;
#endif
