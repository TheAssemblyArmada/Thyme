/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for interrogating the systems graphics capabilities.
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
#include "w3dformat.h"
#include "w3dtypes.h"
#include "wwstring.h"
#include <new>

class DX8Caps
{
    ALLOW_HOOKING
    enum Vendors
    {
        VENDOR_UNKNOWN,
        VENDOR_NVIDIA,
        VENDOR_AMD,
        VENDOR_INTEL,
        VENDOR_S3,
        VENDOR_POWERVR,
        VENDOR_MATROX,
        VENDOR_3DFX,
        VENDOR_3DLABS,
        VENDOR_CIRRUSLOGIC,
        VENDOR_RENDITION,
    };

    enum DriverStatus
    {
        DRIVER_UNKNOWN,
        DRIVER_GOOD,
        DRIVER_OK,
        DRIVER_BAD,
    };

public:
    DX8Caps(w3dhandle_t handle, const w3dcaps_t &caps, WW3DFormat format, const w3dadapterid_t &identifier);
    DX8Caps(w3dhandle_t handle, w3ddevice_t device, WW3DFormat format, const w3dadapterid_t &identifier);
    ~DX8Caps() {}

    void Shutdown();
    void Init_Caps(w3ddevice_t device);
    void Compute_Caps(WW3DFormat format, const w3dadapterid_t &identifier);
    bool Is_Valid_Display_Format(int w, int h, WW3DFormat format);

    bool Supports_Dot3_Blend() const { return m_supportDot3; }
    bool Supports_Large_Point_Size() const { return m_supportLargePointSize; }
    bool Supports_DXTC() const { return m_supportDXTC; }
    bool Supports_ZBias() const { return m_supportZBias; }
    bool Supports_Texture_Format(WW3DFormat format) const { return m_supportTextureFormat[format]; }
    bool Supports_Render_To_Texture_Format(WW3DFormat format) const { return m_supportRenderToTextureFormat[format]; }
    bool Use_Hardware_TnL() const { return m_useHardwareTnL; }
    uint32_t Max_Textures_Per_Pass() const { return m_maxTexturesPerPass; }
    w3dcaps_t Get_DX8_Caps() const { return m_caps; }
    bool Support_Gamma() const { return m_supportGamma; }
    bool Support_Bump_Envmap() const { return m_supportBumpEnvmap; }
    bool Support_Bump_Envmap_Luminance() const { return m_supportBumpEnvmapLuminance; }
    bool Supports_Fog() const { return m_supportFog; }
    bool Supports_Mod_Alpha_Add_Color() const { return m_supportsModulateAlphaAddColor; }
    int Get_Device_Number() const { return m_deviceNumber; }
    int Get_Vendor_Number() const { return m_vendorNumber; }

#ifdef BUILD_WITH_D3D8
    unsigned Get_Max_Tex_Width() const { return m_caps.MaxTextureWidth; }
    unsigned Get_Max_Tex_Height() const { return m_caps.MaxTextureHeight; }
    unsigned Get_Max_Vol_Extent() const { return m_caps.MaxVolumeExtent; }
    unsigned Get_Filter_Caps() const { return m_caps.TextureFilterCaps; }
    unsigned Get_Op_Caps() const { return m_caps.TextureOpCaps; }
#else
    unsigned Get_Max_Tex_Width() const { return 1024; }
    unsigned Get_Max_Tex_Height() const { return 1024; }
    unsigned Get_Max_Vol_Extent() const { return 1024; }
    unsigned Get_Filter_Caps() const { return 0; }
#endif

private:
    void Check_Texture_Format_Support(WW3DFormat format, const w3dcaps_t &caps);
    void Check_Render_To_Texture_Support(WW3DFormat format, const w3dcaps_t &caps);
    void Check_Depth_Stencil_Support(WW3DFormat format, const w3dcaps_t &caps);
    void Check_Texture_Compression_Support(const w3dcaps_t &caps);
    void Check_Bumpmap_Support(const w3dcaps_t &caps);
    void Check_Shader_Support(const w3dcaps_t &caps);
    void Check_Max_Texture_Support(const w3dcaps_t &caps);
    void Check_Driver_Version_Status();
    void Vendor_Specific_Hacks(const w3dadapterid_t &identifier);
    int Define_Vendor(unsigned vendor_id);
    int Get_NVidia_Device(unsigned device_id);
    int Get_AMD_Device(unsigned device_id);
    int Get_Intel_Device(unsigned device_id);
    int Get_S3_Device(unsigned device_id);
    int Get_PowerVR_Device(unsigned device_id);
    int Get_Matrox_Device(unsigned device_id);
    int Get_3Dfx_Device(unsigned device_id);
    int Get_3DLabs_Device(unsigned device_id);
    static bool Has_Feature(unsigned caps, unsigned feature) { return (caps & feature) == feature; }

#ifdef GAME_DLL
    DX8Caps *Hook_Ctor1(w3dhandle_t handle, const w3dcaps_t &caps, WW3DFormat format, const w3dadapterid_t &identifier)
    {
        return new (this) DX8Caps(handle, caps, format, identifier);
    }

    DX8Caps *Hook_Ctor2(w3dhandle_t handle, w3ddevice_t device, WW3DFormat format, const w3dadapterid_t &identifier)
    {
        return new (this) DX8Caps(handle, device, format, identifier);
    }

public:
#endif

private:
    int m_widthLimit;
    int m_heightLimit;
    w3dcaps_t m_caps;
    bool m_useHardwareTnL;
    bool m_supportDXTC;
    bool m_supportGamma;
    bool m_supportNPatches;
    bool m_supportBumpEnvmap;
    bool m_supportBumpEnvmapLuminance;
    bool m_supportTextureFormat[WW3D_FORMAT_COUNT];
    bool m_supportRenderToTextureFormat[WW3D_FORMAT_COUNT];
    bool m_supportDepthStencilFormat[WW3DZ_FORMAT_COUNT];
    bool m_supportZBias;
    bool m_supportAnisotropicFiltering;
    bool m_supportsModulateAlphaAddColor;
    bool m_supportDot3;
    bool m_supportLargePointSize;
    bool m_supportsCubeMap;
    bool m_supportMultiTexture;
    bool m_supportFog;
    uint32_t m_maxTexturesPerPass;
    uint32_t m_vertexShaderVersion;
    uint32_t m_pixelShaderVersion;
    uint32_t m_maxSupportedTextures;
    int m_deviceNumber;
    int m_driverBuildNum;
    int m_driverStatus;
    int m_vendorNumber;
    StringClass m_driverFilename;
    w3dhandle_t m_d3dInterface;
    StringClass m_videoCardSpecString;
    StringClass m_videoCardName;

#ifdef GAME_DLL
    static StringClass &s_capsWorkStrings;
#else
    static StringClass s_capsWorkStrings;
#endif
};
