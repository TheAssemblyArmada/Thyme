# Build and link the DLL.
set(GAMEENGINE_INCLUDES
    editor
    game
    game/client
    game/client/gui
    game/client/input
    game/client/shadow
    game/client/system
    game/client/system/particlesystem
    game/client/videoplayer
    game/common
    game/common/audio
    game/common/compression
    game/common/ini
    game/common/modules
    game/common/rts
    game/common/system
    game/common/thing
    game/logic/map
    game/logic/object
    game/logic/scriptengine
    game/logic/system
    game/network
    platform
    platform/audio/
    platform/directx/
    platform/input/
    platform/video/
    platform/w3dengine/client
    platform/w3dengine/common/thing
    w3d/lib
    w3d/math
    w3d/renderer
    w3d/saveload
)

set(HOOKER_SRC
    hooker/dllmain.cpp
    hooker/hooker.cpp
    hooker/setupglobals_zh.cpp
)

set(HOOKER_WB_SRC
    hooker/setuphooks_wb.cpp
)

set(HOOKER_GAME_SRC
    hooker/setuphooks_zh.cpp
)

set(GAMEENGINE_SRC
    game/copyprotect.cpp
    game/crashwrapper.cpp
    game/force_nocd.cpp
    game/maputil.cpp
    game/weather.cpp
    game/client/baseheightmap.cpp
    game/client/crashpreferences.cpp
    game/client/display.cpp
    game/client/displaystring.cpp
    game/client/displaystringmanager.cpp
    game/client/drawgroupinfo.cpp
    game/client/gameclient.cpp
    game/client/gametext.cpp
    game/client/globallanguage.cpp
    game/client/optionpreferences.cpp
    game/client/shadermanager.cpp
    game/client/terrainroads.cpp
    game/client/terraintex.cpp
    game/client/tiledata.cpp
    game/client/view.cpp
    game/client/water.cpp
    game/client/worldheightmap.cpp
    game/client/gui/gamefont.cpp
    game/client/gui/wininstancedata.cpp
    game/client/image.cpp
    game/client/input/keyboard.cpp
    game/client/input/mouse.cpp
    game/client/shadow/w3dshadow.cpp
    game/client/system/debugdisplay.cpp
    game/client/system/rayeffect.cpp
    game/client/system/particlesystem/particle.cpp
    game/client/system/particlesystem/particleinfo.cpp
    game/client/system/particlesystem/particlesys.cpp
    game/client/system/particlesystem/particlesysinfo.cpp
    game/client/system/particlesystem/particlesysmanager.cpp
    game/client/system/particlesystem/particlesystemplate.cpp
    game/client/videoplayer/videobuffer.cpp
    game/client/videoplayer/videoplayer.cpp
    game/client/videoplayer/videostream.cpp
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
    game/common/mapobject.cpp
    game/common/messagestream.cpp
    game/common/multiplayersettings.cpp
    game/common/randomvalue.cpp
    game/common/registry.cpp
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
    game/common/ini/inidrawgroupinfo.cpp
    game/common/modules/modulefactory.cpp
    game/common/rts/buildinfo.cpp
    game/common/rts/handicap.cpp
    game/common/rts/money.cpp
    game/common/rts/player.cpp
    game/common/rts/playerlist.cpp
    game/common/rts/playertemplate.cpp
    game/common/rts/productionprerequisite.cpp
    game/common/rts/science.cpp
    game/common/rts/sidesinfo.cpp
    game/common/rts/sideslist.cpp
    game/common/rts/teamsinfo.cpp
    game/common/system/archivefile.cpp
    game/common/system/archivefilesystem.cpp
    game/common/system/asciistring.cpp
    game/common/system/cachedfileinputstream.cpp
    game/common/system/datachunk.cpp
    game/common/system/datachunktoc.cpp
    game/common/system/file.cpp
    game/common/system/filesystem.cpp
    game/common/system/functionlexicon.cpp
    game/common/system/gamememory.cpp
    game/common/system/gamememoryinit.cpp
    game/common/system/gamestate.cpp
    game/common/system/gametype.cpp
    game/common/system/geometry.cpp
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
    game/common/thing/moduleinfo.cpp
    game/common/thing/thing.cpp
    game/common/thing/thingfactory.cpp
    game/common/thing/thingtemplate.cpp
    game/logic/map/polygontrigger.cpp
    game/logic/object/armortemplateset.cpp
    game/logic/object/weapon.cpp
    game/logic/object/weapontemplateset.cpp
    game/logic/scriptengine/script.cpp
    game/logic/scriptengine/scriptaction.cpp
    game/logic/scriptengine/scriptcondition.cpp
    game/logic/scriptengine/scriptgroup.cpp
    game/logic/scriptengine/scriptlist.cpp
    game/logic/scriptengine/scriptparam.cpp
    game/logic/system/rankinfo.cpp
    game/logic/system/cavesystem.cpp
    game/network/filetransfer.cpp
    game/network/gamemessageparser.cpp
    game/network/lanapi.cpp
    game/network/lanapiinterface.cpp
    game/network/transport.cpp
    game/network/udp.cpp
    platform/fpusetting.cpp
    platform/input/win32mouse.cpp
    platform/w3dfilesystem.cpp
    platform/w3dfunctionlexicon.cpp
    platform/win32bigfile.cpp
    platform/win32bigfilesystem.cpp
    platform/win32localfile.cpp
    platform/win32localfilesystem.cpp
    platform/w3dengine/client/w3ddebugdisplay.cpp
    platform/w3dengine/client/w3ddisplay.cpp
    platform/w3dengine/client/w3dpoly.cpp
    platform/w3dengine/client/w3droadbuffer.cpp
    platform/w3dengine/client/w3dmouse.cpp
    platform/w3dengine/common/thing/w3dmodulefactory.cpp
    w3d/lib/buff.cpp
    w3d/lib/bufffileclass.cpp
    w3d/lib/chunkio.cpp
    w3d/lib/cpudetect.cpp
    w3d/lib/critsection.cpp
    w3d/lib/ffactory.cpp
    w3d/lib/fileclass.cpp
    w3d/lib/filestraw.cpp
    w3d/lib/iniclass.cpp
    w3d/lib/multilist.cpp
    w3d/lib/nstrdup.cpp
    w3d/lib/mutex.cpp
    w3d/lib/random.cpp
    w3d/lib/rawfileclass.cpp
    w3d/lib/readline.cpp
    w3d/lib/refcount.cpp
    w3d/lib/straw.cpp
    w3d/lib/systimer.cpp
    w3d/lib/targa.cpp
    w3d/lib/thread.cpp
    w3d/lib/threadtrack.cpp
    w3d/lib/wwstring.cpp
    w3d/lib/hash.cpp
    w3d/lib/xstraw.cpp
    w3d/math/aabox.cpp
    w3d/math/colmath.cpp
    w3d/math/colmathaaplane.cpp
    w3d/math/colmathfrustum.cpp
    w3d/math/colmathplane.cpp
    w3d/math/frustum.cpp
    w3d/math/gamemath.cpp
    w3d/math/lineseg.cpp
    w3d/math/matrix3.cpp
    w3d/math/matrix3d.cpp
    w3d/math/matrix4.cpp
    w3d/math/quaternion.cpp
    w3d/math/tri.cpp
    w3d/math/v3_rnd.cpp
    w3d/math/vp.cpp
    w3d/renderer/assetmgr.cpp
    w3d/renderer/bitmaphandler.cpp
    w3d/renderer/boxrobj.cpp
    w3d/renderer/camera.cpp
    w3d/renderer/coltest.cpp
    w3d/renderer/colorspace.cpp
    w3d/renderer/ddsfile.cpp
    w3d/renderer/dx8caps.cpp
    w3d/renderer/dx8fvf.cpp
    w3d/renderer/dx8indexbuffer.cpp
    w3d/renderer/dx8polygonrenderer.cpp
    w3d/renderer/dx8renderer.cpp
    w3d/renderer/dx8texman.cpp
    w3d/renderer/dx8vertexbuffer.cpp
    w3d/renderer/dx8wrapper.cpp
    w3d/renderer/hanimmgr.cpp
    w3d/renderer/htreemgr.cpp
    w3d/renderer/light.cpp
    w3d/renderer/lightenv.cpp
    w3d/renderer/mapper.cpp
    w3d/renderer/mesh.cpp
    w3d/renderer/meshgeometry.cpp
    w3d/renderer/meshmatdesc.cpp
    w3d/renderer/meshmdl.cpp
    w3d/renderer/missing.cpp
    w3d/renderer/matpass.cpp
    w3d/renderer/pointgr.cpp
    w3d/renderer/pot.cpp
    w3d/renderer/predlod.cpp
    w3d/renderer/render2d.cpp
    w3d/renderer/render2dsentence.cpp
    w3d/renderer/rendobj.cpp
    w3d/renderer/rinfo.cpp
    w3d/renderer/seglinerenderer.cpp
    w3d/renderer/shader.cpp
    w3d/renderer/sortingrenderer.cpp
    w3d/renderer/surfaceclass.cpp
    w3d/renderer/synctextureloadtasklist.cpp
    w3d/renderer/texture.cpp
    w3d/renderer/texturebase.cpp
    w3d/renderer/textureloader.cpp
    w3d/renderer/textureloadtask.cpp
    w3d/renderer/textureloadtasklist.cpp
    w3d/renderer/thumbnail.cpp
    w3d/renderer/thumbnailmanager.cpp
    w3d/renderer/vertmaterial.cpp
    w3d/renderer/w3d.cpp
    w3d/renderer/w3dformat.cpp
)

