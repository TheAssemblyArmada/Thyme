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
#include "dx8caps.h"
#include "dx8wrapper.h"
#include <captainslog.h>
#include <cstring>

using std::memcpy;
using std::memset;

#ifndef GAME_DLL
StringClass DX8Caps::s_capsWorkStrings;
#else
#include "hooker.h"
#endif

namespace
{
const char *g_videoCardVendors[] = {
    "Unknown", "NVidia", "ATI", "Intel", "S3", "PowerVR", "Matrox", "3Dfx", "3DLabs", "CirrusLogic", "Rendition"
};

const char *g_nVidiaDevices[] = { "Unknown NVidia device",
    "GeForce3",
    "Quadro2 PRO",
    "GeForce2 Go",
    "GeForce2 ULTRA",
    "GeForce2 GTS",
    "Quadro",
    "GeForce DDR",
    "GeForce 256",
    "TNT2 Aladdin",
    "TNT2",
    "TNT2 ULTRA",
    "TNT2 Vanta",
    "TNT2 M64",
    "TNT",
    "RIVA 128",
    "TNT Vanta",
    "NV1",
    "GeForce2 MXGeForce4 Ti 4600",
    "GeForce4 Ti 4400",
    "GeForce4 Ti",
    "GeForce4 Ti 4200",
    "GeForce4 MX 460",
    "GeForce4 MX 440",
    "GeForce4 MX 420",
    "GeForce4",
    "GeForce4 Go 440",
    "GeForce4 Go 420",
    "GeForce4 Go 420 32M",
    "GeForce4 Go 440 64M",
    "GeForce4 Go",
    "GeForce3 Ti 500",
    "GeForce3 Ti 200",
    "GeForce2 Integrated",
    "GeForce2 Ti",
    "Quadro2 MXR//EX//GO",
    "GeFORCE2_MX 100//200",
    "GeFORCE2_MX 400",
    "Quadro DCC" };

const char *g_AMDDevices[] = { "Unknown ATI Device",
    "Rage II",
    "Rage II+",
    "Rage IIc PCI",
    "Rage IIc AGP",
    "Rage 128 Mobility",
    "Rage 128 Mobility M3",
    "Rage 128 Mobility M4",
    "Rage 128 PRO ULTRA",
    "Rage 128 4X",
    "Rage 128 PRO GL",
    "Rage 128 PRO VR",
    "Rage 128 GL",
    "Rage 128 VR",
    "Rage PRO",
    "Rage PRO Mobility",
    "Mobility Radeon",
    "Mobility Radeon VE(M6)",
    "Radeon VE",
    "Radeon DDR",
    "Radeon",
    "Mobility R7500",
    "R7500" };

const char *g_intelDevices[] = { "Unknown Intel Device", "i810", "i810e", "i815" };
const char *g_s3Devices[] = { "Unknown S3 Device", "Savage MX", "Savage 4", "Savage 200" };
const char *g_powerVRDevices[] = { "Unknown PowerVR Device", "Kyro" };

const char *g_matroxDevices[] = { "Unknown Matrox Device",
    "G550",
    "G400",
    "G200 AGP",
    "G200 PCI",
    "G100 PCI",
    "G100 AGP",
    "Millennium II AGP",
    "Millennium II PCI",
    "Mystique",
    "Millennium",
    "Parhelia",
    "Parhelia AGP 8X" };

const char *g_3DfxDevices[] = {
    "Unknown 3Dfx Device", "Voodoo 5500 AGP", "Voodoo 3", "Banshee", "Voodoo 2", "Voodoo Graphics", "Voodoo Rush"
};

const char *g_3DLabsDevices[16] = { "Unknown 3DLabs Device",
    "Permedia",
    "300SX",
    "500TX",
    "Delta",
    "MX",
    "Gamma",
    "Permedia2S (ST)",
    "Permedia3",
    "R3",
    "Permedia4",
    "R4",
    "G2",
    "Oxygen VX1",
    "TI P1",
    "Permedia2" };
} // namespace

