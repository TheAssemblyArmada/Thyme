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
#pragma once
#include "always.h"
#include "messagestream.h"
#include "subsysteminterface.h"
#include <map>

class GameWindow;

class HotKey
{
public:
    HotKey() : m_window(nullptr) {}

private:
    GameWindow *m_window;
    Utf8String m_key;
    friend class HotKeyManager;
};

class HotKeyManager : public SubsystemInterface
{
public:
    HotKeyManager();
    virtual ~HotKeyManager() override;

    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override {}
    void Add_Hot_Key(GameWindow *window, const Utf8String &key);
    bool Execute_Hot_Key(const Utf8String &key);
    Utf8String Search_Hot_Key(const Utf8String &str);
    Utf8String Search_Hot_Key(const Utf16String &str);

private:
    std::map<Utf8String, HotKey> m_hotKeys;
};

class HotKeyTranslator : public GameMessageTranslator
{
public:
    HotKeyTranslator() {}
    virtual GameMessageDisposition Translate_Game_Message(const GameMessage *msg) override;
    virtual ~HotKeyTranslator() override {}
};

#ifdef GAME_DLL
extern HotKeyManager *&g_theHotKeyManager;
#else
extern HotKeyManager *g_theHotKeyManager;
#endif