# Game binary only source
set(GAMEENGINE_GAME_SRC
    game/main.cpp
    platform/win32gameengine.cpp
)

set(GAMEENGINE_WB_SRC
    editor/editmain.cpp
)

# Don't try and build the miles sound system when not building to link to the original game.
if(NOT STANDALONE AND NOT OPENAL_FOUND AND NOT FFMPEG_FOUND)
    list(APPEND GAMEENGINE_SRC 
    platform/audio/milesaudiofilecache.cpp
    platform/audio/milesaudiomanager.cpp
    platform/video/binkvideoplayer.cpp
    platform/video/binkvideostream.cpp)
endif()

if(DINPUT8_FOUND)
    list(APPEND GAMEENGINE_GAME_SRC
        platform/directx/dinputkeybd.cpp
    )
endif()

if(CMAKE_CONFIGURATION_TYPES)
    # Glob all the header files together to add to the project for msvc/xcode.
    # Not ideal as CMake won't notice if you add any until something else prompts a CMake run
    # but this is largely a convenience feature for MSVC/XCode users.
    file(GLOB_RECURSE GAMEENGINE_HEADERS "*.h")
    set(_GAMEENGINE_SOURCES ${GAMEENGINE_SRC} ${GAMEENGINE_GAME_SRC} ${GAMEENGINE_WB_SRC})

    if(NOT STANDALONE)
        list(APPEND _GAMEENGINE_SOURCES ${HOOKER_SRC} ${HOOKER_GAME_SRC} ${HOOKER_WB_SRC})
    endif()

    # Filter out headers from included 3rd party libraries like STLPort or gamemath.
    list(FILTER GAMEENGINE_HEADERS EXCLUDE REGEX "libs/.*")
    list(FILTER GAMEENGINE_HEADERS EXCLUDE REGEX "base/.*")

    foreach(_GAMEENGINE_SOURCE IN ITEMS ${_GAMEENGINE_SOURCES})
        get_filename_component(_GAMEENGINE_SOURCE_PATH "${_GAMEENGINE_SOURCE}" PATH)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" _GAMEENGINE_GROUP_PATH "${_GAMEENGINE_SOURCE_PATH}")
        string(REPLACE "/" "\\" _GAMEENGINE_GROUP_PATH "${_GAMEENGINE_GROUP_PATH}")
        source_group("Source Files\\${_GAMEENGINE_GROUP_PATH}" FILES "${_GAMEENGINE_SOURCE}")
    endforeach()

    foreach(_GAMEENGINE_HEADER IN ITEMS ${GAMEENGINE_HEADERS})
        get_filename_component(_GAMEENGINE_HEADER_PATH "${_GAMEENGINE_HEADER}" PATH)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" _GAMEENGINE_HGROUP_PATH "${_GAMEENGINE_HEADER_PATH}")
        string(REPLACE "/" "\\" _GAMEENGINE_HGROUP_PATH "${_GAMEENGINE_HGROUP_PATH}")
        source_group("Header Files\\${_GAMEENGINE_HGROUP_PATH}" FILES "${_GAMEENGINE_HEADER}")
    endforeach()
