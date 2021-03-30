# BaseConfig

BaseConfig is intended to be used as a git submodule shared between project hosted under the [Assembly Armarda](https://github.com/TheAssemblyArmada)
banner to provide a consistent interface for various utility macros and functions, compiler extensions and intrinsics.

## Features

### Consistent Sized Types

Mostly makes used of the C99 standard header inttypes.h. Additionally provides a typedef for the ICU UChar type or standard wchar_t type to altenatively 
make use of ICU or WINAPI for UTF-8 <> UTF-16 conversions.

It also provides types that allow aliasing to other types, particularly useful for doing bit manipulation of floating point types which are
otherwise optimised away by some compilers due to C/C++ strict aliasing rules.

### Consistent Compiler and Platform Identification Macros

A set of optional macros that use a consistent convention for identifying popular compilers and platforms.

### Consistent Endian Handling Functions

A set of defines to give consistent byteswapping to/from the host native integer format.

### Embeded Git Version Info

The CMake module will detect git on the compiling platform and interrogate it for commit information for use by programs linking against
base config.

### Consistent Compiler Intriniscs and Macros

Provides a consistent subset of useful compiler intrinsics and macros to check for their availability.
