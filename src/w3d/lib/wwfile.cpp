/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Interface for file IO.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "wwfile.h"

off_t FileClass::Tell()
{
    return Seek(0, FS_SEEK_CURRENT);
}

time_t FileClass::Get_Date_Time()
{
    return 0;
}

bool FileClass::Set_Date_Time(time_t date_time)
{
    return false;
}
