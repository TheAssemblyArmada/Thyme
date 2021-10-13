setlocal

@set ThisDir=%~dp0.
@set SolutionDir=%~1
@set GeneratorName=%~2
@call "%ThisDir%\setup_folders.bat"

:: Get tools
@call "%ThisDir%\get_tools.bat"

@if not exist "%SolutionDir%" mkdir "%SolutionDir%"
cd /d "%SolutionDir%"
:: Generate solution files
"%CMakeDir%\bin\cmake.exe" ^
    -G "%GeneratorName%" ^
    -A Win32 ^
    -S "%ProjectDir%" ^
    -DCMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD=1

cd /d "%cd%"
:: Open cmake gui for user customizations
if not ERRORLEVEL 1 (
    "%CMakeDir%\bin\cmake-gui.exe" "%SolutionDir%"
)

endlocal
