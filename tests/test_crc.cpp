/**
 * @file
 *
 * @author feliwir
 *
 * @brief Set of tests to validate the CRC32 implementation
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include <realcrc.h>
#include <gtest/gtest.h>

TEST(checksum, crc32)
{
    const char *data = "Hello world";
    uint32_t data_crc = 0x8BD69E52;

    EXPECT_EQ(CRC_Memory(data, strlen(data), 0), data_crc);
    EXPECT_EQ(CRC_String(data, 0), data_crc);
    EXPECT_NE(CRC_Stringi(data, 0), data_crc);

    const char *data_upper = "HELLO WORLD";
    uint32_t data_upper_crc = 0x87E5865B;

    EXPECT_EQ(CRC_Memory(data_upper, strlen(data_upper), 0), data_upper_crc);
    EXPECT_EQ(CRC_String(data_upper, 0), data_upper_crc);
    EXPECT_EQ(CRC_Stringi(data_upper, 0), data_upper_crc);
}
