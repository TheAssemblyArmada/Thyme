setlocal

@set ThisDir=%~dp0.
@call "%ThisDir%\scripts\setup_folders.bat"
@call "%ThisDir%\scripts\vs_create_solution.bat" "%SolutionVS2019Dir%" "Visual Studio 16 2019"

endlocal
