# Add platform libraries needed for the build
if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    find_package(DirectX)
    
    if(DirectX_D3D8_INCLUDE_FOUND)
        message("Located Direct3D8 headers.")
        set(PLATFORM_INCLUDES ${PLATFORM_INCLUDES} ${DirectX_D3D8_INCLUDE_DIR})
    endif()
    
    set(PLATFORM_LIBS ${PLATFORM_LIBS} winmm dbghelp ws2_32)
else()
    #No other platforms yet.
endif()

# Build and link the DLL.
set(GAMEENGINE_INCLUDES
    base
    game
    game/client
    game/client/gui
    game/client/input
    game/common
    game/common/audio
    game/common/compression
    game/common/ini
    game/common/modules
    game/common/system
    game/common/rts
    game/logic/object
    game/logic/scriptengine
    game/logic/system
    game/network
    platform
    w3d/lib
    w3d/math
    w3d/renderer
)

set(HOOKER_SRC hooker/dllmain.cpp hooker/hooker.cpp)

set(GAMEENGINE_SRC
    game/copyprotect.cpp
    game/force_nocd.cpp
    game/main.cpp
    game/maputil.cpp
    game/weather.cpp
    game/client/displaystring.cpp
    game/client/displaystringmanager.cpp
    game/client/gametext.cpp
    game/client/globallanguage.cpp
    game/client/optionpreferences.cpp
    game/client/terrainroads.cpp
    game/client/water.cpp
    game/client/gui/gamefont.cpp
    game/client/input/keyboard.cpp
    game/client/input/mouse.cpp
    game/common/bitflags.cpp
    game/common/commandline.cpp
    game/common/commandlist.cpp
    game/common/crc.cpp
    game/common/dict.cpp
    game/common/gameengine.cpp
    game/common/gamelod.cpp
    game/common/gamemain.cpp
    game/common/gamemessage.cpp
    game/common/gamemessagelist.cpp
    game/common/globaldata.cpp
    game/common/namekeygenerator.cpp
    game/common/messagestream.cpp
    game/common/multiplayersettings.cpp
    game/common/randomvalue.cpp
    game/common/staticnamekey.cpp
    game/common/terraintypes.cpp
    game/common/userpreferences.cpp
    game/common/version.cpp
    game/common/audio/audioeventinfo.cpp
    game/common/audio/audioeventrts.cpp
    game/common/audio/audiomanager.cpp
    game/common/audio/audiomisc.cpp
    game/common/audio/audiosettings.cpp
    game/common/audio/musicmanager.cpp
    game/common/audio/soundmanager.cpp
    game/common/compression/compressionmanager.cpp
    game/common/compression/refpack.cpp
    game/common/ini/ini.cpp
    game/common/modules/modulefactory.cpp
    game/common/system/archivefile.cpp
    game/common/system/archivefilesystem.cpp
    game/common/system/asciistring.cpp
    game/common/system/cachedfileinputstream.cpp
    game/common/system/datachunk.cpp
    game/common/system/datachunktoc.cpp
    game/common/system/file.cpp
    game/common/system/filesystem.cpp
    game/common/system/functionlexicon.cpp
    game/common/system/gamedebug.cpp
    game/common/system/gamememory.cpp
    game/common/system/gamememoryinit.cpp
    game/common/system/gamestate.cpp
    game/common/system/gametype.cpp
    game/common/system/kindof.cpp
    game/common/system/localfile.cpp
    game/common/system/localfilesystem.cpp
    game/common/system/memblob.cpp
    game/common/system/memdynalloc.cpp
    game/common/system/mempool.cpp
    game/common/system/mempoolfact.cpp
    game/common/system/ramfile.cpp
    game/common/system/snapshot.cpp
    game/common/system/stackdump.cpp
    game/common/system/streamingarchivefile.cpp
    game/common/system/subsysteminterface.cpp
    game/common/system/unicodestring.cpp
    game/common/system/xfer.cpp
    game/common/system/xfercrc.cpp
    game/common/rts/buildinfo.cpp
    game/common/rts/handicap.cpp
    game/common/rts/money.cpp
    game/common/rts/player.cpp
    game/common/rts/playerlist.cpp
    game/common/rts/science.cpp
    game/common/rts/sidesinfo.cpp
    game/common/rts/sideslist.cpp
    game/common/rts/teamsinfo.cpp
    game/logic/object/weapon.cpp
    game/logic/scriptengine/script.cpp
    game/logic/scriptengine/scriptaction.cpp
    game/logic/scriptengine/scriptcondition.cpp
    game/logic/scriptengine/scriptgroup.cpp
    game/logic/scriptengine/scriptlist.cpp
    game/logic/scriptengine/scriptparam.cpp
    game/logic/system/cavesystem.cpp
    game/network/filetransfer.cpp
    platform/w3dfilesystem.cpp
    platform/w3dfunctionlexicon.cpp
    platform/win32bigfile.cpp
    platform/win32bigfilesystem.cpp
    platform/win32gameengine.cpp
    platform/win32localfile.cpp
    platform/win32localfilesystem.cpp
    w3d/lib/chunkio.cpp
    w3d/lib/cpudetect.cpp
    w3d/lib/critsection.cpp
    w3d/lib/ffactory.cpp
    w3d/lib/fileclass.cpp
    w3d/lib/bufffileclass.cpp
    w3d/lib/rawfileclass.cpp
    w3d/lib/systimer.cpp
    w3d/lib/targa.cpp
    w3d/lib/thread.cpp
    w3d/lib/wwstring.cpp
    w3d/math/gamemath.cpp
    w3d/math/matrix3.cpp
    w3d/math/matrix3d.cpp
    w3d/math/matrix4.cpp
    w3d/math/quaternion.cpp
    w3d/renderer/dx8wrapper.cpp
)

add_library(thyme SHARED ${HOOKER_SRC} ${GAMEENGINE_SRC})

if(MSVC AND NOT STANDALONE) 
    target_include_directories(thyme BEFORE PUBLIC libs/stlport ${PLATFORM_INCLUDES} hooker ${GAMEENGINE_INCLUDES})
else()
    target_include_directories(thyme BEFORE PUBLIC ${PLATFORM_INCLUDES} hooker ${GAMEENGINE_INCLUDES})
endif()

target_link_libraries(thyme ${PLATFORM_LIBS})