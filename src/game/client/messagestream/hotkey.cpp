/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Hotkey
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "hotkey.h"
#include "unicodestring.h"

#ifndef GAME_DLL
HotKeyManager *g_theHotKeyManager;
#endif

Utf8String HotKeyManager::Search_Hot_Key(const Utf16String &key)
{
    if (!key.Is_Empty()) {
        for (const unichar_t *i = key.Str(); i != nullptr && *i != '\0'; i++) {
            if (*i == '&') {
                Utf16String str(Utf16String::s_emptyString);
                str.Concat(i[1]);
                Utf8String str2;
                str2.Translate(str.Str());
                return str2;
            }
        }
    }

    return Utf8String::s_emptyString;
}
