/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Determine platform and operating system information.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "cpudetect.h"
#include "intrinsics.h"
#include "systimer.h"
#include <cstdio>
#include <cstring>

using std::strcat;
using std::strncpy;

#ifdef PLATFORM_WINDOWS
#include <libloaderapi.h>
#include <mmsystem.h>
#include <sysinfoapi.h>
#include <timezoneapi.h>
#endif

#ifdef PLATFORM_APPLE
#include <sys/sysctl.h>
#endif

#ifdef PLATFORM_LINUX
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

namespace
{
struct OSInfoStruct
{
    const char *Code;
    const char *SubCode;
    const char *VersionString;
    uint8_t VersionMajor;
    uint8_t VersionMinor;
    uint16_t VersionSub;
    uint8_t BuildMajor;
    uint8_t BuildMinor;
    uint16_t BuildSub;
};

#ifdef PLATFORM_WINDOWS
OSInfoStruct WindowsVersionTable[] = {
    { "WIN95", "FINAL", "Windows 95", 4, 0, 950, 4, 0, 950 },
    { "WIN95", "A", "Windows 95a OSR1 final Update", 4, 0, 950, 4, 0, 951 },
    { "WIN95", "B20OEM", "Windows 95B OSR 2.0 final OEM", 4, 0, 950, 4, 0, 1111 },
    { "WIN95", "B20UPD", "Windows 95B OSR 2.1 final Update", 4, 0, 950, 4, 3, 1212 },
    { "WIN95", "B21OEM", "Windows 95B OSR 2.1 final OEM", 4, 1, 971, 4, 1, 971 },
    { "WIN95", "C25OEM", "Windows 95C OSR 2.5 final OEM", 4, 0, 950, 4, 3, 1214 },
    { "WIN98", "BETAPRD", "Windows 98 Beta pre-DR", 4, 10, 1351, 4, 10, 1351 },
    { "WIN98", "BETADR", "Windows 98 Beta DR", 4, 10, 1358, 4, 10, 1358 },
    { "WIN98", "BETAE", "Windows 98 early Beta", 4, 10, 1378, 4, 10, 1378 },
    { "WIN98", "BETAE", "Windows 98 early Beta", 4, 10, 1410, 4, 10, 1410 },
    { "WIN98", "BETAE", "Windows 98 early Beta", 4, 10, 1423, 4, 10, 1423 },
    { "WIN98", "BETA1", "Windows 98 Beta 1", 4, 10, 1500, 4, 10, 1500 },
    { "WIN98", "BETA1", "Windows 98 Beta 1", 4, 10, 1508, 4, 10, 1508 },
    { "WIN98", "BETA1", "Windows 98 Beta 1", 4, 10, 1511, 4, 10, 1511 },
    { "WIN98", "BETA1", "Windows 98 Beta 1", 4, 10, 1525, 4, 10, 1525 },
    { "WIN98", "BETA1", "Windows 98 Beta 1", 4, 10, 1535, 4, 10, 1535 },
    { "WIN98", "BETA1", "Windows 98 Beta 1", 4, 10, 1538, 4, 10, 1538 },
    { "WIN98", "BETA1", "Windows 98 Beta 1", 4, 10, 1543, 4, 10, 1543 },
    { "WIN98", "BETA2", "Windows 98 Beta 2", 4, 10, 1544, 4, 10, 1544 },
    { "WIN98", "BETA2", "Windows 98 Beta 2", 4, 10, 1546, 4, 10, 1546 },
    { "WIN98", "BETA2", "Windows 98 Beta 2", 4, 10, 1550, 4, 10, 1550 },
    { "WIN98", "BETA2", "Windows 98 Beta 2", 4, 10, 1559, 4, 10, 1559 },
    { "WIN98", "BETA2", "Windows 98 Beta 2", 4, 10, 1564, 4, 10, 1564 },
    { "WIN98", "BETA2", "Windows 98 Beta 2", 4, 10, 1569, 4, 10, 1569 },
    { "WIN98", "BETA2", "Windows 98 Beta 2", 4, 10, 1577, 4, 10, 1577 },
    { "WIN98", "BETA2", "Windows 98 Beta 2", 4, 10, 1581, 4, 10, 1581 },
    { "WIN98", "BETA2", "Windows 98 Beta 2", 4, 10, 1593, 4, 10, 1593 },
    { "WIN98", "BETA2", "Windows 98 Beta 2", 4, 10, 1599, 4, 10, 1599 },
    { "WIN98", "BETA3", "Windows 98 Beta 3", 4, 10, 1602, 4, 10, 1602 },
    { "WIN98", "BETA3", "Windows 98 Beta 3", 4, 10, 1605, 4, 10, 1605 },
    { "WIN98", "BETA3", "Windows 98 Beta 3", 4, 10, 1614, 4, 10, 1614 },
    { "WIN98", "BETA3", "Windows 98 Beta 3", 4, 10, 1619, 4, 10, 1619 },
    { "WIN98", "BETA3", "Windows 98 Beta 3", 4, 10, 1624, 4, 10, 1624 },
    { "WIN98", "BETA3", "Windows 98 Beta 3", 4, 10, 1629, 4, 10, 1629 },
    { "WIN98", "BETA3", "Windows 98 Beta 3", 4, 10, 1633, 4, 10, 1633 },
    { "WIN98", "BETA3", "Windows 98 Beta 3", 4, 10, 1650, 4, 10, 1650 },
    { "WIN98", "BETA3", "Windows 98 Beta 3", 4, 10, 1650, 4, 10, 1650 },
    { "WIN98", "BETA3", "Windows 98 Beta 3", 4, 10, 1650, 4, 10, 1650 },
    { "WIN98", "BETA3", "Windows 98 Beta 3", 4, 10, 1666, 4, 10, 1666 },
    { "WIN98", "BETA3", "Windows 98 Beta 3", 4, 10, 1671, 4, 10, 1671 },
    { "WIN98", "BETA3", "Windows 98 Beta 3", 4, 10, 1677, 4, 10, 1677 },
    { "WIN98", "BETA3", "Windows 98 Beta 3", 4, 10, 1681, 4, 10, 1681 },
    { "WIN98", "BETA3", "Windows 98 Beta 3", 4, 10, 1687, 4, 10, 1687 },
    { "WIN98", "RC0", "Windows 98 RC0", 4, 10, 1691, 4, 10, 1691 },
    { "WIN98", "RC0", "Windows 98 RC0", 4, 10, 1702, 4, 10, 1702 },
    { "WIN98", "RC0", "Windows 98 RC0", 4, 10, 1708, 4, 10, 1708 },
    { "WIN98", "RC0", "Windows 98 RC0", 4, 10, 1713, 4, 10, 1713 },
    { "WIN98", "RC1", "Windows 98 RC1", 4, 10, 1721, 4, 10, 1721 },
    { "WIN98", "RC2", "Windows 98 RC2", 4, 10, 1723, 4, 10, 1723 },
    { "WIN98", "RC2", "Windows 98 RC2", 4, 10, 1726, 4, 10, 1726 },
    { "WIN98", "RC3", "Windows 98 RC3", 4, 10, 1900, 4, 10, 1900 },
    { "WIN98", "RC4", "Windows 98 RC4", 4, 10, 1900, 4, 10, 1900 },
    { "WIN98", "RC5", "Windows 98 RC5", 4, 10, 1998, 4, 10, 1998 },
    { "WIN98", "FINAL", "Windows 98", 4, 10, 1998, 4, 10, 1998 },
    { "WIN98", "SP1B1", "Windows 98 SP1 Beta 1", 4, 10, 2088, 4, 10, 2088 },
    { "WIN98", "OSR1B1", "Windows 98 OSR1 Beta 1", 4, 10, 2106, 4, 10, 2106 },
    { "WIN98", "OSR1B1", "Windows 98 OSR1 Beta 1", 4, 10, 2120, 4, 10, 2120 },
    { "WIN98", "OSR1B1", "Windows 98 OSR1 Beta 1", 4, 10, 2126, 4, 10, 2126 },
    { "WIN98", "OSR1B1", "Windows 98 OSR1 Beta 1", 4, 10, 2131, 4, 10, 2131 },
    { "WIN98", "SP1B2", "Windows 98 SP1 Beta 2", 4, 10, 2150, 4, 10, 2150 },
    { "WIN98", "SP1B2", "Windows 98 SP1 Beta 2", 4, 10, 2150, 4, 10, 2150 },
    { "WIN98", "SP1", "Windows 98 SP1 final Update", 4, 10, 2000, 4, 10, 2000 },
    { "WIN98", "OSR1B2", "Windows 98 OSR1 Beta 2", 4, 10, 2174, 4, 10, 2174 },
    { "WIN98", "SERC1", "Windows 98 SE RC1", 4, 10, 2183, 4, 10, 2183 },
    { "WIN98", "SERC2", "Windows 98 SE RC2", 4, 10, 2185, 4, 10, 2185 },
    { "WIN98", "SE", "Windows 98 SE", 4, 10, 2222, 4, 10, 2222 },
    { "WINME", "MEBDR1", "Windows ME Beta DR1", 4, 90, 2332, 4, 90, 2332 },
    { "WINME", "MEBDR2", "Windows ME Beta DR2", 4, 90, 2348, 4, 90, 2348 },
    { "WINME", "MEBDR3", "Windows ME Beta DR3", 4, 90, 2358, 4, 90, 2358 },
    { "WINME", "MEBDR4", "Windows ME Beta DR4", 4, 90, 2363, 4, 90, 2363 },
    { "WINME", "MEEB", "Windows ME early Beta", 4, 90, 2368, 4, 90, 2368 },
    { "WINME", "MEEB", "Windows ME early Beta", 4, 90, 2374, 4, 90, 2374 },
    { "WINME", "MEB1", "Windows ME Beta 1", 4, 90, 2380, 4, 90, 2380 },
    { "WINME", "MEB1", "Windows ME Beta 1", 4, 90, 2394, 4, 90, 2394 },
    { "WINME", "MEB1", "Windows ME Beta 1", 4, 90, 2399, 4, 90, 2399 },
    { "WINME", "MEB1", "Windows ME Beta 1", 4, 90, 2404, 4, 90, 2404 },
    { "WINME", "MEB1", "Windows ME Beta 1", 4, 90, 2410, 4, 90, 2410 },
    { "WINME", "MEB1", "Windows ME Beta 1", 4, 90, 2416, 4, 90, 2416 },
    { "WINME", "MEB1", "Windows ME Beta 1", 4, 90, 2419, 4, 90, 2419 },
    { "WINME", "MEB2", "Windows ME Beta 2", 4, 90, 2429, 4, 90, 2429 },
    { "WINME", "MEB2", "Windows ME Beta 2", 4, 90, 2434, 4, 90, 2434 },
    { "WINME", "MEB2", "Windows ME Beta 2", 4, 90, 2443, 4, 90, 2443 },
    { "WINME", "MEB2", "Windows ME Beta 2", 4, 90, 2447, 4, 90, 2447 },
    { "WINME", "MEB2", "Windows ME Beta 2", 4, 90, 2455, 4, 90, 2455 },
    { "WINME", "MEB2", "Windows ME Beta 2", 4, 90, 2460, 4, 90, 2460 },
    { "WINME", "MEB2", "Windows ME Beta 2", 4, 90, 2465, 4, 90, 2465 },
    { "WINME", "MEB2", "Windows ME Beta 2", 4, 90, 2470, 4, 90, 2470 },
    { "WINME", "MEB2", "Windows ME Beta 2", 4, 90, 2474, 4, 90, 2474 },
    { "WINME", "MEB2", "Windows ME Beta 2", 4, 90, 2481, 4, 90, 2481 },
    { "WINME", "MEB2", "Windows ME Beta 2", 4, 90, 2487, 4, 90, 2487 },
    { "WINME", "MEB2", "Windows ME Beta 2", 4, 90, 2491, 4, 90, 2491 },
    { "WINME", "MEB3", "Windows ME Beta 3", 4, 90, 2499, 4, 90, 2499 },
    { "WINME", "MEB3", "Windows ME Beta 3", 4, 90, 2499, 4, 90, 2499 },
    { "WINME", "MEB3", "Windows ME Beta 3", 4, 90, 2509, 4, 90, 2509 },
    { "WINME", "MEB3", "Windows ME Beta 3", 4, 90, 2513, 4, 90, 2513 },
    { "WINME", "MEB3", "Windows ME Beta 3", 4, 90, 2516, 4, 90, 2516 },
    { "WINME", "RC0", "Windows ME RC0", 4, 90, 2525, 4, 90, 2525 },
    { "WINME", "RC1", "Windows ME RC1", 4, 90, 2525, 4, 90, 2525 },
    { "WINME", "RC2", "Windows ME RC2", 4, 90, 2535, 4, 90, 2535 },
    { "WINME", "FINAL", "Windows ME", 4, 90, 3000, 4, 90, 3000 },
    { "", "", "", 4, 90, 3000, 4, 90, 3000 },
};
#endif // PLATFORM_WINDOWS

void Get_OS_Info(OSInfoStruct &os_info,
    uint32_t OSVersionPlatformId,
    uint32_t OSVersionNumberMajor,
    uint32_t OSVersionNumberMinor,
    uint32_t OSVersionBuildNumber)
{
#ifdef PLATFORM_WINDOWS
    uint32_t build_major = (OSVersionBuildNumber & 0xFF000000) >> 24;
    uint32_t build_minor = (OSVersionBuildNumber & 0xFF0000) >> 16;
    uint32_t build_sub = (OSVersionBuildNumber & 0xFFFF);

    switch (OSVersionPlatformId) {
        case VER_PLATFORM_WIN32_WINDOWS: {
            for (int i = 0; i < sizeof(WindowsVersionTable) / sizeof(os_info); ++i) {
                if (WindowsVersionTable[i].VersionMajor == OSVersionNumberMajor
                    && WindowsVersionTable[i].VersionMinor == OSVersionNumberMinor
                    && WindowsVersionTable[i].BuildMajor == build_major && WindowsVersionTable[i].BuildMinor == build_minor
                    && WindowsVersionTable[i].BuildSub == build_sub) {
                    os_info = WindowsVersionTable[i];
                    return;
                }
            }

            os_info.BuildMajor = build_major;
            os_info.BuildMinor = build_minor;
            os_info.BuildSub = build_sub;
            if (OSVersionNumberMajor == 4) {
                //				os_info.SubCode.Format("%d",build_sub);
                os_info.SubCode = "UNKNOWN";
                if (OSVersionNumberMinor == 0) {
                    os_info.Code = "WIN95";
                    return;
                }

                if (OSVersionNumberMinor == 10) {
                    os_info.Code = "WIN98";
                    return;
                }

                if (OSVersionNumberMinor == 90) {
                    os_info.Code = "WINME";
                    return;
                }

                os_info.Code = "WIN9X";

                return;
            }
        } break;
        case VER_PLATFORM_WIN32_NT:
            //		os_info.SubCode.Format("%d",build_sub);
            os_info.SubCode = "UNKNOWN";
            if (OSVersionNumberMajor == 4) {
                os_info.Code = "WINNT";
                return;
            }

            if (OSVersionNumberMajor == 5) {
                if (OSVersionNumberMinor == 0) {
                    os_info.Code = "WIN2K";
                    return;
                }

                if (OSVersionNumberMinor == 1) {
                    os_info.Code = "WINXP";
                    return;
                }

                if (OSVersionNumberMinor == 2) {
                    os_info.Code = "WINXP64";
                    return;
                }
            }

            if (OSVersionNumberMajor == 6) {
                if (OSVersionNumberMinor == 0) {
                    os_info.Code = "WINVISTA";
                    return;
                }

                if (OSVersionNumberMinor == 1) {
                    os_info.Code = "WIN7";
                    return;
                }

                if (OSVersionNumberMinor == 2) {
                    os_info.Code = "WIN8";
                    return;
                }

                if (OSVersionNumberMinor == 3) {
                    os_info.Code = "WIN8.1";
                    return;
                }
            }

            if (OSVersionNumberMajor == 10) {
                os_info.Code = "WIN10";
                return;
            }

            os_info.Code = "WINXX";

            break;

        default:
            os_info = OSInfoStruct{};
            os_info.Code = "UNKNOWN";
            os_info.SubCode = "UNKNOWN";
            os_info.VersionString = "UNKNOWN";
            break;
    }
#else
    os_info = OSInfoStruct{};
    os_info.Code = "UNKNOWN";
    os_info.SubCode = "UNKNOWN";
    os_info.VersionString = "UNKNOWN";
#endif
}

} // namespace

