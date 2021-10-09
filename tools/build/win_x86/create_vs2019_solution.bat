setlocal

set ThisDir=%~dp0.
call "%ThisDir%\scripts\setup_folders.bat"

:: Extract tools if necessary
call "%ThisDir%\scripts\extract_tools.bat"

if not exist "%SolutionVS2019Dir%" mkdir "%SolutionVS2019Dir%"
cd "%SolutionVS2019Dir%"
:: Generate solution files
"%CMakeDir%\bin\cmake.exe" ^
    -G"Visual Studio 16 2019" ^
    -A Win32 ^
    -S "%ProjectDir%" ^
    -DCMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD=1
:: Open cmake gui for user customizations
if not ERRORLEVEL 1 (
    "%CMakeDir%\bin\cmake-gui.exe" "%SolutionVS2019Dir%"
)
cd "%cd%"

endlocal
