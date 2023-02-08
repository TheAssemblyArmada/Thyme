# These tool targets rely on wxwidgets being found.
if(wxWidgets_FOUND)
    include(TargetExports)
    add_subdirectory(debugwindow)
endif()

# These tools are built against the Win32 api.
if(WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_subdirectory(w3dviewer)
    add_subdirectory(wdump)
    add_subdirectory(wndedit)
endif()

# These tools are generic and API independent
add_subdirectory(compressor)