/**
 * 0x00844A90
 */
DX8Caps::DX8Caps(w3dhandle_t handle, const w3dcaps_t &caps, WW3DFormat format, const w3dadapterid_t &identifier) :
    m_widthLimit(0), m_heightLimit(0), m_d3dInterface(handle)
{
#ifdef BUILD_WITH_D3D8
    memcpy(&m_caps, &caps, sizeof(m_caps));
    m_useHardwareTnL = Has_Feature(m_caps.DevCaps, D3DDEVCAPS_HWTRANSFORMANDLIGHT);
#endif
    Compute_Caps(format, identifier);
}

/**
 * 0x00844950
 */
DX8Caps::DX8Caps(w3dhandle_t handle, w3ddevice_t device, WW3DFormat format, const w3dadapterid_t &identifier) :
    m_widthLimit(0), m_heightLimit(0), m_d3dInterface(handle)
{
    Init_Caps(device);
    Compute_Caps(format, identifier);
    captainslog_line("================================================================================\n");
    captainslog_line(static_cast<const char *>(m_videoCardSpecString));
    captainslog_line("\n================================================================================");
}

/**
 * Frees any resources held by this class.
 *
 * 0x00844BA0
 */
void DX8Caps::Shutdown()
{
    s_capsWorkStrings.Release_Resources();
}

/**
 * Initialises caps from a device handle.
 */
void DX8Caps::Init_Caps(w3ddevice_t device)
{
#ifdef BUILD_WITH_D3D8
    device->SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, TRUE);
    DX8CALL(GetDeviceCaps(&m_caps));

    if (Has_Feature(m_caps.DevCaps, D3DDEVCAPS_HWTRANSFORMANDLIGHT)) {
        m_useHardwareTnL = true;
        device->SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, FALSE);
        DX8CALL(GetDeviceCaps(&m_caps));
    } else {
        m_useHardwareTnL = false;
    }
#else
    m_useHardwareTnL = false;
#endif
}

/**
 * Determines the capabilities of the graphics driver and stores them internally.
 *
 * 0x00844BB0
 */
