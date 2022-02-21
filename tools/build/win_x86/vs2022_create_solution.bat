@setlocal

@set BuildDir=%~dp0..
@call "%BuildDir%\win_scripts\setup_folders.bat"
@call "%BuildDir%\win_scripts\vs_create_solution.bat" "%SolutionVS2022Dir%" "Visual Studio 17 2022"

@endlocal
