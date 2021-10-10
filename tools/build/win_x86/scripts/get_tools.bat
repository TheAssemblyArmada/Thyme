setlocal

@set CMakeUrl="https://github.com/Kitware/CMake/releases/download/v3.21.3/cmake-3.21.3-windows-x86_64.zip"
@set ThisDir=%~dp0.
@call "%ThisDir%\setup_folders.bat"

@if not exist "%CMakeDir%\bin\cmake.exe" (
    :: Create directory
    if not exist "%CMakeDir%" mkdir "%CMakeDir%"
    :: Download archive
    if not exist "%CMakeDir%\cmake.zip" (
        curl -L %CMakeUrl% -o "%CMakeDir%\cmake.zip"
    )
    :: Extract archive
    if exist "%CMakeDir%\cmake.zip" (
        tar -x -k -f "%CMakeDir%\cmake.zip" -C "%CMakeDir%" --strip-components=1
        del /q "%CMakeDir%\cmake.zip"
    )
)

endlocal
