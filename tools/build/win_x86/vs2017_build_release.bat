setlocal

@set ThisDir=%~dp0.
@call "%ThisDir%\scripts\setup_folders.bat"
@call "%ThisDir%\scripts\vs_build_release.bat" "%SolutionVS2017Dir%" 14

endlocal
