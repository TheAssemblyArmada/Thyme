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
#include "boxrobj.h"
#include "dx8indexbuffer.h"
#include "dx8renderer.h"
#include "dx8texman.h"
#include "dx8vertexbuffer.h"
#include "ffactory.h"
#include "light.h"
#include "lightenv.h"
#include "missingtexture.h"
#include "pointgr.h"
#include "pot.h"
#include "rect.h"
#include "render2d.h"
#include "sortingrenderer.h"
#include "surfaceclass.h"
#include "texture.h"
#include "textureloader.h"
#include "thread.h"
#include "vertmaterial.h"
#include "w3d.h"
#ifdef BUILD_WITH_D3D8
#include <d3dx8.h>
#endif
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
D3DPRESENT_PARAMETERS DX8Wrapper::s_presentParameters;
D3DCOLOR DX8Wrapper::s_fogColor;
D3DFORMAT DX8Wrapper::s_displayFormat;
#endif
void *DX8Wrapper::s_shadowMap;
#ifdef PLATFORM_WINDOWS
HWND DX8Wrapper::s_hwnd;
#endif
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
#ifdef BUILD_WITH_D3D8
D3DMATRIX DX8Wrapper::s_projectionMatrix;
#else
Matrix4 DX8Wrapper::s_projectionMatrix;
#endif
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
DynamicVectorClass<StringClass> DX8Wrapper::s_renderDeviceNameTable;
DynamicVectorClass<StringClass> DX8Wrapper::s_renderDeviceShortNameTable;
DynamicVectorClass<RenderDeviceDescClass> DX8Wrapper::s_renderDeviceDescriptionTable;
w3dadapterid_t DX8Wrapper::s_currentAdapterIdentifier;
#ifdef BUILD_WITH_D3D8
D3DMATRIX DX8Wrapper::s_DX8Transforms[D3DTS_WORLD1];
#else
Matrix4 DX8Wrapper::s_DX8Transforms[257];
#endif
bool DX8Wrapper::s_EnableTriangleDraw = true;
int DX8Wrapper::s_ZBias;
Vector3 DX8Wrapper::s_ambientColor;
bool DX8Wrapper::s_isDeviceLost;
int DX8Wrapper::s_FPUPreserve;
unsigned long DX8Wrapper::s_vertexShader;
unsigned long DX8Wrapper::s_pixelShader;
LightEnvironmentClass *DX8Wrapper::s_lightEnvironment;
unsigned long DX8Wrapper::s_vertexProcessingBehavior;
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

#ifdef PLATFORM_WINDOWS
bool DX8Wrapper::Init(HWND hwnd, bool lite)
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
#endif

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
    s_currentCaps->Shutdown();
    s_isInitialised = false;
#endif
}

void DX8Wrapper::Do_Onetime_Device_Dependent_Inits()
{
#ifdef BUILD_WITH_D3D8
    Compute_Caps(D3DFormat_To_WW3DFormat(s_displayFormat));
    MissingTexture::Init();
    TextureFilterClass::Init_Filters(W3D::Get_Texture_Filter());
    g_theDX8MeshRenderer.Init();
    BoxRenderObjClass::Init();
    VertexMaterialClass::Init();
    PointGroupClass::Init();
    TextureLoader::Init();
    Set_Default_Global_Render_States();
#endif
}

inline unsigned long F2DW(float f)
{
    return *reinterpret_cast<unsigned long *>(&f);
}

