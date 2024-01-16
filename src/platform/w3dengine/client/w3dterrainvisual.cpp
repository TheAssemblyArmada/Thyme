/**
/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Terrain Visual
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dterrainvisual.h"
#include "cachedfileinputstream.h"
#include "colorspace.h"
#include "coltest.h"
#include "drawable.h"
#include "globaldata.h"
#include "heightmap.h"
#include "mapobject.h"
#include "object.h"
#include "staticnamekey.h"
#include "terraintypes.h"
#include "w3ddisplay.h"
#include "w3dmodeldraw.h"
#include "w3dscene.h"
#include "w3dshadow.h"
#include "w3dsmudge.h"
#include "w3dterraintracks.h"
#include "w3dwater.h"
#include "xfer.h"

W3DTerrainVisual::W3DTerrainVisual() :
    m_baseHeightMap(nullptr), m_waterRenderObj(nullptr), m_heightMap(nullptr), m_waterGridEnabled(false)
{
    g_theWaterRenderObj = nullptr;
}

W3DTerrainVisual::~W3DTerrainVisual()
{
    if (m_baseHeightMap == g_theTerrainRenderObject) {
        g_theTerrainRenderObject = nullptr;
    }

    if (g_theTerrainTracksRenderObjClassSystem != nullptr) {
        delete g_theTerrainTracksRenderObjClassSystem;
        g_theTerrainTracksRenderObjClassSystem = nullptr;
    }

    if (g_theW3DShadowManager != nullptr) {
        delete g_theW3DShadowManager;
        g_theW3DShadowManager = nullptr;
    }

    if (g_theSmudgeManager != nullptr) {
        delete g_theSmudgeManager;
        g_theSmudgeManager = nullptr;
    }

    Ref_Ptr_Release(m_waterRenderObj);
    g_theWaterRenderObj = nullptr;
    Ref_Ptr_Release(m_baseHeightMap);
    Ref_Ptr_Release(m_heightMap);
}

void W3DTerrainVisual::Init()
{
    TerrainVisual::Init();

    m_baseHeightMap = new HeightMapRenderObjClass();
    m_baseHeightMap->Set_Collision_Type(COLLISION_TYPE_ALL | COLLISION_TYPE_0);
    g_theTerrainRenderObject = m_baseHeightMap;

    g_theTerrainTracksRenderObjClassSystem = new TerrainTracksRenderObjClassSystem();
    g_theTerrainTracksRenderObjClassSystem->Init(W3DDisplay::s_3DScene);

    g_theW3DShadowManager = new W3DShadowManager();
    g_theW3DShadowManager->Init();

    m_waterRenderObj = new WaterRenderObjClass();
    g_theWaterRenderObj = m_waterRenderObj;
    m_waterRenderObj->Init(g_theWriteableGlobalData->m_waterPositionZ,
        g_theWriteableGlobalData->m_waterExtentX,
        g_theWriteableGlobalData->m_waterExtentY,
        W3DDisplay::s_3DScene,
        static_cast<WaterRenderObjClass::WaterType>(g_theWriteableGlobalData->m_waterType));
    m_waterRenderObj->Set_Position(Vector3(g_theWriteableGlobalData->m_waterPositionX,
        g_theWriteableGlobalData->m_waterPositionY,
        g_theWriteableGlobalData->m_waterPositionZ));

    g_theSmudgeManager = new W3DSmudgeManager();
    g_theSmudgeManager->Init();

    if (g_theWriteableGlobalData->m_waterType != WaterRenderObjClass::WATER_TYPE_1_FB_REFLECTION) {
        W3DDisplay::s_3DScene->Add_Render_Object(m_waterRenderObj);
    }

    m_waterRenderObj->Toggle_Cloud_Layer(g_theWriteableGlobalData->m_useCloudPlane);
    g_theTerrainVisual->Set_Water_Grid_Height_Clamps(nullptr,
        g_theWriteableGlobalData->m_vertexWaterHeightClampLow[0],
        g_theWriteableGlobalData->m_vertexWaterHeightClampHigh[0]);
    g_theTerrainVisual->Set_Water_Transform(nullptr,
        g_theWriteableGlobalData->m_vertexWaterAngle[0],
        g_theWriteableGlobalData->m_vertexWaterXPos[0],
        g_theWriteableGlobalData->m_vertexWaterYPos[0],
        g_theWriteableGlobalData->m_vertexWaterZPos[0]);
    g_theTerrainVisual->Set_Water_Grid_Resolution(nullptr,
        g_theWriteableGlobalData->m_vertexWaterXGridCells[0],
        g_theWriteableGlobalData->m_vertexWaterYGridCells[0],
        g_theWriteableGlobalData->m_vertexWaterGridSize[0]);
    g_theTerrainVisual->Set_Water_Attenuation_Factors(nullptr,
        g_theWriteableGlobalData->m_vertexWaterAttenuationA[0],
        g_theWriteableGlobalData->m_vertexWaterAttenuationB[0],
        g_theWriteableGlobalData->m_vertexWaterAttenuationC[0],
        g_theWriteableGlobalData->m_vertexWaterAttenuationRange[0]);
    m_waterGridEnabled = false;
}

void W3DTerrainVisual::Reset()
{
    TerrainVisual::Reset();
    m_baseHeightMap->Reset();

    if (g_theW3DShadowManager != nullptr) {
        g_theW3DShadowManager->Reset();
    }

    if (g_theSmudgeManager != nullptr) {
        g_theSmudgeManager->Reset();
    }

    if (g_theTerrainTracksRenderObjClassSystem != nullptr) {
        g_theTerrainTracksRenderObjClassSystem->Reset();
    }

    if (m_waterRenderObj != nullptr) {
        for (int i = 0; i < 5; i++) {
            if (m_currentSkyboxTextures[i].Compare(m_initialSkyboxTextures[i]) != 0) {
                m_waterRenderObj->Replace_Skybox_Texture(m_currentSkyboxTextures[i], m_initialSkyboxTextures[i]);
                m_currentSkyboxTextures[i] = m_initialSkyboxTextures[i];
            }
        }

        m_waterRenderObj->Reset();
    }
}

void W3DTerrainVisual::Update()
{
    TerrainVisual::Update();

    if (m_waterRenderObj != nullptr) {
        m_waterRenderObj->Update();
    }
}

void W3DTerrainVisual::CRC_Snapshot(Xfer *xfer)
{
    TerrainVisual::CRC_Snapshot(xfer);
}

void W3DTerrainVisual::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 3;
    xfer->xferVersion(&version, 3);
    TerrainVisual::Xfer_Snapshot(xfer);
    bool enabled = m_waterGridEnabled;
    xfer->xferBool(&enabled);
    captainslog_relassert(enabled == m_waterGridEnabled, 6, "Water grid enabled doesn't match");

    if (enabled) {
        m_waterRenderObj->Xfer_Snapshot(xfer);
    }

    if (version > 1) {
        unsigned char *data = m_heightMap->Get_Data_Ptr();
        int size = m_heightMap->Get_X_Extent() * m_heightMap->Get_Y_Extent();
        int min_size = size;
        xfer->xferInt(&size);

        if (min_size > size) {
            min_size = size;
        }

        xfer->xferUser(data, min_size);

        if (xfer->Get_Mode() == XFER_LOAD) {
            m_baseHeightMap->Static_Lighting_Changed();
        }
    }

    if (version > 2) {
        m_baseHeightMap->Xfer_Snapshot(xfer);
    }
}

void W3DTerrainVisual::Load_Post_Process()
{
    TerrainVisual::Load_Post_Process();
}

bool W3DTerrainVisual::Load(Utf8String filename)
{
    if (!TerrainVisual::Load(filename)) {
        return false;
    }

    CachedFileInputStream stream;

    if (stream.Open(filename)) {
        if (m_baseHeightMap != nullptr) {
            Ref_Ptr_Release(m_heightMap);
            m_heightMap = new WorldHeightMap(&stream, false);

            for (MapObject *obj = MapObject::Get_First_Map_Object(); obj != nullptr; obj = obj->Get_Next()) {
                if (obj->Is_Light()) {
                    Coord3D loc = *obj->Get_Location();

                    if (loc.z < 0.0f) {
                        loc.z = m_baseHeightMap->Get_Height_Map_Height(loc.x, loc.y, nullptr);
                        loc.z += obj->Get_Properties()->Get_Real(g_lightHeightAboveTerrainKey);

                        LightClass *light = new LightClass();
                        int ambient = obj->Get_Properties()->Get_Int(g_lightAmbientColor);
                        Vector3 ambient_color;
                        Color_To_RGB(ambient_color, ambient);
                        light->Set_Ambient(ambient_color);

                        int diffuse = obj->Get_Properties()->Get_Int(g_lightDiffuseColor);
                        Vector3 diffuse_color;
                        Color_To_RGB(diffuse_color, diffuse);
                        light->Set_Ambient(diffuse_color);

                        light->Set_Position(Vector3(loc.x, loc.y, loc.z));
                        light->Set_Far_Attenuation_Range(obj->Get_Properties()->Get_Real(g_lightInnerRadius),
                            obj->Get_Properties()->Get_Real(g_lightOuterRadius));
                        W3DDisplay::s_3DScene->Add_Render_Object(light);
                        light->Release_Ref();
                    }
                }
            }

            RefMultiListIterator<RenderObjClass> *iterator = W3DDisplay::s_3DScene->Create_Lights_Iterator();
            m_baseHeightMap->Init_Height_Data(
                m_heightMap->Get_Draw_Width(), m_heightMap->Get_Draw_Height(), m_heightMap, iterator, true);

            if (iterator != nullptr) {
                W3DDisplay::s_3DScene->Destroy_Lights_Iterator(iterator);
            }

            W3DDisplay::s_3DScene->Add_Render_Object(m_baseHeightMap);

            if (m_waterRenderObj != nullptr) {
                W3DDisplay::s_3DScene->Add_Render_Object(m_waterRenderObj);
                m_waterRenderObj->Enable_Water_Grid(false);
                m_waterRenderObj->Update_Map_Overrides();
            }

            for (MapObject *obj = MapObject::Get_First_Map_Object(); obj != nullptr; obj = obj->Get_Next()) {
                if (obj->Is_Scorch()) {
                    Coord3D loc = *obj->Get_Location();
                    m_baseHeightMap->Add_Scorch(Vector3(loc.x, loc.y, loc.z),
                        obj->Get_Properties()->Get_Real(g_objectRadius),
                        static_cast<Scorches>(obj->Get_Properties()->Get_Int(g_scorchTypeKey)));
                }
            }

            if (m_waterRenderObj != nullptr) {
                m_waterRenderObj->Load();
            }
        }

        return true;
    } else {
        Ref_Ptr_Release(m_baseHeightMap);
    }

    return false;
}

void W3DTerrainVisual::Get_Terrain_Color_At(float x, float y, RGBColor *color)
{
    if (m_heightMap != nullptr) {
        m_heightMap->Get_Terrain_Color_At(x, y, color);
    }
}

TerrainType *W3DTerrainVisual::Get_Terrain_Tile(float x, float y)
{
    if (m_heightMap == nullptr) {
        return nullptr;
    }

    return g_theTerrainTypes->Find_Terrain(m_heightMap->Get_Terrain_Name_At(x, y));
}

bool W3DTerrainVisual::Intersect_Terrain(Coord3D *ray_start, Coord3D *ray_end, Coord3D *result)
{
    if (ray_start == nullptr || ray_end == nullptr || m_baseHeightMap == nullptr) {
        return false;
    }

    LineSegClass line;
    CastResultStruct res;
    Vector3 start(ray_start->x, ray_start->y, ray_start->z);
    Vector3 end(ray_end->x, ray_end->y, ray_end->z);
    line.Set(start, end);
    RayCollisionTestClass ray(line, &res);

    bool ret = m_baseHeightMap->Cast_Ray(ray);

    if (!ret) {
        return false;
    }

    if (result != nullptr) {
        result->x = ray.m_result->contact_point.X;
        result->y = ray.m_result->contact_point.Y;
        result->z = ray.m_result->contact_point.Z;
    }

    return ret;
}

void W3DTerrainVisual::Enable_Water_Grid(bool enable)
{
    m_waterGridEnabled = enable;

    if (m_waterRenderObj != nullptr) {
        m_waterRenderObj->Enable_Water_Grid(enable);
    }
}

void W3DTerrainVisual::Set_Water_Grid_Height_Clamps(const WaterHandle *handle, float min_z, float max_z)
{
    if (m_waterRenderObj != nullptr) {
        m_waterRenderObj->Set_Grid_Height_Clamps(min_z, max_z);
    }
}

void W3DTerrainVisual::Set_Water_Attenuation_Factors(
    const WaterHandle *handle, float att_0, float att_1, float att_2, float range)
{
    if (m_waterRenderObj != nullptr) {
        m_waterRenderObj->Set_Grid_Change_Attenuation_Factors(att_0, att_1, att_2, range);
    }
}

void W3DTerrainVisual::Set_Water_Transform(const WaterHandle *handle, float angle, float x, float y, float z)
{
    if (m_waterRenderObj != nullptr) {
        m_waterRenderObj->Set_Grid_Transform(angle, x, y, z);
    }
}

void W3DTerrainVisual::Set_Water_Transform(const Matrix3D *transform)
{
    if (m_waterRenderObj != nullptr) {
        m_waterRenderObj->Set_Grid_Transform(transform);
    }
}

void W3DTerrainVisual::Get_Water_Transform(const WaterHandle *handle, Matrix3D *transform)
{
    if (m_waterRenderObj != nullptr) {
        m_waterRenderObj->Get_Grid_Transform(transform);
    }
}

void W3DTerrainVisual::Set_Water_Grid_Resolution(
    const WaterHandle *handle, float grid_cells_x, float grid_cells_y, float cell_size)
{
    if (m_waterRenderObj != nullptr) {
        m_waterRenderObj->Set_Grid_Resolution(grid_cells_x, grid_cells_y, cell_size);
    }
}

void W3DTerrainVisual::Get_Water_Grid_Resolution(
    const WaterHandle *handle, float *grid_cells_x, float *grid_cells_y, float *cell_size)
{
    if (m_waterRenderObj != nullptr) {
        m_waterRenderObj->Get_Grid_Resolution(grid_cells_x, grid_cells_y, cell_size);
    }
}

void W3DTerrainVisual::Change_Water_Height(float wx, float wy, float delta)
{
    if (m_waterRenderObj != nullptr) {
        m_waterRenderObj->Change_Grid_Height(wx, wy, delta);
    }
}

void W3DTerrainVisual::Add_Water_Velocity(float world_x, float world_y, float z_velocity, float preferred_height)
{
    if (m_waterRenderObj != nullptr) {
        m_waterRenderObj->Add_Velocity(world_x, world_y, z_velocity, preferred_height);
    }
}

bool W3DTerrainVisual::Get_Water_Grid_Height(float x, float y, float *height)
{
    if (!m_waterGridEnabled) {
        return false;
    }

    if (m_waterRenderObj == nullptr) {
        return false;
    }

    float gx;
    float gy;

    if (!m_waterRenderObj->World_To_Grid_Space(x, y, gx, gy)) {
        return false;
    }

    m_waterRenderObj->Get_Grid_Vertex_Height(GameMath::Fast_To_Int_Truncate(gx), GameMath::Fast_To_Int_Truncate(gy), height);
    return true;
}

void W3DTerrainVisual::Set_Terrain_Tracks_Detail()
{
    if (g_theTerrainTracksRenderObjClassSystem != nullptr) {
        g_theTerrainTracksRenderObjClassSystem->Set_Detail();
    }
}

void W3DTerrainVisual::Set_Shore_Line_Detail()
{
    if (m_baseHeightMap != nullptr) {
        m_baseHeightMap->Set_Shore_Line_Detail();
    }
}

void W3DTerrainVisual::Add_Faction_Bib(Object *object, bool highlighted, float width)
{
    if (m_heightMap != nullptr) {
        float exit_width = object->Get_Template()->Get_Factory_Exit_Width();
        float bib_width = width + object->Get_Template()->Get_Factory_Extra_Bib_Width();
        GeometryInfo info = object->Get_Geometry_Info();
        float major_radius = info.Get_Major_Radius();
        float minor_radius = info.Get_Minor_Radius();

        if (info.Get_Type() != GEOMETRY_BOX) {
            minor_radius = major_radius;
        }

        Vector3 points[4];
        points[0].X = 0.0f - (bib_width + major_radius);
        points[0].Y = 0.0f - (minor_radius + bib_width);
        points[0].Z = 0.0f;

        points[1].X = major_radius + exit_width + bib_width;
        points[1].Y = 0.0f - (minor_radius + bib_width);
        points[1].Z = 0.0f;

        points[2].X = points[1].X;
        points[2].Y = minor_radius + bib_width;
        points[2].Z = 0.0f;

        points[3].X = points[0].X;
        points[3].Y = minor_radius + bib_width;
        points[3].Z = 0.0f;

        Matrix3D::Transform_Vector(*object->Get_Transform_Matrix(), points[0], &points[0]);
        Matrix3D::Transform_Vector(*object->Get_Transform_Matrix(), points[1], &points[1]);
        Matrix3D::Transform_Vector(*object->Get_Transform_Matrix(), points[2], &points[2]);
        Matrix3D::Transform_Vector(*object->Get_Transform_Matrix(), points[3], &points[3]);

        m_baseHeightMap->Add_Terrain_Bib(points, object->Get_ID(), highlighted);
    }
}

void W3DTerrainVisual::Remove_Faction_Bib(Object *object)
{
    if (m_baseHeightMap != nullptr) {
        m_baseHeightMap->Remove_Terrain_Bib(object->Get_ID());
    }
}

void W3DTerrainVisual::Add_Faction_Bib_Drawable(Drawable *drawable, bool highlighted, float width)
{
    if (m_heightMap != nullptr) {
        const Matrix3D *transform = drawable->Get_Transform_Matrix();
        float exit_width = drawable->Get_Template()->Get_Factory_Exit_Width();
        float bib_width = width + drawable->Get_Template()->Get_Factory_Extra_Bib_Width();
        GeometryInfo info = drawable->Get_Template()->Get_Template_Geometry_Info();
        float major_radius = info.Get_Major_Radius();
        float minor_radius = info.Get_Minor_Radius();

        if (info.Get_Type() != GEOMETRY_BOX) {
            minor_radius = major_radius;
        }

        Vector3 points[4];
        points[0].X = 0.0f - (bib_width + major_radius);
        points[0].Y = 0.0f - (minor_radius + bib_width);
        points[0].Z = 0.0f;

        points[1].X = major_radius + exit_width + bib_width;
        points[1].Y = 0.0f - (minor_radius + bib_width);
        points[1].Z = 0.0f;

        points[2].X = points[1].X;
        points[2].Y = minor_radius + bib_width;
        points[2].Z = 0.0f;

        points[3].X = points[0].X;
        points[3].Y = minor_radius + bib_width;
        points[3].Z = 0.0f;

        Matrix3D::Transform_Vector(*transform, points[0], &points[0]);
        Matrix3D::Transform_Vector(*transform, points[1], &points[1]);
        Matrix3D::Transform_Vector(*transform, points[2], &points[2]);
        Matrix3D::Transform_Vector(*transform, points[3], &points[3]);

        m_baseHeightMap->Add_Terrain_Bib_Drawable(points, drawable->Get_ID(), highlighted);
    }
}

void W3DTerrainVisual::Remove_Faction_Bib_Drawable(Drawable *drawable)
{
    if (m_baseHeightMap != nullptr) {
        m_baseHeightMap->Remove_Terrain_Bib_Drawable(drawable->Get_ID());
    }
}

void W3DTerrainVisual::Remove_All_Bibs()
{
    if (m_baseHeightMap != nullptr) {
        m_baseHeightMap->Remove_All_Terrain_Bibs();
    }
}

void W3DTerrainVisual::Remove_Bib_Highlighting()
{
    if (m_baseHeightMap != nullptr) {
        m_baseHeightMap->Remove_Terrain_Bib_Highlighting();
    }
}

void W3DTerrainVisual::Remove_Trees_And_Props_For_Construction(const Coord3D *pos, const GeometryInfo &geom, float angle)
{
    if (m_baseHeightMap != nullptr) {
        m_baseHeightMap->Remove_Trees_And_Props_For_Construction(pos, geom, angle);
    }
}

void W3DTerrainVisual::Add_Prop(const ThingTemplate *tmplate, const Coord3D *pos, float angle)
{
    BitFlags<MODELCONDITION_COUNT> flags;

    if (g_theWriteableGlobalData->m_weather == WEATHER_SNOWY) {
        flags.Set(MODELCONDITION_SNOW, true);
    }

    if (g_theWriteableGlobalData->m_timeOfDay == TIME_OF_DAY_NIGHT) {
        flags.Set(MODELCONDITION_NIGHT, true);
    }

    Utf8String name;
    float scale = tmplate->Get_Asset_Scale();
    const ModuleData *data = tmplate->Get_Draw_Modules()->Get_Nth_Data(0);

    if (data != nullptr) {
        const W3DModelDrawModuleData *draw_data = data->Get_As_W3D_Model_Draw_Module_Data();

        if (draw_data != nullptr) {
            name = draw_data->Get_Best_Model_Name_For_WB(flags);
        }
    }

    if (m_baseHeightMap != nullptr && name.Is_Not_Empty()) {
        m_baseHeightMap->Add_Prop(1, *pos, angle, scale, name);
    }
}

void W3DTerrainVisual::Set_Raw_Map_Height(const ICoord2D *pos, int height)
{
    if (m_heightMap != nullptr) {
        int x = m_heightMap->Get_Border_Size() + pos->x;
        int y = m_heightMap->Get_Border_Size() + pos->y;
        int map_height = m_heightMap->Get_Height(x, y);

        if (map_height > height) {
            m_heightMap->Set_Height(x, y, height);
            m_baseHeightMap->Static_Lighting_Changed();
        }
    }
}

int W3DTerrainVisual::Get_Raw_Map_Height(const ICoord2D *pos)
{
    if (m_heightMap == nullptr) {
        return 0;
    }

    return m_heightMap->Get_Height(pos->x, pos->y);
}

WorldHeightMap *W3DTerrainVisual::Get_Logic_Height_Map()
{
    return m_heightMap;
}

WorldHeightMap *W3DTerrainVisual::Get_Client_Height_Map()
{
    return m_heightMap;
}

void W3DTerrainVisual::Replace_Skybox_Textures(const Utf8String **oldnames, const Utf8String **newnames)
{
    if (m_waterRenderObj != nullptr) {
        for (int i = 0; i < 5; i++) {
            if (m_initialSkyboxTextures[i].Is_Empty()) {
                m_initialSkyboxTextures[i] = *oldnames[i];
                m_currentSkyboxTextures[i] = *oldnames[i];
            }

            if (m_currentSkyboxTextures[i] != *newnames[i]) {
                m_waterRenderObj->Replace_Skybox_Texture(m_currentSkyboxTextures[i], *newnames[i]);
                m_currentSkyboxTextures[i] = *newnames[i];
            }
        }
    }
}