endif()

# Enable Thyme debug logging.
if(LOGGING)
    set(CAPTNLOG_LEVEL $<$<CONFIG:Debug>:5>$<$<CONFIG:RelWithDebInfo>:3>$<$<CONFIG:Release>:3>$<$<CONFIG:MinSizeRel>:3>)
else()
    set(CAPTNLOG_LEVEL 0)
endif()

# Enable Thyme debug assertions.
if(ASSERTIONS)
    set(CAPTNASSERT_LEVEL $<$<CONFIG:Debug>:2>$<$<CONFIG:RelWithDebInfo>:1>$<$<CONFIG:Release>:1>$<$<CONFIG:MinSizeRel>:1>)
else()
    set(CAPTNASSERT_LEVEL 0)
endif()

# Setup various included libraries
add_subdirectory(libs/captnlog EXCLUDE_FROM_ALL)

# Gather needed link libraries and compile defintions
list(APPEND GAME_LINK_LIBRARIES base captnlog)

if(NOT OPENAL_FOUND AND NOT FFMPEG_FOUND)
    add_subdirectory(libs/bink EXCLUDE_FROM_ALL)
    add_subdirectory(libs/miles EXCLUDE_FROM_ALL)
endif()

if(USE_GAMEMATH)
    add_subdirectory(libs/gamemath EXCLUDE_FROM_ALL)
    list(APPEND GAME_LINK_LIBRARIES gamemath_static_lib)
    list(APPEND GAME_COMPILE_OPTIONS -DBUILD_WITH_GAMEMATH)
