/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base class for keyboard input handlers.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "always.h"
#include "subsysteminterface.h"

struct KeyboardIO
{
    enum StatusType
    {
        STATUS_UNUSED,
        STATUS_USED,
    };

    int8_t key;
    int8_t status;
    uint16_t state;
    uint32_t sequence;
};

struct KeyboardKeyNames
{
    wchar_t std_key;
    wchar_t shifted;
    wchar_t shifted_ex;
};

class Keyboard : public SubsystemInterface
{
    enum
    {
        KEY_COUNT = 256,
        KEY_UP = 1,
        KEY_DOWN = 2,
        KEY_REPEAT_DELAY = 10,
        MODIFIER_LCTRL = 0x4,
        MODIFIER_RCTRL = 0x8,
        MODIFIER_CTRL = MODIFIER_LCTRL | MODIFIER_RCTRL,
        MODIFIER_LSHIFT = 0x10,
        MODIFIER_RSHIFT = 0x20,
        MODIFIER_SHIFT = MODIFIER_LSHIFT | MODIFIER_RSHIFT,
        MODIFIER_LALT = 0x40,
        MODIFIER_RALT = 0x80,
        MODIFIER_ALT = MODIFIER_LALT | MODIFIER_RALT,
        MODIFIER_CAPS = 0x200,
        MODIFIER_SHIFTEX = 0x400,
    };

public:
    enum
    {
        STANDARD,
        SHIFTED,
        SHIFTEDEX,
    };

public:
    Keyboard();
    virtual ~Keyboard() {}

    virtual void Init() override;
    virtual void Reset() override {}
    virtual void Update() override;

    virtual bool Get_Caps_State() = 0;
    virtual void Create_Message_Stream();
    virtual int Get_Key(KeyboardIO *io) = 0;

    wchar_t Get_Printable_Key(uint8_t key, int key_type);
    void Reset_Keys();
    bool Is_Shift();
    bool Is_Ctrl();
    bool Is_Alt();

#ifndef THYME_STANDALONE
    void Hook_Init();
    void Hook_Update();
    void Hook_Create_Message_Stream();
    static void Hook_Me();
#endif
private:
    void Init_Key_Names();
    void Update_Keys();
    wchar_t Translate_Key(wchar_t key);
    int Check_Key_Repeat();

protected:
    uint16_t m_modifiers;
    int8_t m_shiftExKey;
    KeyboardIO m_keys[KEY_COUNT];
    KeyboardIO m_keyStatus[KEY_COUNT];
    KeyboardKeyNames m_keyNames[KEY_COUNT];
    uint32_t m_inputFrame;
};

#ifndef THYME_STANDALONE
#include "hooker.h"

inline void Keyboard::Hook_Me()
{
    Hook_Method(0x0040A120, &Hook_Init);
    Hook_Method(0x0040A120, &Hook_Update);
    Hook_Method(0x00407F50, &Hook_Create_Message_Stream);
    Hook_Method(0x0040A4A0, &Get_Printable_Key);
    Hook_Method(0x0040A150, &Reset_Keys);
    Hook_Method(0x0040A460, &Is_Shift);
    Hook_Method(0x0040A480, &Is_Ctrl);
    Hook_Method(0x0040A490, &Is_Alt);
}

extern Keyboard *&g_theKeyboard;
#else
extern Keyboard *g_theKeyboard;
#endif

#endif // KEYBOARD_H
