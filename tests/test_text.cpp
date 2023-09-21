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

#define GLYPH_DUMP 0

#if GLYPH_DUMP
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void DumpGlyph(FontCharsClass &font, char ch)
{
    int char_height = font.Get_Char_Height();
    int char_width = font.Get_Char_Width(ch);
    uint16_t *buffer = new uint16_t[char_width * char_height];
    font.Blit_Char(ch, buffer, char_width * sizeof(uint16_t), 0, 0);
    uint8_t *img = new uint8_t[char_width * char_height];
    int idx = 0;
    for (int idx = 0; idx < char_width * char_height; idx++) {
        // Extract the 4-bit value of the glyph
        uint8_t value = (buffer[idx] >> 12) << 4;
        img[idx] = value;
    }
    std::string img_name = "glyph_";
    img_name += ch;
    img_name += ".png";
    stbi_write_png(img_name.c_str(), char_width, char_height, 1, img, char_width);
    delete[] buffer;
    delete[] img;
}
#endif

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
#if GLYPH_DUMP
    DumpGlyph(font, 'A');
    DumpGlyph(font, 'g');
#endif
}