char CPUDetectClass::ProcessorLog[1024];
char CPUDetectClass::CompactLog[1024];

int32_t CPUDetectClass::ProcessorType;
int32_t CPUDetectClass::ProcessorFamily;
int32_t CPUDetectClass::ProcessorModel;
int32_t CPUDetectClass::ProcessorRevision;
int32_t CPUDetectClass::ProcessorSpeed;

int64_t CPUDetectClass::ProcessorTicksPerSecond;

// 1.0 / Ticks per second
double CPUDetectClass::InvProcessorTicksPerSecond;

uint32_t CPUDetectClass::FeatureBits;
uint32_t CPUDetectClass::ExtendedFeatureBits;

uint32_t CPUDetectClass::L3CacheSize;
uint32_t CPUDetectClass::L3CacheLineSize;
uint32_t CPUDetectClass::L3CacheSetAssociative;
uint32_t CPUDetectClass::L2CacheSize;
uint32_t CPUDetectClass::L2CacheLineSize;
uint32_t CPUDetectClass::L2CacheSetAssociative;
uint32_t CPUDetectClass::L1DataCacheSize;
uint32_t CPUDetectClass::L1DataCacheLineSize;
uint32_t CPUDetectClass::L1DataCacheSetAssociative;
uint32_t CPUDetectClass::L1InstructionCacheSize;
uint32_t CPUDetectClass::L1InstructionCacheLineSize;
uint32_t CPUDetectClass::L1InstructionCacheSetAssociative;
uint32_t CPUDetectClass::L1InstructionTraceCacheSize;
uint32_t CPUDetectClass::L1InstructionTraceCacheSetAssociative;