void DX8Caps::Compute_Caps(WW3DFormat format, const w3dadapterid_t &identifier)
{
    m_supportMultiTexture = true;
    m_supportFog = true;

    m_videoCardSpecString = "";
    m_videoCardName = "";
    m_driverFilename = "";
#ifdef BUILD_WITH_D3D8
    s_capsWorkStrings.Format("Video Card: %s\n", identifier.Description);
    m_videoCardSpecString += s_capsWorkStrings;
    s_capsWorkStrings.Format("Driver: %s\n", identifier.Driver);
    m_videoCardSpecString += s_capsWorkStrings;
    m_driverFilename = identifier.Driver;
    m_driverBuildNum = identifier.DriverVersion.LowPart & 0xFFFF;
    s_capsWorkStrings.Format("Product=%d, Version=%d, SubVersion=%d, Build=%d\n",
        identifier.DriverVersion.HighPart >> 16,
        identifier.DriverVersion.HighPart & 0xFFFF,
        identifier.DriverVersion.LowPart >> 16,
        identifier.DriverVersion.LowPart & 0xFFFF);
    m_videoCardSpecString += s_capsWorkStrings;
    m_vendorNumber = Define_Vendor(identifier.VendorId);
#else
    m_driverBuildNum = 0;
    m_vendorNumber = VENDOR_UNKNOWN;
#endif

    if (m_vendorNumber == VENDOR_UNKNOWN && m_driverFilename[0] == '3') {
        m_vendorNumber = VENDOR_3DFX;
    }

    s_capsWorkStrings.Format("%s\t", g_videoCardVendors[m_vendorNumber]);
    m_videoCardName += s_capsWorkStrings;
    s_capsWorkStrings.Format("Video Card Chip Vendor: %s\n", g_videoCardVendors[m_vendorNumber]);
    m_videoCardSpecString += s_capsWorkStrings;
    s_capsWorkStrings.Format("Type of chip: ");
    m_videoCardSpecString += s_capsWorkStrings;

#ifdef BUILD_WITH_D3D8
    switch (m_vendorNumber) {
        case VENDOR_NVIDIA:
            m_deviceNumber = Get_NVidia_Device(identifier.DeviceId);
            s_capsWorkStrings.Format("%s", g_nVidiaDevices[m_deviceNumber]);
            break;
        case VENDOR_AMD:
            m_deviceNumber = Get_AMD_Device(identifier.DeviceId);
            s_capsWorkStrings.Format("%s", g_AMDDevices[m_deviceNumber]);
            break;
        case VENDOR_INTEL:
            m_deviceNumber = Get_Intel_Device(identifier.DeviceId);
            s_capsWorkStrings.Format("%s", g_intelDevices[m_deviceNumber]);
            break;
        case VENDOR_S3:
            m_deviceNumber = Get_S3_Device(identifier.DeviceId);
            s_capsWorkStrings.Format("%s", g_s3Devices[m_deviceNumber]);
            break;
        case VENDOR_POWERVR:
            m_deviceNumber = Get_PowerVR_Device(identifier.DeviceId);
            s_capsWorkStrings.Format("%s", g_powerVRDevices[m_deviceNumber]);
            break;
        case VENDOR_MATROX:
            m_deviceNumber = Get_Matrox_Device(identifier.DeviceId);
            s_capsWorkStrings.Format("%s", g_matroxDevices[m_deviceNumber]);
            break;
        case VENDOR_3DFX:
            m_deviceNumber = Get_3Dfx_Device(identifier.DeviceId);
            s_capsWorkStrings.Format("%s", g_3DfxDevices[m_deviceNumber]);
            break;
        case VENDOR_3DLABS:
            m_deviceNumber = Get_3DLabs_Device(identifier.DeviceId);
            s_capsWorkStrings.Format("%s", g_3DLabsDevices[m_deviceNumber]);
            break;
        default:
            m_deviceNumber = 0;
            s_capsWorkStrings.Format("%s", "Unknown");
            break;
    };

    m_videoCardSpecString += s_capsWorkStrings;
    m_videoCardName += s_capsWorkStrings;

    s_capsWorkStrings.Format("\t%d\t", m_driverBuildNum);
    m_videoCardName += s_capsWorkStrings;
    s_capsWorkStrings.Format("\n");
    m_videoCardSpecString += s_capsWorkStrings;
    s_capsWorkStrings.Format("Vendor id: 0x%x\n", identifier.VendorId);
    m_videoCardSpecString += s_capsWorkStrings;
    s_capsWorkStrings.Format("Device id: 0x%x\n", identifier.DeviceId);
    m_videoCardSpecString += s_capsWorkStrings;
    s_capsWorkStrings.Format("SubSys id: 0x%x\n", identifier.SubSysId);
    m_videoCardSpecString += s_capsWorkStrings;
    s_capsWorkStrings.Format("Revision: %d\n", identifier.Revision);
    m_videoCardSpecString += s_capsWorkStrings;

    s_capsWorkStrings.Format("GUID = {0x%x, 0x%x, 0x%x}, {0x%2.2x, 0x%2.2x, 0x%2.2x, 0x%2.2x, "
                             "0x%2.2x, 0x%2.2x, 0x%2.2x, 0x%2.2x}\n",
        identifier.DeviceIdentifier.Data1,
        identifier.DeviceIdentifier.Data2,
        identifier.DeviceIdentifier.Data3,
        identifier.DeviceIdentifier.Data4[0],
        identifier.DeviceIdentifier.Data4[1],
        identifier.DeviceIdentifier.Data4[2],
        identifier.DeviceIdentifier.Data4[3],
        identifier.DeviceIdentifier.Data4[4],
        identifier.DeviceIdentifier.Data4[5],
        identifier.DeviceIdentifier.Data4[6],
        identifier.DeviceIdentifier.Data4[7]);
    m_videoCardSpecString += s_capsWorkStrings;

    m_supportLargePointSize = m_caps.MaxPointSize > 1.0f;
    m_supportNPatches = false;
    m_supportZBias = Has_Feature(m_caps.RasterCaps, D3DPRASTERCAPS_ZBIAS);
    m_supportGamma = Has_Feature(m_caps.Caps2, D3DCAPS2_FULLSCREENGAMMA);
    m_supportsModulateAlphaAddColor = Has_Feature(m_caps.TextureOpCaps, D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR);
    m_supportDot3 = Has_Feature(m_caps.TextureOpCaps, D3DTEXOPCAPS_DOTPRODUCT3);
    m_supportsCubeMap = Has_Feature(m_caps.TextureCaps, D3DPTEXTURECAPS_CUBEMAP);
    m_supportAnisotropicFiltering = Has_Feature(m_caps.TextureFilterCaps, D3DPTFILTERCAPS_MINFANISOTROPIC)
        && Has_Feature(m_caps.TextureFilterCaps, D3DPTFILTERCAPS_MAGFANISOTROPIC);
#else
    m_deviceNumber = 0;
    m_supportLargePointSize = false;
    m_supportZBias = false;
    m_supportNPatches = false;
    m_supportGamma = false;
    m_supportDot3 = false;
    m_supportsModulateAlphaAddColor = false;
    m_supportsCubeMap = false;
    m_supportAnisotropicFiltering = false;
#endif

    s_capsWorkStrings.Format("Hardware T&L support: %s\n", m_useHardwareTnL ? "Yes" : "No");
    m_videoCardSpecString += s_capsWorkStrings;
    s_capsWorkStrings.Format("NPatch support: %s\n", m_supportNPatches ? "Yes" : "No");
    m_videoCardSpecString += s_capsWorkStrings;
    s_capsWorkStrings.Format("ZBias support: %s\n", m_supportZBias ? "Yes" : "No");
    m_videoCardSpecString += s_capsWorkStrings;
    s_capsWorkStrings.Format("Gamma support: %s\n", m_supportGamma ? "Yes" : "No");
    m_videoCardSpecString += s_capsWorkStrings;
    s_capsWorkStrings.Format("ModAlphaAddClr support: %s\n", m_supportsModulateAlphaAddColor ? "Yes" : "No");
    m_videoCardSpecString += s_capsWorkStrings;
    s_capsWorkStrings.Format("Dot3 support: %s\n", m_supportDot3 ? "Yes" : "No");
    m_videoCardSpecString += s_capsWorkStrings;
    s_capsWorkStrings.Format("Anisotropic filtering support: %s\n", m_supportAnisotropicFiltering ? "Yes" : "No");
    m_videoCardSpecString += s_capsWorkStrings;

    Check_Texture_Format_Support(format, m_caps);
    Check_Render_To_Texture_Support(format, m_caps);
    Check_Depth_Stencil_Support(format, m_caps);
    Check_Texture_Compression_Support(m_caps);
    Check_Bumpmap_Support(m_caps);
    Check_Shader_Support(m_caps);
    Check_Driver_Version_Status();
    Check_Max_Texture_Support(m_caps);

#ifdef BUILD_WITH_D3D8
    m_maxTexturesPerPass = m_caps.MaxSimultaneousTextures;
#else
    m_maxTexturesPerPass = 0;
#endif

    s_capsWorkStrings.Format("Max textures per pass: %d\n", m_maxTexturesPerPass);
    m_videoCardSpecString += s_capsWorkStrings;

    Vendor_Specific_Hacks(identifier);

    s_capsWorkStrings = "";
}

