setlocal

@set ThisDir=%~dp0.
@set SolutionDir=%~1
@set Version=%~2
@set VswhereExe="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

@call "%ThisDir%\scripts\setup_folders.bat"

:: Find MSBuild.exe for Visual Studio 2019
for /f "tokens=* usebackq" %%f in (`%VswhereExe% -version %Version% -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
  set MSBuild=%%f
)

:: Build Thyme in release
"%MSBuild%" "%SolutionDir%\thyme.sln" -property:Configuration=Release

endlocal
