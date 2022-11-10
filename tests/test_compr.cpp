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
#include "compressionmanager.h"
#include "refpack.h"
#include "zlibcompr.h"

#define WRITE_DATA 0

TEST(compression, refpack_encode)
{
    auto filepath = Utf8String(TESTDATA_PATH) + "/compr/uncompr.txt";
    BufferedFileClass src_file(filepath.Str());
    ASSERT_TRUE(src_file.Open(FM_READ));
    size_t src_size = src_file.Size();
    uint8_t *src_data = new uint8_t[src_size];
    src_file.Read(src_data, src_size);

    uint8_t *dst_data = new uint8_t[src_size + 8];
    uint32_t dst_size = RefPack_Compress(dst_data, src_data, src_size, false);
    EXPECT_GT(dst_size, 0);
    EXPECT_LT(dst_size, src_size);
#if WRITE_DATA
    auto dst_filepath = Utf8String(TESTDATA_PATH) + "/compr/refpack.data";
    BufferedFileClass dst_file(dst_filepath.Str());
    ASSERT_TRUE(dst_file.Open(FM_WRITE));
    CompressionType compr_type = COMPRESSION_EAR;
    dst_file.Write(CompressionManager::Get_Compression_FourCC(compr_type), 4);
    dst_file.Write((const void *)&src_size, sizeof(uint32_t));
    dst_file.Write(dst_data, dst_size);
#endif
    delete[] src_data;
    delete[] dst_data;
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
    uint32_t dst_size = RefPack_Compress(dst_data, src_data, src_size, true);
    EXPECT_GT(dst_size, 0);
    EXPECT_LT(dst_size, src_size);
#if WRITE_DATA
    auto dst_filepath = Utf8String(TESTDATA_PATH) + "/compr/refpack_quick.data";
    BufferedFileClass dst_file(dst_filepath.Str());
    ASSERT_TRUE(dst_file.Open(FM_WRITE));
    CompressionType compr_type = COMPRESSION_EAR;
    dst_file.Write(CompressionManager::Get_Compression_FourCC(compr_type), 4);
    dst_file.Write((const void *)&src_size, sizeof(uint32_t));
    dst_file.Write(dst_data, dst_size);
#endif
    delete[] src_data;
    delete[] dst_data;
}

class ZlibTest : public ::testing::TestWithParam<int>
{
public:
    void SetUp() override { m_level = GetParam(); }

protected:
    int m_level;
};

TEST_P(ZlibTest, encode)
{
    auto filepath = Utf8String(TESTDATA_PATH) + "/compr/uncompr.txt";
    BufferedFileClass src_file(filepath.Str());
    ASSERT_TRUE(src_file.Open(FM_READ));
    size_t src_size = src_file.Size();
    uint8_t *src_data = new uint8_t[src_size];
    src_file.Read(src_data, src_size);

    uint8_t *dst_data = new uint8_t[src_size + 8];
    uint32_t dst_size = Zlib_Compress(dst_data, src_data, src_size, m_level);
    EXPECT_GT(dst_size, 0);
    EXPECT_LT(dst_size, src_size);
#if WRITE_DATA
    Utf8String dst_filepath;
    dst_filepath.Format("%s/compr/zlib%i.data", TESTDATA_PATH, m_level);
    BufferedFileClass dst_file(dst_filepath.Str());
    ASSERT_TRUE(dst_file.Open(FM_WRITE));
    CompressionType compr_type = (CompressionType)(COMPRESSION_ZL1 + (m_level - 1));
    dst_file.Write(CompressionManager::Get_Compression_FourCC(compr_type), 4);
    dst_file.Write((const void *)&src_size, sizeof(uint32_t));
    dst_file.Write(dst_data, dst_size);
#endif
    delete[] src_data;
    delete[] dst_data;
}

INSTANTIATE_TEST_CASE_P(compression, ZlibTest, testing::Range(1, 10));

class DecodeTest : public ::testing::TestWithParam<std::string>
{
public:
    void SetUp() override { m_file = GetParam(); }

protected:
    std::string m_file;
    CompressionManager m_mngr;
};

TEST_P(DecodeTest, decode)
{
    auto filepath = Utf8String(TESTDATA_PATH) + "/compr/" + m_file.c_str();
    BufferedFileClass src_file(filepath.Str());
    ASSERT_TRUE(src_file.Open(FM_READ));
    size_t src_size = src_file.Size();
    uint8_t *src_data = new uint8_t[src_size];
    src_file.Read(src_data, src_size);

    EXPECT_TRUE(CompressionManager::Is_Data_Compressed(src_data, src_size));
    int dst_size = CompressionManager::Get_Uncompressed_Size(src_data, src_size);
    EXPECT_GT(dst_size, src_size);
    uint8_t *dst_data = new uint8_t[dst_size];
    EXPECT_EQ(CompressionManager::Decompress_Data(src_data, src_size, dst_data, dst_size), dst_size);
    delete[] src_data;
    delete[] dst_data;
}

std::string testfile_list[] = {
    "refpack.data",
    "refpack_quick.data",
#ifdef BUILD_WITH_ZLIB
    "zlib1.data",
    "zlib2.data",
    "zlib3.data",
    "zlib4.data",
    "zlib5.data",
    "zlib6.data",
    "zlib7.data",
    "zlib8.data",
    "zlib9.data",
#endif
};

INSTANTIATE_TEST_CASE_P(compression, DecodeTest, testing::ValuesIn(testfile_list));