/**
 * Checks if a given width and height are within the allowable display dimensions.
 *
 * 0x00846FC0
 */
bool DX8Caps::Is_Valid_Display_Format(int w, int h, WW3DFormat format)
{
    return (m_widthLimit == 0 && m_heightLimit == 0) || (m_widthLimit >= w && m_heightLimit >= h);
}

/**
 * Checks which texture formats the driver reports as supported.
 *
 * 0x008462D0
 */
void DX8Caps::Check_Texture_Format_Support(WW3DFormat format, const w3dcaps_t &caps)
{
    if (format == WW3D_FORMAT_UNKNOWN) {
        memset(m_supportTextureFormat, 0, sizeof(m_supportTextureFormat));

        return;
    }

    m_supportTextureFormat[WW3D_FORMAT_UNKNOWN] = false;

    for (WW3DFormat i = WW3D_FORMAT_UNKNOWN + 1; i < WW3D_FORMAT_COUNT; ++i) {
#ifdef BUILD_WITH_D3D8
        m_supportTextureFormat[i] = SUCCEEDED(m_d3dInterface->CheckDeviceFormat(caps.AdapterOrdinal,
            caps.DeviceType,
            (D3DFORMAT)WW3DFormat_To_D3DFormat(format),
            0,
            D3DRTYPE_TEXTURE,
            (D3DFORMAT)WW3DFormat_To_D3DFormat(i)));
#else
        m_supportTextureFormat[i] = false;
#endif
        if (m_supportTextureFormat[i]) {
            StringClass tmp;
            Get_WW3D_Format_Name(i, tmp);
            s_capsWorkStrings.Format("Supports texture format: %s\n", static_cast<const char *>(tmp));
            m_videoCardSpecString += s_capsWorkStrings;
        }
    }
}

