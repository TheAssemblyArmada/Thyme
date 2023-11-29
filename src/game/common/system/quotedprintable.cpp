/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Quoted Printable
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "quotedprintable.h"
#include <cctype>

char Int_To_Hex_Digit(int num)
{
    if (num >= 16) {
        return '\0';
    }

    if (num >= 10) {
        return num + '7';
    }

    return num + '0';
}

Utf8String Ascii_String_To_Quoted_Printable(Utf8String string)
{
    static char dest[1024];
    const char *str = string.Str();
    int count = 0;

    while (*str != '\0' && count < 1021) {
        if (isalnum(*str)) {
            dest[count++] = *str;
        } else {
            dest[count++] = '_';
            dest[count++] = Int_To_Hex_Digit(*str >> 4);
            dest[count++] = Int_To_Hex_Digit(*str & 15);
        }

        str++;
    }

    dest[count] = '\0';
    return dest;
}
