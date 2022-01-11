/**
 * @file
 *
 * @author Duncans_pumpkin
 *
 * @brief Load Screen handling classes.
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

class GameWindow;
class GameInfo;

class LoadScreen
{
public:
    // wb: 0x009C3EF0
    LoadScreen() : m_loadScreen(nullptr) {}
    virtual ~LoadScreen();

    virtual void Init(GameInfo *game) = 0;
    virtual void Reset() = 0;
    virtual void Update() = 0;
    virtual void Update(int32_t percent);
    virtual void Process_Progress(int32_t playerId, int32_t percentage) = 0;

private:
    GameWindow *m_loadScreen;
};