/**
 * Checks which texture formats the driver reports as supported for render to texture.
 *
 * 0x008464B0
 */
void DX8Caps::Check_Render_To_Texture_Support(WW3DFormat format, const w3dcaps_t &caps)
{
    if (format == WW3D_FORMAT_UNKNOWN) {
        memset(m_supportRenderToTextureFormat, 0, sizeof(m_supportRenderToTextureFormat));

        return;
    }

    m_supportRenderToTextureFormat[WW3D_FORMAT_UNKNOWN] = false;

    for (WW3DFormat i = WW3D_FORMAT_UNKNOWN + 1; i < WW3D_FORMAT_COUNT; ++i) {
#ifdef BUILD_WITH_D3D8
        m_supportRenderToTextureFormat[i] = SUCCEEDED(m_d3dInterface->CheckDeviceFormat(caps.AdapterOrdinal,
            caps.DeviceType,
            (D3DFORMAT)WW3DFormat_To_D3DFormat(format),
            D3DUSAGE_RENDERTARGET,
            D3DRTYPE_TEXTURE,
            (D3DFORMAT)WW3DFormat_To_D3DFormat(i)));
#else
        m_supportRenderToTextureFormat[i] = false;
#endif
        if (m_supportRenderToTextureFormat[i]) {
            StringClass tmp;
            Get_WW3D_Format_Name(i, tmp);
            s_capsWorkStrings.Format("Supports render-to-texture format: %s\n", static_cast<const char *>(tmp));
            m_videoCardSpecString += s_capsWorkStrings;
        }
    }
}

/**
 * Checks which depth stencil formats the driver reports are supported.
 *
 * 0x00846690
 */
void DX8Caps::Check_Depth_Stencil_Support(WW3DFormat format, const w3dcaps_t &caps)
{
    if (format == WW3D_FORMAT_UNKNOWN) {
        memset(m_supportDepthStencilFormat, 0, sizeof(m_supportDepthStencilFormat));

        return;
    }

    m_supportDepthStencilFormat[WW3DZ_FORMAT_UNKNOWN] = false;

    for (WW3DZFormat i = WW3DZ_FORMAT_UNKNOWN + 1; i < WW3DZ_FORMAT_COUNT; ++i) {
#ifdef BUILD_WITH_D3D8
        m_supportDepthStencilFormat[i] = SUCCEEDED(m_d3dInterface->CheckDeviceFormat(caps.AdapterOrdinal,
            caps.DeviceType,
            (D3DFORMAT)WW3DFormat_To_D3DFormat(format),
            D3DUSAGE_DEPTHSTENCIL,
            D3DRTYPE_TEXTURE,
            (D3DFORMAT)WW3DZFormat_To_D3DFormat(i)));
#else
        m_supportDepthStencilFormat[i] = false;
#endif
        if (m_supportDepthStencilFormat[i]) {
            StringClass tmp;
            Get_WW3DZ_Format_Name(i, tmp);
            s_capsWorkStrings.Format("Supports depth stencil format: %s\n", static_cast<const char *>(tmp));
            m_videoCardSpecString += s_capsWorkStrings;
        }
    }
}

