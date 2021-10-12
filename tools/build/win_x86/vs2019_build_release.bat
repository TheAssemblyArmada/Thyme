setlocal

@set ThisDir=%~dp0.
@set VSWhere="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
@call "%ThisDir%\scripts\setup_folders.bat"

:: Find MSBuild.exe for Visual Studio 2019
for /f "tokens=* usebackq" %%f in (`%VSWhere% -version 16 -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
  set MSBuild=%%f
)

:: Build Thyme in release
"%MSBuild%" "%SolutionVS2019Dir%\thyme.sln" -property:Configuration=Release

endlocal
