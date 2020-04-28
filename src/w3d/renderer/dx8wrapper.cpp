/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Wrapper around platform 3D graphics library.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "dx8wrapper.h"
#include "ffactory.h"
#include "missing.h"
#include "rect.h"
#include "render2d.h"
#include "textureloader.h"
#include "thread.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8renderer.h"
#include "texture.h"
#include "w3d.h"
#include "boxrobj.h"
#include "pointgr.h"
#include "vertmaterial.h"
#include <cstdio>
#include <cstring>

using std::memcpy;
using std::memset;
using std::sprintf;
using std::strchr;

#ifdef GAME_DLL
#include "hooker.h"
#else
uint32_t g_numberOfDx8Calls = 0;
#ifdef BUILD_WITH_D3D8
IDirect3D8 *(__stdcall *DX8Wrapper::s_d3dCreateFunction)(unsigned) = nullptr;
HMODULE DX8Wrapper::s_d3dLib = nullptr;
IDirect3D8 *DX8Wrapper::s_d3dInterface;
IDirect3DDevice8 *DX8Wrapper::s_d3dDevice;
D3DMATRIX DX8Wrapper::s_oldPrj;
D3DMATRIX DX8Wrapper::s_oldView;
D3DMATRIX DX8Wrapper::s_oldWorld;
D3DPRESENT_PARAMETERS DX8Wrapper::s_PresentParameters;
D3DCOLOR DX8Wrapper::s_fogColor;
#endif
void *DX8Wrapper::s_shadowMap;
void *DX8Wrapper::s_hwnd;
unsigned DX8Wrapper::s_renderStates[256];
unsigned DX8Wrapper::s_textureStageStates[MAX_TEXTURE_STAGES][32];
w3dbasetexture_t DX8Wrapper::s_textures[MAX_TEXTURE_STAGES];
Vector4 DX8Wrapper::s_vertexShaderConstants[96]; // Not 100% sure this is a Vector4 array
unsigned DX8Wrapper::s_pixelShaderConstants[32]; // Not 100% on type, seems unused.
bool DX8Wrapper::s_isInitialised;
bool DX8Wrapper::s_isWindowed;
bool DX8Wrapper::s_debugIsWindowed;
RenderStateStruct DX8Wrapper::s_renderState;
unsigned DX8Wrapper::s_renderStateChanged;
float DX8Wrapper::s_zNear;
float DX8Wrapper::s_zFar;
Matrix4 DX8Wrapper::s_projectionMatrix;
int DX8Wrapper::s_mainThreadID;
int DX8Wrapper::s_currentRenderDevice = -1;
DX8Caps *DX8Wrapper::s_currentCaps;
int DX8Wrapper::s_resolutionWidth;
int DX8Wrapper::s_resolutionHeight;
int DX8Wrapper::s_bitDepth;
int DX8Wrapper::s_textureBitDepth;
bool DX8Wrapper::s_currentLightEnables[4];
unsigned DX8Wrapper::s_matrixChanges;
unsigned DX8Wrapper::s_materialChanges;
unsigned DX8Wrapper::s_vertexBufferChanges;
unsigned DX8Wrapper::s_indexBufferChanges;
unsigned DX8Wrapper::s_lightChanges;
unsigned DX8Wrapper::s_textureChanges;
unsigned DX8Wrapper::s_renderStateChanges;
unsigned DX8Wrapper::s_textureStageStateChanges;
unsigned DX8Wrapper::s_drawCalls;
unsigned DX8Wrapper::s_lastFrameMatrixChanges;
unsigned DX8Wrapper::s_lastFrameMaterialChanges;
unsigned DX8Wrapper::s_lastFrameVertexBufferChanges;
unsigned DX8Wrapper::s_lastFrameIndexBufferChanges;
unsigned DX8Wrapper::s_lastFrameLightChanges;
unsigned DX8Wrapper::s_lastFrameTextureChanges;
unsigned DX8Wrapper::s_lastFrameRenderStateChanges;
unsigned DX8Wrapper::s_lastFrameTextureStageStateChanges;
unsigned DX8Wrapper::s_lastFrameNumberDX8Calls;
unsigned DX8Wrapper::s_lastFrameDrawCalls;
D3DFORMAT DX8Wrapper::s_displayFormat;
DynamicVectorClass<StringClass> DX8Wrapper::s_renderDeviceNameTable;
DynamicVectorClass<StringClass> DX8Wrapper::s_renderDeviceShortNameTable;
DynamicVectorClass<RenderDeviceDescClass> DX8Wrapper::s_renderDeviceDescriptionTable;
w3dadapterid_t DX8Wrapper::s_currentAdapterIdentifier;
Matrix4 DX8Wrapper::s_DX8Transforms[257];
bool DX8Wrapper::s_EnableTriangleDraw;
int DX8Wrapper::s_ZBias;
Vector3 DX8Wrapper::s_ambientColor;
bool DX8Wrapper::s_isDeviceLost;
int DX8Wrapper::s_FPUPreserve;
DWORD DX8Wrapper::s_vertexShader;
DWORD DX8Wrapper::s_pixelShader;
LightEnvironmentClass *DX8Wrapper::s_lightEnvironment;
DWORD DX8Wrapper::s_vertexProcessingBehavior;
bool DX8Wrapper::s_fogEnable;
w3dsurface_t DX8Wrapper::s_currentRenderTarget;
w3dsurface_t DX8Wrapper::s_currentDepthBuffer;
w3dsurface_t DX8Wrapper::s_defaultRenderTarget;
w3dsurface_t DX8Wrapper::s_defaultDepthBuffer;
bool DX8Wrapper::s_isRenderToTexture;
unsigned int DX8Wrapper::s_drawPolygonLowBoundLimit;
unsigned long DX8Wrapper::s_frameCount;
bool DX8Wrapper::s_DX8SingleThreaded;
DX8_CleanupHook *DX8Wrapper::s_cleanupHook;
#endif

