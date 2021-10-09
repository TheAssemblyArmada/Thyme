setlocal

set ThisDir=%~dp0.
call "%ThisDir%\scripts\setup_folders.bat"

:: Extract tools if necessary
call "%BuildDir%\scripts\extract_tools.bat"

if not exist "%SolutionVS2017Dir%" mkdir "%SolutionVS2017Dir%"
cd "%SolutionVS2017Dir%"
:: Generate solution files
"%CMakeDir%\bin\cmake.exe" -G"Visual Studio 15 2017" -A Win32 -S "%ProjectDir%"
:: Open cmake gui for user customizations
if not ERRORLEVEL 1 (
    "%CMakeDir%\bin\cmake-gui.exe" "%SolutionVS2017Dir%"
)
cd "%cd%"

endlocal
