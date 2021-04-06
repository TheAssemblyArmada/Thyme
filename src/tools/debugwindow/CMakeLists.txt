add_library(debugwindow SHARED)
target_link_libraries(debugwindow PRIVATE ${wxWidgets_LIBRARIES})
target_include_directories(debugwindow PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${wxWidgets_INCLUDE_DIRS})
target_compile_options(debugwindow PRIVATE ${wxWidgets_CXX_FLAGS})
target_compile_definitions(debugwindow PRIVATE ${wxWidgets_DEFINITIONS})
set_target_properties(debugwindow PROPERTIES OUTPUT_NAME DebugWindow PREFIX "")

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_compile_definitions(debugwindow PRIVATE
        -DWIN32_LEAN_AND_MEAN
        -D_CRT_SECURE_NO_DEPRECATE
        -D_CRT_NONSTDC_NO_DEPRECATE
        -D_WINSOCK_DEPRECATED_NO_WARNINGS
    )
endif()

if(NOT wxWidgets_wxrc_EXECUTABLE)
    message(FATAL_ERROR "wxrc resource compiler was not found, please set wxWidgets_wxrc_EXECUTABLE manually.")
endif()

# Generate cpp/h files to embed xml resources in the binary.
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/wxdbgwinui.cpp" "${CMAKE_CURRENT_BINARY_DIR}/wxdbgwinui.h"
    COMMAND ${wxWidgets_wxrc_EXECUTABLE} --cpp-code --extra-cpp-code --output=${CMAKE_CURRENT_BINARY_DIR}/wxdbgwinui.cpp ${CMAKE_CURRENT_SOURCE_DIR}/ui/dbgwinui.xrc
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ui/dbgwinui.xrc"
)

target_sources(debugwindow PRIVATE
    dbgwinapp.cpp
    dbgwinapp.h
    dbgwinframe.cpp
    dbgwinframe.h
    debugwindow.cpp
    debugwindow.h
    dllmain.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/wxdbgwinui.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/wxdbgwinui.h
)
if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(debugwindow PRIVATE debugwindow_win32.cpp)
else()
    target_sources(debugwindow PRIVATE debugwindow_posix.cpp)
endif()

target_exports(debugwindow SYMBOLS
    AdjustVariable
    AdjustVariableAndPause
    AppendMessage
    AppendMessageAndPause
    CanAppContinue
    CreateDebugDialog
    DestroyDebugDialog
    ForceAppContinue
    RunAppFast
    SetFrameNumber
)