uint64_t CPUDetectClass::TotalPhysicalMemory;
uint64_t CPUDetectClass::AvailablePhysicalMemory;
uint64_t CPUDetectClass::TotalPageMemory;
uint64_t CPUDetectClass::AvailablePageMemory;
uint64_t CPUDetectClass::TotalVirtualMemory;
uint64_t CPUDetectClass::AvailableVirtualMemory;

uint32_t CPUDetectClass::OSVersionNumberMajor;
uint32_t CPUDetectClass::OSVersionNumberMinor;
uint32_t CPUDetectClass::OSVersionBuildNumber;
uint32_t CPUDetectClass::OSVersionPlatformId;
char CPUDetectClass::OSVersionExtraInfo[512];

bool CPUDetectClass::HasCPUIDInstruction = false;
bool CPUDetectClass::HasRDTSCInstruction = false;
bool CPUDetectClass::HasSSESupport = false;
bool CPUDetectClass::HasSSE2Support = false;
bool CPUDetectClass::HasCMOVSupport = false;
bool CPUDetectClass::HasMMXSupport = false;
bool CPUDetectClass::Has3DNowSupport = false;
bool CPUDetectClass::HasExtended3DNowSupport = false;

CPUDetectClass::ProcessorManufacturerType CPUDetectClass::ProcessorManufacturer = CPUDetectClass::MANUFACTURER_UNKNOWN;
CPUDetectClass::IntelProcessorType CPUDetectClass::IntelProcessor;
CPUDetectClass::AMDProcessorType CPUDetectClass::AMDProcessor;
CPUDetectClass::VIAProcessorType CPUDetectClass::VIAProcessor;
CPUDetectClass::RiseProcessorType CPUDetectClass::RiseProcessor;

char CPUDetectClass::VendorID[20];
char CPUDetectClass::ProcessorString[48];

const char *CPUDetectClass::Get_Processor_Manufacturer_Name()
{
#if defined PROCESSOR_X86 || defined PROCESSOR_X86_64
    static const char *_manufacturer_names[] = {
        "<Unknown>", "Intel", "UMC", "AMD", "Cyrix", "NextGen", "VIA", "Rise", "Transmeta"
    };

    return _manufacturer_names[ProcessorManufacturer];
#else
    return nullptr;
#endif
}

static uint32_t Calculate_Processor_Speed(int64_t &ticks_per_second)
{
#ifdef HAVE__RDTSC
    uint64_t timer0;
    uint64_t timer1;

    timer0 = __rdtsc();
    timer1 = timer0; // #BUGFIX Initialize to something

    uint32_t start = g_theSysTimer.Get();
    uint32_t elapsed;

    while ((elapsed = g_theSysTimer.Get() - start) < 200) {
        timer1 = __rdtsc();
    }

    int64_t t = timer1 - timer0;

    ticks_per_second = (1000 / 200) * t; // Ticks per second

    return uint32_t(t / (elapsed * 1000));
#else
    ticks_per_second = 1;
    return 0;
#endif
}

void CPUDetectClass::Init_Processor_Speed()
{
    if (!Has_RDTSC_Instruction()) {
        ProcessorSpeed = 0;

        return;
    }

    uint32_t speed1 = Calculate_Processor_Speed(ProcessorTicksPerSecond);
    uint32_t total_speed = speed1;

    for (int i = 0; i < 5; ++i) {
        uint32_t speed2 = Calculate_Processor_Speed(ProcessorTicksPerSecond);
        float rel = float(speed1) / float(speed2);

        if (rel >= 0.95f && rel <= 1.05f) {
            ProcessorSpeed = (speed1 + speed2) / 2;
            InvProcessorTicksPerSecond = 1.0 / double(ProcessorTicksPerSecond);
            return;
        }

        speed1 = speed2;
        total_speed += speed2;
    }

    ProcessorSpeed = total_speed / 6;
    InvProcessorTicksPerSecond = 1.0 / double(ProcessorTicksPerSecond);
}

void CPUDetectClass::Init_Processor_Manufacturer()
{
#if defined PROCESSOR_X86 || defined PROCESSOR_X86_64
    VendorID[0] = 0;
    uint32_t max_cpuid = 0;

    CPUID(max_cpuid, (uint32_t &)VendorID[0], (uint32_t &)VendorID[8], (uint32_t &)VendorID[4], 0);

    ProcessorManufacturer = MANUFACTURER_UNKNOWN;

    if (strcasecmp(VendorID, "GenuineIntel") == 0)
        ProcessorManufacturer = MANUFACTURER_INTEL;
    else if (strcasecmp(VendorID, "AuthenticAMD") == 0)
        ProcessorManufacturer = MANUFACTURER_AMD;
    else if (strcasecmp(VendorID, "AMD ISBETTER") == 0)
        ProcessorManufacturer = MANUFACTURER_AMD;
    else if (strcasecmp(VendorID, "UMC UMC UMC") == 0)
        ProcessorManufacturer = MANUFACTURER_UMC;
    else if (strcasecmp(VendorID, "CyrixInstead") == 0)
        ProcessorManufacturer = MANUFACTURER_CYRIX;
    else if (strcasecmp(VendorID, "NexGenDriven") == 0)
        ProcessorManufacturer = MANUFACTURER_NEXTGEN;
    else if (strcasecmp(VendorID, "CentaurHauls") == 0)
        ProcessorManufacturer = MANUFACTURER_VIA;
    else if (strcasecmp(VendorID, "RiseRiseRise") == 0)
        ProcessorManufacturer = MANUFACTURER_RISE;
    else if (strcasecmp(VendorID, "GenuineTMx86") == 0)
        ProcessorManufacturer = MANUFACTURER_TRANSMETA;
#endif
}

