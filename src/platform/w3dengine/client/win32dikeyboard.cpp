/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Direct Input implementation of the Keyboard interface.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "win32dikeyboard.h"
#include "main.h"
#include <captainslog.h>

DirectInputKeyboard::DirectInputKeyboard() : m_inputInterface(nullptr), m_inputDevice(nullptr)
{
    if (GetKeyState(VK_CAPITAL)) {
        m_modifiers |= MODIFIER_CAPS;
    } else {
        m_modifiers &= ~MODIFIER_CAPS;
    }
}

DirectInputKeyboard::~DirectInputKeyboard()
{
    Close_Keyboard();
}

/**
 * Initialise the subsystem.
 */
void DirectInputKeyboard::Init()
{
    Keyboard::Init();
    Open_Keyboard();
}

/**
 * Checks if the caps lock is on.
 */
bool DirectInputKeyboard::Get_Caps_State()
{
    return (GetKeyState(VK_CAPITAL) & 1) != 0;
}

/**
 * Gets key state.
 */
void DirectInputKeyboard::Get_Key(KeyboardIO *io)
{
    io->key = 0;
    io->sequence = 0;

    if (m_inputDevice == nullptr) {
        return;
    }

    HRESULT res = m_inputDevice->Acquire();
    DIDEVICEOBJECTDATA data;
    DWORD count = 1; // Initial value is how many data objects we are filling.

    if (res == DI_OK || res == DI_NOTATTACHED) {
        res = m_inputDevice->GetDeviceData(sizeof(data), &data, &count, 0);
    }

    if (res == DIERR_INPUTLOST || res == DIERR_NOTACQUIRED) {
        res = m_inputDevice->Acquire();

        if (res == DI_OK || res == DI_NOTATTACHED) {
            io->key = 0xFF;
        }
    } else if (res == DI_OK && count != 0) {
        io->key = data.dwOfs;
        io->sequence = data.dwSequence;
        io->status = 0;
        io->state = (data.dwData & 0x80) == 0 ? KEY_STATE_UP : KEY_STATE_DOWN;
    }
}

/**
 * Internal init function.
 */
void DirectInputKeyboard::Open_Keyboard()
{
    if (FAILED(DirectInput8Create(g_applicationHInstance,
            DIRECTINPUT_VERSION,
            IID_IDirectInput8,
            reinterpret_cast<LPVOID *>(&m_inputInterface),
            nullptr))) {
        captainslog_error("DirectInput8Create failed.");
        Close_Keyboard();
        return;
    }

    if (FAILED(m_inputInterface->CreateDevice(GUID_SysKeyboard, &m_inputDevice, nullptr))) {
        captainslog_error("CreateDevice failed.");
        Close_Keyboard();
        return;
    }

    if (FAILED(m_inputDevice->SetDataFormat(&c_dfDIKeyboard))) {
        captainslog_error("Failed to set the data format.");
        Close_Keyboard();
        return;
    }

    if (FAILED(m_inputDevice->SetCooperativeLevel(g_applicationHWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND))) {
        captainslog_error("Failed to set cooperative level.");
        Close_Keyboard();
        return;
    }

    DIPROPDWORD prop;
    prop.diph.dwSize = sizeof(prop);
    prop.diph.dwHeaderSize = sizeof(prop.diph);
    prop.diph.dwObj = 0; // Must be 0 if dwHow == DIPH_DEVICE
    prop.diph.dwHow = DIPH_DEVICE;
    prop.dwData = 256;

    if (FAILED(m_inputDevice->SetProperty(DIPROP_BUFFERSIZE, (LPDIPROPHEADER)&prop))) {
        captainslog_error("Failed to set buffer size property.");
        Close_Keyboard();
        return;
    }

    m_inputDevice->Acquire();
    captainslog_info("DirectInput keyboard device initialised successfully.");
}

/**
 * Internal deinit function.
 */
void DirectInputKeyboard::Close_Keyboard()
{
    if (m_inputDevice != nullptr) {
        m_inputDevice->Unacquire();
        m_inputDevice->Release();
        m_inputDevice = nullptr;
    }

    if (m_inputInterface != nullptr) {
        m_inputInterface->Release();
        m_inputInterface = nullptr;
    }
}
