/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Cross platform message box invokation function.
 *
 * @copyright CaptainsLog is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "captnmessage.h"
#include <malloc.h>
#include <windows.h>

static wchar_t *to_utf16(const char *str)
{
    int len;
    wchar_t *ret;

    if (str == NULL) {
        return NULL;
    }

    len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);

    if (len == 0) {
        return NULL;
    }

    ret = malloc(len * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, str, -1, ret, len);

    return ret;
}

int captainslog_messagebox(const char *message)
{
    wchar_t *utf16_msg = to_utf16(message);
    int result = MessageBoxW(GetActiveWindow(), utf16_msg, L"", MB_APPLMODAL | MB_ICONERROR| MB_ABORTRETRYIGNORE);
    free(utf16_msg);

    switch (result) {
        case IDRETRY:
            return CAPTMSG_RET;
        case IDIGNORE:
            return CAPTMSG_IGN;
        default:
            return CAPTMSG_ABRT;
    }
}