void DX8Wrapper::Set_Default_Global_Render_States()
{
#ifdef BUILD_WITH_D3D8
    const D3DCAPS8 &caps = Get_Current_Caps()->Get_DX8_Caps();

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

        if (s_d3dDevice) {
            s_d3dDevice->SetTexture(i, nullptr);
        }

        if (s_textures[i]) {
            s_textures[i]->Release();
            s_textures[i] = nullptr;
        }
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

void DX8Wrapper::Do_Onetime_Device_Dependent_Shutdowns()
{
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

    for (unsigned int i = 0; i < Get_Current_Caps()->Max_Textures_Per_Pass(); i++) {
        Ref_Ptr_Release(s_renderState.Textures[i]);
    }

    TextureLoader::Deinit();
    SortingRendererClass::Deinit();
    DynamicVBAccessClass::Deinit();
    DynamicIBAccessClass::Deinit();
    PointGroupClass::Shutdown();
    VertexMaterialClass::Shutdown();
    BoxRenderObjClass::Shutdown();
    g_theDX8MeshRenderer.Shutdown();
    MissingTexture::Deinit();

    if (s_currentCaps) {
        delete s_currentCaps;
        s_currentCaps = nullptr;
    }
}

bool DX8Wrapper::Create_Device()
{
#ifdef BUILD_WITH_D3D8
    D3DCAPS8 caps;
    captainslog_assert(s_d3dDevice == nullptr);

    if (FAILED(s_d3dInterface->GetDeviceCaps(s_currentRenderDevice, D3DDEVTYPE_HAL, &caps))) {
        return false;
    }

    memset(&s_currentAdapterIdentifier, 0, sizeof(s_currentAdapterIdentifier));

    if (FAILED(s_d3dInterface->GetAdapterIdentifier(
            s_currentRenderDevice, D3DENUM_NO_WHQL_LEVEL, &s_currentAdapterIdentifier))) {
        return false;
    }

    s_vertexProcessingBehavior = (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0 ? D3DCREATE_MIXED_VERTEXPROCESSING :
                                                                                        D3DCREATE_SOFTWARE_VERTEXPROCESSING;

    if (s_FPUPreserve) {
        s_vertexProcessingBehavior |= D3DCREATE_FPU_PRESERVE;
    }

    if (SUCCEEDED(s_d3dInterface->CreateDevice(s_currentRenderDevice,
            D3DDEVTYPE_HAL,
            s_hwnd,
            s_vertexProcessingBehavior,
            &s_presentParameters,
            &s_d3dDevice))) {
        Do_Onetime_Device_Dependent_Inits();
        return true;
    }

    if (s_presentParameters.BackBufferFormat != D3DFMT_R5G6B5 && s_presentParameters.BackBufferFormat != D3DFMT_X1R5G5B5
            && s_presentParameters.BackBufferFormat != D3DFMT_A1R5G5B5
        || s_presentParameters.AutoDepthStencilFormat != D3DFMT_D32
            && s_presentParameters.AutoDepthStencilFormat != D3DFMT_D24S8
            && s_presentParameters.AutoDepthStencilFormat != D3DFMT_D24X8) {
        return false;
    }

    s_presentParameters.AutoDepthStencilFormat = D3DFMT_D16;

    if (FAILED(s_d3dInterface->CreateDevice(s_currentRenderDevice,
            D3DDEVTYPE_HAL,
            s_hwnd,
            s_vertexProcessingBehavior,
            &s_presentParameters,
            &s_d3dDevice))) {
        return false;
    }

    Do_Onetime_Device_Dependent_Inits();
    return true;
#else
    return false;
#endif
}

bool DX8Wrapper::Reset_Device(bool reacquire)
{
#ifdef BUILD_WITH_D3D8
    captainslog_warn("Resetting device.");

    if (!s_isInitialised || !s_d3dDevice) {
        captainslog_warn("Device reset failed");
        return false;
    }

    W3D::Invalidate_Textures();

    for (int i = 0; i < VERTEX_BUFFERS; i++) {
        Set_Vertex_Buffer(nullptr, i);
    }

    Set_Index_Buffer(nullptr, 0);

    if (s_cleanupHook) {
        s_cleanupHook->Release_Resources();
    }

    DynamicVBAccessClass::Deinit();
    DynamicIBAccessClass::Deinit();
    DX8TextureManagerClass::Release_Textures();
    memset(s_vertexShaderConstants, 0, sizeof(s_vertexShaderConstants));
    memset(s_pixelShaderConstants, 0, sizeof(s_pixelShaderConstants));
    s_frameCount = 0;

    if (s_d3dDevice->TestCooperativeLevel() == D3DERR_DEVICELOST) {
        return false;
    }

    HRESULT res;
    DX8CALL_HRES(Reset(&s_presentParameters), res);

    if (FAILED(res)) {
        return false;
    }

    if (reacquire) {
        DX8TextureManagerClass::Recreate_Textures();
        if (s_cleanupHook) {
            s_cleanupHook->Re_Acquire_Resources();
        }
    }

    Invalidate_Cached_Render_States();
    Set_Default_Global_Render_States();
    captainslog_warn("Device reset completed");
    return true;
#else
    return false;
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

        if (SUCCEEDED(res)) {
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

                if (SUCCEEDED(res)) {
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

#ifdef BUILD_WITH_D3D8
void DX8Wrapper::Get_Format_Name(unsigned int format, StringClass *format_name)
{
    switch (format) {
        case D3DFMT_DXT5:
            *format_name = "D3DFMT_DXT5";
            break;
        case D3DFMT_UYVY:
            *format_name = "D3DFMT_UYVY";
            break;
        case D3DFMT_DXT4:
            *format_name = "D3DFMT_DXT4";
            break;
        case D3DFMT_DXT2:
            *format_name = "D3DFMT_DXT2";
            break;
        case D3DFMT_YUY2:
            *format_name = "D3DFMT_YUY2";
            break;
        case D3DFMT_DXT3:
            *format_name = "D3DFMT_DXT3";
            break;
        case D3DFMT_DXT1:
            *format_name = "D3DFMT_DXT1";
            break;
        case D3DFMT_R8G8B8:
            *format_name = "D3DFMT_R8G8B8";
            break;
        case D3DFMT_A8R8G8B8:
            *format_name = "D3DFMT_A8R8G8B8";
            break;
        case D3DFMT_X8R8G8B8:
            *format_name = "D3DFMT_X8R8G8B8";
            break;
        case D3DFMT_R5G6B5:
            *format_name = "D3DFMT_R5G6B5";
            break;
        case D3DFMT_X1R5G5B5:
            *format_name = "D3DFMT_X1R5G5B5";
            break;
        case D3DFMT_A1R5G5B5:
            *format_name = "D3DFMT_A1R5G5B5";
            break;
        case D3DFMT_A4R4G4B4:
            *format_name = "D3DFMT_A4R4G4B4";
            break;
        case D3DFMT_R3G3B2:
            *format_name = "D3DFMT_R3G3B2";
            break;
        case D3DFMT_A8:
            *format_name = "D3DFMT_A8";
            break;
        case D3DFMT_A8R3G3B2:
            *format_name = "D3DFMT_A8R3G3B2";
            break;
        case D3DFMT_X4R4G4B4:
            *format_name = "D3DFMT_X4R4G4B4";
            break;
        case D3DFMT_A8P8:
            *format_name = "D3DFMT_A8P8";
            break;
        case D3DFMT_P8:
            *format_name = "D3DFMT_P8";
            break;
        case D3DFMT_L8:
            *format_name = "D3DFMT_L8";
            break;
        case D3DFMT_A8L8:
            *format_name = "D3DFMT_A8L8";
            break;
        case D3DFMT_A4L4:
            *format_name = "D3DFMT_A4L4";
            break;
        case D3DFMT_L6V5U5:
            *format_name = "D3DFMT_L6V5U5";
            break;
        case D3DFMT_X8L8V8U8:
            *format_name = "D3DFMT_X8L8V8U8";
            break;
        case D3DFMT_Q8W8V8U8:
            *format_name = "D3DFMT_Q8W8V8U8";
            break;
        case D3DFMT_V16U16:
            *format_name = "D3DFMT_V16U16";
            break;
        case D3DFMT_W11V11U10:
            *format_name = "D3DFMT_W11V11U10";
            break;
        case D3DFMT_D16_LOCKABLE:
            *format_name = "D3DFMT_D16_LOCKABLE";
            break;
        case D3DFMT_D32:
            *format_name = "D3DFMT_D32";
            break;
        case D3DFMT_D15S1:
            *format_name = "D3DFMT_D15S1";
            break;
        case D3DFMT_D24S8:
            *format_name = "D3DFMT_D24S8";
            break;
        case D3DFMT_D24X8:
            *format_name = "D3DFMT_D24X8";
            break;
        case D3DFMT_D24X4S4:
            *format_name = "D3DFMT_D24X4S4";
            break;
        case D3DFMT_D16:
            *format_name = "D3DFMT_D16";
            break;
        default:
            *format_name = "Unknown";
            break;
    }
}
#endif

bool DX8Wrapper::Set_Render_Device(
    int dev, int resx, int resy, int bits, int windowed, bool resize_window, bool reset_device, bool restore_assets)
{
#ifdef BUILD_WITH_D3D8
    captainslog_assert(s_isInitialised);
    captainslog_assert(dev >= -1);
    captainslog_assert(dev < s_renderDeviceNameTable.Count());

    if ((s_currentRenderDevice == -1) && (dev == -1)) {
        s_currentRenderDevice = 0;
    } else if (dev != -1) {
        s_currentRenderDevice = dev;
    }

    if (resx != -1) {
        s_resolutionWidth = resx;
    }

    if (resy != -1) {
        s_resolutionHeight = resy;
    }

    Render2DClass::Set_Screen_Resolution(RectClass(0, 0, s_resolutionWidth, s_resolutionHeight));

    if (bits != -1) {
        s_bitDepth = bits;
    }

    if (windowed != -1) {
        s_isWindowed = windowed != 0;
    }

    s_debugIsWindowed = s_isWindowed;
    captainslog_debug("Attempting Set_Render_Device: name: %s (%s:%s), width: %d, height: %d, windowed: %d",
        s_renderDeviceNameTable[s_currentRenderDevice].Peek_Buffer(),
        s_renderDeviceDescriptionTable[s_currentRenderDevice].m_driverName.Peek_Buffer(),
        s_renderDeviceDescriptionTable[s_currentRenderDevice].m_driverVersion.Peek_Buffer(),
        s_resolutionWidth,
        s_resolutionHeight,
        s_isWindowed != 0);

    if (resize_window) {
        RECT r;
        r.top = 0;
        r.right = 0;
        r.left = 0;
        r.bottom = 0;
        GetClientRect(s_hwnd, &r);
        if (r.right - r.left != s_resolutionWidth || r.bottom - r.top != s_resolutionHeight) {
            r.right = s_resolutionWidth;
            r.left = 0;
            r.top = 0;
            r.bottom = s_resolutionHeight;
            unsigned long style = GetWindowLong(s_hwnd, GWL_STYLE);
            AdjustWindowRect(&r, style, 0);
            if (windowed) {
                SetWindowPos(s_hwnd, nullptr, 0, 0, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOMOVE);
            } else {
                SetWindowPos(s_hwnd, HWND_TOPMOST, 0, 0, r.right - r.left, r.bottom - r.top, SWP_NOSIZE | SWP_NOMOVE);
            }
        }
    }

    captainslog_assert(reset_device || s_d3dDevice == nullptr);
    memset(&s_presentParameters, 0, sizeof(s_presentParameters));
    s_presentParameters.BackBufferWidth = s_resolutionWidth;
    s_presentParameters.BackBufferHeight = s_resolutionHeight;
    s_presentParameters.MultiSampleType = D3DMULTISAMPLE_NONE;
    s_presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    s_presentParameters.EnableAutoDepthStencil = true;
    s_presentParameters.BackBufferCount = (s_isWindowed == 0) + 1;
    s_presentParameters.hDeviceWindow = s_hwnd;
    s_presentParameters.Flags = 0;
    s_presentParameters.FullScreen_PresentationInterval = 0;
    s_presentParameters.FullScreen_RefreshRateInHz = 0;
    s_presentParameters.Windowed = s_isWindowed;

    if (s_isWindowed) {
        D3DDISPLAYMODE mode;
        mode.Format = D3DFMT_UNKNOWN;
        mode.Height = 0;
        mode.RefreshRate = 0;
        mode.Width = 0;
        s_d3dInterface->GetAdapterDisplayMode(s_currentRenderDevice, &mode);
        s_presentParameters.BackBufferFormat = mode.Format;
        s_displayFormat = mode.Format;

        switch (s_presentParameters.BackBufferFormat) {
            case D3DFMT_X8R8G8B8:
            case D3DFMT_A8R8G8B8:
            case D3DFMT_R8G8B8:
                s_bitDepth = 32;
                if (SUCCEEDED(s_d3dInterface->CheckDeviceType(0, D3DDEVTYPE_HAL, mode.Format, D3DFMT_A8R8G8B8, true))) {
                    s_presentParameters.BackBufferFormat = D3DFMT_A8R8G8B8;
                }
                break;
            case D3DFMT_A4R4G4B4:
            case D3DFMT_A1R5G5B5:
            case D3DFMT_R5G6B5:
                s_bitDepth = 16;
                break;
            case D3DFMT_L8:
            case D3DFMT_A8:
            case D3DFMT_P8:
                s_bitDepth = 8;
                break;
            default:
                return false;
        }

        if (!Find_Z_Mode(
                s_displayFormat, s_presentParameters.BackBufferFormat, &s_presentParameters.AutoDepthStencilFormat)) {
            if (s_bitDepth == 32) {
                s_bitDepth = 16;
                s_presentParameters.BackBufferFormat = D3DFMT_R5G6B5;
                if (!Find_Z_Mode(D3DFMT_R5G6B5, D3DFMT_R5G6B5, &s_presentParameters.AutoDepthStencilFormat)) {
                    s_presentParameters.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
                }
            } else {
                s_presentParameters.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
            }
        }
    } else {
        Find_Color_And_Z_Mode(s_resolutionWidth,
            s_resolutionHeight,
            s_bitDepth,
            &s_displayFormat,
            &s_presentParameters.BackBufferFormat,
            &s_presentParameters.AutoDepthStencilFormat);
    }

    if (s_presentParameters.AutoDepthStencilFormat == D3DFMT_UNKNOWN) {
        s_presentParameters.AutoDepthStencilFormat = D3DFMT_D32;
        if (s_bitDepth != 32) {
            s_presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
        }
    }

    StringClass format_name;
    StringClass backbuffer_name;
    Get_Format_Name(s_displayFormat, &format_name);
    Get_Format_Name(s_presentParameters.BackBufferFormat, &backbuffer_name);
    captainslog_warn("Using Display/BackBuffer Formats: %s/%s", format_name.Peek_Buffer(), backbuffer_name.Peek_Buffer());
    bool res;

    if (reset_device) {
        res = Reset_Device(restore_assets);
    } else {
        res = Create_Device();
    }

    captainslog_warn("Reset/Create_Device done, reset_device=%d, restore_assets=%d", reset_device, restore_assets);
    return res;
#else
    return false;
#endif
}

bool DX8Wrapper::Has_Stencil()
{
#ifdef BUILD_WITH_D3D8
    return s_presentParameters.AutoDepthStencilFormat == D3DFMT_D24S8
        || s_presentParameters.AutoDepthStencilFormat == D3DFMT_D24X4S4;
#else
    return false;
#endif
}

int DX8Wrapper::Get_Render_Device()
{
    return s_currentRenderDevice;
}

int DX8Wrapper::Get_Render_Device_Count()
{
    return s_renderDeviceNameTable.Count();
}

const char *DX8Wrapper::Get_Render_Device_Name(int device_index)
{
    device_index = device_index % s_renderDeviceShortNameTable.Count();
    return s_renderDeviceShortNameTable[device_index];
}

const RenderDeviceDescClass &DX8Wrapper::Get_Render_Device_Desc(int deviceidx)
{
    captainslog_assert(s_isInitialised);

    if ((deviceidx == -1) && (s_currentRenderDevice == -1)) {
        s_currentRenderDevice = 0;
    }

    if (deviceidx == -1) {
        captainslog_assert(s_currentRenderDevice >= 0);
        captainslog_assert(s_currentRenderDevice < s_renderDeviceNameTable.Count());
        return s_renderDeviceDescriptionTable[s_currentRenderDevice];
    }

    captainslog_assert(deviceidx >= 0);
    captainslog_assert(deviceidx < s_renderDeviceNameTable.Count());
    return s_renderDeviceDescriptionTable[deviceidx];
}

bool DX8Wrapper::Set_Device_Resolution(int width, int height, int bits, int windowed, bool resize_window)
{
#ifdef BUILD_WITH_D3D8
    if (!s_d3dDevice) {
        return false;
    }

    if (width != -1) {
        s_resolutionWidth = width;
        s_presentParameters.BackBufferWidth = width;
    }

    if (height != -1) {
        s_resolutionHeight = height;
        s_presentParameters.BackBufferHeight = height;
    }

    if (resize_window) {
        RECT r;
        r.top = 0;
        r.right = 0;
        r.left = 0;
        r.bottom = 0;
        GetClientRect(s_hwnd, &r);

        if (r.right - r.left != s_resolutionWidth || r.bottom - r.top != s_resolutionHeight) {
            r.right = s_resolutionWidth;
            r.left = 0;
            r.top = 0;
            r.bottom = s_resolutionHeight;
            unsigned long style = GetWindowLong(s_hwnd, GWL_STYLE);
            AdjustWindowRect(&r, style, 0);

            if (windowed) {
                SetWindowPos(s_hwnd, nullptr, 0, 0, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOMOVE);
            } else {
                SetWindowPos(s_hwnd, HWND_TOPMOST, 0, 0, r.right - r.left, r.bottom - r.top, SWP_NOSIZE | SWP_NOMOVE);
            }
        }
    }
    return Reset_Device(true);
#else
    return false;
#endif
}

void DX8Wrapper::Get_Device_Resolution(int &width, int &height, int &bit_depth, bool &windowed)
{
    width = s_resolutionWidth;
    height = s_resolutionHeight;
    bit_depth = s_bitDepth;
    windowed = s_isWindowed;
}

void DX8Wrapper::Get_Render_Target_Resolution(int &set_w, int &set_h, int &set_bits, bool &set_windowed)
{
#ifdef BUILD_WITH_D3D8
    captainslog_assert(s_isInitialised);

    if (s_currentRenderTarget) {
        D3DSURFACE_DESC desc;
        s_currentRenderTarget->GetDesc(&desc);
        set_w = desc.Width;
        set_h = desc.Height;
        set_bits = s_bitDepth;
        set_windowed = s_isWindowed;
    } else {
        Get_Device_Resolution(set_w, set_h, set_bits, set_windowed);
    }
#endif
}

#ifdef BUILD_WITH_D3D8
bool DX8Wrapper::Find_Color_And_Z_Mode(
    int resx, int resy, int bitdepth, D3DFORMAT *set_colorbuffer, D3DFORMAT *set_backbuffer, D3DFORMAT *set_zmode)
{
    static D3DFORMAT _formats16[] = { D3DFMT_R5G6B5, D3DFMT_X1R5G5B5, D3DFMT_A1R5G5B5 };
    static D3DFORMAT _formats32[] = {
        D3DFMT_A8R8G8B8,
        D3DFMT_X8R8G8B8,
        D3DFMT_R8G8B8,
    };
    D3DFORMAT *format_table = nullptr;
    int format_count = 0;

    if (s_bitDepth == 16) {
        format_table = _formats16;
        format_count = sizeof(_formats16) / sizeof(D3DFORMAT);
    } else {
        format_table = _formats32;
        format_count = sizeof(_formats32) / sizeof(D3DFORMAT);
    }

    bool found = false;
    unsigned int mode = 0;
    int format_index;

    for (format_index = 0; format_index < format_count; format_index++) {
        found |= Find_Color_Mode(format_table[format_index], resx, resy, &mode);
        if (found)
            break;
    }

    if (!found) {
        return false;
    } else {
        *set_colorbuffer = format_table[format_index];
        *set_backbuffer = format_table[format_index];
        if (bitdepth == 32 && *set_colorbuffer == D3DFMT_X8R8G8B8
            && SUCCEEDED(s_d3dInterface->CheckDeviceType(0, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DFMT_A8R8G8B8, 1))) {
            *set_backbuffer = D3DFMT_A8R8G8B8;
        }
    }

    return Find_Z_Mode(*set_colorbuffer, *set_colorbuffer, set_zmode);
}

bool DX8Wrapper::Find_Color_Mode(D3DFORMAT colorbuffer, int resx, int resy, UINT *mode)
{
    UINT i, j, modemax;
    UINT rx, ry;
    D3DDISPLAYMODE dmode;
    memset(&dmode, 0, sizeof(D3DDISPLAYMODE));
    rx = (unsigned int)resx;
    ry = (unsigned int)resy;
    bool found = false;
    modemax = s_d3dInterface->GetAdapterModeCount(D3DADAPTER_DEFAULT);
    i = 0;

    while (i < modemax && !found) {
        s_d3dInterface->EnumAdapterModes(D3DADAPTER_DEFAULT, i, &dmode);
        if (dmode.Width == rx && dmode.Height == ry && dmode.Format == colorbuffer) {
            captainslog_warn(
                "Found valid color mode.  Width = %d Height = %d Format = %d", dmode.Width, dmode.Height, dmode.Format);
            found = true;
        }
        i++;
    }

    i--;

    if (!found) {
        captainslog_warn(("Failed to find a valid color mode"));
        return false;
    }

    bool stillok = true;
    j = i;

    while (j < modemax && stillok) {
        s_d3dInterface->EnumAdapterModes(D3DADAPTER_DEFAULT, j, &dmode);
        if (dmode.Width == rx && dmode.Height == ry && dmode.Format == colorbuffer) {
            stillok = true;
        } else {
            stillok = false;
        }
        j++;
    }

    if (stillok == false) {
        *mode = j - 2;
    } else {
        *mode = i;
    }

    return true;
}

bool DX8Wrapper::Find_Z_Mode(D3DFORMAT colorbuffer, D3DFORMAT backbuffer, D3DFORMAT *zmode)
{
    if (Test_Z_Mode(colorbuffer, backbuffer, D3DFMT_D24S8)) {
        *zmode = D3DFMT_D24S8;
        captainslog_warn("Found zbuffer mode D3DFMT_D24S8");
        return true;
    }

    if (Test_Z_Mode(colorbuffer, backbuffer, D3DFMT_D32)) {
        *zmode = D3DFMT_D32;
        captainslog_warn("Found zbuffer mode D3DFMT_D32");
        return true;
    }

    if (Test_Z_Mode(colorbuffer, backbuffer, D3DFMT_D24X8)) {
        *zmode = D3DFMT_D24X8;
        captainslog_warn("Found zbuffer mode D3DFMT_D24X8");
        return true;
    }

    if (Test_Z_Mode(colorbuffer, backbuffer, D3DFMT_D24X4S4)) {
        *zmode = D3DFMT_D24X4S4;
        captainslog_warn("Found zbuffer mode D3DFMT_D24X4S4");
        return true;
    }

    if (Test_Z_Mode(colorbuffer, backbuffer, D3DFMT_D16)) {
        *zmode = D3DFMT_D16;
        captainslog_warn("Found zbuffer mode D3DFMT_D16");
        return true;
    }

    if (Test_Z_Mode(colorbuffer, backbuffer, D3DFMT_D15S1)) {
        *zmode = D3DFMT_D15S1;
        captainslog_warn("Found zbuffer mode D3DFMT_D15S1");
        return true;
    }

    captainslog_warn("Failed to find a valid zbuffer mode");
    return false;
}

bool DX8Wrapper::Test_Z_Mode(D3DFORMAT colorbuffer, D3DFORMAT backbuffer, D3DFORMAT zmode)
{
    if (FAILED(s_d3dInterface->CheckDeviceFormat(
            D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, colorbuffer, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, zmode))) {
        captainslog_warn("CheckDeviceFormat failed.  Colorbuffer format = %d  Zbufferformat = %d", colorbuffer, zmode);
        return false;
    }

    if (FAILED(s_d3dInterface->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, colorbuffer, backbuffer, zmode))) {
        captainslog_warn(
            "CheckDepthStencilMatch failed.  Colorbuffer format = %d  Backbuffer format = %d Zbufferformat = %d",
            colorbuffer,
            backbuffer,
            zmode);
        return false;
    }

    return true;
}
#endif

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

void DX8Wrapper::Begin_Statistics()
{
    s_matrixChanges = 0;
    s_materialChanges = 0;
    s_vertexBufferChanges = 0;
    s_indexBufferChanges = 0;
    s_lightChanges = 0;
    s_textureChanges = 0;
    s_renderStateChanges = 0;
    s_textureStageStateChanges = 0;
    g_numberOfDx8Calls = 0;
    s_drawCalls = 0;
}

void DX8Wrapper::End_Statistics()
{
    s_lastFrameMatrixChanges = s_matrixChanges;
    s_lastFrameMaterialChanges = s_materialChanges;
    s_lastFrameVertexBufferChanges = s_vertexBufferChanges;
    s_lastFrameIndexBufferChanges = s_indexBufferChanges;
    s_lastFrameLightChanges = s_lightChanges;
    s_lastFrameTextureChanges = s_textureChanges;
    s_lastFrameRenderStateChanges = s_renderStateChanges;
    s_lastFrameTextureStageStateChanges = s_textureStageStateChanges;
    s_lastFrameNumberDX8Calls = g_numberOfDx8Calls;
    s_lastFrameDrawCalls = s_drawCalls;
}

void DX8Wrapper::Begin_Scene()
{
#ifdef BUILD_WITH_D3D8
    DX8CALL(BeginScene());
#endif
}

void DX8Wrapper::End_Scene(bool flip_frames)
{
#ifdef BUILD_WITH_D3D8
    DX8CALL(EndScene());

    if (flip_frames) {
        HRESULT hr;
        DX8CALL_HRES(Present(nullptr, nullptr, nullptr, nullptr), hr);

        if (FAILED(hr)) {
            s_isDeviceLost = true;
        } else {
            s_isDeviceLost = false;
            s_frameCount++;
        }

        if (hr == D3DERR_DEVICELOST) {
            if (s_d3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
                Reset_Device(true);
            } else {
                ThreadClass::Sleep_Ms(200);
            }
        }
    }

    Set_Vertex_Buffer(nullptr, 0);
    Set_Index_Buffer(nullptr, 0);

    for (unsigned int i = 0; i < s_currentCaps->Max_Textures_Per_Pass(); i++) {
        Set_Texture(i, nullptr);
    }

    Set_Material(nullptr);
#endif
}

void DX8Wrapper::Clear(
    bool clear_color, bool clear_z_stencil, const Vector3 &color, float alpha, float z, unsigned int stencil)
{
#ifdef BUILD_WITH_D3D8
    bool has_stencil = false;
    IDirect3DSurface8 *surface;
    DX8CALL(GetDepthStencilSurface(&surface));

    if (surface) {
        D3DSURFACE_DESC desc;
        surface->GetDesc(&desc);
        if (desc.Format == D3DFMT_D15S1 || desc.Format == D3DFMT_D24S8 || desc.Format == D3DFMT_D24X4S4) {
            has_stencil = true;
        }
        surface->Release();
    }

    unsigned long flags = 0;

    if (clear_color)
        flags |= D3DCLEAR_TARGET;

    if (clear_z_stencil)
        flags |= D3DCLEAR_ZBUFFER;

    if (clear_z_stencil && has_stencil)
        flags |= D3DCLEAR_STENCIL;

    if (flags) {
        DX8CALL(Clear(0, nullptr, flags, Convert_Color(color, alpha), z, stencil));
    }
#endif
}

#ifdef BUILD_WITH_D3D8
void DX8Wrapper::Set_Viewport(CONST D3DVIEWPORT8 *pViewport)
{
    DX8CALL(SetViewport(pViewport));
}
#endif

void DX8Wrapper::Set_Vertex_Buffer(const VertexBufferClass *vb, int number)
{
    s_renderState.vba_offset = 0;
    s_renderState.vba_count = 0;
    VertexBufferClass *buf = s_renderState.vertex_buffers[number];

    if (buf) {
        buf->Release_Engine_Ref();
    }

    VertexBufferClass *v = const_cast<VertexBufferClass *>(vb);
    Ref_Ptr_Set(s_renderState.vertex_buffers[number], v);

    if (vb) {
        vb->Add_Engine_Ref();
        s_renderState.vertex_buffer_types[number] = vb->Type();
    } else {
        s_renderState.vertex_buffer_types[number] = VertexBufferClass::BUFFER_TYPE_INVALID;
    }

    s_renderStateChanged |= VERTEX_BUFFER_CHANGED;
}

void DX8Wrapper::Set_Index_Buffer(const IndexBufferClass *ib, unsigned short index_base_offset)
{
    s_renderState.iba_offset = 0;
    IndexBufferClass *buf = s_renderState.index_buffer;

    if (buf) {
        buf->Release_Engine_Ref();
    }

    IndexBufferClass *i = const_cast<IndexBufferClass *>(ib);
    Ref_Ptr_Set(s_renderState.index_buffer, i);
    s_renderState.index_base_offset = index_base_offset;

    if (ib) {
        ib->Add_Engine_Ref();
        s_renderState.index_buffer_type = ib->Type();
    } else {
        s_renderState.index_buffer_type = IndexBufferClass::BUFFER_TYPE_INVALID;
    }

    s_renderStateChanged |= INDEX_BUFFER_CHANGED;
}

void DX8Wrapper::Set_Vertex_Buffer(const DynamicVBAccessClass &vba)
{
    Set_Vertex_Buffer(nullptr, 1);
    VertexBufferClass *buf = s_renderState.vertex_buffers[0];

    if (buf) {
        buf->Release_Engine_Ref();
    }

    s_renderState.vertex_buffer_types[0] = vba.Get_Type();
    s_renderState.vba_offset = vba.Get_Vertex_Offset();
    s_renderState.vba_count = vba.Get_Vertex_Count();
    VertexBufferClass *v = const_cast<VertexBufferClass *>(vba.Get_Vertex_Buffer());
    Ref_Ptr_Set(s_renderState.vertex_buffers[0], v);
    v->Add_Engine_Ref();
    s_renderStateChanged |= VERTEX_BUFFER_CHANGED;
    s_renderStateChanged |= INDEX_BUFFER_CHANGED;
}

void DX8Wrapper::Set_Index_Buffer(const DynamicIBAccessClass &iba, unsigned short index_base_offset)
{
    IndexBufferClass *buf = s_renderState.index_buffer;

    if (buf) {
        buf->Release_Engine_Ref();
    }

    s_renderState.index_buffer_type = iba.Get_Type();
    s_renderState.iba_offset = iba.Get_Index_Offset();
    IndexBufferClass *i = const_cast<IndexBufferClass *>(iba.Get_Index_Buffer());
    Ref_Ptr_Set(s_renderState.index_buffer, i);
    i->Add_Engine_Ref();
    s_renderStateChanged |= INDEX_BUFFER_CHANGED;
}

void DX8Wrapper::Draw_Sorting_IB_VB(unsigned int primitive_type,
    unsigned short start_index,
    unsigned short polygon_count,
    unsigned short min_vertex_index,
    unsigned short vertex_count)
{
#ifdef BUILD_WITH_D3D8
    captainslog_assert(s_renderState.vertex_buffer_types[0] == VertexBufferClass::BUFFER_TYPE_SORTING
        || s_renderState.vertex_buffer_types[0] == VertexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING);
    captainslog_assert(s_renderState.index_buffer_type == IndexBufferClass::BUFFER_TYPE_SORTING
        || s_renderState.index_buffer_type == IndexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING);

    DynamicVBAccessClass dyn_vb_access(VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, dynamic_fvf_type, vertex_count);
    {
        DynamicVBAccessClass::WriteLockClass lock(&dyn_vb_access);
        VertexFormatXYZNDUV2 *src =
            static_cast<SortingVertexBufferClass *>(s_renderState.vertex_buffers[0])->Get_Sorting_Vertex_Buffer();
        VertexFormatXYZNDUV2 *dest = lock.Get_Formatted_Vertex_Array();
        src += s_renderState.vba_offset + s_renderState.index_base_offset + min_vertex_index;
        unsigned size = dyn_vb_access.FVF_Info().Get_FVF_Size() * vertex_count / sizeof(unsigned);
        unsigned *dest_u = reinterpret_cast<unsigned *>(dest);
        unsigned *src_u = reinterpret_cast<unsigned *>(src);

        for (unsigned i = 0; i < size; ++i) {
            *dest_u++ = *src_u++;
        }
    }

    DX8CALL(SetStreamSource(0,
        static_cast<DX8VertexBufferClass *>(dyn_vb_access.Get_Vertex_Buffer())->Get_DX8_Vertex_Buffer(),
        dyn_vb_access.FVF_Info().Get_FVF_Size()));

    if (dyn_vb_access.FVF_Info().Get_FVF()) {
        DX8CALL(SetVertexShader(dyn_vb_access.FVF_Info().Get_FVF()));
    }

    unsigned index_count = 0;
    s_vertexBufferChanges++;

    switch (primitive_type) {
        case D3DPT_TRIANGLELIST:
            index_count = polygon_count * 3;
            break;
        case D3DPT_TRIANGLESTRIP:
            index_count = polygon_count + 2;
            break;
        case D3DPT_TRIANGLEFAN:
            index_count = polygon_count + 2;
            break;
        default:
            captainslog_assert(0);
            break;
    }

    DynamicIBAccessClass dyn_ib_access(IndexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, index_count);
    {
        DynamicIBAccessClass::WriteLockClass lock(&dyn_ib_access);
        unsigned short *dest = lock.Get_Index_Array();
        unsigned short *src = nullptr;
        src = static_cast<SortingIndexBufferClass *>(s_renderState.index_buffer)->Get_Sorting_Index_Buffer();
        src += s_renderState.iba_offset + start_index;

        for (unsigned short i = 0; i < index_count; ++i) {
            unsigned short index = *src++;
            index -= min_vertex_index;
            captainslog_assert(index < vertex_count);
            *dest++ = index;
        }
    }

    DX8CALL(SetIndices(static_cast<DX8IndexBufferClass *>(dyn_ib_access.Get_Index_Buffer())->Get_DX8_Index_Buffer(),
        dyn_vb_access.Get_Vertex_Offset()));
    s_indexBufferChanges++;
    s_drawCalls++;
    DX8CALL(DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vertex_count, dyn_ib_access.Get_Index_Offset(), polygon_count));
    // Debug_Statistics::Record_DX8_Polys_And_Vertices(polygon_count, vertex_count, s_renderState.shader);
#endif
}

void DX8Wrapper::Draw(unsigned int primitive_type,
    unsigned short start_index,
    unsigned short polygon_count,
    unsigned short min_vertex_index,
    unsigned short vertex_count)
{
#ifdef BUILD_WITH_D3D8
    if (!s_drawPolygonLowBoundLimit || s_drawPolygonLowBoundLimit < polygon_count) {
        Apply_Render_State_Changes();

        if (!s_EnableTriangleDraw)
            return;

        if (vertex_count < 3) {
            min_vertex_index = 0;

            switch (s_renderState.vertex_buffer_types[0]) {
                case VertexBufferClass::BUFFER_TYPE_DX8:
                case VertexBufferClass::BUFFER_TYPE_SORTING:
                    vertex_count = s_renderState.vertex_buffers[0]->Get_Vertex_Count() - s_renderState.index_base_offset
                        - s_renderState.vba_offset - min_vertex_index;
                    break;
                case VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8:
                case VertexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING:
                    vertex_count = s_renderState.vba_count;
                    break;
            }
        }
    }
    switch (s_renderState.vertex_buffer_types[0]) {
        case VertexBufferClass::BUFFER_TYPE_DX8:
        case VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8:

            switch (s_renderState.index_buffer_type) {
                case IndexBufferClass::BUFFER_TYPE_DX8:
                case IndexBufferClass::BUFFER_TYPE_DYNAMIC_DX8: {
                    // Debug_Statistics::Record_DX8_Polys_And_Vertices(polygon_count, vertex_count, s_renderState.shader);
                    s_drawCalls++;
                    DX8CALL(DrawIndexedPrimitive((D3DPRIMITIVETYPE)primitive_type,
                        min_vertex_index,
                        vertex_count,
                        start_index + s_renderState.iba_offset,
                        polygon_count));
                } break;
                case IndexBufferClass::BUFFER_TYPE_SORTING:
                case IndexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING:
                    captainslog_dbgassert(0, "VB and IB must of same type (sorting or dx8)");
                    break;
                case IndexBufferClass::BUFFER_TYPE_INVALID:
                    captainslog_assert(0);
                    break;
            }
            break;
        case VertexBufferClass::BUFFER_TYPE_SORTING:
        case VertexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING:

            switch (s_renderState.index_buffer_type) {
                case IndexBufferClass::BUFFER_TYPE_DX8:
                case IndexBufferClass::BUFFER_TYPE_DYNAMIC_DX8:
                    captainslog_dbgassert(0, "VB and IB must of same type (sorting or dx8)");
                    break;
                case IndexBufferClass::BUFFER_TYPE_SORTING:
                case IndexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING:
                    Draw_Sorting_IB_VB(primitive_type, start_index, polygon_count, min_vertex_index, vertex_count);
                    break;
                case IndexBufferClass::BUFFER_TYPE_INVALID:
                    captainslog_assert(0);
                    break;
            }
            break;
        case VertexBufferClass::BUFFER_TYPE_INVALID:
            captainslog_assert(0);
            break;
    }
#endif
}

void DX8Wrapper::Draw_Triangles(unsigned int buffer_type,
    unsigned short start_index,
    unsigned short polygon_count,
    unsigned short min_vertex_index,
    unsigned short vertex_count)
{
#ifdef BUILD_WITH_D3D8
    if (buffer_type == VertexBufferClass::BUFFER_TYPE_SORTING
        || buffer_type == VertexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING) {
        SortingRendererClass::Insert_Triangles(start_index, polygon_count, min_vertex_index, vertex_count);
    } else {
        Draw(D3DPT_TRIANGLELIST, start_index, polygon_count, min_vertex_index, vertex_count);
    }
#endif
}

void DX8Wrapper::Draw_Triangles(
    unsigned short start_index, unsigned short polygon_count, unsigned short min_vertex_index, unsigned short vertex_count)
{
#ifdef BUILD_WITH_D3D8
    Draw(D3DPT_TRIANGLELIST, start_index, polygon_count, min_vertex_index, vertex_count);
#endif
}

void DX8Wrapper::Draw_Strip(
    unsigned short start_index, unsigned short polygon_count, unsigned short min_vertex_index, unsigned short vertex_count)
{
#ifdef BUILD_WITH_D3D8
    Draw(D3DPT_TRIANGLESTRIP, start_index, polygon_count, min_vertex_index, vertex_count);
#endif
}

void DX8Wrapper::Apply_Render_State_Changes()
{
#ifdef BUILD_WITH_D3D8
    if (s_renderStateChanged) {

        if ((s_renderStateChanged & SHADER_CHANGED) != 0) {
            s_renderState.shader.Apply();
        }

        unsigned mask2 = TEXTURE0_CHANGED;

        for (unsigned i = 0; i < s_currentCaps->Max_Textures_Per_Pass(); i++, mask2 <<= 1) {
            if (s_renderStateChanged & mask2) {
                if (s_renderState.Textures[i]) {
                    s_renderState.Textures[i]->Apply(i);
                } else {
                    TextureBaseClass::Apply_Null(i);
                }
            }
        }

        if (s_renderStateChanged & MATERIAL_CHANGED) {
            VertexMaterialClass *material = const_cast<VertexMaterialClass *>(s_renderState.material);
            if (material) {
                material->Apply();
            } else {
                VertexMaterialClass::Apply_Null();
            }
        }

        if (s_renderStateChanged & LIGHTS_CHANGED) {
            unsigned mask = LIGHT0_CHANGED;
            for (int index = 0; index < GFX_LIGHT_COUNT; ++index, mask <<= 1) {
                if (s_renderStateChanged & mask) {
                    if (s_renderState.LightEnable[index]) {
                        Set_DX8_Light(index, &s_renderState.Lights[index]);
                    } else {
                        Set_DX8_Light(index, nullptr);
                    }
                }
            }
        }

        if (s_renderStateChanged & WORLD_CHANGED) {
            Set_DX8_Transform(D3DTS_WORLD, s_renderState.world);
        }

        if (s_renderStateChanged & VIEW_CHANGED) {
            Set_DX8_Transform(D3DTS_VIEW, s_renderState.view);
        }

        if (s_renderStateChanged & VERTEX_BUFFER_CHANGED) {
            for (int i = 0; i < VERTEX_BUFFERS; i++) {
                if (s_renderState.vertex_buffers[i]) {
                    switch (s_renderState.vertex_buffer_types[i]) {
                        case VertexBufferClass::BUFFER_TYPE_DX8:
                        case VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8:
                            DX8CALL(SetStreamSource(i,
                                static_cast<DX8VertexBufferClass *>(s_renderState.vertex_buffers[i])
                                    ->Get_DX8_Vertex_Buffer(),
                                s_renderState.vertex_buffers[i]->FVF_Info().Get_FVF_Size()));
                            s_vertexBufferChanges++;
                            if (s_renderState.vertex_buffers[i]->FVF_Info().Get_FVF()) {
                                s_vertexShader = s_renderState.vertex_buffers[i]->FVF_Info().Get_FVF();
                                DX8CALL(SetVertexShader(s_vertexShader));
                            }
                            break;
                        case VertexBufferClass::BUFFER_TYPE_SORTING:
                        case VertexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING:
                            break;
                        default:
                            captainslog_assert(0);
                    }
                } else {
                    DX8CALL(SetStreamSource(i, NULL, 0));
                    s_vertexBufferChanges++;
                }
            }
        }

        if (s_renderStateChanged & INDEX_BUFFER_CHANGED) {
            if (s_renderState.index_buffer) {
                switch (s_renderState.index_buffer_type) {
                    case IndexBufferClass::BUFFER_TYPE_DX8:
                    case IndexBufferClass::BUFFER_TYPE_DYNAMIC_DX8:
                        DX8CALL(SetIndices(
                            static_cast<DX8IndexBufferClass *>(s_renderState.index_buffer)->Get_DX8_Index_Buffer(),
                            s_renderState.index_base_offset + s_renderState.vba_offset));
                        s_indexBufferChanges++;
                        break;
                    case IndexBufferClass::BUFFER_TYPE_SORTING:
                    case IndexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING:
                        break;
                    default:
                        captainslog_assert(0);
                }
            } else {
                DX8CALL(SetIndices(NULL, 0));
                s_indexBufferChanges++;
            }
        }

        s_renderStateChanged &= ((unsigned)WORLD_IDENTITY | (unsigned)VIEW_IDENTITY);
    }
#endif
}

w3dtexture_t DX8Wrapper::Create_Texture(
    unsigned width, unsigned height, WW3DFormat format, MipCountType mip_level_count, w3dpool_t pool, bool rendertarget)
{
#ifdef BUILD_WITH_D3D8
    IDirect3DTexture8 *texture = nullptr;
    captainslog_assert(format != D3DFMT_P8);

    if (!rendertarget) {
        HRESULT hr = D3DXCreateTexture(
            s_d3dDevice, width, height, mip_level_count, 0, (D3DFORMAT)WW3DFormat_To_D3DFormat(format), pool, &texture);

        if (hr == D3DERR_OUTOFVIDEOMEMORY) {
            captainslog_warn("Error: Out of memory while creating texture. Trying to release assets...");
            TextureBaseClass::Invalidate_Old_Unused_Textures(5000);
            W3D::Invalidate_Mesh_Cache();
            hr = D3DXCreateTexture(
                s_d3dDevice, width, height, mip_level_count, 0, (D3DFORMAT)WW3DFormat_To_D3DFormat(format), pool, &texture);

            if (FAILED(hr)) {
                StringClass str;
                Get_WW3D_Format_Name(format, str);
                captainslog_warn(
                    "...Texture creation failed. (%d x %d, format: %s, mips: %d", width, height, str, mip_level_count);
            } else {
                captainslog_warn("...Texture creation succesful.");
            }
        }
        return texture;
    }

    HRESULT hr = D3DXCreateTexture(s_d3dDevice,
        width,
        height,
        mip_level_count,
        D3DUSAGE_RENDERTARGET,
        (D3DFORMAT)WW3DFormat_To_D3DFormat(format),
        pool,
        &texture);

    if (hr == D3DERR_NOTAVAILABLE) {
        return nullptr;
    }

    if (hr == D3DERR_OUTOFVIDEOMEMORY) {
        captainslog_warn("Error: Out of memory while creating render target. Trying to release assets...");
        TextureBaseClass::Invalidate_Old_Unused_Textures(5000);
        W3D::Invalidate_Mesh_Cache();
        hr = D3DXCreateTexture(s_d3dDevice,
            width,
            height,
            mip_level_count,
            D3DUSAGE_RENDERTARGET,
            (D3DFORMAT)WW3DFormat_To_D3DFormat(format),
            pool,
            &texture);

        if (FAILED(hr)) {
            captainslog_warn("...Render target creation failed.");
        } else {
            captainslog_warn("...Render target creation succesful.");
        }

        if (hr == D3DERR_OUTOFVIDEOMEMORY) {
            return nullptr;
        }
    }
    return texture;
#else
    return w3dtexture_t();
#endif
}

w3dtexture_t DX8Wrapper::Create_Texture(w3dsurface_t surface, MipCountType mip_level_count)
{
#ifdef BUILD_WITH_D3D8
    D3DSURFACE_DESC desc;
    memset(&desc, 0, sizeof(desc));
    surface->GetDesc(&desc);
    IDirect3DTexture8 *texture = Create_Texture(
        desc.Width, desc.Height, D3DFormat_To_WW3DFormat(desc.Format), mip_level_count, D3DPOOL_MANAGED, false);
    IDirect3DSurface8 *surf = nullptr;
    texture->GetSurfaceLevel(0, &surf);
    D3DXLoadSurfaceFromSurface(surf, nullptr, nullptr, surface, nullptr, nullptr, D3DX_FILTER_BOX, 0);
    surf->Release();

    if (mip_level_count != 1) {
        D3DXFilterTexture(texture, nullptr, 0, D3DX_FILTER_BOX);
    }

    return texture;
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
            strlcpy_tpl(buf, name);
            char *ext = strchr(buf, '.');

            if (strlen(ext) == 4) {
                if (strncasecmp(ext, ".tga", 4) != 0) {
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

#ifdef BUILD_WITH_D3D8
void DX8Wrapper::Compute_Caps(WW3DFormat display_format)
{
    delete s_currentCaps;
    s_currentCaps = new DX8Caps(s_d3dInterface, s_d3dDevice, display_format, Get_Current_Adapter_Identifier());
}
#endif

void DX8Wrapper::Set_Light(unsigned index, const LightClass &light)
{
#ifdef BUILD_WITH_D3D8
    D3DLIGHT8 dlight;
    Vector3 temp;
    memset(&dlight, 0, sizeof(D3DLIGHT8));

    switch (light.Get_Type()) {
        case LightClass::POINT: {
            dlight.Type = D3DLIGHT_POINT;
            break;
        }
        case LightClass::DIRECTIONAL: {
            dlight.Type = D3DLIGHT_DIRECTIONAL;
            break;
        }
        case LightClass::SPOT: {
            dlight.Type = D3DLIGHT_SPOT;
            break;
        }
    }

    light.Get_Diffuse(&temp);
    temp *= light.Get_Intensity();
    dlight.Diffuse.r = temp.X;
    dlight.Diffuse.g = temp.Y;
    dlight.Diffuse.b = temp.Z;
    dlight.Diffuse.a = 1.0f;

    light.Get_Specular(&temp);
    temp *= light.Get_Intensity();
    dlight.Specular.r = temp.X;
    dlight.Specular.g = temp.Y;
    dlight.Specular.b = temp.Z;
    dlight.Specular.a = 1.0f;

    light.Get_Ambient(&temp);
    temp *= light.Get_Intensity();
    dlight.Ambient.r = temp.X;
    dlight.Ambient.g = temp.Y;
    dlight.Ambient.b = temp.Z;
    dlight.Ambient.a = 1.0f;

    temp = light.Get_Position();
    dlight.Position.x = temp.X;
    dlight.Position.y = temp.Y;
    dlight.Position.z = temp.Z;

    light.Get_Spot_Direction(temp);
    dlight.Direction.x = temp.X;
    dlight.Direction.y = temp.Y;
    dlight.Direction.z = temp.Z;

    dlight.Range = light.Get_Attenuation_Range();
    dlight.Falloff = light.Get_Spot_Exponent();
    dlight.Theta = light.Get_Spot_Angle();
    dlight.Phi = light.Get_Spot_Angle();

    double a, b;
    light.Get_Far_Attenuation_Range(a, b);

    dlight.Attenuation0 = 1.0f;

    if (fabs(a - b) < 1e-5) {
        dlight.Attenuation1 = 0.0f;
    } else {
        dlight.Attenuation1 = (float)(1.0 / a);
    }

    dlight.Attenuation2 = 0.0f;

    Set_DX8_Light(index, &dlight);
#endif
}

void DX8Wrapper::Set_Light_Environment(LightEnvironmentClass *light_env)
{
#ifdef BUILD_WITH_D3D8
    if (light_env) {

        int light_count = light_env->Get_Light_Count();
        Set_DX8_Render_State(D3DRS_AMBIENT, Convert_Color(light_env->Get_Equivalent_Ambient(), 0.0f));

        D3DLIGHT8 light;
        memset(&light, 0, sizeof(D3DLIGHT8));
        light.Type = D3DLIGHT_DIRECTIONAL;
        int light_index;

        for (light_index = 0; light_index < light_count; ++light_index) {
            (Vector3 &)light.Diffuse = light_env->Get_Light_Diffuse(light_index);
            Vector3 dir = -light_env->Get_Light_Direction(light_index);
            light.Direction = (const D3DVECTOR &)(dir);

            if (!light_index) {
                light.Specular.r = 1.0f;
                light.Specular.g = 1.0f;
                light.Specular.b = 1.0f;
            }

            if (light_env->Is_Point_Light(light_index)) {
                light.Range = light_env->Get_Point_Outer_Radius(light_index);
                (Vector3 &)light.Diffuse = light_env->Get_Point_Diffuse(light_index);
                (Vector3 &)light.Ambient = light_env->Get_Point_Ambient(light_index);
                (Vector3 &)light.Position = light_env->Get_Point_Center(light_index);
                light.Type = D3DLIGHT_POINT;
                light.Attenuation0 = 1.0f;

                if (GameMath::Fabs(
                        light_env->Get_Point_Inner_Radius(light_index) - light_env->Get_Point_Outer_Radius(light_index))
                    < 1e-5) {
                    light.Attenuation1 = 0.0f;
                } else {
                    light.Attenuation1 = 0.1f / light_env->Get_Point_Inner_Radius(light_index);
                }

                light.Attenuation2 =
                    8.0f / (light_env->Get_Point_Outer_Radius(light_index) * light_env->Get_Point_Outer_Radius(light_index));
            }

            Set_Light(light_index, &light);
        }

        for (; light_index < GFX_LIGHT_COUNT; ++light_index) {
            Set_Light(light_index, nullptr);
        }
    }
#endif
}

#ifdef BUILD_WITH_D3D8
IDirect3DSurface8 *DX8Wrapper::Get_DX8_Front_Buffer()
{
    D3DDISPLAYMODE mode;
    DX8CALL(GetDisplayMode(&mode));
    IDirect3DSurface8 *fb = nullptr;
    DX8CALL(CreateImageSurface(mode.Width, mode.Height, D3DFMT_A8R8G8B8, &fb));
    DX8CALL(GetFrontBuffer(fb));
    return fb;
}
#endif

SurfaceClass *DX8Wrapper::Get_DX8_Back_Buffer(unsigned int num)
{
#ifdef BUILD_WITH_D3D8
    IDirect3DSurface8 *bb;
    SurfaceClass *surf = NULL;
    DX8CALL(GetBackBuffer(num, D3DBACKBUFFER_TYPE_MONO, &bb));

    if (bb) {
        surf = new SurfaceClass(bb);
        bb->Release();
    }

    return surf;
#else
    return nullptr;
#endif
}

TextureClass *DX8Wrapper::Create_Render_Target(int width, int height, WW3DFormat format)
{
#ifdef BUILD_WITH_D3D8
    g_numberOfDx8Calls++;

    if (format == WW3D_FORMAT_UNKNOWN) {
        D3DDISPLAYMODE mode;
        DX8CALL(GetDisplayMode(&mode));
        format = D3DFormat_To_WW3DFormat(mode.Format);
    }

    if (s_currentCaps->Supports_Render_To_Texture_Format(format)) {
        float f = width;
        D3DCAPS8 caps = s_currentCaps->Get_DX8_Caps();
        if (height > 0 && height < width) {
            f = height;
        }
        float f2 = (float)Find_POT((int)f);
        if (f2 > caps.MaxTextureWidth) {
            f2 = caps.MaxTextureWidth;
        }
        if (f2 > caps.MaxTextureHeight) {
            f2 = caps.MaxTextureHeight;
        }
        TextureClass *texture =
            new TextureClass((unsigned int)f2, (unsigned int)f2, format, MIP_LEVELS_1, POOL_DEFAULT, true, true);
        if (!texture->Peek_Platform_Base_Texture()) {
            captainslog_warn("DX8Wrapper - Render target creation failed!");
            Ref_Ptr_Release(texture);
            return nullptr;
        }
        return texture;
    } else {
        captainslog_warn("DX8Wrapper - Render target format is not supported");
    }

    return nullptr;
#else
    return nullptr;
#endif
}

void DX8Wrapper::Set_Render_Target_With_Z(TextureClass *texture, ZTextureClass *z_texture)
{
#ifdef BUILD_WITH_D3D8
    captainslog_assert(texture != nullptr);
    IDirect3DSurface8 *d3d_surf = texture->Get_D3D_Surface_Level(0);
    captainslog_assert(d3d_surf != nullptr);

    if (z_texture) {
        captainslog_dbgassert(0, "ZTextureClass not supported");
    }

    Set_Render_Target(d3d_surf, true);
    d3d_surf->Release();
    s_isRenderToTexture = true;
#endif
}

void DX8Wrapper::Set_Render_Target(w3dsurface_t render_target, bool use_default_depth_buffer)
{
#ifdef BUILD_WITH_D3D8
    if (render_target == nullptr || render_target == s_defaultRenderTarget) {
        if (s_currentRenderTarget) {
            captainslog_assert(s_defaultRenderTarget != nullptr);
        }

        if (s_defaultRenderTarget != nullptr) {
            DX8CALL(SetRenderTarget(s_defaultRenderTarget, s_defaultDepthBuffer));
            s_defaultRenderTarget->Release();
            s_defaultRenderTarget = nullptr;

            if (s_defaultDepthBuffer != nullptr) {
                s_defaultDepthBuffer->Release();
                s_defaultDepthBuffer = nullptr;
            }
        }

        if (s_currentRenderTarget != nullptr) {
            s_currentRenderTarget->Release();
            s_currentRenderTarget = nullptr;
        }

        if (s_currentDepthBuffer != nullptr) {
            s_currentDepthBuffer->Release();
            s_currentDepthBuffer = nullptr;
        }

    } else if (render_target != s_currentRenderTarget) {
        captainslog_assert(s_defaultRenderTarget == nullptr);

        if (s_defaultDepthBuffer == nullptr) {
            DX8CALL(GetDepthStencilSurface(&s_defaultDepthBuffer));
        }

        if (s_defaultRenderTarget == nullptr) {
            DX8CALL(GetRenderTarget(&s_defaultRenderTarget));
        }

        if (s_currentRenderTarget != nullptr) {
            s_currentRenderTarget->Release();
            s_currentRenderTarget = nullptr;
        }

        if (s_currentDepthBuffer != nullptr) {
            s_currentDepthBuffer->Release();
            s_currentDepthBuffer = nullptr;
        }

        s_currentRenderTarget = render_target;
        captainslog_assert(s_currentRenderTarget != nullptr);

        if (s_currentRenderTarget != nullptr) {
            s_currentRenderTarget->AddRef();

            if (use_default_depth_buffer) {
                DX8CALL(SetRenderTarget(s_currentRenderTarget, s_defaultDepthBuffer));
            } else {
                DX8CALL(SetRenderTarget(s_currentRenderTarget, nullptr));
            }
        }
    }
    s_isRenderToTexture = false;
#endif
}

void DX8Wrapper::Set_Gamma(float gamma, float bright, float contrast, bool calibrate, bool uselimit)
{
#ifdef BUILD_WITH_D3D8
    gamma = std::clamp(gamma, 0.6f, 6.0f);
    bright = std::clamp(bright, -0.5f, 0.5f);
    contrast = std::clamp(contrast, 0.5f, 2.0f);
    float oo_gamma = 1.0f / gamma;
    g_numberOfDx8Calls++;
    unsigned long flag = (calibrate ? D3DSGR_CALIBRATE : D3DSGR_NO_CALIBRATION);
    D3DGAMMARAMP ramp;
    float limit;

    if (uselimit) {
        limit = (contrast - 1) / 2 * contrast;
    } else {
        limit = 0.0f;
    }

    for (int i = 0; i < 256; i++) {
        float in, out;
        in = i / 256.0f;
        float x = in - limit;
        x = std::clamp(x, 0.0f, 1.0f);
        x = powf(x, oo_gamma);
        out = contrast * x + bright;
        out = std::clamp(out, 0.0f, 1.0f);
        ramp.red[i] = (WORD)(out * 65535);
        ramp.green[i] = (WORD)(out * 65535);
        ramp.blue[i] = (WORD)(out * 65535);
    }

    if (s_currentCaps->Support_Gamma()) {
        s_d3dDevice->SetGammaRamp(flag, &ramp);
    } else {
        HWND hwnd = GetDesktopWindow();
        HDC hdc = GetDC(hwnd);

        if (hdc) {
            SetDeviceGammaRamp(hdc, &ramp);
            ReleaseDC(hwnd, hdc);
        }
    }
#endif
}

#ifdef BUILD_WITH_D3D8
const char *DX8Wrapper::Get_DX8_Render_State_Name(D3DRENDERSTATETYPE state)
{
    switch (state) {
        case D3DRS_ZENABLE:
            return "D3DRS_ZENABLE";
        case D3DRS_FILLMODE:
            return "D3DRS_FILLMODE";
        case D3DRS_SHADEMODE:
            return "D3DRS_SHADEMODE";
        case D3DRS_LINEPATTERN:
            return "D3DRS_LINEPATTERN";
        case D3DRS_ZWRITEENABLE:
            return "D3DRS_ZWRITEENABLE";
        case D3DRS_ALPHATESTENABLE:
            return "D3DRS_ALPHATESTENABLE";
        case D3DRS_LASTPIXEL:
            return "D3DRS_LASTPIXEL";
        case D3DRS_SRCBLEND:
            return "D3DRS_SRCBLEND";
        case D3DRS_DESTBLEND:
            return "D3DRS_DESTBLEND";
        case D3DRS_CULLMODE:
            return "D3DRS_CULLMODE";
        case D3DRS_ZFUNC:
            return "D3DRS_ZFUNC";
        case D3DRS_ALPHAREF:
            return "D3DRS_ALPHAREF";
        case D3DRS_ALPHAFUNC:
            return "D3DRS_ALPHAFUNC";
        case D3DRS_DITHERENABLE:
            return "D3DRS_DITHERENABLE";
        case D3DRS_ALPHABLENDENABLE:
            return "D3DRS_ALPHABLENDENABLE";
        case D3DRS_FOGENABLE:
            return "D3DRS_FOGENABLE";
        case D3DRS_SPECULARENABLE:
            return "D3DRS_SPECULARENABLE";
        case D3DRS_ZVISIBLE:
            return "D3DRS_ZVISIBLE";
        case D3DRS_FOGCOLOR:
            return "D3DRS_FOGCOLOR";
        case D3DRS_FOGTABLEMODE:
            return "D3DRS_FOGTABLEMODE";
        case D3DRS_FOGSTART:
            return "D3DRS_FOGSTART";
        case D3DRS_FOGEND:
            return "D3DRS_FOGEND";
        case D3DRS_FOGDENSITY:
            return "D3DRS_FOGDENSITY";
        case D3DRS_EDGEANTIALIAS:
            return "D3DRS_EDGEANTIALIAS";
        case D3DRS_ZBIAS:
            return "D3DRS_ZBIAS";
        case D3DRS_RANGEFOGENABLE:
            return "D3DRS_RANGEFOGENABLE";
        case D3DRS_STENCILENABLE:
            return "D3DRS_STENCILENABLE";
        case D3DRS_STENCILFAIL:
            return "D3DRS_STENCILFAIL";
        case D3DRS_STENCILZFAIL:
            return "D3DRS_STENCILZFAIL";
        case D3DRS_STENCILPASS:
            return "D3DRS_STENCILPASS";
        case D3DRS_STENCILFUNC:
            return "D3DRS_STENCILFUNC";
        case D3DRS_STENCILREF:
            return "D3DRS_STENCILREF";
        case D3DRS_STENCILMASK:
            return "D3DRS_STENCILMASK";
        case D3DRS_STENCILWRITEMASK:
            return "D3DRS_STENCILWRITEMASK";
        case D3DRS_TEXTUREFACTOR:
            return "D3DRS_TEXTUREFACTOR";
        case D3DRS_WRAP0:
            return "D3DRS_WRAP0";
        case D3DRS_WRAP1:
            return "D3DRS_WRAP1";
        case D3DRS_WRAP2:
            return "D3DRS_WRAP2";
        case D3DRS_WRAP3:
            return "D3DRS_WRAP3";
        case D3DRS_WRAP4:
            return "D3DRS_WRAP4";
        case D3DRS_WRAP5:
            return "D3DRS_WRAP5";
        case D3DRS_WRAP6:
            return "D3DRS_WRAP6";
        case D3DRS_WRAP7:
            return "D3DRS_WRAP7";
        case D3DRS_CLIPPING:
            return "D3DRS_CLIPPING";
        case D3DRS_LIGHTING:
            return "D3DRS_LIGHTING";
        case D3DRS_AMBIENT:
            return "D3DRS_AMBIENT";
        case D3DRS_FOGVERTEXMODE:
            return "D3DRS_FOGVERTEXMODE";
        case D3DRS_COLORVERTEX:
            return "D3DRS_COLORVERTEX";
        case D3DRS_LOCALVIEWER:
            return "D3DRS_LOCALVIEWER";
        case D3DRS_NORMALIZENORMALS:
            return "D3DRS_NORMALIZENORMALS";
        case D3DRS_DIFFUSEMATERIALSOURCE:
            return "D3DRS_DIFFUSEMATERIALSOURCE";
        case D3DRS_SPECULARMATERIALSOURCE:
            return "D3DRS_SPECULARMATERIALSOURCE";
        case D3DRS_AMBIENTMATERIALSOURCE:
            return "D3DRS_AMBIENTMATERIALSOURCE";
        case D3DRS_EMISSIVEMATERIALSOURCE:
            return "D3DRS_EMISSIVEMATERIALSOURCE";
        case D3DRS_VERTEXBLEND:
            return "D3DRS_VERTEXBLEND";
        case D3DRS_CLIPPLANEENABLE:
            return "D3DRS_CLIPPLANEENABLE";
        case D3DRS_SOFTWAREVERTEXPROCESSING:
            return "D3DRS_SOFTWAREVERTEXPROCESSING";
        case D3DRS_POINTSIZE:
            return "D3DRS_POINTSIZE";
        case D3DRS_POINTSIZE_MIN:
            return "D3DRS_POINTSIZE_MIN";
        case D3DRS_POINTSPRITEENABLE:
            return "D3DRS_POINTSPRITEENABLE";
        case D3DRS_POINTSCALEENABLE:
            return "D3DRS_POINTSCALEENABLE";
        case D3DRS_POINTSCALE_A:
            return "D3DRS_POINTSCALE_A";
        case D3DRS_POINTSCALE_B:
            return "D3DRS_POINTSCALE_B";
        case D3DRS_POINTSCALE_C:
            return "D3DRS_POINTSCALE_C";
        case D3DRS_MULTISAMPLEANTIALIAS:
            return "D3DRS_MULTISAMPLEANTIALIAS";
        case D3DRS_MULTISAMPLEMASK:
            return "D3DRS_MULTISAMPLEMASK";
        case D3DRS_PATCHEDGESTYLE:
            return "D3DRS_PATCHEDGESTYLE";
        case D3DRS_PATCHSEGMENTS:
            return "D3DRS_PATCHSEGMENTS";
        case D3DRS_DEBUGMONITORTOKEN:
            return "D3DRS_DEBUGMONITORTOKEN";
        case D3DRS_POINTSIZE_MAX:
            return "D3DRS_POINTSIZE_MAX";
        case D3DRS_INDEXEDVERTEXBLENDENABLE:
            return "D3DRS_INDEXEDVERTEXBLENDENABLE";
        case D3DRS_COLORWRITEENABLE:
            return "D3DRS_COLORWRITEENABLE";
        case D3DRS_TWEENFACTOR:
            return "D3DRS_TWEENFACTOR";
        case D3DRS_BLENDOP:
            return "D3DRS_BLENDOP";
        default:
            return "UNKNOWN";
    }
}

const char *DX8Wrapper::Get_DX8_Texture_Stage_State_Name(D3DTEXTURESTAGESTATETYPE state)
{
    switch (state) {
        case D3DTSS_COLOROP:
            return "D3DTSS_COLOROP";
        case D3DTSS_COLORARG1:
            return "D3DTSS_COLORARG1";
        case D3DTSS_COLORARG2:
            return "D3DTSS_COLORARG2";
        case D3DTSS_ALPHAOP:
            return "D3DTSS_ALPHAOP";
        case D3DTSS_ALPHAARG1:
            return "D3DTSS_ALPHAARG1";
        case D3DTSS_ALPHAARG2:
            return "D3DTSS_ALPHAARG2";
        case D3DTSS_BUMPENVMAT00:
            return "D3DTSS_BUMPENVMAT00";
        case D3DTSS_BUMPENVMAT01:
            return "D3DTSS_BUMPENVMAT01";
        case D3DTSS_BUMPENVMAT10:
            return "D3DTSS_BUMPENVMAT10";
        case D3DTSS_BUMPENVMAT11:
            return "D3DTSS_BUMPENVMAT11";
        case D3DTSS_TEXCOORDINDEX:
            return "D3DTSS_TEXCOORDINDEX";
        case D3DTSS_ADDRESSU:
            return "D3DTSS_ADDRESSU";
        case D3DTSS_ADDRESSV:
            return "D3DTSS_ADDRESSV";
        case D3DTSS_BORDERCOLOR:
            return "D3DTSS_BORDERCOLOR";
        case D3DTSS_MAGFILTER:
            return "D3DTSS_MAGFILTER";
        case D3DTSS_MINFILTER:
            return "D3DTSS_MINFILTER";
        case D3DTSS_MIPFILTER:
            return "D3DTSS_MIPFILTER";
        case D3DTSS_MIPMAPLODBIAS:
            return "D3DTSS_MIPMAPLODBIAS";
        case D3DTSS_MAXMIPLEVEL:
            return "D3DTSS_MAXMIPLEVEL";
        case D3DTSS_MAXANISOTROPY:
            return "D3DTSS_MAXANISOTROPY";
        case D3DTSS_BUMPENVLSCALE:
            return "D3DTSS_BUMPENVLSCALE";
        case D3DTSS_BUMPENVLOFFSET:
            return "D3DTSS_BUMPENVLOFFSET";
        case D3DTSS_TEXTURETRANSFORMFLAGS:
            return "D3DTSS_TEXTURETRANSFORMFLAGS";
        case D3DTSS_ADDRESSW:
            return "D3DTSS_ADDRESSW";
        case D3DTSS_COLORARG0:
            return "D3DTSS_COLORARG0";
        case D3DTSS_ALPHAARG0:
            return "D3DTSS_ALPHAARG0";
        case D3DTSS_RESULTARG:
            return "D3DTSS_RESULTARG";
        default:
            return "UNKNOWN";
    }
}

void DX8Wrapper::Get_DX8_Render_State_Value_Name(StringClass &name, D3DRENDERSTATETYPE state, unsigned value)
{
    switch (state) {
        case D3DRS_ZENABLE:
            name = Get_DX8_ZBuffer_Type_Name(value);
            break;

        case D3DRS_FILLMODE:
            name = Get_DX8_Fill_Mode_Name(value);
            break;

        case D3DRS_SHADEMODE:
            name = Get_DX8_Shade_Mode_Name(value);
            break;

        case D3DRS_LINEPATTERN:
        case D3DRS_FOGCOLOR:
        case D3DRS_ALPHAREF:
        case D3DRS_STENCILMASK:
        case D3DRS_STENCILWRITEMASK:
        case D3DRS_TEXTUREFACTOR:
        case D3DRS_AMBIENT:
        case D3DRS_CLIPPLANEENABLE:
        case D3DRS_MULTISAMPLEMASK:
            name.Format("0x%x", value);
            break;

        case D3DRS_ZWRITEENABLE:
        case D3DRS_ALPHATESTENABLE:
        case D3DRS_LASTPIXEL:
        case D3DRS_DITHERENABLE:
        case D3DRS_ALPHABLENDENABLE:
        case D3DRS_FOGENABLE:
        case D3DRS_SPECULARENABLE:
        case D3DRS_STENCILENABLE:
        case D3DRS_RANGEFOGENABLE:
        case D3DRS_EDGEANTIALIAS:
        case D3DRS_CLIPPING:
        case D3DRS_LIGHTING:
        case D3DRS_COLORVERTEX:
        case D3DRS_LOCALVIEWER:
        case D3DRS_NORMALIZENORMALS:
        case D3DRS_SOFTWAREVERTEXPROCESSING:
        case D3DRS_POINTSPRITEENABLE:
        case D3DRS_POINTSCALEENABLE:
        case D3DRS_MULTISAMPLEANTIALIAS:
        case D3DRS_INDEXEDVERTEXBLENDENABLE:
            name = value ? "TRUE" : "FALSE";
            break;

        case D3DRS_SRCBLEND:
        case D3DRS_DESTBLEND:
            name = Get_DX8_Blend_Name(value);
            break;

        case D3DRS_CULLMODE:
            name = Get_DX8_Cull_Mode_Name(value);
            break;

        case D3DRS_ZFUNC:
        case D3DRS_ALPHAFUNC:
        case D3DRS_STENCILFUNC:
            name = Get_DX8_Cmp_Func_Name(value);
            break;

        case D3DRS_ZVISIBLE:
            name = "NOTSUPPORTED";
            break;

        case D3DRS_FOGTABLEMODE:
        case D3DRS_FOGVERTEXMODE:
            name = Get_DX8_Fog_Mode_Name(value);
            break;

        case D3DRS_FOGSTART:
        case D3DRS_FOGEND:
        case D3DRS_FOGDENSITY:
        case D3DRS_POINTSIZE:
        case D3DRS_POINTSIZE_MIN:
        case D3DRS_POINTSCALE_A:
        case D3DRS_POINTSCALE_B:
        case D3DRS_POINTSCALE_C:
        case D3DRS_PATCHSEGMENTS:
        case D3DRS_POINTSIZE_MAX:
        case D3DRS_TWEENFACTOR:
            name.Format("%f", *reinterpret_cast<float *>(&value));
            break;

        case D3DRS_ZBIAS:
        case D3DRS_STENCILREF:
            name.Format("%d", value);
            break;

        case D3DRS_STENCILFAIL:
        case D3DRS_STENCILZFAIL:
        case D3DRS_STENCILPASS:
            name = Get_DX8_Stencil_Op_Name(value);
            break;

        case D3DRS_WRAP0:
        case D3DRS_WRAP1:
        case D3DRS_WRAP2:
        case D3DRS_WRAP3:
        case D3DRS_WRAP4:
        case D3DRS_WRAP5:
        case D3DRS_WRAP6:
        case D3DRS_WRAP7:
            name = "0";
            if (value & D3DWRAP_U)
                name += "|D3DWRAP_U";
            if (value & D3DWRAP_V)
                name += "|D3DWRAP_V";
            if (value & D3DWRAP_W)
                name += "|D3DWRAP_W";
            break;

        case D3DRS_DIFFUSEMATERIALSOURCE:
        case D3DRS_SPECULARMATERIALSOURCE:
        case D3DRS_AMBIENTMATERIALSOURCE:
        case D3DRS_EMISSIVEMATERIALSOURCE:
            name = Get_DX8_Material_Source_Name(value);
            break;

        case D3DRS_VERTEXBLEND:
            name = Get_DX8_Vertex_Blend_Flag_Name(value);
            break;

        case D3DRS_PATCHEDGESTYLE:
            name = Get_DX8_Patch_Edge_Style_Name(value);
            break;

        case D3DRS_DEBUGMONITORTOKEN:
            name = Get_DX8_Debug_Monitor_Token_Name(value);
            break;

        case D3DRS_COLORWRITEENABLE:
            name = "0";
            if (value & D3DCOLORWRITEENABLE_RED)
                name += "|D3DCOLORWRITEENABLE_RED";
            if (value & D3DCOLORWRITEENABLE_GREEN)
                name += "|D3DCOLORWRITEENABLE_GREEN";
            if (value & D3DCOLORWRITEENABLE_BLUE)
                name += "|D3DCOLORWRITEENABLE_BLUE";
            if (value & D3DCOLORWRITEENABLE_ALPHA)
                name += "|D3DCOLORWRITEENABLE_ALPHA";
            break;
        case D3DRS_BLENDOP:
            name = Get_DX8_Blend_Op_Name(value);
            break;
        default:
            name.Format("UNKNOWN (%d)", value);
            break;
    }
}

void DX8Wrapper::Get_DX8_Texture_Stage_State_Value_Name(StringClass &name, D3DTEXTURESTAGESTATETYPE state, unsigned value)
{
    switch (state) {
        case D3DTSS_COLOROP:
        case D3DTSS_ALPHAOP:
            name = Get_DX8_Texture_Op_Name(value);
            break;

        case D3DTSS_COLORARG0:
        case D3DTSS_COLORARG1:
        case D3DTSS_COLORARG2:
        case D3DTSS_ALPHAARG0:
        case D3DTSS_ALPHAARG1:
        case D3DTSS_ALPHAARG2:
        case D3DTSS_RESULTARG:
            name = Get_DX8_Texture_Arg_Name(value);
            break;

        case D3DTSS_ADDRESSU:
        case D3DTSS_ADDRESSV:
        case D3DTSS_ADDRESSW:
            name = Get_DX8_Texture_Address_Name(value);
            break;

        case D3DTSS_MAGFILTER:
        case D3DTSS_MINFILTER:
        case D3DTSS_MIPFILTER:
            name = Get_DX8_Texture_Filter_Name(value);
            break;

        case D3DTSS_TEXTURETRANSFORMFLAGS:
            name = Get_DX8_Texture_Transform_Flag_Name(value);
            break;

        // Floating point values
        case D3DTSS_MIPMAPLODBIAS:
        case D3DTSS_BUMPENVMAT00:
        case D3DTSS_BUMPENVMAT01:
        case D3DTSS_BUMPENVMAT10:
        case D3DTSS_BUMPENVMAT11:
        case D3DTSS_BUMPENVLSCALE:
        case D3DTSS_BUMPENVLOFFSET:
            name.Format("%f", *reinterpret_cast<float *>(&value));
            break;

        case D3DTSS_TEXCOORDINDEX:
            if ((value & 0xffff0000) == D3DTSS_TCI_CAMERASPACENORMAL) {
                name.Format("D3DTSS_TCI_CAMERASPACENORMAL|%d", value & 0xffff);
            } else if ((value & 0xffff0000) == D3DTSS_TCI_CAMERASPACEPOSITION) {
                name.Format("D3DTSS_TCI_CAMERASPACEPOSITION|%d", value & 0xffff);
            } else if ((value & 0xffff0000) == D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR) {
                name.Format("D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR|%d", value & 0xffff);
            } else {
                name.Format("%d", value);
            }
            break;

        // Integer value
        case D3DTSS_MAXMIPLEVEL:
        case D3DTSS_MAXANISOTROPY:
            name.Format("%d", value);
            break;
        // Hex values
        case D3DTSS_BORDERCOLOR:
            name.Format("0x%x", value);
            break;

        default:
            name.Format("UNKNOWN (%d)", value);
            break;
    }
}

void DX8Wrapper::Log_DX8_ErrorCode(HRESULT error)
{
    // This made use the d3d8x part of the sdk found in the DirectX 8.1 SDK which is hard to find.
    captainslog_error("Direct3D8 generated error %x.", error);
}

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

WW3DFormat DX8Wrapper::Get_Back_Buffer_Format()
{
#ifdef BUILD_WITH_D3D8
    return D3DFormat_To_WW3DFormat(s_presentParameters.BackBufferFormat);
#else
    return WW3D_FORMAT_UNKNOWN;
#endif
}