bool DX8Wrapper::Init(void *hwnd, bool lite)
{
#ifdef BUILD_WITH_D3D8
    memset(s_textures, 0, sizeof(s_textures));
    memset(s_renderStates, 0, sizeof(s_renderStates));
    memset(s_textureStageStates, 0, sizeof(s_textureStageStates));
    memset(s_vertexShaderConstants, 0, sizeof(s_vertexShaderConstants));
    memset(s_pixelShaderConstants, 0, sizeof(s_pixelShaderConstants));
    memset(&s_renderState, 0, sizeof(s_renderState));
    s_shadowMap = nullptr;
    s_hwnd = hwnd;
    s_mainThreadID = ThreadClass::Get_Current_Thread_ID(); // Init only called from main thread so this is fine.
    s_currentRenderDevice = -1;
    s_resolutionWidth = 640;
    s_resolutionHeight = 480;
    Render2DClass::Set_Screen_Resolution(RectClass(0.0f, 0.0f, 640.0f, 480.0f));
    s_isWindowed = s_debugIsWindowed = false;
    memset(s_currentLightEnables, 0, sizeof(s_currentLightEnables));
    s_bitDepth = 32;
    s_d3dInterface = nullptr;
    s_d3dDevice = nullptr;
    Reset_Statistics();
    Invalidate_Cached_Render_States();

    // If we are doing a full init we need to load d3d8 as well.
    if (!lite) {
        s_d3dLib = LoadLibraryA("d3d8.dll");

        if (s_d3dLib == nullptr) {
            captainslog_error("Failed to load d3d8.dll");
            return false;
        }

        s_d3dCreateFunction =
            reinterpret_cast<IDirect3D8 *(__stdcall *)(unsigned)>(GetProcAddress(s_d3dLib, "Direct3DCreate8"));

        if (s_d3dCreateFunction == nullptr) {
            captainslog_error("Failed to get address of Direct3DCreate8.");
            return false;
        }

        s_d3dInterface = s_d3dCreateFunction(D3D_SDK_VERSION);

        if (s_d3dInterface == nullptr) {
            captainslog_error("Failed to create D3D8 interface.");
            return false;
        }

        s_isInitialised = true;
        Enumerate_Devices();
    }

    return true;
#else
    return false;
#endif
}

