/**
 * @file
 *
 * @author feliwir
 *
 * @brief Set of tests to validate audio components
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include <audiomanager.h>
#include <gtest/gtest.h>
#include <win32localfilesystem.h>
#ifdef BUILD_WITH_OPENAL
#include <alaudiomanager.h>
#endif

extern LocalFileSystem *g_theLocalFileSystem;

void test_audiomanager(AudioManager &mngr)
{
    // Init would be the correct call, but we don't have any INIs we can load
    mngr.Open_Device();

    auto empty = mngr.Get_Empty_Audio_Event();
    EXPECT_FALSE(mngr.Is_Playing_Already(empty));
    EXPECT_FALSE(mngr.Is_Valid_Audio_Event(empty));
    EXPECT_FALSE(mngr.Is_Music_Playing());

    auto filepath = Utf8String(TESTDATA_PATH) + "/audio/pcm1644m.wav";
    EXPECT_FLOAT_EQ(mngr.Get_File_Length_MS(filepath), 6687.28);

    mngr.Close_Device();
}

#ifdef BUILD_WITH_OPENAL
TEST(audio, alaudiomanager)
{
    g_theLocalFileSystem = new Win32LocalFileSystem;
    Thyme::ALAudioManager mngr;
    mngr.Set_Cache_Max_Size(0xFFFFFF);

    test_audiomanager(mngr);

    delete g_theLocalFileSystem;
}
#endif
