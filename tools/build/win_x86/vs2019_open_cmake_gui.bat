setlocal

@set ThisDir=%~dp0.
@call "%ThisDir%\scripts\setup_folders.bat"
@call "%ThisDir%\scripts\vs_open_cmake_gui.bat" "%SolutionVS2019Dir%"

endlocal