void DX8Wrapper::Shutdown()
{
#ifdef BUILD_WITH_D3D8
    if (s_d3dDevice) {
        Set_Render_Target(nullptr);
        Release_Device();
    }
    if (s_d3dInterface) {
        s_d3dInterface->Release();
        s_d3dInterface = nullptr;
    }
    if (s_currentCaps) {
        for (unsigned int i = 0; i < s_currentCaps->Max_Textures_Per_Pass(); i++) {
            if (s_textures[i]) {
                s_textures[i]->Release();
                s_textures[i] = nullptr;
            }
        }
    }
    if (s_d3dLib) {
        FreeLibrary(s_d3dLib);
        s_d3dLib = nullptr;
    }
    s_renderDeviceNameTable.Delete_All();
    s_renderDeviceShortNameTable.Delete_All();
    s_renderDeviceDescriptionTable.Delete_All();
#endif
}

void DX8Wrapper::Set_Render_Target(w3dsurface_t *render_target, bool use_default_depth_buffer)
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x008047E0, 0x004FF9A0));
#endif
}

void DX8Wrapper::Do_Onetime_Device_Dependent_Shutdowns()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x00800B60, 0x004F9D70));
#endif
}

void DX8Wrapper::Release_Device()
{
#ifdef BUILD_WITH_D3D8
    if (s_d3dDevice) {
        for (int i = 0; i < MAX_TEXTURE_STAGES; i++) {
            DX8CALL(SetTexture(i, nullptr));
        }
        DX8CALL(SetStreamSource(0, nullptr, 0));
        DX8CALL(SetIndices(nullptr, 0));
        for (int i = 0; i < VERTEX_BUFFERS; i++) {
            if (s_renderState.vertex_buffers[i]) {
                s_renderState.vertex_buffers[i]->Release_Engine_Ref();
            }
            Ref_Ptr_Release(s_renderState.vertex_buffers[i]);
        }
        if (s_renderState.index_buffer) {
            s_renderState.index_buffer->Release_Engine_Ref();
            Ref_Ptr_Release(s_renderState.index_buffer);
        }
        Do_Onetime_Device_Dependent_Shutdowns();
        s_d3dDevice->Release();
        s_d3dDevice = nullptr;
    }
#endif
}

void DX8Wrapper::Enumerate_Devices()
{
#ifdef BUILD_WITH_D3D8
    int adapter_count = s_d3dInterface->GetAdapterCount();

    for (int i = 0; i < adapter_count; i++) {
        D3DADAPTER_IDENTIFIER8 id;
        memset(&id, 0, sizeof(id));
        HRESULT res = s_d3dInterface->GetAdapterIdentifier(i, D3DENUM_NO_WHQL_LEVEL, &id);

        if (res == D3D_OK) {
            RenderDeviceDescClass desc;
            desc.m_deviceName = id.Description;
            desc.m_driverName = id.Driver;
            char buf[64];

            sprintf(buf,
                "%d.%d.%d.%d",
                (id.DriverVersion.HighPart >> 16) & 0xFFFF,
                id.DriverVersion.HighPart & 0xFFFF,
                (id.DriverVersion.LowPart >> 16) & 0xFFFF,
                id.DriverVersion.LowPart & 0xFFFF);

            desc.m_driverVersion = buf;
            s_d3dInterface->GetDeviceCaps(i, D3DDEVTYPE_HAL, &desc.m_caps);
            s_d3dInterface->GetAdapterIdentifier(i, D3DENUM_NO_WHQL_LEVEL, &desc.m_adapterID);

            DX8Caps dx8caps(s_d3dInterface, desc.m_caps, WW3D_FORMAT_UNKNOWN, desc.m_adapterID);
            desc.m_resArray.Delete_All();
            int mode_count = s_d3dInterface->GetAdapterModeCount(i);

            for (int j = 0; j < mode_count; j++) {
                D3DDISPLAYMODE d3dmode;
                memset(&d3dmode, 0, sizeof(d3dmode));
                res = s_d3dInterface->EnumAdapterModes(i, j, &d3dmode);

                if (res == D3D_OK) {
                    int bits = 0;

                    switch (d3dmode.Format) {
                        case D3DFMT_R8G8B8:
                        case D3DFMT_A8R8G8B8:
                        case D3DFMT_X8R8G8B8:
                            bits = 32;
                            break;

                        case D3DFMT_R5G6B5:
                        case D3DFMT_X1R5G5B5:
                            bits = 16;
                            break;
                        default:
                            break;
                    }

                    if (dx8caps.Is_Valid_Display_Format(
                            d3dmode.Width, d3dmode.Height, D3DFormat_To_WW3DFormat(d3dmode.Format))
                        && bits != 0) {
                        desc.Add_Resolution(d3dmode.Width, d3dmode.Height, bits);
                    }
                }
            }

            if (desc.m_resArray.Count() > 0) {
                StringClass device_name(id.Description, true);
                s_renderDeviceNameTable.Add(device_name);
                s_renderDeviceShortNameTable.Add(device_name);
                s_renderDeviceDescriptionTable.Add(desc);
            }
        }
    }
#endif
}

