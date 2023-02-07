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
#include <fstream>
#include <gtest/gtest.h>
#include <string>

#include "always.h"
#include "compressionmanager.h"
#include "refpack.h"
#include "zlibcompr.h"

#define WRITE_DATA 0

size_t get_filesize(std::ifstream &fin)
{
    fin.seekg(0, std::ios::end);
    auto size = fin.tellg();
    fin.seekg(0, std::ios::beg);
    return size;
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
            uint32_t fourcc = CompressionManager::Get_Compression_FourCC(type);
            char fourcc_str[4];
            memcpy(fourcc_str, &fourcc, sizeof(fourcc_str));
            return fourcc_str;
        }
    };

protected:
    CompressionType m_type;
};

TEST_P(CompressionTest, encode)
{
    auto filepath = std::string(TESTDATA_PATH) + "/compr/uncompr.txt";
    std::ifstream src_file(filepath, std::ios::binary);
    ASSERT_TRUE(src_file.good()) << "Failed to open: " << filepath;
    size_t src_size = get_filesize(src_file);
    std::unique_ptr<uint8_t[]> src_data(new uint8_t[src_size]);
    src_file.read(reinterpret_cast<char *>(src_data.get()), src_size);

    uint32_t dst_size = CompressionManager::Get_Max_Compressed_Size(src_size, m_type);
    std::unique_ptr<uint8_t[]> dst_data(new uint8_t[dst_size]);
    dst_size = CompressionManager::Compress_Data(m_type, src_data.get(), src_size, dst_data.get(), dst_size);
    EXPECT_GT(dst_size, 0);
    EXPECT_LT(dst_size, src_size);

#if WRITE_DATA
    auto dst_filepath = std::string(TESTDATA_PATH) + "/compr/blob" + std::to_string(m_type) + ".data";
    std::ofstream dst_file(dst_filepath, std::ios::binary);
    ASSERT_TRUE(dst_file.good()) << "Failed to open: " << dst_filepath;
    dst_file.write(reinterpret_cast<char *>(dst_data.get()), dst_size);
#endif
}

TEST_P(CompressionTest, decode)
{
    auto filepath = std::string(TESTDATA_PATH) + "/compr/blob" + std::to_string(m_type) + ".data";
    std::ifstream src_file(filepath, std::ios::binary);
    ASSERT_TRUE(src_file.good()) << "Failed to open: " << filepath;
    size_t src_size = get_filesize(src_file);
    std::unique_ptr<uint8_t[]> src_data(new uint8_t[src_size]);
    src_file.read(reinterpret_cast<char *>(src_data.get()), src_size);

    EXPECT_TRUE(CompressionManager::Is_Data_Compressed(src_data.get(), src_size));
    int dst_size = CompressionManager::Get_Uncompressed_Size(src_data.get(), src_size);
    EXPECT_GT(dst_size, src_size);
    std::unique_ptr<uint8_t[]> dst_data(new uint8_t[dst_size]);
    EXPECT_EQ(CompressionManager::Decompress_Data(src_data.get(), src_size, dst_data.get(), dst_size), dst_size);
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