void CPUDetectClass::Process_Cache_Info(uint32_t value)
{
    switch (value) {
        case 0x00: // Null
            break;

        case 0x01: // Instruction TLB, 4K pages, 4-way set associative, 32 entries
            break;

        case 0x02: // Instruction TLB, 4M pages, fully associative, 2 entries
            break;

        case 0x03: // Data TLB, 4K pages, 4-way set associative, 64 entries
            break;

        case 0x04: // Data TLB, 4M pages, 4-way set associative, 8 entries
            break;

        case 0x06: // Instruction cache, 8K, 4-way set associative, 32 byte line size
            L1InstructionCacheSize = 8 * 1024;
            L1InstructionCacheLineSize = 32;
            L1InstructionCacheSetAssociative = 4;
            break;

        case 0x08: // Instruction cache 16K, 4-way set associative, 32 byte line size
            L1InstructionCacheSize = 16 * 1024;
            L1InstructionCacheLineSize = 32;
            L1InstructionCacheSetAssociative = 4;
            break;

        case 0x0A: // Data cache, 8K, 2-way set associative, 32 byte line size
            L1DataCacheSize = 8 * 1024;
            L1DataCacheLineSize = 32;
            L1DataCacheSetAssociative = 2;
            break;

        case 0x0C: // Data cache, 16K, 4-way set associative, 32 byte line size
            L1DataCacheSize = 16 * 1024;
            L1DataCacheLineSize = 32;
            L1DataCacheSetAssociative = 4;
            break;

        case 0x40: // No L2 cache (P6 family), or No L3 cache (Pentium 4 processor)
            // Nice of Intel, Pentium4 has an exception and this field is defined as "no L3 cache"
            if (ProcessorManufacturer != MANUFACTURER_INTEL || ProcessorFamily != 0xF) {
                L2CacheSize = 0;
                L2CacheLineSize = 0;
                L2CacheSetAssociative = 0;
            }
            break;

        case 0x41: // Unified cache, 32 byte cache line,4-way set associative, 128K
            L2CacheSize = 128 * 1024;
            L2CacheLineSize = 32;
            L2CacheSetAssociative = 4;
            break;

        case 0x42: // Unified cache, 32 byte cache line, 4-way set associative, 256K
            L2CacheSize = 256 * 1024;
            L2CacheLineSize = 32;
            L2CacheSetAssociative = 4;
            break;

        case 0x43: // Unified cache, 32 byte cache line, 4-way set associative, 512K
            L2CacheSize = 512 * 1024;
            L2CacheLineSize = 32;
            L2CacheSetAssociative = 4;
            break;

        case 0x44: // Unified cache, 32 byte cache line, 4-way set associative, 1M
            L2CacheSize = 1024 * 1024;
            L2CacheLineSize = 32;
            L2CacheSetAssociative = 4;
            break;

        case 0x45: // Unified cache, 32 byte cache line, 4-way set associative, 2M
            L2CacheSize = 2048 * 1024;
            L2CacheLineSize = 32;
            L2CacheSetAssociative = 4;
            break;

        case 0x50: // Instruction TLB, 4K, 2M or 4M pages, fully associative, 64 entries
            break;

        case 0x51: // Instruction TLB, 4K, 2M or 4M pages, fully associative, 128 entries
            break;

        case 0x52: // Instruction TLB, 4K, 2M or 4M pages, fully associative, 256 entries
            break;

        case 0x5B: // Data TLB, 4K or 4M pages, fully associative, 64 entries
            break;

        case 0x5C: // Data TLB, 4K or 4M pages, fully associative, 128 entries
            break;

        case 0x5D: // Data TLB, 4K or 4M pages, fully associative, 256 entries
            break;

        case 0x66: // Data cache, sectored, 64 byte cache line, 4 way set associative, 8K
            L1DataCacheSize = 8 * 1024;
            L1DataCacheLineSize = 64;
            L1DataCacheSetAssociative = 4;
            break;

        case 0x67: // Data cache, sectored, 64 byte cache line, 4 way set associative, 16K
            L1DataCacheSize = 16 * 1024;
            L1DataCacheLineSize = 64;
            L1DataCacheSetAssociative = 4;
            break;

        case 0x68: // Data cache, sectored, 64 byte cache line, 4 way set associative, 32K
            L1DataCacheSize = 32 * 1024;
            L1DataCacheLineSize = 64;
            L1DataCacheSetAssociative = 4;
            break;

        case 0x70: // Instruction Trace cache, 8 way set associative, 12K uOps
            L1InstructionTraceCacheSize = 12 * 1024;
            L1InstructionTraceCacheSetAssociative = 8;
            break;

        case 0x71: // Instruction Trace cache, 8 way set associative, 16K uOps
            L1InstructionTraceCacheSize = 16 * 1024;
            L1InstructionTraceCacheSetAssociative = 8;
            break;

        case 0x72: // Instruction Trace cache, 8 way set associative, 32K uOps
            L1InstructionTraceCacheSize = 32 * 1024;
            L1InstructionTraceCacheSetAssociative = 8;
            break;

        case 0x79: // Unified cache, sectored, 64 byte cache line, 8 way set associative, 128K
            L2CacheSize = 128 * 1024;
            L2CacheLineSize = 64;
            L2CacheSetAssociative = 8;
            break;

        case 0x7A: // Unified cache, sectored, 64 byte cache line, 8 way set associative, 256K
            L2CacheSize = 256 * 1024;
            L2CacheLineSize = 64;
            L2CacheSetAssociative = 8;
            break;

        case 0x7B: // Unified cache, sectored, 64 byte cache line, 8 way set associative, 512K
            L2CacheSize = 512 * 1024;
            L2CacheLineSize = 64;
            L2CacheSetAssociative = 8;
            break;

        case 0x7C: // Unified cache, sectored, 64 byte cache line, 8 way set associative, 1M
            L2CacheSize = 1024 * 1024;
            L2CacheLineSize = 64;
            L2CacheSetAssociative = 8;
            break;

        case 0x82: // Unified cache, 32 byte cache line, 8 way set associative, 256K
            L2CacheSize = 256 * 1024;
            L2CacheLineSize = 32;
            L2CacheSetAssociative = 8;
            break;

        case 0x83: // Unified cache, 32 byte cache line, 8 way set associative, 512K
            L2CacheSize = 512 * 1024;
            L2CacheLineSize = 32;
            L2CacheSetAssociative = 8;
            break;

        case 0x84: // Unified cache, 32 byte cache line, 8 way set associative, 1M
            L2CacheSize = 1024 * 1024;
            L2CacheLineSize = 32;
            L2CacheSetAssociative = 8;
            break;

        case 0x85: // Unified cache, 32 byte cache line, 8 way set associative, 2M
            L2CacheSize = 2048 * 1024;
            L2CacheLineSize = 32;
            L2CacheSetAssociative = 8;
            break;

        case 0x86: // Unified cache, 64 byte cache line, 4 way set associative, 512K
            L2CacheSize = 512 * 1024;
            L2CacheLineSize = 64;
            L2CacheSetAssociative = 4;
            break;

        case 0x87: // Unified cache, 64 byte cache line, 8 way set associative, 1M
            L2CacheSize = 1024 * 1024;
            L2CacheLineSize = 64;
            L2CacheSetAssociative = 8;
            break;

        default:
            break;
    }
}

void CPUDetectClass::Process_Extended_Cache_Info()
{
    CPUIDStruct max_ext(0x80000000);

    if (max_ext.eax >= 0x80000006) {
        CPUIDStruct l1info(0x80000005);
        CPUIDStruct l2info(0x80000006);

        // L1 data cache
        L1DataCacheLineSize = l1info.ecx & 0xFF;
        L1DataCacheSetAssociative = (l1info.ecx >> 16) & 0xFF;
        L1DataCacheSize = ((l1info.ecx >> 24) & 0xFF) * 1024;

        // L1 instruction cache
        L1InstructionCacheLineSize = l1info.edx & 0xFF;
        L1InstructionCacheSetAssociative = (l1info.edx >> 16) & 0xFF;
        L1InstructionCacheSize = ((l1info.edx >> 24) & 0xFF) * 1024;

        // L2 cache
        L2CacheSize = ((l2info.ecx >> 16) & 0xFFFF) * 1024;
        L2CacheLineSize = l2info.ecx & 0xFF;

        switch ((l2info.ecx >> 12) & 0xF) {
            case 0:
                L2CacheSetAssociative = 0;
                break;
            case 1:
                L2CacheSetAssociative = 1;
                break;
            case 2:
                L2CacheSetAssociative = 2;
                break;
            case 4:
                L2CacheSetAssociative = 4;
                break;
            case 6:
                L2CacheSetAssociative = 8;
                break;
            case 8:
                L2CacheSetAssociative = 16;
                break;
            case 15:
                L2CacheSetAssociative = 0xFF;
                break;
            default:
                break;
        }
    }
}

void CPUDetectClass::Process_Intel_Cache_Info()
{
    for (uint32_t count = 0;; ++count) {
        CPUIDCountStruct id(4, count);

        switch (id.eax & 0x1F) {
            case INTEL_CACHE_END:
                return;
            case INTEL_CACHE_DATA: // fallthrough
            case INTEL_CACHE_UNIFIED:
                switch ((id.eax >> 5) & 0x07) {
                    case 1: {
                        L1DataCacheSetAssociative = ((id.ebx >> 22) & 0x3FF) + 1;
                        L1DataCacheLineSize = (id.ebx & 0x0FFF) + 1;
                        L1DataCacheSize =
                            L1DataCacheSetAssociative * (((id.ebx >> 12) & 0x3FF) + 1) * L1DataCacheLineSize * (id.ecx + 1);
                        L1InstructionCacheSetAssociative = L1DataCacheSetAssociative;
                        L1InstructionCacheLineSize = L1DataCacheLineSize;
                        L1InstructionCacheSize = L1DataCacheSize;
                    } break;
                    case 2: {
                        L2CacheSetAssociative = ((id.ebx >> 22) & 0x3FF) + 1;
                        L2CacheLineSize = (id.ebx & 0x0FFF) + 1;
                        L2CacheSize =
                            L2CacheSetAssociative * (((id.ebx >> 12) & 0x3FF) + 1) * L2CacheLineSize * (id.ecx + 1);
                    } break;
                    case 3: {
                        L3CacheSetAssociative = ((id.ebx >> 22) & 0x3FF) + 1;
                        L3CacheLineSize = (id.ebx & 0x0FFF) + 1;
                        L3CacheSize =
                            L3CacheSetAssociative * (((id.ebx >> 12) & 0x3FF) + 1) * L3CacheLineSize * (id.ecx + 1);
                    } break;
                    default:
                        break;
                }

                break;
            default:
                break;
        }
    }
}

