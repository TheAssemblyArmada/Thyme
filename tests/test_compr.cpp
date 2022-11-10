/**
 * @file
 *
 * @author feliwir
 *
 * @brief Set of tests to validate the different compression algorithms
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include <gtest/gtest.h>

#include "always.h"
#include "asciistring.h"
#include "bufffileclass.h"
#include "refpack.h"
#include "zlibcompr.h"

#define WRITE_DATA 1

TEST(compression, refpack_encode)
{
    auto filepath = Utf8String(TESTDATA_PATH) + "/compr/uncompr.txt";
    BufferedFileClass src_file(filepath.Str());
    ASSERT_TRUE(src_file.Open(FM_READ));
    size_t src_size = src_file.Size();
    uint8_t *src_data = new uint8_t[src_size];
    src_file.Read(src_data, src_size);

    uint8_t *dst_data = new uint8_t[src_size + 8];
    int dst_size = RefPack_Compress(dst_data, src_data, src_size, false);
    EXPECT_GT(dst_size, 0);
    EXPECT_LT(dst_size, src_size);
#if WRITE_DATA
    auto dst_filepath = Utf8String(TESTDATA_PATH) + "/compr/refpack.data";
    BufferedFileClass dst_file(dst_filepath.Str());
    ASSERT_TRUE(dst_file.Open(FM_WRITE));
    dst_file.Write(dst_data, dst_size);
#endif
}

TEST(compression, refpack_encode_quick)
{
    auto filepath = Utf8String(TESTDATA_PATH) + "/compr/uncompr.txt";
    BufferedFileClass src_file(filepath.Str());
    ASSERT_TRUE(src_file.Open(FM_READ));
    size_t src_size = src_file.Size();
    uint8_t *src_data = new uint8_t[src_size];
    src_file.Read(src_data, src_size);

    uint8_t *dst_data = new uint8_t[src_size + 8];
    int dst_size = RefPack_Compress(dst_data, src_data, src_size, true);
    EXPECT_GT(dst_size, 0);
    EXPECT_LT(dst_size, src_size);
#if WRITE_DATA
    auto dst_filepath = Utf8String(TESTDATA_PATH) + "/compr/refpack_quick.data";
    BufferedFileClass dst_file(dst_filepath.Str());
    ASSERT_TRUE(dst_file.Open(FM_WRITE));
    dst_file.Write(dst_data, dst_size);
#endif
}

class ZLibEncodeTest : public ::testing::TestWithParam<int>
{
public:
    void SetUp() override { m_level = GetParam(); }

protected:
    int m_level;
};

TEST_P(ZLibEncodeTest, encode)
{
    auto filepath = Utf8String(TESTDATA_PATH) + "/compr/uncompr.txt";
    BufferedFileClass src_file(filepath.Str());
    ASSERT_TRUE(src_file.Open(FM_READ));
    size_t src_size = src_file.Size();
    uint8_t *src_data = new uint8_t[src_size];
    src_file.Read(src_data, src_size);

    uint8_t *dst_data = new uint8_t[src_size + 8];
    int dst_size = Zlib_Compress(dst_data, src_data, src_size, m_level);
    EXPECT_GT(dst_size, 0);
    EXPECT_LT(dst_size, src_size);
#if WRITE_DATA
    Utf8String dst_filepath;
    dst_filepath.Format("%s/compr/zlib%i.data", TESTDATA_PATH, m_level);
    BufferedFileClass dst_file(dst_filepath.Str());
    ASSERT_TRUE(dst_file.Open(FM_WRITE));
    dst_file.Write(dst_data, dst_size);
#endif
}

INSTANTIATE_TEST_CASE_P(zlib, ZLibEncodeTest, testing::Range(1, 10));