/**
 * Checks which texture compressions formats the driver reports are supported.
 *
 * 0x008461E0
 */
void DX8Caps::Check_Texture_Compression_Support(const w3dcaps_t &caps)
{
    m_supportDXTC = m_supportTextureFormat[WW3D_FORMAT_DXT1] || m_supportTextureFormat[WW3D_FORMAT_DXT2]
        || m_supportTextureFormat[WW3D_FORMAT_DXT3] || m_supportTextureFormat[WW3D_FORMAT_DXT4]
        || m_supportTextureFormat[WW3D_FORMAT_DXT5];

    s_capsWorkStrings.Format("Texture compression support: %s\n", m_supportDXTC ? "Yes" : "No");
    m_videoCardSpecString += s_capsWorkStrings;
}

/**
 * Checks whether the driver reports bumpmaps are supported.
 *
 * 0x00846040
 */
void DX8Caps::Check_Bumpmap_Support(const w3dcaps_t &caps)
{
#ifdef BUILD_WITH_D3D8
    m_supportBumpEnvmap = Has_Feature(caps.TextureOpCaps, D3DTEXOPCAPS_BUMPENVMAP);
    m_supportBumpEnvmapLuminance = Has_Feature(caps.TextureOpCaps, D3DTEXOPCAPS_BUMPENVMAPLUMINANCE);
#else
    m_supportBumpEnvmap = false;
    m_supportBumpEnvmapLuminance = false;
#endif
    s_capsWorkStrings.Format("Bumpmap support: %s\n", m_supportBumpEnvmap ? "Yes" : "No");
    m_videoCardSpecString += s_capsWorkStrings;
    s_capsWorkStrings.Format("Bumpmap luminance support: %s\n", m_supportBumpEnvmapLuminance ? "Yes" : "No");
    m_videoCardSpecString += s_capsWorkStrings;
}

/**
 * Checks what shader versions the driver reports as supported.
 *
 * 0x00846870
 */
void DX8Caps::Check_Shader_Support(const w3dcaps_t &caps)
{
#ifdef BUILD_WITH_D3D8
    m_vertexShaderVersion = caps.VertexShaderVersion;
    m_pixelShaderVersion = caps.PixelShaderVersion;
#else
    m_vertexShaderVersion = 0;
    m_pixelShaderVersion = 0;
#endif
    s_capsWorkStrings.Format("Vertex shader version: %d.%d, Pixel shader version: %d.%d\n",
        (m_vertexShaderVersion >> 8) & 0xFF,
        m_vertexShaderVersion & 0xFF,
        (m_pixelShaderVersion >> 8) & 0xFF,
        m_pixelShaderVersion & 0xFF);
    m_videoCardSpecString += s_capsWorkStrings;
}

/**
 * Checks how many textures at once the driver reports as supported.
 */
void DX8Caps::Check_Max_Texture_Support(const w3dcaps_t &caps)
{
#ifdef BUILD_WITH_D3D8
    m_maxSupportedTextures = caps.MaxSimultaneousTextures;
#else
    m_maxSupportedTextures = 0;
#endif
}

/**
 * Checks if the driver versions is known good or bad.
 *
 * 0x00846960
 */
void DX8Caps::Check_Driver_Version_Status()
{
    // Any modern GPU/driver qualifies as "good" for the purposes of this function.
    m_driverStatus = DRIVER_GOOD;
    s_capsWorkStrings = "Driver version status: Good\n";
    m_videoCardSpecString += s_capsWorkStrings;
}

