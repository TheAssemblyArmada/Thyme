@setlocal

@set BuildDir=%~dp0..
@call "%BuildDir%\win_scripts\setup_folders.bat"
@call "%BuildDir%\win_scripts\vs_open_cmake_gui.bat" "%SolutionVS2017Dir%"

@endlocal
