/**
 * @file
 *
 * @author xezon
 *
 * @brief Parabolic Ease class
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

class ParabolicEase
{
public:
    // Valid ease range is 0..1 where [in] is smaller or equal [1-out].

    ParabolicEase(float in, float out);

    void Set_Ease_Times(float in, float out);

    float operator()(float param);

private:
    float m_easeIn;
    float m_easeOut;
};
