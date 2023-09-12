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
#include <render2dsentence.h>
#include <gtest/gtest.h>

TEST(text, render)
{
    FontCharsClass font;
    font.Initialize_GDI_Font("Arial", 12, false);
}
