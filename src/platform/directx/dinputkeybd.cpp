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
#include "dinputkeybd.h"
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
    DWORD data_fetched = 1; // Initial value is how many data objects we are filling.

    if (res == DI_OK || res == DI_NOTATTACHED) {
        res = m_inputDevice->GetDeviceData(sizeof(data), &data, &data_fetched, 0);
    }

    if (res == DIERR_INPUTLOST || res == DIERR_NOTACQUIRED) {
        res = m_inputDevice->Acquire();

        if (res == DI_OK || res == DI_NOTATTACHED) {
            io->key = 0xFF;
        }
    } else if (res == DI_OK && data_fetched != 0) {
        io->key = data.dwOfs;
        io->sequence = data.dwSequence;
        io->status = 0;
        io->state = !((data.dwData & 0x80) == 0) + 1;
    }
}

/**
 * Internal init function.
 */
void DirectInputKeyboard::Open_Keyboard()
{
    if (DirectInput8Create(
            GetModuleHandleA(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8A, (LPVOID *)&m_inputInterface, 0)
        < 0) {
        captainslog_error("DirectInput8Create failed.");
        Close_Keyboard();
        return;
    }

    if (m_inputInterface->CreateDevice(GUID_SysKeyboard, &m_inputDevice, 0) < 0) {
        captainslog_error("CreateDevice failed.");
        Close_Keyboard();
        return;
    }

    if (m_inputDevice->SetDataFormat(&c_dfDIKeyboard) < 0) {
        captainslog_error("Failed to set the data format.");
        Close_Keyboard();
        return;
    }

    if (m_inputDevice->SetCooperativeLevel(g_applicationHWnd, 6) < 0) {
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

    if (m_inputDevice->SetProperty(DIPROP_BUFFERSIZE, (LPDIPROPHEADER)&prop) < 0) {
        captainslog_error("Failed to set buffer size property.");
        Close_Keyboard();
        return;
    }

    if (m_inputDevice->Acquire() < 0) {
        captainslog_error("Failed to acquire device.");
    } else {
        captainslog_info("DirectInput keyboard device initialised successfully.");
    }
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
