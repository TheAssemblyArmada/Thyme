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
#pragma once

#include "always.h"
#include "wwstring.h"

class CPUDetectInitClass;

class CPUDetectClass
{
    friend class CPUDetectInitClass;

public:
    enum ProcessorManufacturerType
    {
        MANUFACTURER_UNKNOWN,
        MANUFACTURER_INTEL,
        MANUFACTURER_UMC,
        MANUFACTURER_AMD,
        MANUFACTURER_CYRIX,
        MANUFACTURER_NEXTGEN,
        MANUFACTURER_VIA,
        MANUFACTURER_RISE,
        MANUFACTURER_TRANSMETA
    };

    enum IntelProcessorType
    {
        INTEL_PROCESSOR_UNKNOWN,
        INTEL_PROCESSOR_80386,
        INTEL_PROCESSOR_80486DX,
        INTEL_PROCESSOR_80486SX,
        INTEL_PROCESSOR_80486DX2,
        INTEL_PROCESSOR_80486SL,
        INTEL_PROCESSOR_80486SX2,
        INTEL_PROCESSOR_80486DX2_WB,
        INTEL_PROCESSOR_80486DX4,
        INTEL_PROCESSOR_80486DX4_WB,
        INTEL_PROCESSOR_PENTIUM,
        INTEL_PROCESSOR_PENTIUM_OVERDRIVE,
        INTEL_PROCESSOR_PENTIUM_MMX,
        INTEL_PROCESSOR_PENTIUM_PRO_SAMPLE,
        INTEL_PROCESSOR_PENTIUM_PRO,
        INTEL_PROCESSOR_PENTIUM_II_OVERDRIVE,
        INTEL_PROCESSOR_PENTIUM_II_MODEL_3,
        INTEL_PROCESSOR_PENTIUM_II_MODEL_4,
        INTEL_PROCESSOR_CELERON_MODEL_5,
        INTEL_PROCESSOR_PENTIUM_II_MODEL_5,
        INTEL_PROCESSOR_PENTIUM_II_XEON_MODEL_5,
        INTEL_PROCESSOR_CELERON_MODEL_6,
        INTEL_PROCESSOR_PENTIUM_III_MODEL_7,
        INTEL_PROCESSOR_PENTIUM_III_XEON_MODEL_7,
        INTEL_PROCESSOR_CELERON_MODEL_8,
        INTEL_PROCESSOR_PENTIUM_III_MODEL_8,
        INTEL_PROCESSOR_PENTIUM_III_XEON_MODEL_8,
        INTEL_PROCESSOR_PENTIUM_III_XEON_MODEL_A,
        INTEL_PROCESSOR_PENTIUM_III_MODEL_B,
        INTEL_PROCESSOR_PENTIUM4
    };

    enum AMDProcessorType
    {
        AMD_PROCESSOR_UNKNOWN,
        AMD_PROCESSOR_486DX2,
        AMD_PROCESSOR_486DX4,
        AMD_PROCESSOR_5x86,
        AMD_PROCESSOR_486,
        AMD_PROCESSOR_K5_MODEL0,
        AMD_PROCESSOR_K5_MODEL1,
        AMD_PROCESSOR_K5_MODEL2,
        AMD_PROCESSOR_K5_MODEL3,
        AMD_PROCESSOR_K6_MODEL6,
        AMD_PROCESSOR_K6_MODEL7,
        AMD_PROCESSOR_K6_2_3DNOW_MODEL8,
        AMD_PROCESSOR_K6_3_3DNOW_MODEL9,
        AMD_PROCESSOR_K6_3_PLUS,
        AMD_PROCESSOR_K6,
        AMD_PROCESSOR_ATHLON_025,
        AMD_PROCESSOR_ATHLON_018,
        AMD_PROCESSOR_DURON,
        AMD_PROCESSOR_ATHLON_018_IL2,
        AMD_PROCESSOR_ATHLON
    };

    enum VIAProcessorType
    {
        VIA_PROCESSOR_UNKNOWN,
        VIA_PROCESSOR_IDT_C6_WINCHIP,
        VIA_PROCESSOR_IDT_C6_WINCHIP2,
        VIA_PROCESSOR_IDT_C6_WINCHIP3,
        VIA_PROCESSOR_CYRIX_III_SAMUEL
    };

