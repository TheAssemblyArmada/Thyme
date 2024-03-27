/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Implementation of LocalFile file io interface. Despite the class name, the API used is Posix even on windows.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"
#include "localfile.h"

namespace Thyme
{
class StdLocalFileSystem;
}

class Win32LocalFile : public LocalFile
{
    IMPLEMENT_POOL(Win32LocalFile);

    friend class Win32LocalFileSystem;
    friend class Thyme::StdLocalFileSystem;

protected:
    virtual ~Win32LocalFile() override {}
};