void CPUDetectClass::Init_Cache()
{
    CPUIDStruct max_reg(0);
    CPUIDStruct cache_id(2);

    if (Get_Processor_Manufacturer() == MANUFACTURER_INTEL && max_reg.eax >= 4) {
        Process_Intel_Cache_Info();
    } else if ((cache_id.eax & 0xFF) == 1) {
        if (!(cache_id.eax & 0x80000000)) {
            Process_Cache_Info((cache_id.eax >> 24) & 0xFF);
            Process_Cache_Info((cache_id.eax >> 16) & 0xFF);
            Process_Cache_Info((cache_id.eax >> 8) & 0xFF);
        }

        if (!(cache_id.ebx & 0x80000000)) {
            Process_Cache_Info((cache_id.ebx >> 24) & 0xFF);
            Process_Cache_Info((cache_id.ebx >> 16) & 0xFF);
            Process_Cache_Info((cache_id.ebx >> 8) & 0xFF);
            Process_Cache_Info((cache_id.ebx) & 0xFF);
        }

        if (!(cache_id.ecx & 0x80000000)) {
            Process_Cache_Info((cache_id.ecx >> 24) & 0xFF);
            Process_Cache_Info((cache_id.ecx >> 16) & 0xFF);
            Process_Cache_Info((cache_id.ecx >> 8) & 0xFF);
            Process_Cache_Info((cache_id.ecx) & 0xFF);
        }

        if (!(cache_id.edx & 0x80000000)) {
            Process_Cache_Info((cache_id.edx >> 24) & 0xFF);
            Process_Cache_Info((cache_id.edx >> 16) & 0xFF);
            Process_Cache_Info((cache_id.edx >> 8) & 0xFF);
            Process_Cache_Info((cache_id.edx) & 0xFF);
        }
    } else {
        Process_Extended_Cache_Info();
    }
}

void CPUDetectClass::Init_Intel_Processor_Type()
{
    switch (ProcessorFamily) {
        case 3:
            IntelProcessor = INTEL_PROCESSOR_80386;
            break;
        case 4:
            switch (ProcessorModel) {
                default:
                case 0:
                case 1:
                    IntelProcessor = INTEL_PROCESSOR_80486DX;
                    break;
                case 2:
                    IntelProcessor = INTEL_PROCESSOR_80486SX;
                    break;
                case 3:
                    IntelProcessor = INTEL_PROCESSOR_80486DX2;
                    break;
                case 4:
                    IntelProcessor = INTEL_PROCESSOR_80486SL;
                    break;
                case 5:
                    IntelProcessor = INTEL_PROCESSOR_80486SX2;
                    break;
                case 7:
                    IntelProcessor = INTEL_PROCESSOR_80486DX2_WB;
                    break;
                case 8:
                    IntelProcessor = INTEL_PROCESSOR_80486DX4;
                    break;
                case 9:
                    IntelProcessor = INTEL_PROCESSOR_80486DX4_WB;
                    break;
            }

            break;
        case 5:
            switch (ProcessorModel) {
                default:
                case 0:
                    IntelProcessor = INTEL_PROCESSOR_PENTIUM;
                    break;
                case 1:
                    IntelProcessor = INTEL_PROCESSOR_PENTIUM;
                    break;
                case 2:
                    IntelProcessor = INTEL_PROCESSOR_PENTIUM;
                    break;
                case 3:
                    IntelProcessor = INTEL_PROCESSOR_PENTIUM_OVERDRIVE;
                    break;
                case 4:
                    IntelProcessor = INTEL_PROCESSOR_PENTIUM_MMX;
                    break;
                case 5:
                    IntelProcessor = INTEL_PROCESSOR_PENTIUM_OVERDRIVE;
                    break;
                case 6:
                    IntelProcessor = INTEL_PROCESSOR_PENTIUM_OVERDRIVE;
                    break;
                case 7:
                    IntelProcessor = INTEL_PROCESSOR_PENTIUM;
                    break;
                case 8:
                    IntelProcessor = INTEL_PROCESSOR_PENTIUM_MMX;
                    break;
            }
            break;
        case 6:
            switch (ProcessorModel) {
                default:
                case 0:
                    IntelProcessor = INTEL_PROCESSOR_PENTIUM_PRO_SAMPLE;
                    break;
                case 1:
                    IntelProcessor = INTEL_PROCESSOR_PENTIUM_PRO;
                    break;
                case 3:
                    if (ProcessorType == 0) {
                        IntelProcessor = INTEL_PROCESSOR_PENTIUM_II_MODEL_3;
                    } else {
                        IntelProcessor = INTEL_PROCESSOR_PENTIUM_II_OVERDRIVE;
                    }

                    break;
                case 4:
                    IntelProcessor = INTEL_PROCESSOR_PENTIUM_II_MODEL_4;
                    break;
                case 5: {
                    CPUIDStruct cache_id(2);

                    if (L2CacheSize == 0) {
                        IntelProcessor = INTEL_PROCESSOR_CELERON_MODEL_5;
                    } else if (L2CacheSize <= 512 * 1024) {
                        IntelProcessor = INTEL_PROCESSOR_PENTIUM_II_MODEL_5;
                    } else {
                        IntelProcessor = INTEL_PROCESSOR_PENTIUM_II_XEON_MODEL_5;
                    }
                } break;

                case 6:
                    IntelProcessor = INTEL_PROCESSOR_CELERON_MODEL_6;
                    break;
                case 7:
                    if (L2CacheSize <= 512 * 1024) {
                        IntelProcessor = INTEL_PROCESSOR_PENTIUM_III_MODEL_7;
                    } else {
                        IntelProcessor = INTEL_PROCESSOR_PENTIUM_III_XEON_MODEL_7;
                    }

                    break;
                case 8:
                    // This could be PentiumIII Coppermine or Celeron with SSE, or a Xeon
                    {
                        CPUIDStruct brand(1);
                        switch (brand.ebx & 0xFF) {
                            case 0x1:
                                IntelProcessor = INTEL_PROCESSOR_CELERON_MODEL_8;
                                break;
                            case 0x2:
                            case 0x4:
                                IntelProcessor = INTEL_PROCESSOR_PENTIUM_III_MODEL_8;
                                break;
                            case 0x3:
                            case 0xE:
                                IntelProcessor = INTEL_PROCESSOR_PENTIUM_III_XEON_MODEL_8;
                                break;
                        }
                    }

                    break;
                case 0xA:
                    IntelProcessor = INTEL_PROCESSOR_PENTIUM_III_XEON_MODEL_A;
                    break;
                case 0xB:
                    IntelProcessor = INTEL_PROCESSOR_PENTIUM_III_MODEL_B;
                    break;
            }

            break;
        case 0xF:
            IntelProcessor = INTEL_PROCESSOR_PENTIUM4;
            break;
        default:
            IntelProcessor = INTEL_PROCESSOR_PENTIUM_III_MODEL_B;
            break;
    }
}

