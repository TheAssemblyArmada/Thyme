# FindDirectX
# -------
#
# Find the DirectX libraries and programs. Searches most recent SDK first.
#
# This module supports multiple components.
# Components can include any of: ``ddraw``, ``dsound``, ``d3d``,
# ``d3dx``, ``d3d8``, ``d3dx8``, ``d3d9``, ``d3dx9``, ``dxgi``,
# ``d3d10``, ``d3d10_1``, ``d3dx10``, ``d3d11``, ``d3d11_1``, ``d3d11_2``,
# ``d3dx11``, ``d2d1``
#
# This module reports information about the DirectX installation in
# several variables.  General variables::
#
#   DirectX_FOUND - true if the main programs and libraries were found
#   DirectX_LIBRARIES - component libraries to be linked
#   DirectX_INCLUDE_DIRS - the directories containing the DirectX headers
#
# Imported targets::
#
#   DirectX::<C>
#
# Where ``<C>`` is the name of an DirectX component, for example
# ``DirectX::d3d9``.
#
# DirectX component libraries are reported in::
#
#   <C>_FOUND - ON if component was found
#   <C>_LIBRARIES - libraries for component
#
# Note that ``<C>`` is the uppercased name of the component.
include(CheckIncludeFileCXX)
include(FindPackageMessage)

