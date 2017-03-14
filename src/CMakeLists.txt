# Add platform libraries needed for the build
if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    set(PLATFORM_LIBS ${PLATFORM_LIBS} winmm)
else()
    #No other platforms yet.
endif()

# Build and link the DLL.
set(GAMEENGINE_INCLUDES
    base
    game
    game/client
    game/common
    game/common/ini
    game/common/system
    platform
    w3d/lib
)

set(HOOKER_SRC hooker/dllmain.cpp hooker/hooker.cpp)

set(GAMEENGINE_SRC
    game/copyprotect.cpp
    game/force_nocd.cpp
    game/main.cpp
    game/client/gametext.cpp
    game/common/gameengine.cpp
    game/common/globaldata.cpp
    game/common/namekeygenerator.cpp
    game/common/version.cpp
    game/common/ini/ini.cpp
    game/common/system/archivefile.cpp
    game/common/system/archivefilesystem.cpp
    game/common/system/asciistring.cpp
    game/common/system/file.cpp
    game/common/system/filesystem.cpp
    game/common/system/gamedebug.cpp
    game/common/system/gamememory.cpp
    game/common/system/gamememoryinit.cpp
    game/common/system/localfile.cpp
    game/common/system/localfilesystem.cpp
    game/common/system/memblob.cpp
    game/common/system/memdynalloc.cpp
    game/common/system/mempool.cpp
    game/common/system/mempoolfact.cpp
    game/common/system/ramfile.cpp
    game/common/system/streamingarchivefile.cpp
    game/common/system/subsysteminterface.cpp
    game/common/system/unicodestring.cpp
    game/common/system/xfer.cpp
    platform/win32bigfile.cpp
    platform/win32bigfilesystem.cpp
    platform/win32gameengine.cpp
    platform/win32localfile.cpp
    platform/win32localfilesystem.cpp
    w3d/lib/critsection.cpp
)

add_library(thyme SHARED ${HOOKER_SRC} ${GAMEENGINE_SRC})
target_include_directories(thyme BEFORE PUBLIC libs/stlport hooker ${GAMEENGINE_INCLUDES})

target_link_libraries(thyme ${PLATFORM_LIBS})