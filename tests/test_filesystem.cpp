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
#include <stdlocalfilesystem.h>
#include <win32bigfile.h>
#include <win32bigfilesystem.h>
#include <win32localfilesystem.h>

extern LocalFileSystem *g_theLocalFileSystem;

TEST(filesystem, win32bigfile)
{
    g_theLocalFileSystem = new Win32LocalFileSystem;

    Win32BIGFileSystem bigfilesystem;
    ArchiveFile *bigfile = bigfilesystem.Open_Archive_File(Utf8String(TESTDATA_PATH) + "/test.big");
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

    // b.txt does not exist
    File *file_b = bigfile->Open_File("b.txt", File::READ);
    ASSERT_EQ(file_b, nullptr);
    // c.txt does exist
    File *file_c = bigfile->Open_File("c.txt", File::READ);
    ASSERT_NE(file_c, nullptr);
    EXPECT_EQ(file_c->Size(), 16);
    EXPECT_EQ(file_c->Read(dst_buf, sizeof(dst_buf)), 16);
    EXPECT_EQ(Utf8String(dst_buf), "This is sample C");

    delete g_theLocalFileSystem;
}

class FileSystemTest : public ::testing::TestWithParam<LocalFileSystem *>
{
    void SetUp() override { m_filesystem = GetParam(); }
    void TearDown() override { }

protected:
    LocalFileSystem *m_filesystem;
};

TEST_P(FileSystemTest, exists_file)
{
    EXPECT_TRUE(m_filesystem->Does_File_Exist(Utf8String(TESTDATA_PATH) + "/test.big"));
    EXPECT_FALSE(m_filesystem->Does_File_Exist(Utf8String(TESTDATA_PATH) + "/non_existant.big"));
}

TEST_P(FileSystemTest, open_file)
{
    EXPECT_NE(m_filesystem->Open_File(Utf8String(TESTDATA_PATH) + "/test.big", File::READ), nullptr);
    EXPECT_EQ(m_filesystem->Open_File(Utf8String(TESTDATA_PATH) + "/non_existant.big", File::READ), nullptr);
}

TEST_P(FileSystemTest, file_info)
{
    FileInfo file_info;
    EXPECT_TRUE(m_filesystem->Get_File_Info(Utf8String(TESTDATA_PATH) + "/test.big", &file_info));
    EXPECT_EQ(file_info.file_size_low, 84);
}

std::array<LocalFileSystem *, 2> filesystem_list = {
    new Win32LocalFileSystem,
    new Thyme::StdLocalFileSystem,
};

// TODO: enable pretty printing
INSTANTIATE_TEST_CASE_P(filesystem, FileSystemTest, ::testing::ValuesIn(filesystem_list));