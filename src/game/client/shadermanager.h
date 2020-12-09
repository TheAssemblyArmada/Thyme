/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Shader Manager
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
#include "dx8wrapper.h"
#include "gamelod.h"
#include "texture.h"
#include "view.h"

struct D3DXMATRIX;

enum CustomScenePassModes
{
    MODE_DEFAULT,
    MODE_MASK,
};

class W3DFilterInterface
{
public:
    virtual int Init() = 0;
    virtual int Shutdown() { return 1; }
    virtual bool Pre_Render(bool &skip, CustomScenePassModes &mode)
    {
        skip = false;
        return false;
    }
    virtual bool Post_Render(FilterModes mode, Coord2D &delta, bool &b) { return false; }
    virtual bool Setup(FilterModes mode) { return false; }
    virtual int Set(FilterModes mode) = 0;
    virtual void Reset() = 0;
};

class W3DShaderInterface
{
public:
    int Get_Num_Passes() { return m_numPasses; }
    virtual int Set(int pass);
    virtual void Reset();
    virtual int Init() = 0;
    virtual int Shutdown();

private:
    int m_numPasses;
};

class W3DShaderManager
{
public:
    enum ShaderTypes
    {
        ST_INVALID,
        ST_TERRAIN,
        ST_TERRAIN_NOISE1,
        ST_TERRAIN_NOISE2,
        ST_TERRAIN_NOISE12,
        ST_SHROUD_TEXTURE,
        ST_MASK_TEXTURE,
        ST_ROAD,
        ST_ROAD_NOISE1,
        ST_ROAD_NOISE2,
        ST_ROAD_NOISE12,
        ST_CLOUD_TEXTURE,
        ST_FLAT_TERRAIN,
        ST_FLAT_TERRAIN_NOISE1,
        ST_FLAT_TERRAIN_NOISE2,
        ST_FLAT_TERRAIN_NOISE12,
        ST_FLAT_SHROUD_TEXTURE,
        ST_MAX,
    };

    static void Set_Texture(int index, TextureClass *texture) { s_textures[index] = texture; }
    static void Init();
    static void Shutdown();
    static int Get_Shader_Passes(ShaderTypes shader);
    static int Set_Shader(ShaderTypes shader, int pass);
    static void Reset_Shader(ShaderTypes shader);
    static bool Filter_Pre_Render(FilterTypes filter, bool &skip, CustomScenePassModes &mode);
    static bool Filter_Post_Render(FilterTypes filter, FilterModes mode, Coord2D &delta, bool &b);
    static bool Filter_Setup(FilterTypes filter, FilterModes mode);
    static void Draw_Viewport(unsigned int color);
    static void Start_Render_To_Texture();
    static w3dtexture_t End_Render_To_Texture();
    static w3dtexture_t Get_Render_Texture();
    static GPUType Get_Chipset();
    static long Load_And_Create_D3D_Shader(
        const char *path, const unsigned long *decleration, unsigned long usage, bool type, unsigned long *handle);
    static bool Set_Shroud_Tex(int stage);
    static TextureClass *Get_Shader_Texture(int stage) { return s_textures[stage]; }
    static ShaderTypes Get_Current_Shader() { return s_currentShader; }
    static bool Render_Surfaces_Valid() { return s_oldRenderSurface && s_newRenderSurface; }
    static bool Get_Render_To_Texture() { return s_renderingToTexture; }

private:
    static int s_currentShaderPass;
    static FilterTypes s_currentFilter;
    static w3dsurface_t s_oldRenderSurface;
    static w3dtexture_t s_renderTexture;
    static w3dsurface_t s_newRenderSurface;
    static w3dsurface_t s_oldDepthSurface;
#ifdef GAME_DLL
    static ARRAY_DEC(TextureClass *, s_textures, MAX_TEXTURE_STAGES);
    static bool &s_renderingToTexture;
    static ShaderTypes &s_currentShader;
#else
    static TextureClass *s_textures[MAX_TEXTURE_STAGES];
    static bool s_renderingToTexture;
    static ShaderTypes s_currentShader;
#endif
};

