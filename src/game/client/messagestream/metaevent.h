/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Meta Map
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
#include "gamemessage.h"
#include "ini.h"
#include "mempoolobj.h"
#include "messagestream.h"
#include "subsysteminterface.h"
#include "unicodestring.h"

enum MappableKeyCategories
{
    CATEGORY_CONTROL,
    CATEGORY_INFORMATION,
    CATEGORY_INTERFACE,
    CATEGORY_SELECTION,
    CATEGORY_TAUNT,
    CATEGORY_TEAM,
    CATEGORY_MISC,
    CATEGORY_DEBUG,
    CATEGORY_NUM_CATEGORIES,
};

enum MappableKeyType
{
    MK_ESC = 0x1,
    MK_BACKSPACE = 0xe,
    MK_ENTER = 0x1c,
    MK_SPACE = 0x39,
    MK_TAB = 0xf,
    MK_F1 = 0x3b,
    MK_F2 = 0x3c,
    MK_F3 = 0x3d,
    MK_F4 = 0x3e,
    MK_F5 = 0x3f,
    MK_F6 = 0x40,
    MK_F7 = 0x41,
    MK_F8 = 0x42,
    MK_F9 = 0x43,
    MK_F10 = 0x44,
    MK_F11 = 0x57,
    MK_F12 = 0x58,
    MK_A = 0x1e,
    MK_B = 0x30,
    MK_C = 0x2e,
    MK_D = 0x20,
    MK_E = 0x12,
    MK_F = 0x21,
    MK_G = 0x22,
    MK_H = 0x23,
    MK_I = 0x17,
    MK_J = 0x24,
    MK_K = 0x25,
    MK_L = 0x26,
    MK_M = 0x32,
    MK_N = 0x31,
    MK_O = 0x18,
    MK_P = 0x19,
    MK_Q = 0x10,
    MK_R = 0x13,
    MK_S = 0x1f,
    MK_T = 0x14,
    MK_U = 0x16,
    MK_V = 0x2f,
    MK_W = 0x11,
    MK_X = 0x2d,
    MK_Y = 0x15,
    MK_Z = 0x2c,
    MK_1 = 0x2,
    MK_2 = 0x3,
    MK_3 = 0x4,
    MK_4 = 0x5,
    MK_5 = 0x6,
    MK_6 = 0x7,
    MK_7 = 0x8,
    MK_8 = 0x9,
    MK_9 = 0xa,
    MK_0 = 0xb,
    MK_MINUS = 0xc,
    MK_EQUAL = 0xd,
    MK_LBRACKET = 0x1a,
    MK_RBRACKET = 0x1b,
    MK_SEMICOLON = 0x27,
    MK_APOSTROPHE = 0x28,
    MK_TICK = 0x29,
    MK_BACKSLASH = 0x2b,
    MK_COMMA = 0x33,
    MK_PERIOD = 0x34,
    MK_SLASH = 0x35,
    MK_KP1 = 0x4F,
    MK_KP2 = 0x50,
    MK_KP3 = 0x51,
    MK_KP4 = 0x4B,
    MK_KP5 = 0x4C,
    MK_KP6 = 0x4D,
    MK_KP7 = 0x47,
    MK_KP8 = 0x48,
    MK_KP9 = 0x49,
    MK_KP0 = 0x52,
    MK_KPSPLASH = 0xB5,
    MK_UP = 0xc8,
    MK_DOWN = 0xd0,
    MK_LEFT = 0xcb,
    MK_RIGHT = 0xcd,
    MK_HOME = 0xc7,
    MK_END = 0xcf,
    MK_PGUP = 0xc9,
    MK_PGDN = 0xd1,
    MK_INS = 0xd2,
    MK_DEL = 0xd3,
    MK_NONE = 0x0,
};

enum MappableKeyTransition
{
    DOWN,
    UP,
    DOUBLEDOWN,
};

enum MappableKeyModState
{
    NONE = 0,
    CTRL = 1 << 2,
    ALT = 1 << 6,
    SHIFT = 1 << 4,

    CTRL_ALT = CTRL | ALT,

    SHIFT_CTRL = SHIFT | CTRL,
    SHIFT_ALT = SHIFT | ALT,
    SHIFT_ALT_CTRL = SHIFT | ALT | CTRL,
};

enum CommandUsableInType
{
    COMMANDUSABLE_NONE,
    COMMANDUSABLE_SHELL,
    COMMANDUSABLE_GAME,
};

class MetaMapRec : public MemoryPoolObject
{
    IMPLEMENT_POOL(MetaMapRec);

public:
    MetaMapRec() {}
    virtual ~MetaMapRec() override {}

    MetaMapRec *m_next;
    GameMessage::MessageType m_meta;
    MappableKeyType m_key;
    MappableKeyTransition m_transition;
    MappableKeyModState m_modState;
    CommandUsableInType m_usableIn;
    MappableKeyCategories m_category;
    Utf16String m_description;
    Utf16String m_displayName;
};

class MetaMap : public SubsystemInterface
{
public:
    MetaMap() : m_metaMaps(nullptr) {}
    virtual ~MetaMap() override;
    virtual void Init() override {}
    virtual void Reset() override {}
    virtual void Update() override {}

    GameMessage::MessageType Find_Game_Message_Meta_Type(const char *type);
    MetaMapRec *Get_Meta_Map_Rec(GameMessage::MessageType t);
    static void Parse_Meta_Map(INI *ini);

private:
    MetaMapRec *m_metaMaps;
};

class MetaEventTranslator : public GameMessageTranslator
{
public:
    MetaEventTranslator();
    virtual GameMessageDisposition Translate_Game_Message(const GameMessage *msg) override;
    virtual ~MetaEventTranslator() override {}

private:
    int m_lastKeyDown;
    int m_lastModState;
    ICoord2D m_lastButtonPos[3];
    bool m_lastButtonState[3];
};

#ifdef GAME_DLL
extern MetaMap *&g_theMetaMap;
#else
extern MetaMap *g_theMetaMap;
#endif
