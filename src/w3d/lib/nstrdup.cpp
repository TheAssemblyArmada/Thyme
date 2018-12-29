/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief strdup using operator new[] rather than malloc.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "nstrdup.h"
#include <cstring>

/**
 * This is being kept around to ensure memory allocated in new code can be freed in original code and vice versa.
 */
char *nstrdup(const char *str)
{
    char *nstr = NULL;

    if (str != NULL) {
        nstr = new char[strlen(str) + 1];

        if (nstr != NULL) {
            std::strcpy(nstr, str);
        }
    }

    return nstr;
}
