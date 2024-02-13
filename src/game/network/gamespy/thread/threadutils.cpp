/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Thread Utils
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "threadutils.h"
#if defined PLATFORM_WINDOWS
#include <utf.h>
#endif

std::wstring MultiByteToWideCharSingleLine(const char *orig)
{
#ifdef PLATFORM_WINDOWS
    size_t len = strlen(orig);
    unichar_t *wstr = new unichar_t[len + 1];
    MultiByteToWideChar(CP_UTF8, 0, orig, -1, wstr, len);
    unichar_t *c = L'\0';

    do {
        c = wcschr(wstr, L'\n');

        if (c != nullptr) {
            *c = L' ';
        }
    } while (c != L'\0');

    do {
        c = wcschr(wstr, L'\r');

        if (c != nullptr) {
            *c = L' ';
        }
    } while (c != L'\0');

    wstr[len] = L'\0';
    std::wstring ret(wstr);
    delete[] wstr;
    return ret;
#else
    // TODO other platforms
    return L"";
#endif
}

std::string WideCharStringToMultiByte(const unichar_t *orig)
{
#ifdef PLATFORM_WINDOWS
    std::string ret;
    int len = WideCharToMultiByte(CP_UTF8, 0, orig, wcslen(orig), 0, 0, 0, 0);

    if (len > 0) {
        char *str = new char[len];
        WideCharToMultiByte(CP_UTF8, 0, orig, -1, str, len, 0, 0);
        str[len - 1] = '\0';
        ret = str;
        delete[] str;
    }

    return ret;
#else
    // TODO other platforms
    return "";
#endif
}
