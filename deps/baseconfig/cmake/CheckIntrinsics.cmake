include(CheckCSourceCompiles)
include(CheckSymbolExists)
include(CheckIncludeFile)

check_include_file(intrin.h HAVE_INTRIN_H)
check_include_file(x86intrin.h HAVE_X86INTRIN_H)
check_include_file(cpuid.h HAVE_CPUID_H)

# check_symbol_exists doesn't work for some intrinsics, have to use check_cxx_source_compiles instead.
if(HAVE_X86INTRIN_H)
    check_c_source_compiles("
        #include <x86intrin.h>
        int main() {
            return __rdtsc();
        }" HAVE__RDTSC_GCC)
        set(HAVE__RDTSC ${HAVE__RDTSC_GCC})

    
    check_c_source_compiles("
        #include <x86intrin.h>
        int main() {
            return __readeflags();
        }" HAVE__READEFLAGS_GCC)
        set(HAVE__READEFLAGS ${HAVE__READEFLAGS_GCC})
    
    check_c_source_compiles("
        #include <x86intrin.h>
        int main() {
            unsigned tmp = 0x00200000;
            __writeeflags(tmp);      
            return 0;
        }" HAVE__WRITEEFLAGS_GCC)
    set(HAVE__WRITEEFLAGS ${HAVE__WRITEEFLAGS_GCC})
    
    check_c_source_compiles("
        #include <x86intrin.h>
        #include <stdint.h>
        int main(int argc, char **argv) {
        #ifdef __x86_64__
            uint64_t temp1 = __rorq(argc, 4);
        #else 
            uint64_t temp1 = 0;
        #endif
            uint32_t temp2 = __rold(temp1, 4);
            uint16_t temp3 = __rorw(temp2, 4);
            uint8_t temp4 = __rolb(temp3, 4);
            return temp1 + temp2 + temp3 + temp4;
        }" HAVE_GCC_ROTATE)
endif()

if(HAVE_CPUID_H)
    check_symbol_exists(__cpuid "cpuid.h" HAVE__CPUID_GCC)
    check_symbol_exists(__cpuid_count "cpuid.h" HAVE__CPUID_COUNT)
endif()
    
if(HAVE_INTRIN_H)
    if(NOT HAVE__RDTSC)
        check_c_source_compiles("
        #include <intrin.h>
        int main() {
            return __rdtsc();
        }" HAVE__RDTSC_MSVC)
        set(HAVE__RDTSC ${HAVE__RDTSC_MSVC})
    endif()

        
    if(NOT HAVE__READEFLAGS)
        check_c_source_compiles("
            #include <intrin.h>
            int main() {
                return __readeflags();
            }" HAVE__READEFLAGS_MSVC)
        set(HAVE__READEFLAGS ${HAVE__READEFLAGS_MSVC})
    endif()
        
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

    if(NOT HAVE__CPUID_GCC)
        check_c_source_compiles("
            #include <intrin.h>
            int main() {
                int cpuid_type = 0;
                int regs[4];
                __cpuid(regs, cpuid_type);
            }" HAVE__CPUID_MSVC)
    endif()

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
    
    if(NOT HAVE_GCC_ROTATE)
        check_c_source_compiles("
            #include <intrin.h>
            #include <stdint.h>
            #include <stdlib.h>
            int main(int argc, char **argv) {
                uint64_t temp1 = _rotr64(argc, 4);
                uint32_t temp2 = _rotl(temp1, 4);
                uint16_t temp3 = _rotr16(temp2, 4);
                uint8_t temp4 = _rotl8(temp3, 4);
                return temp1 + temp2 + temp3 + temp4;
            }" HAVE_MSVC_ROTATE)
    endif()
    
    check_c_source_compiles("
        #include <intrin.h>
        int main() {
        __debugbreak();
            return 0;
        }" HAVE__DEBUGBREAK)
endif()
