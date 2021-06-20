add_library(particleeditor SHARED)
target_link_libraries(particleeditor PRIVATE ${wxWidgets_LIBRARIES} base captnlog)

if(USE_GAMEMATH)
    target_link_libraries(particleeditor PRIVATE gamemath_static_lib)
    target_compile_definitions(particleeditor PRIVATE -DBUILD_WITH_GAMEMATH)
endif()

target_include_directories(particleeditor PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${wxWidgets_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/game
    ${CMAKE_SOURCE_DIR}/src/game/client
    ${CMAKE_SOURCE_DIR}/src/game/client/gui
    ${CMAKE_SOURCE_DIR}/src/game/client/input
    ${CMAKE_SOURCE_DIR}/src/game/client/shadow
    ${CMAKE_SOURCE_DIR}/src/game/client/shader
    ${CMAKE_SOURCE_DIR}/src/game/client/system
    ${CMAKE_SOURCE_DIR}/src/game/client/system/particlesystem
    ${CMAKE_SOURCE_DIR}/src/game/client/videoplayer
    ${CMAKE_SOURCE_DIR}/src/game/common
    ${CMAKE_SOURCE_DIR}/src/game/common/audio
    ${CMAKE_SOURCE_DIR}/src/game/common/compression
    ${CMAKE_SOURCE_DIR}/src/game/common/ini
    ${CMAKE_SOURCE_DIR}/src/game/common/modules
    ${CMAKE_SOURCE_DIR}/src/game/common/rts
    ${CMAKE_SOURCE_DIR}/src/game/common/system
    ${CMAKE_SOURCE_DIR}/src/game/common/thing
    ${CMAKE_SOURCE_DIR}/src/game/logic/ai
    ${CMAKE_SOURCE_DIR}/src/game/logic/map
    ${CMAKE_SOURCE_DIR}/src/game/logic/object
    ${CMAKE_SOURCE_DIR}/src/game/logic/scriptengine
    ${CMAKE_SOURCE_DIR}/src/game/logic/system
    ${CMAKE_SOURCE_DIR}/src/game/client/system/particlesystem
    ${CMAKE_SOURCE_DIR}/src/w3d/lib
    ${CMAKE_SOURCE_DIR}/src/w3d/math
    ${CMAKE_SOURCE_DIR}/src/w3d/renderer
    ${CMAKE_SOURCE_DIR}/src/w3d/saveload
)
target_compile_options(particleeditor PRIVATE ${wxWidgets_CXX_FLAGS})
target_compile_definitions(particleeditor PRIVATE ${wxWidgets_DEFINITIONS})

if (NOT STANDALONE)
    target_compile_definitions(particleeditor PRIVATE -DGAME_DLL)
endif()

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_compile_definitions(particleeditor PRIVATE
        -DWIN32_LEAN_AND_MEAN
        -D_CRT_SECURE_NO_DEPRECATE
        -D_CRT_NONSTDC_NO_DEPRECATE
        -D_WINSOCK_DEPRECATED_NO_WARNINGS
        -DwxUSE_RC_MANIFEST
    )

    
    include(ProductVersion)

    generate_product_version(PARTICLEED_RC
        NAME "Particle Editor Plugin DLL"
        BUNDLE "Thyme"
        VERSION_MAJOR 1
        VERSION_MINOR 0
        COMPANY_NAME "Assembly Armada"
        COMPANY_COPYRIGHT "Built from code licensed under GPLv2 or later."
        ORIGINAL_FILENAME "ParticleEditor.dll"
        RCFILE_PREFIX "ParticleEditor"
    )
endif()

if(NOT wxWidgets_wxrc_EXECUTABLE)
    message(FATAL_ERROR "wxrc resource compiler was not found, please set wxWidgets_wxrc_EXECUTABLE manually.")
endif()

# Generate cpp/h files to embed xml resources in the binary.
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/wxpartedui.cpp" "${CMAKE_CURRENT_BINARY_DIR}/wxpartedui.h"
    COMMAND ${wxWidgets_wxrc_EXECUTABLE} --cpp-code --extra-cpp-code --output=${CMAKE_CURRENT_BINARY_DIR}/wxpartedui.cpp ${CMAKE_CURRENT_SOURCE_DIR}/ui/partedui.xrc
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ui/partedui.xrc"
)

target_sources(particleeditor PRIVATE
    dllmain.cpp
    coloralpha.cpp
    coloralpha.h
    emissionpanels.cpp
    emissionpanels.h
    emitswitch.cpp
    emitswitch.h
    moreparams.cpp
    moreparams.h
    partedapp.cpp
    partedapp.h
    partedframe.cpp
    partedframe.h
    particleeditor.cpp
    particleeditor.h
    particlepanels.cpp
    particlepanels.h
    velocitypanels.cpp
    velocitypanels.h
    ${CMAKE_CURRENT_BINARY_DIR}/wxpartedui.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/wxpartedui.h
    ${PARTICLEED_RC}
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(particleeditor PRIVATE particleeditor_win32.cpp)
else()
    target_sources(particleeditor PRIVATE particleeditor_posix.cpp)
endif()

set_target_properties(particleeditor PROPERTIES OUTPUT_NAME ParticleEditor PREFIX "")

target_exports(particleeditor SYMBOLS
    HasRequestedKillAllSystems
    HasRequestedReload
    AppendParticleSystem
    AppendThingTemplate
    CreateParticleSystemDialog
    DestroyParticleSystemDialog
    GetNewParticleCap
    GetSelectedParticleAsciiStringParm
    GetSelectedParticleSystemName
    HasUpdatedSelectedParticleSystem
    NextParticleEditorBehavior
    RemoveAllParticleSystems
    RemoveAllThingTemplates
    ShouldBusyWait
    ShouldReloadTextures
    ShouldUpdateParticleCap
    ShouldWriteINI
    UpdateCurrentNumParticles
    UpdateCurrentParticleCap
    UpdateCurrentParticleSystem
    UpdateParticleAsciiStringParm
    UpdateSystemUseParameters
)
