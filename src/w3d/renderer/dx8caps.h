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

class DX8Caps
{
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

    void Shutdown();
    void Init_Caps(w3ddevice_t device);
    void Compute_Caps(WW3DFormat format, const w3dadapterid_t &identifier);
    bool Is_Valid_Display_Format(int w, int h, WW3DFormat format);

private:
    void Check_Texture_Format_Support(WW3DFormat format, const w3dcaps_t &caps);
    void Check_Render_To_Texture_Support(WW3DFormat format, const w3dcaps_t &caps);
    void Check_Depth_Stencil_Support(WW3DFormat format, const w3dcaps_t &caps);
    void Check_Texture_Compression_Support(const w3dcaps_t &caps);
    void Check_Bumpmap_Support(const w3dcaps_t &caps);
    void Check_Shader_Support(const w3dcaps_t &caps);
    void Check_Max_Texture_Support(const w3dcaps_t &caps);
    void Check_Driver_Version_Status();
    void Vendor_Specific_Hacks();
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

private:
    int m_widthLimit;
    int m_heightLimit;
    w3dcaps_t m_caps;
    bool m_useTnL;
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
    bool m_supportDot3Blend;
    bool m_supportLargePointSize;
    bool m_supportsCubeMap;
    bool m_supportMultiTexture;
    bool m_supportFog;
    int m_maxTexturesPerPass;
    unsigned m_vertexShaderVersion;
    unsigned m_pixelShaderVersion;
    int m_maxSupportedTextures;
    int m_deviceNumber;
    int m_driverBuildNum;
    int m_driverStatus;
    int m_vendorNumber;
    StringClass m_driverFilename;
    w3dhandle_t m_d3dInterface;
    StringClass m_videoCardSpecString;
    StringClass m_videoCardName;

#ifndef THYME_STANDALONE
    static StringClass &s_videoCardDetails;
#else
    static StringClass s_videoCardDetails;
#endif
};

#ifndef THYME_STANDALONE
#include "hooker.h"
#endif
