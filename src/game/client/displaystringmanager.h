/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Virtual base class for managing DisplayString based classes.
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
#include "displaystring.h"
#include "subsysteminterface.h"

class DisplayStringManager : public SubsystemInterface
{
public:
    DisplayStringManager();
    virtual ~DisplayStringManager();

    virtual void Init() override {}
    virtual void Reset() override {}
    virtual void Update() override {}
    virtual DisplayString *New_Display_String() = 0;
    virtual void Free_Display_String(DisplayString *string) = 0;
    virtual DisplayString *Get_Group_Numeral_String(int num) = 0;
    virtual DisplayString *Get_Formation_Letter_String() = 0;

protected:
    void Link(DisplayString *string);
    void Unlink(DisplayString *string);

protected:
    DisplayString *m_stringList;
    DisplayString *m_currentCheckpoint;
};

#ifdef GAME_DLL
extern DisplayStringManager *&g_theDisplayStringManager;
#else
extern DisplayStringManager *g_theDisplayStringManager;
#endif
