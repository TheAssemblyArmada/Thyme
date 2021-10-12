# Build scripts for Windows x86

## Quick description

Install Visual Studio 2019. Run *vs2019_create_solution.bat*. Open *.solution\vs2019\thyme.sln*. Done.

## More detailed description

vs2017\* scripts require Visual Studio 2017 installation.
vs2019\* scripts require Visual Studio 2019 installation.

| Script                | Purpose                                                                                                       |
|-----------------------|---------------------------------------------------------------------------------------------------------------|
| \*create_solution.bat | Installs cmake and generates a solution for Visual Studio in **.solution** folder.                            |
| \*build_release.bat   | Builds release configuration from Visual Studio solution.                                                     |
| \*open_cmake_gui.bat  | Opens CMake Gui for Visual Studio solution.                                                                   |