endif()

# Add this file as a library as a hack so that this file isn't compiled against STLPort
add_library(crash_handler STATIC game/crashhandler.cpp)
list(APPEND GAME_LINK_LIBRARIES crash_handler)
target_include_directories(crash_handler PRIVATE hooker ${GAMEENGINE_INCLUDES})
target_link_libraries(crash_handler PRIVATE base captnlog)

if(ICU_FOUND)
    target_link_libraries(crash_handler PRIVATE ICU::data ICU::i18n ICU::io ICU::tu ICU::uc)
    target_compile_options(crash_handler PRIVATE -DBUILD_WITH_ICU)
endif()

if(NOT STANDALONE)
    target_compile_options(crash_handler PRIVATE -DGAME_DLL)
endif()

if(USE_CRASHPAD)
    add_subdirectory(libs/crashpad)
    # Rename the crash handler so it won't conflict with any other installs of crashpad.
    set_target_properties(crashpad_handler PROPERTIES OUTPUT_NAME thymecrashhandler)
    list(APPEND GAME_LINK_LIBRARIES crashpad_client)
    list(APPEND GAME_COMPILE_OPTIONS -DBUILD_WITH_CRASHPAD)
    target_compile_options(crash_handler PRIVATE -DBUILD_WITH_CRASHPAD)
    target_link_libraries(crash_handler PRIVATE crashpad_client)
endif()

if (MSVC AND NOT STANDALONE)
    list(APPEND GAME_COMPILE_OPTIONS -DTHYME_USE_STLPORT)
endif()

if(STANDALONE)
    list(APPEND GAME_COMPILE_OPTIONS "ALLOW_HOOKING=")
else()
    list(APPEND GAME_COMPILE_OPTIONS "ALLOW_HOOKING=friend void Setup_Hooks()\\\;" -DGAME_DLL)

    if(NOT OPENAL_FOUND AND NOT FFMPEG_FOUND)
        list(APPEND GAME_LINK_LIBRARIES binkstub milesstub)
        list(APPEND GAME_COMPILE_OPTIONS -DBUILD_WITH_BINK -DBUILD_WITH_MILES)
    endif()
