setlocal

set ThisDir=%~dp0.
call "%ThisDir%\setup_folders.bat"

if not exist "%zDir%" mkdir "%zDir%"
if not exist "%CMakeDir%" mkdir "%CMakeDir%"

:: Extract 7z files if necessary
tar.exe -x -k -f "%ToolsDir%\archive\7z\7z1900_x64.zip" -C "%zDir%"

:: Extract cmake files if necessary
"%zDir%\7z.exe" x "%ToolsDir%\archive\cmake\cmake-3.21.3-windows-x86_64.7z" -o"%CMakeDir%" -aos

endlocal
