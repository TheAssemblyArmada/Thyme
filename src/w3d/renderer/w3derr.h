/**
 * @file
 *
 * @author tomsons26
 *
 * @brief W3D error codes.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

enum W3DErrorType
{
    W3D_ERROR_OK,
    W3D_ERROR_GENERIC,
    W3D_ERROR_LOAD_FAILED,
    W3D_ERROR_SAVE_FAILED,
    W3D_ERROR_WINDOW_NOT_OPEN,
    W3D_ERROR_INITIALIZATION_FAILED,
};
