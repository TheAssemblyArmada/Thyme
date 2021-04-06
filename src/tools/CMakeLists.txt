# These tool targets rely on wxwidgets being found.
if(wxWidgets_FOUND)
    include(TargetExports)
    add_subdirectory(debugwindow)
endif()