/**
 * Performs any hacks needed where driver reports support where none exists or is poor.
 *
 * 0x00846FF0
 */
void DX8Caps::Vendor_Specific_Hacks(const w3dadapterid_t &identifier)
{
    // none of the things that this function does matter for modern GPUs and drivers.
}

/**
 * Determines the vendor from the PCI vendor ID.
 */
int DX8Caps::Define_Vendor(unsigned vendor_id)
{
    switch (vendor_id) {
        case 0x109D: // Zida
        case 0x1142: // Alliance ProMotion
        case 0x121A: // 3Dfx
            return VENDOR_3DFX;
        case 0x104A: // PowerVR
            return VENDOR_POWERVR;
        case 0x1002: // ATI/AMD
            return VENDOR_AMD;
        case 0x102B: // Matrox
            return VENDOR_MATROX;
        case 0x10DE: // nVidia
        case 0x12D2: // nVidia/ST Joint
        case 0x14AF: // Guillemot Corporation
            return VENDOR_NVIDIA;
        case 0x104C: // Texas Instruments FireGL
        case 0x3D3D: // 3D Labs
            return VENDOR_3DLABS;
        case 0x5333: // S3
            return VENDOR_S3;
        case 0x8086: // Intel
            return VENDOR_INTEL;
        default:
            break;
    }

    return VENDOR_UNKNOWN;
}

/**
 * Determins device number from PCI device ID.
 *
 * 0x00844550
 */
int DX8Caps::Get_NVidia_Device(unsigned device_id)
{
    switch (device_id) {
        case 0x171:
            return 24;
        case 0x202:
            return 32;
        case 0x176:
            return 29;
        case 0x173:
            return 26;
        case 0x172:
            return 25;
        case 0x174:
            return 27;
        case 0x175:
            return 28;
        case 0x1A0:
            return 34;
        case 0x200:
            return 1;
        case 0x201:
            return 33;
        case 0x178:
            return 31;
        case 0x179:
            return 30;
        case 0x2D:
        case 0x4D20:
        case 0x5620:
            return 13;
        case 0x251:
            return 20;
        case 0x252:
            return 21;
        case 0x253:
            return 22;
        case 0x203:
            return 39;
        case 0x250:
            return 19;
        case 0x100:
        case 0x5020:
            return 8;
        case 0x5008:
            return 16;
        case 0x29:
        case 0x5820:
            return 11;
        case 0x5810:
            return 10;
        case 0x101:
            return 7;
        case 0x113:
            return 36;
        case 0x110:
            return 38;
        case 0x111:
            return 37;
        case 0x112:
            return 3;
        case 0x103:
            return 6;
        case 0x152:
            return 4;
        case 0x153:
            return 2;
        case 0x170:
            return 23;
        case 0x150:
            return 5;
        case 0x151:
            return 35;
        case 0x28:
            return 10;
        case 0x20:
            return 14;
        case 0x18:
            return 15;
        case 0x8:
            return 17;
        case 0x2C:
            return 12;
        case 0xA0:
            return 9;
        default:
            break;
    }

    return 0;
}

/**
 * Determins device number from PCI device ID.
 *
 * 0x00844220
 */