void CPUDetectClass::Init_AMD_Processor_Type()
{
    switch (ProcessorFamily) {
        case 4:
            switch (ProcessorModel) {
                case 3:
                    AMDProcessor = AMD_PROCESSOR_486DX2;
                    break;
                case 7:
                    AMDProcessor = AMD_PROCESSOR_486DX4;
                    break;
                case 8:
                    AMDProcessor = AMD_PROCESSOR_486DX4;
                    break;
                case 9:
                    AMDProcessor = AMD_PROCESSOR_486DX4;
                    break;
                case 0xe:
                    AMDProcessor = AMD_PROCESSOR_5x86;
                    break;
                case 0xf:
                    AMDProcessor = AMD_PROCESSOR_5x86;
                    break;
                default:
                    AMDProcessor = AMD_PROCESSOR_486;
            }
            break;
        case 5:
            switch (ProcessorModel) {
                case 0:
                    AMDProcessor = AMD_PROCESSOR_K5_MODEL0;
                    break;
                case 1:
                    AMDProcessor = AMD_PROCESSOR_K5_MODEL1;
                    break;
                case 2:
                    AMDProcessor = AMD_PROCESSOR_K5_MODEL2;
                    break;
                case 3:
                    AMDProcessor = AMD_PROCESSOR_K5_MODEL3;
                    break;
                case 6:
                    AMDProcessor = AMD_PROCESSOR_K6_MODEL6;
                    break;
                case 7:
                    AMDProcessor = AMD_PROCESSOR_K6_MODEL7;
                    break;
                case 8:
                    AMDProcessor = AMD_PROCESSOR_K6_2_3DNOW_MODEL8;
                    break;
                case 9:
                    AMDProcessor = AMD_PROCESSOR_K6_3_3DNOW_MODEL9;
                    break;
                case 0xd:
                    AMDProcessor = AMD_PROCESSOR_K6_3_PLUS;
                    break;
                default:
                    AMDProcessor = AMD_PROCESSOR_K6;
            }
            break;
        case 6:
            switch (ProcessorModel) {
                case 1:
                    AMDProcessor = AMD_PROCESSOR_ATHLON_025;
                    break;
                case 2:
                    AMDProcessor = AMD_PROCESSOR_ATHLON_018;
                    break;
                case 3:
                    AMDProcessor = AMD_PROCESSOR_DURON;
                    break;
                case 4:
                    AMDProcessor = AMD_PROCESSOR_ATHLON_018_IL2;
                    break;
                default:
                    AMDProcessor = AMD_PROCESSOR_ATHLON;
                    break;
            }
            break;
        default:
            break;
    }
}

