[![windows](https://github.com/TheAssemblyArmada/Thyme/workflows/windows/badge.svg)](https://github.com/TheAssemblyArmarda/Thyme/actions)  
[![linux](https://github.com/TheAssemblyArmada/Thyme/workflows/linux/badge.svg)](https://github.com/TheAssemblyArmarda/Thyme/actions)
[![codecov](https://codecov.io/gh/TheAssemblyArmada/Thyme/branch/develop/graph/badge.svg)](https://codecov.io/gh/TheAssemblyArmada/Thyme)

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

Currently Thyme can only be run on Windows.

To run Thyme, you first need to have Command & Conquer Generals: Zero Hour installed. If you don't have the original game disc, Generals: Zero Hour can currently be bought most easily as a downloadable version that is part of the Command & Conquer The Ultimate Collection from [Origin](https://www.origin.com/en-us/store/command-and-conquer/command-and-conquer-the-ultimate-collection/ultimate-collection) or a third-party like [Amazon](https://www.amazon.com/Command-Conquer-Ultimate-Collection-PC/dp/B0095C0I4W).

If you wish to use our custom launcher without messing with patches or anything, only an install original Windows disc based English language version of Generals: Zero Hour (patched to version 1.04) is supported. That 1.04 patch can be downloaded [here](https://web.archive.org/web/20170130195007/http://ftp.ea.com/pub/eapacific/generals/generalszh/DownloadablePatches/GeneralsZH-104-english.exe) if your installation hasn't been updated yet.

If you don't have the disc version or just want to run an executable directly, you will need to generate `thyme.exe` using the exe patching guide on the [Thyme Wiki](https://github.com/TheAssemblyArmada/Thyme/wiki).

Now you have the Generals: Zero Hour installation ready, you can [download the latest version](https://github.com/TheAssemblyArmada/Thyme/releases). You will have to expand the assets to download the latest thyme_x86_dll.zip. You could also [build Thyme yourself](#building-thyme-yourself).

The easiest way to get Thyme to actually run is to place `thymelauncher.exe` together with `thyme.dll` in the Generals: Zero Hour installation folder. You can then launch `thymelauncher.exe` from that directory or create a shortcut to them.

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
