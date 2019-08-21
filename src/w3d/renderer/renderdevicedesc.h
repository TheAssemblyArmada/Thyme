/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Holds information about the graphics drive and hardware.
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
#include "vector.h"
#include "wwstring.h"

#ifdef BUILD_WITH_D3D8
#include <d3d8.h>
#endif

class ResolutionDescClass
{
public:
    ResolutionDescClass() {}
    ResolutionDescClass(int w, int h, int d) : m_width(w), m_height(h), m_bitDepth(d) {}
    int Get_Width() const { return m_width; }
    int Get_Height() const { return m_height; }
    int Get_Depth() const { return m_bitDepth; }

    bool operator==(const ResolutionDescClass &that) { return this == &that; }
    bool operator!=(const ResolutionDescClass &that) { return this != &that; }

private:
    int m_width;
    int m_height;
    int m_bitDepth;
    int m_refreshRate;
};

class RenderDeviceDescClass
{
    friend class DX8Wrapper;

public:
    RenderDeviceDescClass() {}
    bool operator==(const RenderDeviceDescClass &that) { return this == &that; }
    bool operator!=(const RenderDeviceDescClass &that) { return this != &that; }

private:
    StringClass m_deviceName;
    StringClass m_deviceVendor;
    StringClass m_devicePlatform;
    StringClass m_driverName;
    StringClass m_driverVendor;
    StringClass m_driverVersion;
    StringClass m_hardwareName;
    StringClass m_hardwareVendor;
    StringClass m_hardwareChipset;
#ifdef BUILD_WITH_D3D8
    D3DCAPS8 m_caps;
    D3DADAPTER_IDENTIFIER8 m_adapterID;
#endif
    DynamicVectorClass<ResolutionDescClass> m_resArray;
};