# Build and link the launcher.
include(CMakeParseArguments)
set(GenerateSetsailLauncherCurrentDir ${CMAKE_CURRENT_LIST_DIR})

function(generate_setsail_launcher outfiles)
    set (options)
    set (oneValueArgs
        PREFIX
        ENTRYPOINT
        DLLNAME
        EXENAME
        HASH
        LAUNCHER)
    set (multiValueArgs
        EXTRA_SOURCES)
    cmake_parse_arguments(SETSAIL "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    if(NOT SETSAIL_PREFIX)
        set(SETSAIL_PREFIX setsail)
    endif()

    if(NOT SETSAIL_ENTRYPOINT)
        set(SETSAIL_ENTRYPOINT 0)
    endif()

    if(NOT SETSAIL_DLLNAME)
        set(SETSAIL_DLLNAME "${SETSAIL_PREFIX}.dll")
    endif()

    if(NOT SETSAIL_EXENAME)
        set(SETSAIL_EXENAME "${SETSAIL_PREFIX}.exe")
    endif()

     if(NOT SETSAIL_HASH)
        set(SETSAIL_HASH "deadbeef")
    endif()

    if(NOT SETSAIL_LAUNCHER)
        set(SETSAIL_LAUNCHER ${SETSAIL_PREFIX}launcher)
    endif()

    set(_LauncherSourceFile ${CMAKE_CURRENT_BINARY_DIR}/${SETSAIL_PREFIX}launcher.cpp)
    message("GenerateSetsailLauncherCurrentDir is ${GenerateSetsailLauncherCurrentDir}")
    configure_file(${GenerateSetsailLauncherCurrentDir}/SetSail.in ${_LauncherSourceFile} @ONLY)

    add_executable(${SETSAIL_LAUNCHER} ${_LauncherSourceFile} ${SETSAIL_EXTRA_SOURCES})
    target_link_options(${SETSAIL_LAUNCHER} PRIVATE /subsystem:windows /ENTRY:WinMainCRTStartup)
    target_compile_definitions(${SETSAIL_LAUNCHER} PRIVATE _CRT_SECURE_NO_WARNINGS)
    message("Configuring a launcher as ${SETSAIL_LAUNCHER} using entry at ${SETSAIL_ENTRYPOINT} to inject ${SETSAIL_DLLNAME} into ${SETSAIL_EXENAME}")
    set (${outfiles} ${${SETSAIL_LAUNCHER}} PARENT_SCOPE)
endfunction()
