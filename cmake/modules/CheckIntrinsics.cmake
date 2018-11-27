include(CheckCSourceCompiles)
include(CheckSymbolExists)

# check_symbol_exists doesn't work for some intrinsics, have to use check_cxx_source_compiles instead.
check_c_source_compiles("
    #include <x86intrin.h>
    int main() {
        return __rdtsc();
    }" HAVE__RDTSC_GCC)
    set(HAVE__RDTSC ${HAVE__RDTSC_GCC})

if(NOT HAVE__RDTSC)
    check_c_source_compiles("
    #include <intrin.h>
    int main() {
        return __rdtsc();
    }" HAVE__RDTSC_MSVC)
    set(HAVE__RDTSC ${HAVE__RDTSC_MSVC})
endif()

check_c_source_compiles("
    #include <x86intrin.h>
    int main() {
        return __readeflags();
    }" HAVE__READEFLAGS_GCC)
    set(HAVE__READEFLAGS ${HAVE__READEFLAGS_GCC})
    
if(NOT HAVE__READEFLAGS)
    check_c_source_compiles("
        #include <intrin.h>
        int main() {
            return __readeflags();
        }" HAVE__READEFLAGS_MSVC)
    set(HAVE__READEFLAGS ${HAVE__READEFLAGS_MSVC})
endif()

check_c_source_compiles("
        #include <x86intrin.h>
        int main() {
            unsigned tmp = 0x00200000;
            __writeeflags(tmp);      
            return 0;
        }" HAVE__WRITEEFLAGS_GCC)
    set(HAVE__WRITEEFLAGS ${HAVE__WRITEEFLAGS_GCC})
    
if(NOT HAVE__WRITEEFLAGS)
    check_c_source_compiles("
        #include <intrin.h>
        int main() {
            unsigned tmp = 0x00200000;
            __writeeflags(tmp);      
            return 0;
        }" HAVE__WRITEEFLAGS_MSVC)
    set(HAVE__WRITEEFLAGS ${HAVE__WRITEEFLAGS_MSVC})
endif()

check_symbol_exists(__cpuid "cpuid.h" HAVE__CPUID_GCC)
if(NOT HAVE__CPUID_GCC)
    check_c_source_compiles("
        #include <intrin.h>
        int main() {
            int cpuid_type = 0;
            int regs[4];
            __cpuid(regs, cpuid_type);
        }" HAVE__CPUID_MSVC)
endif()

check_symbol_exists(__cpuid_count "cpuid.h" HAVE__CPUID_COUNT)
if(NOT HAVE__CPUID_GCC)
    check_c_source_compiles("
        #include <intrin.h>
        int main() {
            int count = 1;
            int cpuid_type = 0;
            int regs[4];
            __cpuidex(regs, cpuid_type, count);
        }" HAVE__CPUIDEX)
endif()

check_c_source_compiles("
    #include <intrin.h>
    int main() {
    __debugbreak();
        return 0;
    }" HAVE__DEBUGBREAK)
