@setlocal

@set BuildDir=%~dp0..
@call "%BuildDir%\win_scripts\setup_folders.bat"
@call "%BuildDir%\win_scripts\vs_build_release.bat" "%SolutionVS2017Dir%" 14

@endlocal
