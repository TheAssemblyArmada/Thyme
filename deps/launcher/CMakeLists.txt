# Build launchers for the various binaries we support
include(ProductVersion)
include(SetSail)
    
generate_product_version(
    THYME_SETSAIL_RC
    NAME "Thyme Launcher"
    BUNDLE "SetSail Launchers"
    VERSION_MAJOR 1
    VERSION_MINOR 0
    COMPANY_NAME "Assembly Armada"
    COMPANY_COPYRIGHT "Code released under GPLv2 or later."
    ORIGINAL_FILENAME "setsaillauncher.exe"
    RCFILE_PREFIX "thyme"
    ICON "${CMAKE_CURRENT_SOURCE_DIR}/launcher.ico"
)
    
generate_setsail_launcher(THYMELAUNCHER_EXE PREFIX "thyme" ENTRYPOINT 0x008E0778 EXENAME "game.dat" HASH "5b805b4ecba79581fe8ec454b5acf7b7e13d4dd5" EXTRA_SOURCES ${THYME_SETSAIL_RC} ${CMAKE_CURRENT_SOURCE_DIR}/launcher.ico)

if(BUILD_EDITOR)
    generate_setsail_launcher(THYMEWBLAUNCHER_EXE PREFIX "thymeedit" ENTRYPOINT 0x00B88F90 EXENAME "WorldBuilder.exe" HASH "3c1a1d431eae665978c9406f05c427252bc4e768" EXTRA_SOURCES ${THYME_SETSAIL_RC} ${CMAKE_CURRENT_SOURCE_DIR}/launcher.ico)
endif()