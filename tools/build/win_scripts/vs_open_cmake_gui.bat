setlocal

@set ThisDir=%~dp0.
@set SolutionDir=%~1

@call "%ThisDir%\setup_folders.bat"

"%CMakeDir%\bin\cmake-gui.exe" "%SolutionDir%"

endlocal