void DX8Wrapper::Log_DX8_ErrorCode(unsigned error)
{
    // This made use the d3d8x part of the sdk found in the DirectX 8.1 SDK which is hard to find.
    captainslog_error("Direct3D8 generated error %x.", error);
}

void DX8Wrapper::Get_Device_Resolution(int &width, int &height, int &bit_depth, bool &windowed)
{
    width = s_resolutionWidth;
    height = s_resolutionHeight;
    bit_depth = s_bitDepth;
    windowed = s_isWindowed;
}

w3dtexture_t DX8Wrapper::Create_Texture(
    unsigned width, unsigned height, WW3DFormat format, MipCountType mip_level_count, w3dpool_t pool, bool rendertarget)
{
#ifdef GAME_DLL
    return Call_Function<w3dtexture_t, unsigned, unsigned, WW3DFormat, MipCountType, w3dpool_t, bool>(
        PICK_ADDRESS(0x008036F0, 0x004FDF00), width, height, format, mip_level_count, pool, rendertarget);
#else
    return w3dtexture_t();
#endif
}

w3dtexture_t DX8Wrapper::Create_Texture(w3dsurface_t surface, MipCountType mip_level_count)
{
#ifdef GAME_DLL
    return Call_Function<w3dtexture_t, w3dsurface_t, MipCountType>(
        PICK_ADDRESS(0x00803990, 0x004FE200), surface, mip_level_count);
#else
    return w3dtexture_t();
#endif
}

w3dsurface_t DX8Wrapper::Create_Surface(unsigned width, unsigned height, WW3DFormat format)
{
#ifdef BUILD_WITH_D3D8
    w3dsurface_t surf = W3D_TYPE_INVALID_SURFACE;
    DX8CALL(CreateImageSurface(width, height, (D3DFORMAT)WW3DFormat_To_D3DFormat(format), &surf));

    return surf;
#else
    return w3dsurface_t();
#endif
}

w3dsurface_t DX8Wrapper::Create_Surface(const char *name)
{
    char buf[PATH_MAX];

    { // auto_file_ptr scope.
        auto_file_ptr fptr(g_theFileFactory, name);

        // If the initial file is tga and isn't available try the dds version.
        if (!fptr->Is_Available()) {
            strlcpy(buf, name, sizeof(buf));
            char *ext = strchr(buf, '.');

            if (strlen(ext) == 4) {
                if (strncasecmp(ext, ".tga", 4)) {
                    strcpy(ext, ".dds");
                }
            }

            auto_file_ptr altptr(g_theFileFactory, buf);

            if (!altptr->Is_Available()) {
                return MissingTexture::Create_Missing_Surface();
            }
        }
    }

    return TextureLoader::Load_Surface_Immediate(name, WW3D_FORMAT_UNKNOWN, true);
}

void DX8Wrapper::Reset_Statistics()
{
    s_matrixChanges = 0;
    s_materialChanges = 0;
    s_vertexBufferChanges = 0;
    s_indexBufferChanges = 0;
    s_lightChanges = 0;
    s_textureChanges = 0;
    s_renderStateChanges = 0;
    s_textureStageStateChanges = 0;
    s_drawCalls = 0;
    s_lastFrameMatrixChanges = 0;
    s_lastFrameMaterialChanges = 0;
    s_lastFrameVertexBufferChanges = 0;
    s_lastFrameIndexBufferChanges = 0;
    s_lastFrameLightChanges = 0;
    s_lastFrameTextureChanges = 0;
    s_lastFrameRenderStateChanges = 0;
    s_lastFrameTextureStageStateChanges = 0;
    s_lastFrameNumberDX8Calls = 0;
    s_lastFrameDrawCalls = 0;
}

