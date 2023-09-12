/**
 * @file
 *
 * @author feliwir
 *
 * @brief Set of tests to validate our text rendering
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include <gtest/gtest.h>
#include <render2dsentence.h>

TEST(text, render)
{
    FontCharsClass font;
    font.Initialize_GDI_Font("Arial", 12, false);
    if (!font.Is_Font("Arial", 12, false)) {
        captainslog_info("Failed to initialize font. Stopping test");
        return;
    }
    // Depending on which font is used as a fallback, there might be a slight difference
    EXPECT_NEAR(font.Get_Char_Height(), 18, 1);
    uint16_t buffer[18 * 18];
    font.Blit_Char('A', buffer, 18, 0, 0);
}
