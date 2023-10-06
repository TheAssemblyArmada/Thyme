/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Display String Manager
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
#include "displaystringmanager.h"

class W3DDisplayStringManager : public DisplayStringManager
{
public:
    W3DDisplayStringManager();
    virtual ~W3DDisplayStringManager() override;

    virtual void PostProcessLoad() override;
    virtual void Update() override;
    virtual DisplayString *New_Display_String() override;
    virtual void Free_Display_String(DisplayString *string) override;
    virtual DisplayString *Get_Group_Numeral_String(int num) override;
    virtual DisplayString *Get_Formation_Letter_String() override;

private:
    enum
    {
        NUM_GROUP_STRINGS = 10
    };

    DisplayString *m_groupNumeralStrings[NUM_GROUP_STRINGS];
    DisplayString *m_formationLetterString;
};
