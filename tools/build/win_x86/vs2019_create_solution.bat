@setlocal

@set BuildDir=%~dp0..
@call "%BuildDir%\win_scripts\setup_folders.bat"
@call "%BuildDir%\win_scripts\vs_create_solution.bat" "%SolutionVS2019Dir%" "Visual Studio 16 2019"

@endlocal