class ShroudTextureShader : public W3DShaderInterface
{
public:
    virtual int Set(int pass) override;
    virtual void Reset() override;
    virtual int Init() override;

private:
    int m_pass;
};

class FlatShroudTextureShader : public W3DShaderInterface
{
public:
    virtual int Set(int pass) override;
    virtual void Reset() override;
    virtual int Init() override;

private:
    int m_pass;
};

class MaskTextureShader : public W3DShaderInterface
{
public:
    virtual int Set(int pass) override;
    virtual void Reset() override;
    virtual int Init() override;
};

class TerrainShader2Stage : public W3DShaderInterface
{
public:
    virtual int Set(int pass) override;
    virtual void Reset() override;
    virtual int Init() override;

#ifdef BUILD_WITH_D3D8
    void Update_Noise_1(D3DXMATRIX *dest_matrix, D3DXMATRIX *cur_view_inverse, bool do_update);
    void Update_Noise_2(D3DXMATRIX *dest_matrix, D3DXMATRIX *cur_view_inverse, bool do_update);
#endif

private:
    float m_xSlidePerSecond;
    float m_ySlidePerSecond;
    int m_curTick;
    float m_xOffset;
    float m_yOffset;
};

class FlatTerrainShader2Stage : public W3DShaderInterface
{
public:
    virtual int Set(int pass) override;
    virtual void Reset() override;
    virtual int Init() override;
};

class FlatTerrainShaderPixelShader : public W3DShaderInterface
{
public:
    virtual int Set(int pass) override;
    virtual void Reset() override;
    virtual int Init() override;
    virtual int Shutdown() override;

private:
    unsigned long m_dwBase1PixelShader;
    unsigned long m_dwBaseNoise1PixelShader;
    unsigned long m_dwBaseNoise2PixelShader;
    unsigned long m_dwBase2PixelShader;
};

class TerrainShader8Stage : public W3DShaderInterface
{
public:
    virtual int Set(int pass) override;
    virtual void Reset() override;
    virtual int Init() override;
};

class TerrainShaderPixelShader : public W3DShaderInterface
{
public:
    virtual int Set(int pass) override;
    virtual void Reset() override;
    virtual int Init() override;
    virtual int Shutdown() override;

private:
    unsigned long m_dwBasePixelShader;
    unsigned long m_dwBaseNoise1PixelShader;
    unsigned long m_dwBaseNoise2PixelShader;
};

class CloudTextureShader : public W3DShaderInterface
{
public:
    virtual int Set(int pass) override;
    virtual void Reset() override;
    virtual int Init() override;

private:
    int m_pass;
};

class RoadShaderPixelShader : public W3DShaderInterface
{
public:
    virtual int Set(int pass) override;
    virtual void Reset() override;
    virtual int Init() override;
    virtual int Shutdown() override;

private:
    unsigned long m_dwBaseNoise2PixelShader;
};

class RoadShader2Stage : public W3DShaderInterface
{
public:
    virtual int Set(int pass) override;
    virtual void Reset() override;
    virtual int Init() override;
};

class ScreenDefaultFilter : public W3DFilterInterface
{
public:
    virtual int Init() override;
    virtual bool Pre_Render(bool &skip, CustomScenePassModes &mode) override;
    virtual bool Post_Render(FilterModes mode, Coord2D &delta, bool &b) override;
    virtual bool Setup(FilterModes mode) override { return true; }
    virtual int Set(FilterModes mode) override;
    virtual void Reset() override;
};

class ScreenBWFilter : public W3DFilterInterface
{
public:
    virtual int Init() override;
    virtual int Shutdown() override;
    virtual bool Pre_Render(bool &skip, CustomScenePassModes &mode) override;
    virtual bool Post_Render(FilterModes mode, Coord2D &delta, bool &b) override;
    virtual bool Setup(FilterModes mode) override { return true; }
    virtual int Set(FilterModes mode) override;
    virtual void Reset() override;
    static void Set_Fade_Parameters(int frames, int direction)
    {
        s_curFadeFrame = 0;
        s_fadeFrames = frames;
        s_fadeDirection = direction;
    }

private:
    static int s_fadeFrames;
    static int s_fadeDirection;
    static int s_curFadeFrame;
    static float s_curFadeValue;

