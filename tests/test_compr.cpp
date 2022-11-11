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

// Some special refpack mode that doesn't seem to be used by the game. Test it anyways :)
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

    delete[] src_data;
    delete[] dst_data;
}

class CompressionTest : public ::testing::TestWithParam<CompressionType>
{
public:
    void SetUp() override { m_type = GetParam(); }

    struct PrintToStringParamName
    {
        template<class ParamType> std::string operator()(const testing::TestParamInfo<ParamType> &info) const
        {
            auto type = static_cast<CompressionType>(info.param);
            return CompressionManager::Get_Compression_FourCC(type);
        }
    };

protected:
    CompressionType m_type;
};

TEST_P(CompressionTest, encode)
{
    auto filepath = Utf8String(TESTDATA_PATH) + "/compr/uncompr.txt";
    BufferedFileClass src_file(filepath.Str());
    ASSERT_TRUE(src_file.Open(FM_READ)) << "Failed to open: " << filepath.Str();
    size_t src_size = src_file.Size();
    uint8_t *src_data = new uint8_t[src_size];
    src_file.Read(src_data, src_size);

    uint32_t dst_size = CompressionManager::Get_Max_Compressed_Size(src_size, m_type);
    uint8_t *dst_data = new uint8_t[dst_size];
    dst_size = CompressionManager::Compress_Data(m_type, src_data, src_size, dst_data, dst_size);
    EXPECT_GT(dst_size, 0);
    EXPECT_LT(dst_size, src_size);
#if WRITE_DATA
    auto dst_filepath = Utf8String(TESTDATA_PATH) + "/compr/blob" + std::to_string(m_type).c_str() + ".data";
    BufferedFileClass dst_file(dst_filepath.Str());
    ASSERT_TRUE(dst_file.Open(FM_WRITE));
    dst_file.Write(dst_data, dst_size);
#endif
    delete[] src_data;
    delete[] dst_data;
}

TEST_P(CompressionTest, decode)
{
    auto filepath = Utf8String(TESTDATA_PATH) + "/compr/blob" + std::to_string(m_type).c_str() + ".data";
    BufferedFileClass src_file(filepath.Str());
    ASSERT_TRUE(src_file.Open(FM_READ)) << "Failed to open: " << filepath.Str();
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

CompressionType compression_types[] = {
    COMPRESSION_EAR,
#ifdef BUILD_WITH_ZLIB
    COMPRESSION_ZL1,
    COMPRESSION_ZL2,
    COMPRESSION_ZL3,
    COMPRESSION_ZL4,
    COMPRESSION_ZL5,
    COMPRESSION_ZL6,
    COMPRESSION_ZL7,
    COMPRESSION_ZL8,
    COMPRESSION_ZL9,
#endif
};

INSTANTIATE_TEST_CASE_P(
    compression, CompressionTest, testing::ValuesIn(compression_types), CompressionTest::PrintToStringParamName());
