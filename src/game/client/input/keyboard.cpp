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
#include "keyboard.h"
#include "messagestream.h"
#include <cwctype>

#ifndef GAME_DLL
Keyboard *g_theKeyboard = nullptr;
#endif

namespace
{
// clang-format off
KeyboardKeyNames g_defaultNames[] = {
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'1', L'!', L'\0' }, { L'2', L'@', L'\0' }, // 4
    { L'3', L'#', L'\0' }, { L'4', L'$', L'\0' }, { L'5', L'%', L'\0' }, { L'6', L'^', L'\0' }, // 8
    { L'7', L'&', L'\0' }, { L'8', L'*', L'\0' }, { L'9', L'(', L'\0' }, { L'0', L')', L'\0' }, // 12 
    { L'-', L'_', L'\0' }, { L'=', L'+', L'\0' }, { L'\b', L'\b', L'\0' }, { L'\t', L'\t', L'\0' }, // 16
    { L'q', L'Q', L'\0' }, { L'w', L'W', L'\0' }, { L'e', L'E', L'\0' }, { L'r', L'R', L'\0' }, // 20
    { L't', L'T', L'\0' }, { L'y', L'Y', L'\0' }, { L'u', L'U', L'\0' }, { L'i', L'I', L'\0' }, // 24
    { L'o', L'O', L'\0' }, { L'p', L'P', L'\0' }, { L'[', L'{', L'\0' }, { L']', L'}', L'\0' }, // 28
    { L'\n', L'\n', L'\0' }, { L'\0', L'\0', L'\0' }, { L'a', L'A', L'\0' }, { L's', L'S', L'\0' }, // 32
    { L'd', L'D', L'\0' }, { L'f', L'F', L'\0' }, { L'g', L'G', L'\0' }, { L'h', L'H', L'\0' }, // 36
    { L'j', L'J', L'\0' }, { L'k', L'K', L'\0' }, { L'l', L'L', L'\0' }, { L';', L':', L'\0' }, // 40
    { L'\'', L'"', L'\0' }, { L'`', L'~', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\\', L'|', L'\0' }, // 44
    { L'z', L'Z', L'\0' }, { L'x', L'X', L'\0' }, { L'c', L'C', L'\0' }, { L'v', L'V', L'\0' }, // 48
    { L'b', L'B', L'\0' }, { L'n', L'N', L'\0' }, { L'm', L'M', L'\0' }, { L'<', L',', L'\0' }, // 52
    { L'.', L'>', L'\0' }, { L'/', L'?', L'\0' }, { L'\0', L'\0', L'\0' }, { L'*', L'*', L'\0' }, // 56
    { L'\0', L'\0', L'\0' }, { L' ', L' ', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 60
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 64
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 68
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'7', L'7', L'\0' }, // 72
    { L'8', L'8', L'\0' }, { L'9', L'9', L'\0' }, { L'-', L'-', L'\0' }, { L'4', L'4', L'\0' }, // 76
    { L'5', L'5', L'\0' }, { L'6', L'6', L'\0' }, { L'+', L'+', L'\0' }, { L'1', L'1', L'\0' }, // 80
    { L'2', L'2', L'\0' }, { L'3', L'3', L'\0' }, { L'0', L'0', L'\0' }, { L'.', L'.', L'\0' }, // 84
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 88
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 92
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 96
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 100
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 104
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 108
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 112
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 116
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 120
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 124
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 128
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 132
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 136
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 140
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 144
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 148
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 152
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 156
    { L'\n', L'\n', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 160
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 164
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 168
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 172
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 176
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 180
    { L'\0', L'\0', L'\0' }, { L'/', L'/', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 184
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 188
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 192
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 196
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 200
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 204
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 208
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 212
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 216
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 220
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 224
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 228
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 232
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 236
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 240
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 244
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 248
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, // 252
    { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' }, { L'\0', L'\0', L'\0' } // 256
};
// clang-format on
} // namespace

/**
 * 0x0040A0A0
 */
Keyboard::Keyboard() : m_modifiers(0), m_shiftExKey(0), m_inputFrame(0)
{
    memset(m_keys, 0, sizeof(m_keys));
    memset(m_keyStatus, 0, sizeof(m_keyStatus));
    memset(m_keyNames, 0, sizeof(m_keyNames));
}

/**
 * @brief Initialise the subsystem.
 *
 * 0x0040A120
 */
void Keyboard::Init()
{
    Init_Key_Names();
    m_inputFrame = 0;
}

/**
 * @brief Perform update logic for the subsystem.
 *
 * 0x0040A120
 */
void Keyboard::Update()
{
    ++m_inputFrame;
    Update_Keys();
}

/**
 * @brief Creates a message stream from the keyboard state.
 *
 * 0x00407F50
 */
void Keyboard::Create_Stream_Messages()
{
    if (g_theMessageStream != nullptr) {
        GameMessage *msg = nullptr;

        for (KeyboardIO *keyio = Get_First_Key(); keyio->key != 0; ++keyio) {
            if (keyio->state & KEY_STATE_DOWN) {
                msg = g_theMessageStream->Append_Message(GameMessage::MSG_RAW_KEY_DOWN);
                captainslog_dbgassert(msg != nullptr, "Unable to append key down message to stream");

            } else if (keyio->state & KEY_STATE_UP) {
                msg = g_theMessageStream->Append_Message(GameMessage::MSG_RAW_KEY_UP);
                captainslog_dbgassert(msg != nullptr, "Unable to append key up message to stream");

            } else {
                captainslog_debug("Unknown key state when creating msg stream");
            }

            if (msg != nullptr) {
                msg->Append_Int_Arg(keyio->key);
                msg->Append_Int_Arg(keyio->state);
            }
        }
    }
}

/**
 * @brief Get printable value for a given key.
 *
 * 0x0040A4A0
 */
wchar_t Keyboard::Get_Printable_Key(uint8_t key, int key_type)
{
    // This should be impossible.
    if (key >= KEY_COUNT) {
        return L'\0';
    }

    switch (key_type) {
        case STANDARD:
            return m_keyNames[key].std_key;
        case SHIFTED:
            return m_keyNames[key].shifted;
        case SHIFTEDEX:
            return m_keyNames[key].shifted_ex;
        default:
            break;
    }

    return L'\0';
}

/**
 * @brief Resets the key state.
 *
 * 0x0040A150
 */
void Keyboard::Reset_Keys()
{
    memset(m_keys, 0, sizeof(m_keys));
    memset(m_keyStatus, 0, sizeof(m_keyStatus));
    m_modifiers = 0;

    if (Get_Caps_State()) {
        m_modifiers |= MODIFIER_CAPS;
    }
}

/**
 * 0x0040A460
 */
bool Keyboard::Is_Shift() const
{
    return m_modifiers & MODIFIER_LSHIFT || m_modifiers & MODIFIER_RSHIFT || m_modifiers & MODIFIER_SHIFTEX;
}

/**
 * 0x0040A480
 */
bool Keyboard::Is_Ctrl() const
{
    return m_modifiers & MODIFIER_LCTRL || m_modifiers & MODIFIER_RCTRL;
}

/**
 * 0x0040A490
 */
bool Keyboard::Is_Alt() const
{
    return m_modifiers & MODIFIER_LALT || m_modifiers & MODIFIER_RALT;
}

/**
 * @brief Initialise the mapping of key values to the printable characters they correspond to.
 *
 * 0x00408180
 */
void Keyboard::Init_Key_Names()
{
    // Original code initialised based on language variable, but variable was never changed from default.
    memcpy(m_keyNames, g_defaultNames, sizeof(m_keyNames));
    m_shiftExKey = 0;
}

/**
 * @brief Update the state of the keys in our keyboard object.
 *
 * 0x00407FC0
 */
void Keyboard::Update_Keys()
{
    // Get the position of the first unused key.
    int pos = 0;
    while (true) {
        Get_Key(&m_keys[pos]);

        if (m_keys[pos].key == 0xFF) {
            Reset_Keys();
            pos = 0;
        }

        if (m_keys[pos].key != 0xFF) {
            if (m_keys[pos++].key == 0) {
                break;
            }
        }
    }

    for (int i = 0; m_keys[i].key != 0; ++i) {
        m_keyStatus[m_keys[i].key].state = m_keys[i].state;
        m_keyStatus[m_keys[i].key].status = m_keys[i].status;
        m_keyStatus[m_keys[i].key].sequence = m_inputFrame;

        if (m_keys[i].key == KEY_TAB) {
            if (m_keyStatus[KEY_LALT].state & KEY_STATE_DOWN || m_keyStatus[KEY_RALT].state & KEY_STATE_DOWN) {
                m_keys[i].status = KeyboardIO::STATUS_USED;
            }
        } else {
            if (m_keys[i].key == KEY_CAPITAL || m_keys[i].key == KEY_LCONTROL || m_keys[i].key == KEY_RCONTROL
                || m_keys[i].key == KEY_LSHIFT || m_keys[i].key == KEY_RSHIFT || m_keys[i].key == KEY_LALT
                || m_keys[i].key == KEY_RALT) {
                Translate_Key(m_keys[i].key);
            }
        }
    }

    Check_Key_Repeat();

    if (m_modifiers != 0) {
        for (int i = 0; m_keys[i].key != 0; ++i) {
            m_keys[i].state |= m_modifiers;
        }
    }
}

/**
 * @brief Translate keypresses to printable characters and handle modifier key presses.
 *
 * 0x0040A190
 */
wchar_t Keyboard::Translate_Key(wchar_t key)
{
    if (key >= KEY_COUNT) {
        return key;
    }

    switch (key) {
        case KEY_CAPITAL:
            if (m_keyStatus[key].status == KeyboardIO::STATUS_UNUSED) {
                if (m_keyStatus[key].state & KEY_STATE_DOWN) {
                    if (m_modifiers & MODIFIER_CAPS) {
                        m_modifiers &= ~MODIFIER_CAPS;
                    } else {
                        m_modifiers |= MODIFIER_CAPS;
                    }
                }

                m_keyStatus[key].status = KeyboardIO::STATUS_USED;
            }
            break;
        case KEY_LSHIFT:
            if (m_keyStatus[key].state & KEY_STATE_DOWN) {
                m_modifiers |= MODIFIER_LSHIFT;
            } else {
                m_modifiers &= ~MODIFIER_LSHIFT;
            }
            break;
        case KEY_RSHIFT:
            if (m_keyStatus[key].state & KEY_STATE_DOWN) {
                m_modifiers |= MODIFIER_RSHIFT;
            } else {
                m_modifiers &= ~MODIFIER_RSHIFT;
            }
            break;
        case KEY_LCONTROL:
            if (m_keyStatus[key].state & KEY_STATE_DOWN) {
                m_modifiers |= MODIFIER_LCTRL;
            } else {
                m_modifiers &= ~MODIFIER_LCTRL;
            }
            break;
        case KEY_RCONTROL:
            if (m_keyStatus[key].state & KEY_STATE_DOWN) {
                m_modifiers |= MODIFIER_RCTRL;
            } else {
                m_modifiers &= ~MODIFIER_RCTRL;
            }
            break;
        case KEY_LALT:
            if (m_keyStatus[key].state & KEY_STATE_DOWN) {
                m_modifiers |= MODIFIER_LALT;
            } else {
                m_modifiers &= ~MODIFIER_LALT;
            }
            break;
        case KEY_RALT:
            if (m_keyStatus[key].state & KEY_STATE_DOWN) {
                m_modifiers |= MODIFIER_RALT;
            } else {
                m_modifiers &= ~MODIFIER_RALT;
            }
            break;
        default:
            if (m_shiftExKey == key) {
                if (m_keyStatus[m_shiftExKey].state & KEY_STATE_DOWN) {
                    m_modifiers |= MODIFIER_SHIFTEX;
                } else {
                    m_modifiers &= ~MODIFIER_SHIFTEX;
                }
            } else {
                if (m_modifiers & MODIFIER_SHIFTEX) {
                    return m_keyNames[key].shifted_ex;
                }

                if (Is_Shift() || Get_Caps_State() && iswalpha(m_keyNames[key].std_key)) {
                    return m_keyNames[key].shifted;
                }

                return m_keyNames[key].std_key;
            }

            break;
    }

    return L'\0';
}

/**
 * @brief Check for a key being held down for the purpose of repeating a character.
 *
 * 0x004080F0
 */
bool Keyboard::Check_Key_Repeat()
{
    // Find first KeyboardIO with 0 key.
    int i;

    for (i = 0; m_keys[i].key != 0; ++i) {
    }

    // Check for repeat status.
    for (int j = 0; j < KEY_COUNT; ++j) {
        if (m_keyStatus[j].state & KEY_STATE_DOWN && m_inputFrame - m_keyStatus[j].sequence > KEY_REPEAT_DELAY) {
            m_keys[i].key = j;
            m_keys[i].state = KEY_STATE_AUTOREPEAT | KEY_STATE_DOWN;
            m_keys[i].status = 0;
            m_keys[i + 1].key = 0;

            // Update key input frame data.
            for (int k = 0; k < KEY_COUNT; ++k) {
                m_keyStatus[k].sequence = m_inputFrame;
            }
            m_keyStatus[j].sequence = m_inputFrame - 12;
            return true;
        }
    }

    return false;
}
