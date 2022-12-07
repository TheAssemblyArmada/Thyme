/**
 * @file
 *
 * @author feliwir
 *
 * @brief Set of tests to validate filesystem components
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include <gtest/gtest.h>
#include <win32bigfile.h>
#include <win32bigfilesystem.h>
#include <win32localfilesystem.h>
#ifdef BUILD_WITH_STDFS
#include <stdlocalfilesystem.h>
#endif

extern LocalFileSystem *g_theLocalFileSystem;

TEST(filesystem, win32bigfile)
{
    g_theLocalFileSystem = new Win32LocalFileSystem;

    Win32BIGFileSystem bigfilesystem;
    ArchiveFile *bigfile = bigfilesystem.Open_Archive_File((Utf8String(TESTDATA_PATH) + "/filesystem/test.big").Str());
    ASSERT_NE(bigfile, nullptr);

    char dst_buf[256];
    memset(dst_buf, 0, sizeof(dst_buf));

    // a.txt does exist
    File *file_a = bigfile->Open_File("a.txt", File::READ);
    ASSERT_NE(file_a, nullptr);
    EXPECT_EQ(file_a->Size(), 16);
    EXPECT_EQ(file_a->Read(dst_buf, sizeof(dst_buf)), 16);
    EXPECT_EQ(Utf8String(dst_buf), "This is sample A");
    memset(dst_buf, 0, sizeof(dst_buf));
    file_a->Close();

    // b.txt does not exist
    File *file_b = bigfile->Open_File("b.txt", File::READ);
    ASSERT_EQ(file_b, nullptr);
    // c.txt does exist
    File *file_c = bigfile->Open_File("c.txt", File::READ);
    ASSERT_NE(file_c, nullptr);
    EXPECT_EQ(file_c->Size(), 16);
    EXPECT_EQ(file_c->Read(dst_buf, sizeof(dst_buf)), 16);
    EXPECT_EQ(Utf8String(dst_buf), "This is sample C");
    file_c->Close();

    delete bigfile;
    delete g_theLocalFileSystem;
}

class FileSystemTest : public ::testing::TestWithParam<std::shared_ptr<LocalFileSystem>>
{
public:
    void SetUp() override { m_filesystem = GetParam().get(); }
    void TearDown() override {}

    struct PrintToStringParamName
    {
        template<class ParamType> std::string operator()(const testing::TestParamInfo<ParamType> &info) const
        {
            auto filesystem = std::dynamic_pointer_cast<LocalFileSystem>(info.param);

            if (std::dynamic_pointer_cast<Win32LocalFileSystem>(filesystem) != nullptr) {
                return "Win32LocalFileSystem";
            }
#ifdef BUILD_WITH_STDFS
            if (std::dynamic_pointer_cast<Thyme::StdLocalFileSystem>(filesystem) != nullptr) {
                return "StdLocalFileSystem";
            }
#endif
            return "Unknown";
        }
    };

protected:
    LocalFileSystem *m_filesystem;
};

TEST_P(FileSystemTest, exists_file)
{
    EXPECT_TRUE(m_filesystem->Does_File_Exist((Utf8String(TESTDATA_PATH) + "/filesystem/test.big").Str()));
    EXPECT_FALSE(m_filesystem->Does_File_Exist((Utf8String(TESTDATA_PATH) + "/filesystem/non_existant.big").Str()));
}

TEST_P(FileSystemTest, open_file)
{
    auto file = m_filesystem->Open_File((Utf8String(TESTDATA_PATH) + "/filesystem/test.big").Str(), File::READ);
    EXPECT_NE(file, nullptr);
    file->Close();
    EXPECT_EQ(m_filesystem->Open_File((Utf8String(TESTDATA_PATH) + "/filesystem/non_existant.big").Str(), File::READ), nullptr);
}

TEST_P(FileSystemTest, file_info)
{
    FileInfo file_info;
    EXPECT_TRUE(m_filesystem->Get_File_Info((Utf8String(TESTDATA_PATH) + "/filesystem/test.big").Str(), &file_info));
    EXPECT_EQ(file_info.file_size_low, 84);
}

TEST_P(FileSystemTest, list_dir_filtered)
{
    std::set<Utf8String, rts::less_than_nocase<Utf8String>> files;
    m_filesystem->Get_File_List_In_Directory((Utf8String(TESTDATA_PATH) + "/filesystem/").Str(), "", "*.big", files, true);
    EXPECT_EQ(files.size(), 1);
}

TEST_P(FileSystemTest, list_dir_empty_filter)
{
    std::set<Utf8String, rts::less_than_nocase<Utf8String>> files;
    m_filesystem->Get_File_List_In_Directory((Utf8String(TESTDATA_PATH) + "/filesystem/").Str(), "", "", files, true);
    EXPECT_EQ(files.size(), 0);
}

TEST_P(FileSystemTest, list_dir_unfiltered)
{
    std::set<Utf8String, rts::less_than_nocase<Utf8String>> files;
    m_filesystem->Get_File_List_In_Directory((Utf8String(TESTDATA_PATH) + "/filesystem/").Str(), "", "*", files, true);
    EXPECT_EQ(files.size(), 2);
}

std::shared_ptr<LocalFileSystem> filesystem_list[] = {
    std::make_shared<Win32LocalFileSystem>(),
#ifdef BUILD_WITH_STDFS
    std::make_shared<Thyme::StdLocalFileSystem>(),
#endif
};

INSTANTIATE_TEST_CASE_P(
    filesystem, FileSystemTest, testing::ValuesIn(filesystem_list), FileSystemTest::PrintToStringParamName());