void DX8Wrapper::Invalidate_Cached_Render_States()
{
#ifdef BUILD_WITH_D3D8
    for (int i = 0; i < sizeof(s_renderStates) / sizeof(unsigned); i++) {
        s_renderStates[i] = 0x12345678;
    }
    for (int i = 0; i < MAX_TEXTURE_STAGES; i++) {
        for (int j = 0; j < 32; j++) {
            s_textureStageStates[i][j] = 0x12345678;
        }
        DX8CALL(SetTexture(i, nullptr));
        s_textures[i]->Release();
        s_textures[i] = nullptr;
    }
    ShaderClass::Invalidate();
    for (int i = 0; i < VERTEX_BUFFERS; i++) {
        if (s_renderState.vertex_buffers[i]) {
            s_renderState.vertex_buffers[i]->Release_Engine_Ref();
        }
        Ref_Ptr_Release(s_renderState.vertex_buffers[i]);
    }
    if (s_renderState.index_buffer) {
        s_renderState.index_buffer->Release_Engine_Ref();
        Ref_Ptr_Release(s_renderState.index_buffer);
    }
    Ref_Ptr_Release(s_renderState.material);
    for (int i = 0; i < MAX_TEXTURE_STAGES; i++) {
        Ref_Ptr_Release(s_renderState.Textures[i]);
    }
    memset(s_DX8Transforms, 0, sizeof(s_DX8Transforms));
#endif
}

void DX8Wrapper::Do_Onetime_Device_Dependent_Inits()
{
    Compute_Caps(D3DFormat_To_WW3DFormat(s_displayFormat));
    MissingTexture::Init();
    TextureFilterClass::Init_Filters((TextureFilterClass::TextureFilterMode)W3D::Get_Texture_Filter());
    g_theDX8MeshRenderer.Init();
    BoxRenderObjClass::Init();
    VertexMaterialClass::Init();
    PointGroupClass::Init();
    TextureLoader::Init();
    Set_Default_Global_Render_States();
}

inline DWORD F2DW(float f)
{
    return *((unsigned *)&f);
}