function(_DirectX_FIND)
    if(MSVC)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(_dx_lib_suffixes lib/x64 lib)
        else()
            set(_dx_lib_suffixes lib/x86 lib)
        endif()

        # Can't use "$ENV{ProgramFiles(x86)}" to avoid violating CMP0053.  See
        # http://public.kitware.com/pipermail/cmake-developers/2014-October/023190.html
        set (ProgramFiles_x86 "ProgramFiles(x86)")
        if("$ENV{${ProgramFiles_x86}}")
            set(ProgramFiles "$ENV{${ProgramFiles_x86}}")
        else()
            set(ProgramFiles "$ENV{ProgramFiles}")
        endif()
        
        set(DirectX_SDK_PATHS)
        
        set(_dx_quiet)
        if(DirectX_FIND_QUIETLY)
            set(_dx_quiet QUIET)
        endif()
        
        find_package(WindowsSDK ${_dx_quiet})
        
        if(WINDOWSSDK_FOUND)
            foreach(_dir ${WINDOWSSDK_DIRS})
                get_windowssdk_include_dirs(${_dir} _include_dirs)
                if(_include_dirs)
                    list(APPEND DirectX_SDK_PATHS ${_include_dirs})
                endif()
            endforeach()
        endif()
        
        macro(_append_dxsdk_in_inclusive_range _low _high)
            if((NOT MSVC_VERSION LESS ${_low}) AND (NOT MSVC_VERSION GREATER ${_high}))
                list(APPEND DirectX_SDK_PATHS ${ARGN})
            endif()
        endmacro()
        
        # Find this file as the older DXSDK paths being listed before this will causes errors.
        find_path(WINSDK_INCLUDES
            NAMES "rpcsal.h"
            PATHS ${DirectX_SDK_PATHS}
            PATH_SUFFIXES include directx
            NO_DEFAULT_PATH
        )
        
        message("Found rpcsal.h in ${WINSDK_INCLUDES}")
        
        _append_dxsdk_in_inclusive_range(1500 1600 "${_PROG_FILES}/Microsoft DirectX SDK (June 2010)")
        _append_dxsdk_in_inclusive_range(1400 1600
            "${_PROG_FILES}/Microsoft DirectX SDK (February 2010)"
            "${_PROG_FILES}/Microsoft DirectX SDK (August 2009)"
            "${_PROG_FILES}/Microsoft DirectX SDK (March 2009)"
            "${_PROG_FILES}/Microsoft DirectX SDK (November 2008)"
            "${_PROG_FILES}/Microsoft DirectX SDK (August 2008)"
            "${_PROG_FILES}/Microsoft DirectX SDK (June 2008)"
            "${_PROG_FILES}/Microsoft DirectX SDK (March 2008)")
        _append_dxsdk_in_inclusive_range(1310 1500
            "${_PROG_FILES}/Microsoft DirectX SDK (November 2007)"
            "${_PROG_FILES}/Microsoft DirectX SDK (August 2007)"
            "${_PROG_FILES}/Microsoft DirectX SDK (June 2007)"
            "${_PROG_FILES}/Microsoft DirectX SDK (April 2007)"
            "${_PROG_FILES}/Microsoft DirectX SDK (February 2007)"
            "${_PROG_FILES}/Microsoft DirectX SDK (December 2006)"
            "${_PROG_FILES}/Microsoft DirectX SDK (October 2006)"
            "${_PROG_FILES}/Microsoft DirectX SDK (August 2006)"
            "${_PROG_FILES}/Microsoft DirectX SDK (June 2006)"
            "${_PROG_FILES}/Microsoft DirectX SDK (April 2006)"
            "${_PROG_FILES}/Microsoft DirectX SDK (February 2006)")
        
        if(DirectX_ROOT)
            list(APPEND DirectX_SDK_PATHS "${DirectX_ROOT}")
        endif()
        
        file(TO_CMAKE_PATH "$ENV{DXSDK_DIR}" ENV_DXSDK_DIR)
        if(ENV_DXSDK_DIR)
            list(APPEND DirectX_SDK_PATHS ${ENV_DXSDK_DIR})
        endif()
    elseif(WATCOM)
        set(_dx_lib_suffixes directx)
        set(DirectX_SDK_PATHS "$ENV{WATCOM}/lib386/nt" "$ENV{WATCOM}/h/nt/")
    else()
        set(CMAKE_FIND_LIBRARY_PREFIXES "lib" "")
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll.a" ".a" ".lib")
        set(_dx_lib_suffixes lib)
        set(DirectX_SDK_PATHS "$ENV{MINGW_PREFIX}" "$ENV{MINGW_PREFIX}/$ENV{MINGW_CHOST}")
    endif()
    
    set(DirectX_REQUIRED_LIBS_FOUND ON)   
    
    if(WINSDK_INCLUDES)
        list(APPEND DirectX_INCLUDE_DIR "${WINSDK_INCLUDES}")
    endif()
    
    foreach(component ${DirectX_FIND_COMPONENTS})
        string(TOUPPER "${component}" component_upcase)
        set(component_cache "DirectX_${component_upcase}_LIBRARY")
        set(component_include "DirectX_${component_upcase}_INCLUDE_DIR")
        set(component_cache_release "${component_cache}_RELEASE")
        set(component_cache_debug "${component_cache}_DEBUG")
        set(component_found "${component_upcase}_FOUND")

        # Search default locations
        find_path (${component_include}
            NAMES "${component}.h"
            PATHS ${DirectX_SDK_PATHS}
            PATH_SUFFIXES include directx
            DOC "DirectX ${component} include directory"
        )
        
        if(${component_include})
            list(APPEND DirectX_INCLUDE_DIR "${${component_include}}")
        endif()

        find_library("${component_cache_release}"
            NAMES ${component}
            PATHS ${DirectX_SDK_PATHS}
            PATH_SUFFIXES ${_dx_lib_suffixes}
            DOC "DirectX ${component} library"
        )

        include(SelectLibraryConfigurations)
        select_library_configurations(DirectX_${component_upcase})
        mark_as_advanced("${component_cache_release}")
        
        if(${component_cache})
            set("${component_found}" ON)
            list(APPEND DirectX_LIBRARY "${${component_cache}}")
        endif()
        
        mark_as_advanced("${component_found}")
        set("${component_cache}" "${${component_cache}}" PARENT_SCOPE)
        set("${component_found}" "${${component_found}}" PARENT_SCOPE)
        
        if(${component_found})
            if (DirectX_FIND_REQUIRED_${component})
                list(APPEND DirectX_LIBS_FOUND "${component} (required)")
            else()
                list(APPEND DirectX_LIBS_FOUND "${component} (optional)")
            endif()
        else()
            if (DirectX_FIND_REQUIRED_${component})
                set(DirectX_REQUIRED_LIBS_FOUND OFF)
                list(APPEND DirectX_LIBS_NOTFOUND "${component} (required)")
            else()
                list(APPEND DirectX_LIBS_NOTFOUND "${component} (optional)")
            endif()
        endif()
    endforeach()
    
    set(_DirectX_REQUIRED_LIBS_FOUND "${DirectX_REQUIRED_LIBS_FOUND}" PARENT_SCOPE)
    set(DirectX_LIBRARY "${DirectX_LIBRARY}" PARENT_SCOPE)
    if(DirectX_INCLUDE_DIR)
        list(REMOVE_DUPLICATES DirectX_INCLUDE_DIR)
    endif()
    set(DirectX_INCLUDE_DIR "${DirectX_INCLUDE_DIR}" PARENT_SCOPE)
    
    find_program (DirectX_FXC_EXECUTABLE fxc
        PATHS ${DirectX_SDK_PATHS}
        PATH_SUFFIXES Utilities/bin/x86
        DOC "Path to fxc.exe executable."
    )
    
    if(NOT DirectX_FIND_QUIETLY)
        if(DirectX_INCLUDE_DIR)
            message(STATUS "Found the following DirectX include directories:")
            foreach(found ${DirectX_INCLUDE_DIR})
                message(STATUS "  ${found}")
            endforeach()
        endif()
        
        if(DirectX_LIBS_FOUND)
            message(STATUS "Found the following DirectX libraries:")
            foreach(found ${DirectX_LIBS_FOUND})
                message(STATUS "  ${found}")
            endforeach()
        endif()
        
        if(DirectX_LIBS_NOTFOUND)
            message(STATUS "The following DirectX libraries were not found:")
            foreach(notfound ${DirectX_LIBS_NOTFOUND})
                message(STATUS "  ${notfound}")
            endforeach()
        endif()
    endif()
