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
#pragma once

#include "always.h"
#include "keyboard.h"
#include <new>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

class DirectInputKeyboard : public Keyboard
{
public:
    DirectInputKeyboard();
    virtual ~DirectInputKeyboard();

    virtual void Init() override;

    virtual bool Get_Caps_State() override;
    virtual void Get_Key(KeyboardIO *io) override;

#ifdef GAME_DLL
    DirectInputKeyboard *Hook_Ctor() { return new (this) DirectInputKeyboard; }
#endif
private:
    void Open_Keyboard();
    void Close_Keyboard();

private:
    LPDIRECTINPUT8A m_inputInterface;
    LPDIRECTINPUTDEVICE8A m_inputDevice;
};
