setlocal

@set ThisDir=%~dp0.
@call "%ThisDir%\scripts\setup_folders.bat"
@call "%ThisDir%\scripts\vs_create_solution.bat" "%SolutionVS2017Dir%" "Visual Studio 15 2017"

endlocal
