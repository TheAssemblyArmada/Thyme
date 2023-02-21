[![windows](https://github.com/TheAssemblyArmada/Thyme/workflows/windows/badge.svg)](https://github.com/TheAssemblyArmarda/Thyme/actions)  
[![linux](https://github.com/TheAssemblyArmada/Thyme/workflows/linux/badge.svg)](https://github.com/TheAssemblyArmarda/Thyme/actions)
[![codecov](https://codecov.io/gh/TheAssemblyArmada/Thyme/branch/develop/graph/badge.svg)](https://codecov.io/gh/TheAssemblyArmada/Thyme)

[![en-GB](https://img.shields.io/badge/lang-en--GB-green.svg)](https://github.com/TheAssemblyArmada/Thyme/README.md)
[![de-DE](https://img.shields.io/badge/lang-de--DE-green.svg)](https://github.com/TheAssemblyArmada/Thyme/README.de-DE.md)

**Thyme** is an open source re-implementation of Command & Conquer Generals: Zero Hour.
Zero Hour is the expansion pack of Command & Conquer Generals, released in 2003.

This project is a bottom-up rewrite of Generals: Zero Hour,
using the original binary to provide functions that have not been implemented yet.
The intention is to allow the fixing of bugs, implementation of new
features and porting to platforms unsupported by the original.

Please note that the Thyme developers have no affiliation with EA.
EA has not endorsed and does not support this product.

## Chat

You can chat and discuss the development of Thyme on our [Discord channel](https://discord.gg/UnWK2Tw).

## Running Thyme


**As of now (Feb. 2023) Thyme only runs on Windows and must be compiled by yourself.**

In order to run Thyme, you must meet one of the following prerequisites:

1. You have an installation from the original English version on disc (**NOT** "The First Decade", 
"The Ultimate Collection" or any other bundled version) which **must** be patched to version 1.04
2. You have got hold of the game.dat from the installation referred to in point 1, and copied it to the root directory 
of the C6C General ZH installation

If you have either of these options ready, you can proceed with building and compiling the `thyme.dll` and 
`avifil32.dll` as described [here](#building-thyme-yourself).

Once you have successfully created these DLLs, copy them to the C&C Generals ZH root directory.

Next, rename the game.dat in your ZH root directory to `game.exe` (the name doesn't matter as long as it has the .exe 
extension).

Congratulations, you should now be able to start Thyme by running `game.exe`!

To check if Thyme is loaded successfully, you can start the game from CMD or PowerShell 
(right click in the folder while holding down the Shift key and select "Open PowerShell") with the following command:

`.\game.exe -win`

This should start the game in windowed mode, so you can see if the window has a name that includes "Thyme".


If you have any problems getting Thyme to work following this guide, feel free to contact us on our [Discord](#Chat).

### Disclaimer

Thyme is in heavy development and as such, development builds may not behave correctly compared to the original game. We will endevour to create numbered releases that are as free from bugs as possible but even they may have some unexpected behaviour.

### Anti-virus warning

Anti-virus software like Windows Defender could mark Thyme as a virus. This is a false-positive. We can assure you that Thyme is fully safe to use. If you are still unsure about this, you can just [build Thyme yourself](#building-thyme-yourself).

### Common Issues

A black screen at startup is normally a sign that you have gentool installed. Due to how Thyme currently works, you will need to teporarily remove or rename the gentool d3d8.dll file.

## Building Thyme yourself

In order to generate a working binary, currently you must build the project using
a version of the Microsoft Visual Studio compiler (2017 and above). Building with Clang against a Windows SDK is in theory
possible but is untested for a cross compile solution.

It is also possible to build a standalone binary if you want to test compiling on other platforms.

Check the [Thyme Wiki](https://github.com/TheAssemblyArmada/Thyme/wiki) for [detailed build instructions](https://github.com/TheAssemblyArmada/Thyme/wiki/Compiling-Thyme).

### Linux and macOS

Native support for Linux and macOS is planned for the future, but because of how
the project is developed, a fully playable native binary will not be possible for some time.
In the mean time, using Wine on Linux and macOS, should be possible but
is currently untested.

If you are interested in developing the cross platform aspect of the project
it is now possible to build the project standalone by passing ```-DSTANDALONE=TRUE``` to CMake when configuring the project
but the resulting binary currently has very limited functionality.

## Contributing

If you are interested in contributing to Thyme, you will need some knowledge of C++
as a minimum requirement. Join the developer chat listed above for more information on
what else you will need, such as the idc file for the current map of the binary.

You can also check the [wiki](https://github.com/TheAssemblyArmada/Thyme/wiki) for more information.

## Licence

The source code provided in this repository for
Thyme is licenced under the [GNU General Public License version 2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html)
or later with an exception allowing the resulting code to be linked against a closed source
binary which will be in place until Thyme no longer relies on the original.

## Other Open-Game Projects

Below is a list of similar projects and their respective original games:

 * [Alive](https://github.com/AliveTeam/alive_reversing) - Abe's Exodus
 * [dethrace](https://github.com/jeff-1amstudios/dethrace) - Carmageddon
 * [Devilution](https://github.com/diasurgical/devilution) - Diablo
 * [OpenDUNE](https://github.com/OpenDUNE/OpenDUNE) - Dune 2
 * [OpenFodder](https://github.com/OpenFodder/openfodder) - Cannon Fodder
 * [OpenLoco](https://github.com/OpenLoco/OpenLoco) - Locomotion 
 * [OpenMC2](https://github.com/LRFLEW/OpenMC2) - Midnight Club 2
 * [OpenRCT2](https://github.com/OpenRCT2/OpenRCT2) - RollerCoaster Tycoon 2
 * [OpenTTD](https://www.openttd.org) - Transport Tycoon Deluxe
 * [Vanilla Conquer](https://github.com/TheAssemblyArmada/Vanilla-Conquer) - Tiberian Dawn & Red Alert

There is also a [Wikipedia page for open source games](https://en.wikipedia.org/wiki/List_of_open-source_video_games).
