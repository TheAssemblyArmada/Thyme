add_executable(w3dview WIN32)
set(CMAKE_MFC_FLAG 2) #Enable MFC
target_sources(w3dview PRIVATE
    assetinfo.cpp
    colorbar.cpp
    colorpicker.cpp
    colorpickerdialogclass.cpp
    colorutils.cpp
    datatreeview.cpp
    deviceselectiondialog.cpp
    emittercolorproppage.cpp
    emitterframeproppage.cpp
    emittergeneralproppage.cpp
    emitterinstancelist.cpp
    emitterlinegroupproppage.cpp
    emitterlineproppage.cpp
    emitterparticleproppage.cpp
    emitterphysicsproppage.cpp
    emitterpropertysheet.cpp
    emitterrotationproppage.cpp
    emittersizeproppage.cpp
    emitteruserproppage.cpp
    graphicview.cpp
    mainfrm.cpp
    opacitysettingsdialog.cpp
    particleblurtimekeydialog.cpp
    particleframekeydialog.cpp
    particlerotationkeydialog.cpp
    particlesizedialog.cpp
    restrictedfiledialog.cpp
    toolbar.cpp
    utils.cpp
    viewerscene.cpp
    volumerandomdialog.cpp
    w3dview.cpp
    w3dviewdoc.cpp
    w3dviewview.cpp
    ${CMAKE_SOURCE_DIR}/resources/w3dviewer/w3dview.rc
)
add_definitions(-D_AFXDLL)
target_include_directories(w3dview PRIVATE ${CMAKE_SOURCE_DIR}/resources/w3dviewer)
target_link_libraries(w3dview PRIVATE thyme_lib)