endfunction()

if(WIN32)
    _DirectX_FIND()

    include(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(DirectX
                                  FOUND_VAR DirectX_FOUND
                                  REQUIRED_VARS DirectX_INCLUDE_DIR
                                                DirectX_LIBRARY
                                                _DirectX_REQUIRED_LIBS_FOUND
                                  FAIL_MESSAGE "Failed to find all DirectX components")

    unset(_DirectX_REQUIRED_LIBS_FOUND)
    
    if(DirectX_FOUND)
        set(DirectX_INCLUDE_DIRS "${DirectX_INCLUDE_DIR}")
        set(DirectX_LIBRARIES "${DirectX_LIBRARY}")
  
        foreach(_DirectX_component ${DirectX_FIND_COMPONENTS})
            string(TOUPPER "${_DirectX_component}" _DirectX_component_upcase)
            set(_DirectX_component_cache "DirectX_${_DirectX_component_upcase}_LIBRARY")
            set(_DirectX_component_cache_release "DirectX_${_DirectX_component_upcase}_LIBRARY_RELEASE")
            set(_DirectX_component_cache_debug "DirectX_${_DirectX_component_upcase}_LIBRARY_DEBUG")
            set(_DirectX_component_lib "DirectX_${_DirectX_component_upcase}_LIBRARIES")
            set(_DirectX_component_found "${_DirectX_component_upcase}_FOUND")
            set(_DirectX_imported_target "DirectX::${_DirectX_component}")
        
            if(${_DirectX_component_found})
                set("${_DirectX_component_lib}" "${${_DirectX_component_cache}}")
          
                if(NOT TARGET ${_DirectX_imported_target})
                    add_library(${_DirectX_imported_target} UNKNOWN IMPORTED)
                
                    if(DirectX_INCLUDE_DIR)
                        set_target_properties(${_DirectX_imported_target} PROPERTIES
                            INTERFACE_INCLUDE_DIRECTORIES "${DirectX_INCLUDE_DIR}")
                    endif()
                
                    if(EXISTS "${${_DirectX_component_cache}}")
                        set_target_properties(${_DirectX_imported_target} PROPERTIES
                            IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                            IMPORTED_LOCATION "${${_DirectX_component_cache}}")
                    endif()
                
                    if(EXISTS "${${_DirectX_component_cache_release}}")
                        set_property(TARGET ${_DirectX_imported_target} APPEND PROPERTY
                            IMPORTED_CONFIGURATIONS RELEASE)
                        set_target_properties(${_DirectX_imported_target} PROPERTIES
                            IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
                            IMPORTED_LOCATION_RELEASE "${${_DirectX_component_cache_release}}")
                    endif()
            
                    if(EXISTS "${${_DirectX_component_cache_debug}}")
                        set_property(TARGET ${_DirectX_imported_target} APPEND PROPERTY
                            IMPORTED_CONFIGURATIONS DEBUG)
                        set_target_properties(${_DirectX_imported_target} PROPERTIES
                            IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
                            IMPORTED_LOCATION_DEBUG "${${_DirectX_component_cache_debug}}")
                    endif()
                endif()
            endif()
        
            unset(_DirectX_component_upcase)
            unset(_DirectX_component_cache)
            unset(_DirectX_component_lib)
            unset(_DirectX_component_found)
            unset(_DirectX_imported_target)
        endforeach()
    endif()
endif ()
