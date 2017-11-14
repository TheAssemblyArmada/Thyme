# Thyme

[![Build status](https://ci.appveyor.com/api/projects/status/thd1as29wfnkh586?svg=true)](https://ci.appveyor.com/project/AdamMondez/thyme)

An Open Source Re-Implementation of Generals: Zero Hour

This project is a bottom-up rewrite of Generals: Zero Hour,
using the original binary to provide functions that have not been implemented yet.
The intention is to allow the fixing of bugs, implementation of new
features and porting to platforms unsupported by the original.

## Building

In order to generate a working binary, currently you must build the project using
a version of the Microsoft Visual Studio compiler (2015 and above). Building using MinGW-w64
is also tested periodically, but because of differences in the compiler ABI, dll's that it generates
will not work correctly with the original game binary. Once the reimplementation is complete
it should be possible to build with any C++11 compiler.
 
In order to build, use CMake to generate a solution outside of the src directory.
For example, when using the CMake GUI and the source is at C:\path\to\thyme set the build
path to C:\path\to\thyme\build.

## Running

To run the game, you need to first have Generals: Zero Hour installed.
The easiest way to get the game to run is to place both `launchthyme.exe`
and `thyme.dll` in the same directory as `game.dat`. You can then
launch `launchthyme.exe` from the directory, or use a shortcut to `launchthyme.exe`
to launch the game. The launcher will also pass any command line parameters on.

Currently only the original disc based english language version patched to version 1.04 is 
supported, though adding the `game.dat` from that version to a TFD or Origin
install should also allow those to work.

### Linux and macOS

Native support for Linux and macOS is planned for the future, but because of how
the project is developed, a native binary will not be possible for some time.
In the mean time, using Wine on Linux and macOS, should be possible but
is currently untested.

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
 * [OpenFodder] (https://github.com/OpenFodder/openfodder) - Cannon Fodder

There is also the [Wikipedia page for open source games](https://en.wikipedia.org/wiki/List_of_open-source_video_games).

## Contributing

If you are interested in contributing to Thyme, you will need some knowledge of C++
as a minimum requirement. Join the developer chat listed below for more information on
what else you will need such as the idc file for the current map of the binary.

You can also check the [wiki](https://github.com/TheAssemblyArmada/Thyme/wiki) for more information.

## Contact

You can discuss development of this project on freenode in the #thyme channel.