    enum RiseProcessorType
    {
        RISE_PROCESSOR_UNKNOWN,
        RISE_PROCESSOR_DRAGON_025,
        RISE_PROCESSOR_DRAGON_018,
        RISE_PROCESSOR_DRAGON2_025,
        RISE_PROCESSOR_DRAGON2_018
    };

    enum IntelCacheType
    {
        INTEL_CACHE_END,
        INTEL_CACHE_DATA,
        INTEL_CACHE_INST,
        INTEL_CACHE_UNIFIED,
    };

    static ProcessorManufacturerType Get_Processor_Manufacturer() { return ProcessorManufacturer; }
    static const char *Get_Processor_Manufacturer_Name();

    static bool Has_CPUID_Instruction() { return HasCPUIDInstruction; }
    static bool Has_RDTSC_Instruction() { return HasRDTSCInstruction; }
    static bool Has_CMOV_Instruction() { return HasCMOVSupport; }
    static bool Has_MMX_Instruction_Set() { return HasMMXSupport; }
    static bool Has_SSE_Instruction_Set() { return HasSSESupport; }
    static bool Has_SSE2_Instruction_Set() { return HasSSE2Support; }
    static bool Has_3DNow_Instruction_Set() { return Has3DNowSupport; }
    static bool Has_Extended_3DNow_Instruction_Set() { return HasExtended3DNowSupport; }

    static IntelProcessorType Get_Intel_Processor() { return IntelProcessor; }
    static AMDProcessorType Get_AMD_Processor() { return AMDProcessor; }
    static VIAProcessorType Get_VIA_Processor() { return VIAProcessor; }
    static RiseProcessorType Get_Rise_Processor() { return RiseProcessor; }

    // Speed is calculated once during static initialisation, depending on CPU model
    // speed state of CPU could affect this.
    static int32_t Get_Processor_Speed() { return ProcessorSpeed; }
    static int64_t Get_Processor_Ticks_Per_Second() { return ProcessorTicksPerSecond; } // Ticks per second
    static double Get_Inv_Processor_Ticks_Per_Second() { return InvProcessorTicksPerSecond; } // 1.0 / Ticks per second

    static uint32_t Get_Feature_Bits() { return FeatureBits; }
    static uint32_t Get_Extended_Feature_Bits() { return ExtendedFeatureBits; }

    static uint32_t Get_L3_Cache_Size() { return L3CacheSize; }
    static uint32_t Get_L3_Cache_Line_Size() { return L3CacheLineSize; }
    static uint32_t Get_L3_Cache_Set_Associative() { return L3CacheSetAssociative; }
    static uint32_t Get_L2_Cache_Size() { return L2CacheSize; }
    static uint32_t Get_L2_Cache_Line_Size() { return L2CacheLineSize; }
    static uint32_t Get_L2_Cache_Set_Associative() { return L2CacheSetAssociative; }
    static uint32_t Get_L1_Data_Cache_Size() { return L1DataCacheSize; }
    static uint32_t Get_L1_Data_Cache_Line_Size() { return L1DataCacheLineSize; }
    static uint32_t Get_L1_Data_Cache_Set_Associative() { return L1DataCacheSetAssociative; }
    static uint32_t Get_L1_Instruction_Cache_Size() { return L1InstructionCacheSize; }
    static uint32_t Get_L1_Instruction_Cache_Line_Size() { return L1InstructionCacheLineSize; }
    static uint32_t Get_L1_Instruction_Cache_Set_Associative() { return L1InstructionCacheSetAssociative; }
    static uint32_t Get_L1_Instruction_Trace_Cache_Size() { return L1InstructionTraceCacheSize; }
    static uint32_t Get_L1_Instruction_Trace_Cache_Set_Associative() { return L1InstructionTraceCacheSetAssociative; }

    static uint64_t Get_Total_Physical_Memory() { return TotalPhysicalMemory; }
    static uint64_t Get_Available_Physical_Memory() { return AvailablePhysicalMemory; }
    static uint64_t Get_Total_Page_File_Size() { return TotalPageMemory; }
    static uint64_t Get_Available_Page_File_Size() { return AvailablePageMemory; }
    static uint64_t Get_Total_Virtual_Memory() { return TotalVirtualMemory; }
    static uint64_t Get_Available_Virtual_Memory() { return AvailableVirtualMemory; }

    static uint32_t Get_Processor_Type() { return ProcessorType; }

    static const char *Get_Processor_String() { return ProcessorString; }
    static const char *Get_Processor_Log() { return ProcessorLog; }
    static const char *Get_Compact_Log() { return CompactLog; }