endif()

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    list(APPEND GAME_LINK_LIBRARIES winmm)
endif()

if(ICU_FOUND)
    list(APPEND GAME_LINK_LIBRARIES ICU::data ICU::i18n ICU::io ICU::tu ICU::uc)
    list(APPEND GAME_COMPILE_OPTIONS -DBUILD_WITH_ICU)
endif()

if(D3D8_FOUND)
    list(APPEND GAME_LINK_LIBRARIES d3d8 d3dx8)
    list(APPEND GAME_COMPILE_OPTIONS -DBUILD_WITH_D3D8)

    if(MSVC)
        list(APPEND GAME_LINK_LIBRARIES legacy_stdio_definitions)
    endif()
endif()

if(DINPUT8_FOUND)
    list(APPEND GAME_LINK_LIBRARIES dxguid dinput8)
    list(APPEND GAME_COMPILE_OPTIONS -DBUILD_WITH_DINPUT)
endif()

if(STANDALONE)
    add_executable(thyme ${GAMEENGINE_GAME_SRC} ${GAMEENGINE_SRC} ${GAMEENGINE_HEADERS})
    target_include_directories(thyme PRIVATE ${GAMEENGINE_INCLUDES})
else()
    add_library(thyme SHARED ${HOOKER_SRC} ${HOOKER_GAME_SRC} ${GAMEENGINE_GAME_SRC} ${GAMEENGINE_SRC} ${GAMEENGINE_HEADERS})
    target_include_directories(thyme BEFORE PRIVATE libs/stlport)
    target_include_directories(thyme PRIVATE hooker ${GAMEENGINE_INCLUDES})
    target_compile_definitions(thyme PRIVATE -DGAME_DLL -D_USE_32BIT_TIME_T)
endif()

if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4 AND ${CMAKE_VERSION} VERSION_GREATER "3.13.0")    
    target_link_directories(thyme BEFORE PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/olddx)
    target_include_directories(thyme PRIVATE libs/olddx)
endif()

target_link_libraries(thyme ${GAME_LINK_LIBRARIES})
target_compile_definitions(thyme PRIVATE ${GAME_COMPILE_OPTIONS})
target_compile_definitions(thyme PRIVATE $<$<CONFIG:DEBUG>:GAME_DEBUG>)

# Note that we don't care about which math lib we link against for worldbuilder.
# Since worldbuilder isn't multiplayer we don't need to worry about sources of indeterminancy
if(BUILD_EDITOR)
    set(CMAKE_MFC_FLAG 2) #Enable MFC
    if(STANDALONE)
        add_executable(thymeedit ${GAMEENGINE_WB_SRC} ${GAMEENGINE_SRC} ${GAMEENGINE_HEADERS})
        target_include_directories(thymeedit PRIVATE ${GAMEENGINE_INCLUDES})
    else()
        add_library(thymeedit SHARED ${HOOKER_SRC} ${HOOKER_WB_SRC} ${GAMEENGINE_WB_SRC} ${GAMEENGINE_SRC} ${GAMEENGINE_HEADERS})
        target_include_directories(thymeedit BEFORE PRIVATE libs/stlport)
        target_include_directories(thymeedit PRIVATE hooker ${GAMEENGINE_INCLUDES})
        target_compile_definitions(thymeedit PRIVATE -DGAME_DLL -DGAME_DEBUG_STRUCTS -DBUILD_EDITOR -D_USE_32BIT_TIME_T)
    endif()
    
    if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4 AND ${CMAKE_VERSION} VERSION_GREATER "3.13.0")    
        target_link_directories(thymeedit BEFORE PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/olddx)
        target_include_directories(thymeedit PRIVATE libs/olddx)
    endif()
    target_link_libraries(thymeedit ${GAME_LINK_LIBRARIES})
    target_compile_definitions(thymeedit PRIVATE ${GAME_COMPILE_OPTIONS})
    target_compile_definitions(thymeedit PRIVATE $<$<CONFIG:DEBUG>:GAME_DEBUG>)
endif()
