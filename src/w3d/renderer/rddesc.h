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
#include "w3dtypes.h"
#include "wwstring.h"

class ResolutionDescClass
{
public:
    ResolutionDescClass()
    {
        // #BUGFIX Initialize all members
        m_width = 0;
        m_height = 0;
        m_bitDepth = 0;
        m_refreshRate = 0;
    }
    ResolutionDescClass(int w, int h, int d) : m_width(w), m_height(h), m_bitDepth(d)
    {
        // #BUGFIX Initialize all members
        m_refreshRate = 0;
    }
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

    void Add_Resolution(int w, int h, int b)
    {
        bool found = false;

        for (int i = 0; i < m_resArray.Count(); ++i) {
            if (m_resArray[i].Get_Width() == w && m_resArray[i].Get_Height() == h && m_resArray[i].Get_Depth() == b) {
                found = true;
            }
        }

        if (!found) {
            m_resArray.Add(ResolutionDescClass(w, h, b));
        }
    }

    const DynamicVectorClass<ResolutionDescClass> &Get_Resolution_Array() const { return m_resArray; }
    const char *Get_Device_Name() const { return m_deviceName; }
    const char *Get_Device_Vendor() const { return m_deviceVendor; }
    const char *Get_Device_Platform() const { return m_devicePlatform; }
    const char *Get_Driver_Name() const { return m_driverName; }
    const char *Get_Driver_Vendor() const { return m_driverVendor; }
    const char *Get_Driver_Version() const { return m_driverVersion; }
    const char *Get_Hardware_Name() const { return m_hardwareName; }
    const char *Get_Hardware_Vendor() const { return m_hardwareVendor; }
    const char *Get_Hardware_Chipset() const { return m_hardwareChipset; }

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
    w3dcaps_t m_caps;
    w3dadapterid_t m_adapterID;
    DynamicVectorClass<ResolutionDescClass> m_resArray;
};