int DX8Caps::Get_AMD_Device(unsigned device_id)
{
    switch (device_id) {
        case 0x514C:
        case 0x514E:
        case 0x514F:
            return 23;
        case 0x5047:
        case 0x5048:
        case 0x5049:
        case 0x5050:
        case 0x5051:
        case 0x5052:
        case 0x5053:
        case 0x5054:
        case 0x5055:
        case 0x5056:
        case 0x5057:
        case 0x5058:
            return 11;
        case 0x5144:
        case 0x5145:
        case 0x5146:
        case 0x5147:
            return 19;
        case 0x5157:
            return 22;
        case 0x5158:
        case 0x5159:
            return 18;
        case 0x534E:
            return 9;
        case 0x5446:
        case 0x544C:
        case 0x5452:
        case 0x5453:
        case 0x5454:
        case 0x5455:
            return 8;
        case 0x5656:
            return 3;
        case 0x524B:
        case 0x524C:
        case 0x5345:
        case 0x5346:
        case 0x5347:
            return 13;
        case 0x534C:
            return 12;
        case 0x5041:
        case 0x5042:
        case 0x5043:
        case 0x5044:
        case 0x5045:
        case 0x5046:
            return 10;
        case 0x4759:
        case 0x475A:
        case 0x4757:
            return 4;
        case 0x4742:
        case 0x4744:
        case 0x4749:
        case 0x474C:
        case 0x474D:
        case 0x474E:
        case 0x474F:
        case 0x4750:
        case 0x4752:
            return 14;
        case 0x4755:
            return 2;
        case 0x4754:
            return 1;
        case 0x4756:
            return 3;
        case 0x4C50:
        case 0x4C52:
        case 0x4C42:
        case 0x4C49:
        case 0x4C4D:
            return 15;
        case 0x4D46:
        case 0x4D4C:
            return 7;
        case 0x4C59:
        case 0x4C5A:
            return 17;
        case 0x4C57:
            return 21;
        case 0x4C45:
        case 0x4C46:
            return 6;
        default:
            break;
    }

    return 0;
}

/**
 * Determins device number from PCI device ID.
 */
int DX8Caps::Get_Intel_Device(unsigned device_id)
{
    switch (device_id) {
        case 0x7123:
            return 1;
        case 0x1132:
            return 3;
        case 0x7121:
            return 2;
        default:
            break;
    }

    return 0;
}

/**
 * Determins device number from PCI device ID.
 */
int DX8Caps::Get_S3_Device(unsigned device_id)
{
    switch (device_id) {
        case 0x8C10:
            return 1;
        case 0x9102:
            return 3;
        case 0x8A22:
            return 2;
        default:
            break;
    }

    return 0;
}

/**
 * Determins device number from PCI device ID.
 */
int DX8Caps::Get_PowerVR_Device(unsigned device_id)
{
    switch (device_id) {
        case 0x10:
            return 1;
        default:
            break;
    }

    return 0;
}

/**
 * Determins device number from PCI device ID.
 */
int DX8Caps::Get_Matrox_Device(unsigned device_id)
{
    switch (device_id) {
        case 0x521:
            return 3;
        case 0x51B:
            return 8;
        case 0x519:
            return 10;
        case 0x51A:
            return 9;
        case 0x51F:
            return 7;
        case 0x520:
            return 4;
        case 0x528:
            return 12;
        case 0x525:
            return 2;
        case 0x527:
            return 11;
        case 0x1001:
            return 6;
        case 0x2527:
            return 1;
        case 0x1000:
            return 5;
        default:
            break;
    }

    return 0;
}

/**
 * Determins device number from PCI device ID.
 */
int DX8Caps::Get_3Dfx_Device(unsigned device_id)
{
    switch (device_id) {
        case 0x1:
            return 5;
        case 0x2:
            return 4;
        case 0x3:
            return 3;
        case 0x5:
            return 2;
        case 0x9:
            return 1;
        case 0x643D:
        case 0x8626:
            return 6;
        default:
            break;
    }

    return 0;
}

/**
 * Determins device number from PCI device ID.
 */
int DX8Caps::Get_3DLabs_Device(unsigned device_id)
{
    switch (device_id) {
        case 0x1:
            return 2;
        case 0x2:
            return 3;
        case 0x3:
            return 4;
        case 0x4:
            return 1;
        case 0x6:
            return 5;
        case 0x7:
            return 15;
        case 0x8:
            return 6;
        case 0x9:
            return 7;
        case 0xA:
            return 8;
        case 0xB:
            return 9;
        case 0xC:
            return 10;
        case 0xD:
            return 11;
        case 0xE:
            return 12;
        case 0x3D04:
            return 14;
        case 0x3D07:
            return 15;
        case 0x4C59:
            return 13;
        default:
            break;
    }

    return 0;
}
