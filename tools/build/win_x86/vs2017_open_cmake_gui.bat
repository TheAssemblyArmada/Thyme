setlocal

@set ThisDir=%~dp0.
@call "%ThisDir%\scripts\setup_folders.bat"

"%CMakeDir%\bin\cmake-gui.exe" "%SolutionVS2017Dir%"

endlocal
