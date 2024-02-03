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
#include "gametext.h"
#include "gamewindow.h"
#include "unicodestring.h"

#ifndef GAME_DLL
HotKeyManager *g_theHotKeyManager;
#endif

HotKeyManager::HotKeyManager() {}

HotKeyManager::~HotKeyManager()
{
    m_hotKeys.clear();
}

void HotKeyManager::Init()
{
    m_hotKeys.clear();
}

void HotKeyManager::Reset()
{
    m_hotKeys.clear();
}

Utf8String HotKeyManager::Search_Hot_Key(const Utf8String &str)
{
    return Search_Hot_Key(g_theGameText->Fetch(str));
}

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

void HotKeyManager::Add_Hot_Key(GameWindow *window, const Utf8String &key)
{
    Utf8String key_lower(key);
    key_lower.To_Lower();
    auto it = m_hotKeys.find(key_lower);

    if (it != m_hotKeys.end()) {
        captainslog_dbgassert("Hotkey %s is already mapped to window %s, current window is %s",
            key_lower.Str(),
            it->second.m_window->Win_Get_Instance_Data()->m_decoratedNameString.Str(),
            window->Win_Get_Instance_Data()->m_decoratedNameString.Str());
    } else {
        HotKey hotkey;
        hotkey.m_key.Set(key_lower);
        hotkey.m_window = window;
        m_hotKeys[key_lower] = hotkey;
    }
}

GameMessageDisposition HotKeyTranslator::Translate_Game_Message(const GameMessage *msg)
{
#ifdef GAME_DLL
    return Call_Method<GameMessageDisposition, HotKeyTranslator, const GameMessage *>(
        PICK_ADDRESS(0x005AE8C0, 0x008F84E0), this, msg);
#else
    return KEEP_MESSAGE;
#endif
}