    static bool CPUID(uint32_t &u_eax_, uint32_t &u_ebx_, uint32_t &u_ecx_, uint32_t &u_edx_, uint32_t cpuid_type);
    static bool CPUID_Count(
        uint32_t &u_eax_, uint32_t &u_ebx_, uint32_t &u_ecx_, uint32_t &u_edx_, uint32_t cpuid_type, uint32_t count);

private:
    static void Init_CPUID_Instruction();
    static void Init_Processor_Speed();
    static void Init_Processor_String();
    static void Init_Processor_Manufacturer();
    static void Init_Processor_Family();
    static void Init_Processor_Features();
    static void Init_Memory();
    static void Init_OS();

    static void Init_Intel_Processor_Type();
    static void Init_AMD_Processor_Type();
    static void Init_VIA_Processor_Type();
    static void Init_Rise_Processor_Type();

    static void Init_Cache();

    static void Init_Processor_Log();
    static void Init_Compact_Log();

    static void Process_Cache_Info(uint32_t value);
    static void Process_Extended_Cache_Info();
    static void Process_Intel_Cache_Info();

    static char ProcessorLog[1024];
    static char CompactLog[1024];

    static int32_t ProcessorType;
    static int32_t ProcessorFamily;
    static int32_t ProcessorModel;
    static int32_t ProcessorRevision;
    static int32_t ProcessorSpeed;
    static int64_t ProcessorTicksPerSecond; // Ticks per second
    static double InvProcessorTicksPerSecond; // 1.0 / Ticks per second

    static ProcessorManufacturerType ProcessorManufacturer;
    static IntelProcessorType IntelProcessor;
    static AMDProcessorType AMDProcessor;
    static VIAProcessorType VIAProcessor;
    static RiseProcessorType RiseProcessor;

    static uint32_t FeatureBits;
    static uint32_t ExtendedFeatureBits;

    // L3 cache information
    static uint32_t L3CacheSize;
    static uint32_t L3CacheLineSize;
    static uint32_t L3CacheSetAssociative;

    // L2 cache information
    static uint32_t L2CacheSize;
    static uint32_t L2CacheLineSize;
    static uint32_t L2CacheSetAssociative;

    // L1 data cache information
    static uint32_t L1DataCacheSize;
    static uint32_t L1DataCacheLineSize;
    static uint32_t L1DataCacheSetAssociative;

    // L1 instruction cache information
    static uint32_t L1InstructionCacheSize;
    static uint32_t L1InstructionCacheLineSize;
    static uint32_t L1InstructionCacheSetAssociative;

    // L1 instruction trace cache information
    static uint32_t L1InstructionTraceCacheSize;
    static uint32_t L1InstructionTraceCacheSetAssociative;

    static uint64_t TotalPhysicalMemory;
    static uint64_t AvailablePhysicalMemory;
    static uint64_t TotalPageMemory;
    static uint64_t AvailablePageMemory;
    static uint64_t TotalVirtualMemory;
    static uint64_t AvailableVirtualMemory;

    static uint32_t OSVersionNumberMajor;
    static uint32_t OSVersionNumberMinor;
    static uint32_t OSVersionBuildNumber;
    static uint32_t OSVersionPlatformId;
    static char OSVersionExtraInfo[512];

    static bool HasCPUIDInstruction;
    static bool HasRDTSCInstruction;
    static bool HasSSESupport;
    static bool HasSSE2Support;
    static bool HasCMOVSupport;
    static bool HasMMXSupport;
    static bool Has3DNowSupport;
    static bool HasExtended3DNowSupport;
    static char VendorID[20];
    static char ProcessorString[48];
};

class CPUIDStruct
{
public:
    CPUIDStruct(uint32_t cpuid_type)
    {
        if (!CPUDetectClass::Has_CPUID_Instruction()) {
            eax = ebx = ecx = edx = 0;

            return;
        }

        CPUDetectClass::CPUID(eax, ebx, ecx, edx, cpuid_type);
    }

    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
};

class CPUIDCountStruct
{
public:
    CPUIDCountStruct(uint32_t cpuid_type, uint32_t count)
    {
        if (!CPUDetectClass::Has_CPUID_Instruction()) {
            eax = ebx = ecx = edx = 0;

            return;
        }

        CPUDetectClass::CPUID_Count(eax, ebx, ecx, edx, cpuid_type, count);
    }

    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
};