void CPUDetectClass::Init_VIA_Processor_Type()
{
    switch (ProcessorFamily) {
        case 5:
            switch (ProcessorModel) {
                case 4:
                    VIAProcessor = VIA_PROCESSOR_IDT_C6_WINCHIP;
                    break;
                case 8:
                    VIAProcessor = VIA_PROCESSOR_IDT_C6_WINCHIP2;
                    break;
                case 9:
                    VIAProcessor = VIA_PROCESSOR_IDT_C6_WINCHIP3;
                    break;
                default:
                    break;
            }
            break;
        case 6:
            switch (ProcessorModel) {
                case 4:
                    VIAProcessor = VIA_PROCESSOR_CYRIX_III_SAMUEL;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void CPUDetectClass::Init_Rise_Processor_Type()
{
    switch (ProcessorFamily) {
        case 5:
            switch (ProcessorModel) {
                case 0:
                    RiseProcessor = RISE_PROCESSOR_DRAGON_025;
                    break;
                case 2:
                    RiseProcessor = RISE_PROCESSOR_DRAGON_018;
                    break;
                case 8:
                    RiseProcessor = RISE_PROCESSOR_DRAGON2_025;
                    break;
                case 9:
                    RiseProcessor = RISE_PROCESSOR_DRAGON2_018;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void CPUDetectClass::Init_Processor_Family()
{
    uint32_t signature = 0;
    uint32_t notneeded1;
    uint32_t notneeded2;
    uint32_t features;

    CPUID(signature, notneeded1, notneeded2, features, 1);

    ProcessorType = (signature >> 12) & 0x0F;
    ProcessorFamily = (signature >> 8) & 0x0F;
    ProcessorModel = (signature >> 4) & 0x0F;
    ProcessorRevision = (signature)&0x0F;

    IntelProcessor = INTEL_PROCESSOR_UNKNOWN;
    AMDProcessor = AMD_PROCESSOR_UNKNOWN;
    VIAProcessor = VIA_PROCESSOR_UNKNOWN;
    RiseProcessor = RISE_PROCESSOR_UNKNOWN;

    Init_Cache();

    switch (ProcessorManufacturer) {
        case MANUFACTURER_INTEL:
            Init_Intel_Processor_Type();
            break;
        case MANUFACTURER_AMD:
            Init_AMD_Processor_Type();
            break;
        case MANUFACTURER_VIA:
            Init_VIA_Processor_Type();
            break;
        case MANUFACTURER_RISE:
            Init_Rise_Processor_Type();
            break;
        default:
            break;
    }
}

void CPUDetectClass::Init_Processor_String()
{
    if (!Has_CPUID_Instruction()) {
        ProcessorString[0] = '\0';
    }

    CPUIDStruct ext_id(0x80000000);

    if (ext_id.eax & 0x80000000) {
        CPUDetectClass::CPUID((uint32_t &)ProcessorString[0],
            (uint32_t &)ProcessorString[4],
            (uint32_t &)ProcessorString[8],
            (uint32_t &)ProcessorString[12],
            0x80000002);

        CPUDetectClass::CPUID((uint32_t &)ProcessorString[16],
            (uint32_t &)ProcessorString[20],
            (uint32_t &)ProcessorString[24],
            (uint32_t &)ProcessorString[28],
            0x80000003);

        CPUDetectClass::CPUID((uint32_t &)ProcessorString[32],
            (uint32_t &)ProcessorString[36],
            (uint32_t &)ProcessorString[40],
            (uint32_t &)ProcessorString[44],
            0x80000004);

        strtrim(ProcessorString);
    } else {
        char str[2048] = { 0 };

        const char *processor_manufacturer_name = Get_Processor_Manufacturer_Name();
        if (processor_manufacturer_name != nullptr) {
            strcat(str, processor_manufacturer_name);
        }

        if (ProcessorManufacturer == MANUFACTURER_INTEL) {
            strcat(str, " ");

            switch (IntelProcessor) {
                case INTEL_PROCESSOR_80386:
                    strcat(str, "80386");
                    break;
                case INTEL_PROCESSOR_80486DX:
                    strcat(str, "80486DX");
                    break;
                case INTEL_PROCESSOR_80486SX:
                    strcat(str, "80486SX");
                    break;
                case INTEL_PROCESSOR_80486DX2:
                    strcat(str, "80486DX2");
                    break;
                case INTEL_PROCESSOR_80486SL:
                    strcat(str, "80486SL");
                    break;
                case INTEL_PROCESSOR_80486SX2:
                    strcat(str, "80486SX2");
                    break;
                case INTEL_PROCESSOR_80486DX2_WB:
                    strcat(str, "80486DX2(WB)");
                    break;
                case INTEL_PROCESSOR_80486DX4:
                    strcat(str, "80486DX4");
                    break;
                case INTEL_PROCESSOR_80486DX4_WB:
                    strcat(str, "80486DX4(WB)");
                    break;
                case INTEL_PROCESSOR_PENTIUM:
                    strcat(str, "Pentium");
                    break;
                case INTEL_PROCESSOR_PENTIUM_OVERDRIVE:
                    strcat(str, "Pentium Overdrive");
                    break;
                case INTEL_PROCESSOR_PENTIUM_MMX:
                    strcat(str, "Pentium MMX");
                    break;
                case INTEL_PROCESSOR_PENTIUM_PRO_SAMPLE:
                    strcat(str, "Pentium Pro (Engineering Sample)");
                    break;
                case INTEL_PROCESSOR_PENTIUM_PRO:
                    strcat(str, "Pentium Pro");
                    break;
                case INTEL_PROCESSOR_PENTIUM_II_OVERDRIVE:
                    strcat(str, "Pentium II Overdrive");
                    break;
                case INTEL_PROCESSOR_PENTIUM_II_MODEL_3:
                    strcat(str, "Pentium II, model 3");
                    break;
                case INTEL_PROCESSOR_PENTIUM_II_MODEL_4:
                    strcat(str, "Pentium II, model 4");
                    break;
                case INTEL_PROCESSOR_CELERON_MODEL_5:
                    strcat(str, "Celeron, model 5");
                    break;
                case INTEL_PROCESSOR_PENTIUM_II_MODEL_5:
                    strcat(str, "Pentium II, model 5");
                    break;
                case INTEL_PROCESSOR_PENTIUM_II_XEON_MODEL_5:
                    strcat(str, "Pentium II Xeon, model 5");
                    break;
                case INTEL_PROCESSOR_CELERON_MODEL_6:
                    strcat(str, "Celeron, model 6");
                    break;
                case INTEL_PROCESSOR_PENTIUM_III_MODEL_7:
                    strcat(str, "Pentium III, model 7");
                    break;
                case INTEL_PROCESSOR_PENTIUM_III_XEON_MODEL_7:
                    strcat(str, "Pentium III Xeon, model 7");
                    break;
                case INTEL_PROCESSOR_CELERON_MODEL_8:
                    strcat(str, "Celeron, model 8");
                    break;
                case INTEL_PROCESSOR_PENTIUM_III_MODEL_8:
                    strcat(str, "Pentium III, model 8");
                    break;
                case INTEL_PROCESSOR_PENTIUM_III_XEON_MODEL_8:
                    strcat(str, "Pentium III Xeon, model 8");
                    break;
                case INTEL_PROCESSOR_PENTIUM_III_XEON_MODEL_A:
                    strcat(str, "Pentium III Xeon, model A");
                    break;
                case INTEL_PROCESSOR_PENTIUM_III_MODEL_B:
                    strcat(str, "Pentium III, model B");
                    break;
                case INTEL_PROCESSOR_PENTIUM4:
                    strcat(str, "Pentium4");
                    break;
                case INTEL_PROCESSOR_UNKNOWN:
                default:
                    strcat(str, "<UNKNOWN>");
                    break;
            }
        }

        strncpy(ProcessorString, str, sizeof(ProcessorString));
    }
}

void CPUDetectClass::Init_CPUID_Instruction()
{
#ifdef PROCESSOR_X86_64
    // 64bit x86 CPU always has it.
    HasCPUIDInstruction = true;
#elif defined HAVE__READEFLAGS && defined HAVE__WRITEEFLAGS
    // 32bit x86 CPU might not have CPUID instruction, though unlikely.
    uint32_t old_eflg;
    uint32_t new_eflg;

    // These intrinsics should be available on recent MSVC and GCC, no more ifdefs.
    old_eflg = __readeflags();
    new_eflg = old_eflg ^ 0x00200000;
    __writeeflags(new_eflg);
    new_eflg = __readeflags();

    HasCPUIDInstruction = new_eflg != old_eflg;
#else
    // None x86 hardware doesn't.
    HasCPUIDInstruction = false;
#endif
}

void CPUDetectClass::Init_Processor_Features()
{
    if (!CPUDetectClass::Has_CPUID_Instruction()) {
        return;
    }

    CPUIDStruct id(1);
    FeatureBits = id.edx;
    HasRDTSCInstruction = (!!(FeatureBits & (1 << 4)));
    HasCMOVSupport = (!!(FeatureBits & (1 << 15)));
    HasMMXSupport = (!!(FeatureBits & (1 << 23)));
    HasSSESupport = !!(FeatureBits & (1 << 25));
    HasSSE2Support = !!(FeatureBits & (1 << 26));
    Has3DNowSupport = false;
    ExtendedFeatureBits = 0;

    if (ProcessorManufacturer == MANUFACTURER_AMD) {
        if (Has_CPUID_Instruction()) {
            CPUIDStruct max_ext_id(0x80000000);

            if (max_ext_id.eax >= 0x80000001) { // Signature & features field available?
                CPUIDStruct ext_signature(0x80000001);
                ExtendedFeatureBits = ext_signature.edx;
                Has3DNowSupport = !!(ExtendedFeatureBits & 0x80000000);
                HasExtended3DNowSupport = !!(ExtendedFeatureBits & 0x40000000);
            }
        }
    }
}

void CPUDetectClass::Init_Memory()
{
#if defined PLATFORM_WINDOWS
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);
    TotalPhysicalMemory = mem.ullTotalPhys;
    AvailablePhysicalMemory = mem.ullAvailPhys;
    TotalPageMemory = mem.ullTotalPageFile;
    AvailablePageMemory = mem.ullAvailPageFile;
    TotalVirtualMemory = mem.ullTotalVirtual;
    AvailableVirtualMemory = mem.ullAvailVirtual;
#elif defined PLATFORM_LINUX
    struct sysinfo mem;
    sysinfo(&mem);
    TotalPhysicalMemory = mem.totalram * mem.mem_unit;
    AvailablePhysicalMemory = mem.freeram * mem.mem_unit;
    TotalPageMemory = mem.totalswap * mem.mem_unit;
    AvailablePageMemory = mem.freeswap * mem.mem_unit;
    TotalVirtualMemory = uintptr_t(intptr_t(-1)); // Should give ~4GB on 32bit and considerably more on 64bit.
    AvailableVirtualMemory = uintptr_t(intptr_t(-1));
#endif
}

void CPUDetectClass::Init_OS()
{
    OSVersionExtraInfo[0] = '\0';
#if defined PLATFORM_WINDOWS
    typedef LONG(WINAPI * getinfoptr_t)(PRTL_OSVERSIONINFOW);
    HMODULE nt_lib = LoadLibraryExA("ntdll", NULL, 0);
    if (nt_lib != nullptr) {
        getinfoptr_t RtlGetVersion = (getinfoptr_t)::GetProcAddress(nt_lib, "RtlGetVersion");
        bool done = false;

        if (RtlGetVersion != nullptr) {
            RTL_OSVERSIONINFOW os = { 0 };
            os.dwOSVersionInfoSize = sizeof(os);
            RtlGetVersion(&os);

            OSVersionNumberMajor = os.dwMajorVersion;
            OSVersionNumberMinor = os.dwMinorVersion;
            OSVersionBuildNumber = os.dwBuildNumber;
            OSVersionPlatformId = os.dwPlatformId;

            done = true;
        }

        FreeLibrary(nt_lib);

        if (done) {
            return;
        }
    }

    OSVersionNumberMajor = 6;
    OSVersionNumberMinor = 2;
    OSVersionBuildNumber = 0;
    OSVersionPlatformId = 2;
#elif defined PLATFORM_OSX
    char str[256];
    size_t size = sizeof(str);
    int ret = sysctlbyname("kern.osrelease", str, &size, NULL, 0);
    int major;
    int minor;
    int build;

    sscanf(str, "%d.%d.%d", &major, &minor, &build);

    OSVersionNumberMajor = 10;
    OSVersionNumberMinor = major - 4;
    OSVersionBuildNumber = minor;
    OSVersionPlatformId = -1;
#elif defined PLATFORM_LINUX
    struct utsname uts;
    uname(&uts);

    int major;
    int minor;
    int build;

    sscanf(uts.release, "%d.%d.%d", &major, &minor, &build);

    OSVersionNumberMajor = major;
    OSVersionNumberMinor = minor;
    OSVersionBuildNumber = build;
    OSVersionPlatformId = -1;
#else
    OSVersionNumberMajor = 0;
    OSVersionNumberMinor = 0;
    OSVersionBuildNumber = 0;
    OSVersionPlatformId = -1;
#endif
}

bool CPUDetectClass::CPUID(uint32_t &u_eax_, uint32_t &u_ebx_, uint32_t &u_ecx_, uint32_t &u_edx_, uint32_t cpuid_type)
{
    if (!Has_CPUID_Instruction()) {
        return false;
    }

    int32_t regs[4] = { 0 };

#ifdef HAVE__CPUID
    __cpuidc(regs, cpuid_type);
#endif

    u_eax_ = regs[0];
    u_ebx_ = regs[1];
    u_ecx_ = regs[2];
    u_edx_ = regs[3];

    return true;
}

bool CPUDetectClass::CPUID_Count(
    uint32_t &u_eax_, uint32_t &u_ebx_, uint32_t &u_ecx_, uint32_t &u_edx_, uint32_t cpuid_type, uint32_t count)
{
    if (!Has_CPUID_Instruction()) {
        return false;
    }

    int32_t regs[4] = { 0 };

#ifdef HAVE__CPUIDEX
    __cpuidex(regs, cpuid_type, count);
#endif

    u_eax_ = regs[0];
    u_ebx_ = regs[1];
    u_ecx_ = regs[2];
    u_edx_ = regs[3];

    return true;
}

void CPUDetectClass::Init_Processor_Log()
{
#define CPU_LOG(n, ...) \
    do { \
        snprintf(work, sizeof(work), n, ##__VA_ARGS__); \
        strlcat_tpl(ProcessorLog, work); \
    } while (false)

    char work[256];

    CPU_LOG("Operating system: ");

    switch (OSVersionPlatformId) {
        case 0:
            CPU_LOG("Windows 3.1");
            break;
        case 1:
            CPU_LOG("Windows 9x");
            break;
        case 2:
            CPU_LOG("Windows NT");
            break;
        default:
            CPU_LOG(PLATFORM_NAME);
            break;
    }

    CPU_LOG("\n");

#if defined PLATFORM_WINDOWS
    CPU_LOG("Operating system version: %u.%u\n", OSVersionNumberMajor, OSVersionNumberMinor);
    CPU_LOG("Operating system build: %u.%u.%u\n",
        (OSVersionBuildNumber & 0xFF000000) >> 24,
        (OSVersionBuildNumber & 0xFF0000) >> 16,
        (OSVersionBuildNumber & 0xFFFF));
    // CPU_LOG("OS-Info: %s\n", OSVersionExtraInfo);
#elif defined PLATFORM_OSX || defined PLATFORM_LINUX
    CPU_LOG("Operating system version: %d.%d.%d\n", OSVersionNumberMajor, OSVersionNumberMinor, OSVersionBuildNumber);
#endif

    CPU_LOG("Processor: %s\n", CPUDetectClass::Get_Processor_String());
    CPU_LOG("Clock speed: ~%dMHz\n", CPUDetectClass::Get_Processor_Speed());

    StringClass cpu_type;
    switch (CPUDetectClass::Get_Processor_Type()) {
        case 0:
            cpu_type = "Original OEM";
            break;
        case 1:
            cpu_type = "Overdrive";
            break;
        case 2:
            cpu_type = "Dual";
            break;
        case 3:
            cpu_type = "*Intel Reserved*";
            break;
        default:
            break;
    }

    // CPU_LOG("Processor type: %s\n", cpu_type);

    CPU_LOG("\n");

    CPU_LOG("Total physical memory: %" PRIu64 "Mb\n", Get_Total_Physical_Memory() / (1024 * 1024));
    CPU_LOG("Available physical memory: %" PRIu64 "Mb\n", Get_Available_Physical_Memory() / (1024 * 1024));
    CPU_LOG("Total page file size: %" PRIu64 "Mb\n", Get_Total_Page_File_Size() / (1024 * 1024));
    CPU_LOG("Total available page file size: %" PRIu64 "Mb\n", Get_Available_Page_File_Size() / (1024 * 1024));
    CPU_LOG("Total virtual memory: %" PRIu64 "Mb\n", Get_Total_Virtual_Memory() / (1024 * 1024));
    CPU_LOG("Available virtual memory: %" PRIu64 "Mb\n", Get_Available_Virtual_Memory() / (1024 * 1024));

    CPU_LOG("\n");

    CPU_LOG("CPUID: %s\n", CPUDetectClass::Has_CPUID_Instruction() ? "Yes" : "No");
    CPU_LOG("RDTSC: %s\n", CPUDetectClass::Has_RDTSC_Instruction() ? "Yes" : "No");
    CPU_LOG("CMOV: %s\n", CPUDetectClass::Has_CMOV_Instruction() ? "Yes" : "No");
    CPU_LOG("MMX: %s\n", CPUDetectClass::Has_MMX_Instruction_Set() ? "Yes" : "No");
    CPU_LOG("SSE: %s\n", CPUDetectClass::Has_SSE_Instruction_Set() ? "Yes" : "No");
    CPU_LOG("SSE2: %s\n", CPUDetectClass::Has_SSE2_Instruction_Set() ? "Yes" : "No");
    CPU_LOG("3DNow!: %s\n", CPUDetectClass::Has_3DNow_Instruction_Set() ? "Yes" : "No");
    CPU_LOG("Extended 3DNow!: %s\n", CPUDetectClass::Has_Extended_3DNow_Instruction_Set() ? "Yes" : "No");
    CPU_LOG("CPU Feature bits: 0x%x\n", CPUDetectClass::Get_Feature_Bits());
    CPU_LOG("Ext. CPU Feature bits: 0x%x\n", CPUDetectClass::Get_Extended_Feature_Bits());

    CPU_LOG("\n");

    if (CPUDetectClass::Get_L1_Data_Cache_Size() > 0) {
        CPU_LOG("L1 Data Cache: %u byte cache lines, %u way set associative, %uk\n",
            CPUDetectClass::Get_L1_Data_Cache_Line_Size(),
            CPUDetectClass::Get_L1_Data_Cache_Set_Associative(),
            CPUDetectClass::Get_L1_Data_Cache_Size() / 1024u);
    } else {
        CPU_LOG("L1 Data Cache: None\n");
    }

    if (CPUDetectClass::Get_L1_Instruction_Cache_Size() > 0) {
        CPU_LOG("L1 Instruction Cache: %u byte cache lines, %u way set associative, %uk\n",
            CPUDetectClass::Get_L1_Instruction_Cache_Line_Size(),
            CPUDetectClass::Get_L1_Instruction_Cache_Set_Associative(),
            CPUDetectClass::Get_L1_Instruction_Cache_Size() / 1024u);
    } else {
        CPU_LOG("L1 Instruction Cache: None\n");
    }

    if (CPUDetectClass::Get_L1_Instruction_Trace_Cache_Size() > 0) {
        CPU_LOG("L1 Instruction Trace Cache: %u way set associative, %uk �OPs\n",
            CPUDetectClass::Get_L1_Instruction_Cache_Set_Associative(),
            CPUDetectClass::Get_L1_Instruction_Cache_Size() / 1024u);
    } else {
        CPU_LOG("L1 Instruction Trace Cache: None\n");
    }

    if (CPUDetectClass::Get_L2_Cache_Size() > 0) {
        CPU_LOG("L2 Cache: %u byte cache lines, %u way set associative, %uk\n",
            CPUDetectClass::Get_L2_Cache_Line_Size(),
            CPUDetectClass::Get_L2_Cache_Set_Associative(),
            CPUDetectClass::Get_L2_Cache_Size() / 1024u);
    } else {
        CPU_LOG("L2 cache: None\n");
    }

    if (CPUDetectClass::Get_L3_Cache_Size() > 0) {
        CPU_LOG("L3 Cache: %u byte cache lines, %u way set associative, %uk\n",
            CPUDetectClass::Get_L3_Cache_Line_Size(),
            CPUDetectClass::Get_L3_Cache_Set_Associative(),
            CPUDetectClass::Get_L3_Cache_Size() / 1024u);
    } else {
        CPU_LOG("L3 cache: None\n");
    }

    CPU_LOG("\n");
#undef CPU_LOG
}

void CPUDetectClass::Init_Compact_Log()
{
#define COMPACT_LOG(n, ...) \
    do { \
        snprintf(work, sizeof(work), n, ##__VA_ARGS__); \
        strlcat_tpl(CompactLog, work); \
    } while (false)

    char work[256];

#ifdef PLATFORM_WINDOWS
    TIME_ZONE_INFORMATION time_zone;
    GetTimeZoneInformation(&time_zone);
    COMPACT_LOG("%d\t", time_zone.Bias);
#endif

    OSInfoStruct os_info;
    Get_OS_Info(os_info, OSVersionPlatformId, OSVersionNumberMajor, OSVersionNumberMinor, OSVersionBuildNumber);
    COMPACT_LOG("%s\t", os_info.Code);

    if (!strcasecmp(os_info.SubCode, "UNKNOWN")) {
        COMPACT_LOG("%u\t", OSVersionBuildNumber & 0xFFFF);
    } else {
        COMPACT_LOG("%s\t", os_info.SubCode);
    }

    const char *processor_manufacturer_name = Get_Processor_Manufacturer_Name();
    COMPACT_LOG("%s\t%d\t", processor_manufacturer_name ? processor_manufacturer_name : "", Get_Processor_Speed());

    COMPACT_LOG("%" PRIu64 "\t", Get_Total_Physical_Memory() / (1024 * 1024) + 1);

    COMPACT_LOG("%x\t%x\t", Get_Feature_Bits(), Get_Extended_Feature_Bits());
#undef COMPACT_LOG
}

// Initialises the info in the CPUID static class during static init before main is run.
static class CPUDetectInitClass
{
public:
    CPUDetectInitClass()
    {
        CPUDetectClass::Init_CPUID_Instruction();
        // The game assumes its going to run on x86 hardware, hopefully we will be
        // able to change that eventually.
        if (CPUDetectClass::Has_CPUID_Instruction()) {
            CPUDetectClass::Init_Processor_Manufacturer();
            CPUDetectClass::Init_Processor_Family();
            CPUDetectClass::Init_Processor_String();
            CPUDetectClass::Init_Processor_Features();
            CPUDetectClass::Init_Memory();
            CPUDetectClass::Init_OS();
        }
        CPUDetectClass::Init_Processor_Speed();

        CPUDetectClass::Init_Processor_Log();
        CPUDetectClass::Init_Compact_Log();
    }
} _CPU_Detect_Init;