    unsigned long m_dwBWPixelShader;
    friend class ScreenBWFilterDOT3;
};

class ScreenBWFilterDOT3 : public W3DFilterInterface
{
public:
    virtual int Init() override;
    virtual int Shutdown() override;
    virtual bool Pre_Render(bool &skip, CustomScenePassModes &mode) override;
    virtual bool Post_Render(FilterModes mode, Coord2D &delta, bool &b) override;
    virtual bool Setup(FilterModes mode) override { return true; }
    virtual int Set(FilterModes mode) override;
    virtual void Reset() override;
};

class ScreenCrossFadeFilter : public W3DFilterInterface
{
public:
    virtual int Init() override;
    virtual int Shutdown() override;
    virtual bool Pre_Render(bool &skip, CustomScenePassModes &mode) override;
    virtual bool Post_Render(FilterModes mode, Coord2D &delta, bool &b) override;
    virtual bool Setup(FilterModes mode) override { return true; }
    virtual int Set(FilterModes mode) override;
    virtual void Reset() override;

    bool Update_Fade_Level();

    static float Get_Current_Fade_Value() { return s_curFadeValue; }
    static TextureClass *Get_Fade_Pattern_Texture() { return s_fadePatternTexture; }
    static void Set_Fade_Parameters(int frames, int direction)
    {
        s_curFadeFrame = 0;
        s_fadeFrames = frames;
        s_fadeDirection = direction;
    }

private:
    static int s_fadeFrames;
    static int s_curFadeFrame;
    static int s_fadeDirection;
    static float s_curFadeValue;
    static TextureClass *s_fadePatternTexture;
    static bool s_skipRender;
};

class ScreenMotionBlurFilter : public W3DFilterInterface
{
public:
    ScreenMotionBlurFilter();

    virtual int Init() override;
    virtual int Shutdown() override;
    virtual bool Pre_Render(bool &skip, CustomScenePassModes &mode) override;
    virtual bool Post_Render(FilterModes mode, Coord2D &delta, bool &b) override;
    virtual bool Setup(FilterModes mode) override;
    virtual int Set(FilterModes mode) override;
    virtual void Reset() override;

    static void Set_Zoom_To_Pos(const Coord3D *pos)
    {
        s_zoomToPos = *pos;
        s_zoomToValid = true;
    }

private:
    static Coord3D s_zoomToPos;
    static bool s_zoomToValid;

    int m_maxCount;
    int m_lastFrame;
    bool m_decrement;
    bool m_skipRender;
    bool m_additive;
    bool m_doZoomTo;
    Coord2D m_priorDelta;
    int m_panFactor;
};

extern ScreenDefaultFilter g_screenDefaultFilter;
extern ScreenBWFilter g_screenBWFilter;
extern ScreenBWFilterDOT3 g_screenBWFilterDOT3;
extern ScreenCrossFadeFilter g_screenCrossFadeFilter;
extern ScreenMotionBlurFilter g_screenMotionBlurFilter;

extern ShroudTextureShader g_shroudTextureShader;
extern FlatShroudTextureShader g_flatShroudTextureShader;
extern MaskTextureShader g_maskTextureShader;
extern TerrainShader2Stage g_terrainShader2Stage;
extern FlatTerrainShader2Stage g_flatTerrainShader2Stage;
extern FlatTerrainShaderPixelShader g_flatTerrainShaderPixelShader;
extern TerrainShader8Stage g_terrainShader8Stage;
extern TerrainShaderPixelShader g_terrainShaderPixelShader;
extern CloudTextureShader g_cloudTextureShader;
extern RoadShaderPixelShader g_roadShaderPixelShader;
extern RoadShader2Stage g_roadShader2Stage;

extern W3DFilterInterface *g_w3dFilters[FT_MAX];
extern W3DShaderInterface *g_w3dShaders[W3DShaderManager::ST_MAX];
extern int g_w3dShadersPassCount[W3DShaderManager::ST_MAX];
