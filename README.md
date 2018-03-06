# Thyme

[![Build status](https://ci.appveyor.com/api/projects/status/thd1as29wfnkh586?svg=true)](https://ci.appveyor.com/project/AdamMondez/thyme)
[![Build Status](https://travis-ci.org/TheAssemblyArmada/Thyme.svg?branch=develop)](https://travis-ci.org/TheAssemblyArmada/Thyme)

Thyme is an open source re-implementation of Command & Conquer Generals: Zero Hour. Zero Hour is the expansion pack of Command & Conquer Generals, released in 2003.

This project is a bottom-up rewrite of Generals: Zero Hour,
using the original binary to provide functions that have not been implemented yet.
The intention is to allow the fixing of bugs, implementation of new
features and porting to platforms unsupported by the original.

## Chat

You can chat and discuss the development of Thyme on our [Discord channel](https://discord.gg/YhdMbvD) or on freenode in the #thyme channel.

## Running Thyme

Currently Thyme can only be run on Windows.

To run Thyme, you first need to have Command & Conquer Generals: Zero Hour installed. If you don't have the original game disc, Generals: Zero Hour can currently be bought most easily as a downloadable version that is part of the Command & Conquer The Ultimate Collection from [Origin](https://www.origin.com/en-us/store/command-and-conquer/command-and-conquer-the-ultimate-collection/ultimate-collection) or a third-party like [Amazon](https://www.amazon.com/Command-Conquer-Ultimate-Collection-PC/dp/B0095C0I4W).

Currently only the installed original Windows disc based english language version of Generals: Zero Hour (patched to version 1.04) is supported. That 1.04 patch can be downloaded [here](ftp.ea.com/pub/eapacific/generals/generalszh/DownloadablePatches/GeneralsZH-104-english.exe) if your installation hasn't been updated yet. Thyme should also work with the pre-patched installation of The First Decade or The Ultimate Collection (from Origin). This can be achieved by adding the `game.dat` file from from the the original disc based english language version to the Generals: Zero Hour installation directory of the TFD or TUC version.

Now you have the Generals: Zero Hour installation ready, you can [download the latest version](https://ci.appveyor.com/project/AdamMondez/thyme). You will have to click on one of the Job names and after that you can click on 'artefacts' to get the link to the downloadable zipped version of Thyme. You could also [build Thyme yourself](#building-thyme-yourself).

The easiest way to get Thyme to actually run is to place both `launchthyme.exe` and `thyme.dll` in the same directory as `game.dat` in the installation folder of Generals: Zero Hour. You can then launch `launchthyme.exe` from that directory. You can also use a shortcut to `launchthyme.exe` to launch the game. The launcher will also pass any command line parameters on to get for examples mods to run.

### Anti-virus warning

Anti-virus software like Windows Defender could mark Thyme as a virus. This is a false-positive. We can assure you that Thyme is fully safe to use. If you are still unsure about this, you can just [build Thyme yourself](#building-thyme-yourself).

## Building Thyme yourself

In order to generate a working binary, currently you must build the project using
a version of the Microsoft Visual Studio compiler (2015 and above). Building with Clang against a Windows SDK is in theory
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

 * [OpenRCT2](https://github.com/OpenRCT2/OpenRCT2) - RollerCoaster Tycoon 2
 * [OpenTTD](https://www.openttd.org/) - Transport Tycoon Deluxe
 * [OpenMC2](https://github.com/LRFLEW/OpenMC2) - Midnight Club 2
 * [OpenDUNE](https://github.com/OpenDUNE/OpenDUNE) - Dune 2
 * [OpenFodder](https://github.com/OpenFodder/openfodder) - Cannon Fodder

There is also a [Wikipedia page for open source games](https://en.wikipedia.org/wiki/List_of_open-source_video_games).
