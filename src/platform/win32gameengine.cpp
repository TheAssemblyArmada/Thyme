#include "win32gameengine.h"

LocalFileSystem *Win32GameEngine::Create_Local_Filesystem()
{
    return new Win32LocalFileSystem;
}
