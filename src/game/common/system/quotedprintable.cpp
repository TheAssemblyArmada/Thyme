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

int Hex_Digit_To_Int(char c)
{
    if (c <= '9' && c >= '0') {
        return c - '0';
    }

    if (c <= 'f' && c >= 'a') {
        return c - ('a' - 10);
    }

    if (c > 'F' || c < 'A') {
        return 0;
    }

    return c - ('A' - 10);
}

Utf8String Quoted_Printable_To_Ascii_String(Utf8String string)
{
    static char dest[1024];
    char *buf = dest;

    for (const char *c = string.Str(); c[0] != '\0'; c++) {
        if (c[0] == '_') {
            if (c[1] == '\0') {
                break;
            }

            *buf = Hex_Digit_To_Int(*++c);

            if (c[1] != '\0') {
                *buf *= 16;
                *buf = Hex_Digit_To_Int(*++c) | *buf;
            }
        } else {
            *buf = c[0];
        }

        buf++;
    }

    *buf = '\0';
    return dest;
}
