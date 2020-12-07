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

#include "always.h"
#include "subsysteminterface.h"

struct KeyboardIO
{
    enum StatusType
    {
        STATUS_UNUSED,
        STATUS_USED,
    };

    uint8_t key;
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
protected:
    enum
    {
        KEY_COUNT = 256,
        KEY_UP = 1,
        KEY_DOWN = 2,
        KEY_REPEAT_DELAY = 10,
        MODIFIER_LCTRL = 0x4,
        MODIFIER_RCTRL = 0x8,
        MODIFIER_LSHIFT = 0x10,
        MODIFIER_RSHIFT = 0x20,
        MODIFIER_LALT = 0x40,
        MODIFIER_RALT = 0x80,
        KEY_STATE_AUTOREPEAT = 0x100,
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
    virtual void Create_Stream_Messages();
    virtual void Get_Key(KeyboardIO *io) = 0;

    wchar_t Get_Printable_Key(uint8_t key, int key_type);
    void Reset_Keys();
    bool Is_Shift() const;
    bool Is_Ctrl() const;
    bool Is_Alt() const;

    uint16_t Get_Modifiers() const { return m_modifiers; }
    KeyboardIO *Get_First_Key() { return m_keys; }
    int8_t Get_Key_Status(uint8_t index) const { return m_keyStatus[index].status; }
    bool Get_Key_State_Bit(uint8_t index, uint16_t val) const { return (val & m_keyStatus[index].state) != 0; }
    uint32_t Get_Key_Sequence(uint8_t index) const { return m_keyStatus[index].sequence; }

    void Set_Key_Status(uint8_t index, KeyboardIO::StatusType stat) { m_keyStatus[index].status = stat; }
    void Set_Key_State(uint8_t index, uint16_t stat) { m_keyStatus[index].state = stat; }

private:
    void Init_Key_Names();
    void Update_Keys();
    wchar_t Translate_Key(wchar_t key);
    bool Check_Key_Repeat();

protected:
    uint16_t m_modifiers;
    int8_t m_shiftExKey;
    KeyboardIO m_keys[KEY_COUNT];
    KeyboardIO m_keyStatus[KEY_COUNT];
    KeyboardKeyNames m_keyNames[KEY_COUNT];
    uint32_t m_inputFrame;
};

#ifdef GAME_DLL
extern Keyboard *&g_theKeyboard;
#else
extern Keyboard *g_theKeyboard;
#endif