void DX8Wrapper::Set_Default_Global_Render_States()
{
#ifdef BUILD_WITH_D3D8
    const D3DCAPS8 &caps = Get_Caps()->Get_DX8_Caps();

    Set_DX8_Render_State(D3DRS_RANGEFOGENABLE, (caps.RasterCaps & D3DPRASTERCAPS_FOGRANGE) ? TRUE : FALSE);
    Set_DX8_Render_State(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
    Set_DX8_Render_State(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
    Set_DX8_Render_State(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
    Set_DX8_Render_State(D3DRS_COLORVERTEX, TRUE);
    Set_DX8_Render_State(D3DRS_ZBIAS, 0);
    Set_DX8_Texture_Stage_State(1, D3DTSS_BUMPENVLSCALE, F2DW(1.0f));
    Set_DX8_Texture_Stage_State(1, D3DTSS_BUMPENVLOFFSET, F2DW(0.0f));
    Set_DX8_Texture_Stage_State(0, D3DTSS_BUMPENVMAT00, F2DW(1.0f));
    Set_DX8_Texture_Stage_State(0, D3DTSS_BUMPENVMAT01, F2DW(0.0f));
    Set_DX8_Texture_Stage_State(0, D3DTSS_BUMPENVMAT10, F2DW(0.0f));
    Set_DX8_Texture_Stage_State(0, D3DTSS_BUMPENVMAT11, F2DW(1.0f));
#endif
}

void DX8Wrapper::Compute_Caps(WW3DFormat display_format)
{
#ifdef BUILD_WITH_D3D8
    delete s_currentCaps;
    s_currentCaps = new DX8Caps(s_d3dInterface, s_d3dDevice, display_format, Get_Current_Adapter_Identifier());
#endif
}

#ifdef BUILD_WITH_D3D8
// Inlined in DX8Wrapper::Get_DX8_Texture_Stage_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Texture_Op_Name(unsigned value)
{
    switch (value) {
        case D3DTOP_DISABLE:
            return "D3DTOP_DISABLE";
        case D3DTOP_SELECTARG1:
            return "D3DTOP_SELECTARG1";
        case D3DTOP_SELECTARG2:
            return "D3DTOP_SELECTARG2";
        case D3DTOP_MODULATE:
            return "D3DTOP_MODULATE";
        case D3DTOP_MODULATE2X:
            return "D3DTOP_MODULATE2X";
        case D3DTOP_MODULATE4X:
            return "D3DTOP_MODULATE4X";
        case D3DTOP_ADD:
            return "D3DTOP_ADD";
        case D3DTOP_ADDSIGNED:
            return "D3DTOP_ADDSIGNED";
        case D3DTOP_ADDSIGNED2X:
            return "D3DTOP_ADDSIGNED2X";
        case D3DTOP_SUBTRACT:
            return "D3DTOP_SUBTRACT";
        case D3DTOP_ADDSMOOTH:
            return "D3DTOP_ADDSMOOTH";
        case D3DTOP_BLENDDIFFUSEALPHA:
            return "D3DTOP_BLENDDIFFUSEALPHA";
        case D3DTOP_BLENDTEXTUREALPHA:
            return "D3DTOP_BLENDTEXTUREALPHA";
        case D3DTOP_BLENDFACTORALPHA:
            return "D3DTOP_BLENDFACTORALPHA";
        case D3DTOP_BLENDTEXTUREALPHAPM:
            return "D3DTOP_BLENDTEXTUREALPHAPM";
        case D3DTOP_BLENDCURRENTALPHA:
            return "D3DTOP_BLENDCURRENTALPHA";
        case D3DTOP_PREMODULATE:
            return "D3DTOP_PREMODULATE";
        case D3DTOP_MODULATEALPHA_ADDCOLOR:
            return "D3DTOP_MODULATEALPHA_ADDCOLOR";
        case D3DTOP_MODULATECOLOR_ADDALPHA:
            return "D3DTOP_MODULATECOLOR_ADDALPHA";
        case D3DTOP_MODULATEINVALPHA_ADDCOLOR:
            return "D3DTOP_MODULATEINVALPHA_ADDCOLOR";
        case D3DTOP_MODULATEINVCOLOR_ADDALPHA:
            return "D3DTOP_MODULATEINVCOLOR_ADDALPHA";
        case D3DTOP_BUMPENVMAP:
            return "D3DTOP_BUMPENVMAP";
        case D3DTOP_BUMPENVMAPLUMINANCE:
            return "D3DTOP_BUMPENVMAPLUMINANCE";
        case D3DTOP_DOTPRODUCT3:
            return "D3DTOP_DOTPRODUCT3";
        case D3DTOP_MULTIPLYADD:
            return "D3DTOP_MULTIPLYADD";
        case D3DTOP_LERP:
            return "D3DTOP_LERP";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Texture_Stage_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Texture_Arg_Name(unsigned value)
{
    switch (value) {
        case D3DTA_CURRENT:
            return "D3DTA_CURRENT";
        case D3DTA_DIFFUSE:
            return "D3DTA_DIFFUSE";
        case D3DTA_SELECTMASK:
            return "D3DTA_SELECTMASK";
        case D3DTA_SPECULAR:
            return "D3DTA_SPECULAR";
        case D3DTA_TEMP:
            return "D3DTA_TEMP";
        case D3DTA_TEXTURE:
            return "D3DTA_TEXTURE";
        case D3DTA_TFACTOR:
            return "D3DTA_TFACTOR";
        case D3DTA_ALPHAREPLICATE:
            return "D3DTA_ALPHAREPLICATE";
        case D3DTA_COMPLEMENT:
            return "D3DTA_COMPLEMENT";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Texture_Stage_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Texture_Filter_Name(unsigned value)
{
    switch (value) {
        case D3DTEXF_NONE:
            return "D3DTEXF_NONE";
        case D3DTEXF_POINT:
            return "D3DTEXF_POINT";
        case D3DTEXF_LINEAR:
            return "D3DTEXF_LINEAR";
        case D3DTEXF_ANISOTROPIC:
            return "D3DTEXF_ANISOTROPIC";
        case D3DTEXF_FLATCUBIC:
            return "D3DTEXF_FLATCUBIC";
        case D3DTEXF_GAUSSIANCUBIC:
            return "D3DTEXF_GAUSSIANCUBIC";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Texture_Stage_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Texture_Address_Name(unsigned value)
{
    switch (value) {
        case D3DTADDRESS_WRAP:
            return "D3DTADDRESS_WRAP";
        case D3DTADDRESS_MIRROR:
            return "D3DTADDRESS_MIRROR";
        case D3DTADDRESS_CLAMP:
            return "D3DTADDRESS_CLAMP";
        case D3DTADDRESS_BORDER:
            return "D3DTADDRESS_BORDER";
        case D3DTADDRESS_MIRRORONCE:
            return "D3DTADDRESS_MIRRORONCE";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Texture_Stage_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Texture_Transform_Flag_Name(unsigned value)
{
    switch (value) {
        case D3DTTFF_DISABLE:
            return "D3DTTFF_DISABLE";
        case D3DTTFF_COUNT1:
            return "D3DTTFF_COUNT1";
        case D3DTTFF_COUNT2:
            return "D3DTTFF_COUNT2";
        case D3DTTFF_COUNT3:
            return "D3DTTFF_COUNT3";
        case D3DTTFF_COUNT4:
            return "D3DTTFF_COUNT4";
        case D3DTTFF_PROJECTED:
            return "D3DTTFF_PROJECTED";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_ZBuffer_Type_Name(unsigned value)
{
    switch (value) {
        case D3DZB_FALSE:
            return "D3DZB_FALSE";
        case D3DZB_TRUE:
            return "D3DZB_TRUE";
        case D3DZB_USEW:
            return "D3DZB_USEW";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Fill_Mode_Name(unsigned value)
{
    switch (value) {
        case D3DFILL_POINT:
            return "D3DFILL_POINT";
        case D3DFILL_WIREFRAME:
            return "D3DFILL_WIREFRAME";
        case D3DFILL_SOLID:
            return "D3DFILL_SOLID";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Shade_Mode_Name(unsigned value)
{
    switch (value) {
        case D3DSHADE_FLAT:
            return "D3DSHADE_FLAT";
        case D3DSHADE_GOURAUD:
            return "D3DSHADE_GOURAUD";
        case D3DSHADE_PHONG:
            return "D3DSHADE_PHONG";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Blend_Name(unsigned value)
{
    switch (value) {
        case D3DBLEND_ZERO:
            return "D3DBLEND_ZERO";
        case D3DBLEND_ONE:
            return "D3DBLEND_ONE";
        case D3DBLEND_SRCCOLOR:
            return "D3DBLEND_SRCCOLOR";
        case D3DBLEND_INVSRCCOLOR:
            return "D3DBLEND_INVSRCCOLOR";
        case D3DBLEND_SRCALPHA:
            return "D3DBLEND_SRCALPHA";
        case D3DBLEND_INVSRCALPHA:
            return "D3DBLEND_INVSRCALPHA";
        case D3DBLEND_DESTALPHA:
            return "D3DBLEND_DESTALPHA";
        case D3DBLEND_INVDESTALPHA:
            return "D3DBLEND_INVDESTALPHA";
        case D3DBLEND_DESTCOLOR:
            return "D3DBLEND_DESTCOLOR";
        case D3DBLEND_INVDESTCOLOR:
            return "D3DBLEND_INVDESTCOLOR";
        case D3DBLEND_SRCALPHASAT:
            return "D3DBLEND_SRCALPHASAT";
        case D3DBLEND_BOTHSRCALPHA:
            return "D3DBLEND_BOTHSRCALPHA";
        case D3DBLEND_BOTHINVSRCALPHA:
            return "D3DBLEND_BOTHINVSRCALPHA";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Cull_Mode_Name(unsigned value)
{
    switch (value) {
        case D3DCULL_NONE:
            return "D3DCULL_NONE";
        case D3DCULL_CW:
            return "D3DCULL_CW";
        case D3DCULL_CCW:
            return "D3DCULL_CCW";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Cmp_Func_Name(unsigned value)
{
    switch (value) {
        case D3DCMP_NEVER:
            return "D3DCMP_NEVER";
        case D3DCMP_LESS:
            return "D3DCMP_LESS";
        case D3DCMP_EQUAL:
            return "D3DCMP_EQUAL";
        case D3DCMP_LESSEQUAL:
            return "D3DCMP_LESSEQUAL";
        case D3DCMP_GREATER:
            return "D3DCMP_GREATER";
        case D3DCMP_NOTEQUAL:
            return "D3DCMP_NOTEQUAL";
        case D3DCMP_GREATEREQUAL:
            return "D3DCMP_GREATEREQUAL";
        case D3DCMP_ALWAYS:
            return "D3DCMP_ALWAYS";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Fog_Mode_Name(unsigned value)
{
    switch (value) {
        case D3DFOG_NONE:
            return "D3DFOG_NONE";
        case D3DFOG_EXP:
            return "D3DFOG_EXP";
        case D3DFOG_EXP2:
            return "D3DFOG_EXP2";
        case D3DFOG_LINEAR:
            return "D3DFOG_LINEAR";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Stencil_Op_Name(unsigned value)
{
    switch (value) {
        case D3DSTENCILOP_KEEP:
            return "D3DSTENCILOP_KEEP";
        case D3DSTENCILOP_ZERO:
            return "D3DSTENCILOP_ZERO";
        case D3DSTENCILOP_REPLACE:
            return "D3DSTENCILOP_REPLACE";
        case D3DSTENCILOP_INCRSAT:
            return "D3DSTENCILOP_INCRSAT";
        case D3DSTENCILOP_DECRSAT:
            return "D3DSTENCILOP_DECRSAT";
        case D3DSTENCILOP_INVERT:
            return "D3DSTENCILOP_INVERT";
        case D3DSTENCILOP_INCR:
            return "D3DSTENCILOP_INCR";
        case D3DSTENCILOP_DECR:
            return "D3DSTENCILOP_DECR";
        default:
            return "UNKNOWN";
    }
}

const char *DX8Wrapper::Get_DX8_Material_Source_Name(unsigned value)
{
    switch (value) {
        case D3DMCS_MATERIAL:
            return "D3DMCS_MATERIAL";
        case D3DMCS_COLOR1:
            return "D3DMCS_COLOR1";
        case D3DMCS_COLOR2:
            return "D3DMCS_COLOR2";
        default:
            return "UNKNOWN";
    }
}

const char *DX8Wrapper::Get_DX8_Vertex_Blend_Flag_Name(unsigned value)
{
    switch (value) {
        case D3DVBF_DISABLE:
            return "D3DVBF_DISABLE";
        case D3DVBF_1WEIGHTS:
            return "D3DVBF_1WEIGHTS";
        case D3DVBF_2WEIGHTS:
            return "D3DVBF_2WEIGHTS";
        case D3DVBF_3WEIGHTS:
            return "D3DVBF_3WEIGHTS";
        case D3DVBF_TWEENING:
            return "D3DVBF_TWEENING";
        case D3DVBF_0WEIGHTS:
            return "D3DVBF_0WEIGHTS";
        default:
            return "UNKNOWN";
    }
}

const char *DX8Wrapper::Get_DX8_Patch_Edge_Style_Name(unsigned value)
{
    switch (value) {
        case D3DPATCHEDGE_DISCRETE:
            return "D3DPATCHEDGE_DISCRETE";
        case D3DPATCHEDGE_CONTINUOUS:
            return "D3DPATCHEDGE_CONTINUOUS";
        default:
            return "UNKNOWN";
    }
}

const char *DX8Wrapper::Get_DX8_Debug_Monitor_Token_Name(unsigned value)
{
    switch (value) {
        case D3DDMT_ENABLE:
            return "D3DDMT_ENABLE";
        case D3DDMT_DISABLE:
            return "D3DDMT_DISABLE";
        default:
            return "UNKNOWN";
    }
}

const char *DX8Wrapper::Get_DX8_Blend_Op_Name(unsigned value)
{
    switch (value) {
        case D3DBLENDOP_ADD:
            return "D3DBLENDOP_ADD";
        case D3DBLENDOP_SUBTRACT:
            return "D3DBLENDOP_SUBTRACT";
        case D3DBLENDOP_REVSUBTRACT:
            return "D3DBLENDOP_REVSUBTRACT";
        case D3DBLENDOP_MIN:
            return "D3DBLENDOP_MIN";
        case D3DBLENDOP_MAX:
            return "D3DBLENDOP_MAX";
        default:
            return "UNKNOWN";
    }
}
#endif
