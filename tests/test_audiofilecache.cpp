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
#include <audioeventinfo.h>
#include <audioeventrts.h>
#include <gtest/gtest.h>
#include <win32localfilesystem.h>
#ifdef BUILD_WITH_FFMPEG
#include <ffmpegaudiofilecache.h>
#endif

extern LocalFileSystem *g_theLocalFileSystem;

#ifdef BUILD_WITH_FFMPEG
TEST(audio, ffmpegaudiofilecache)
{
    g_theLocalFileSystem = new Win32LocalFileSystem;
    Thyme::FFmpegAudioFileCache cache;

    EXPECT_EQ(cache.Get_Max_Size(), 0);
    EXPECT_EQ(cache.Get_Current_Size(), 0);

    uint8_t *data = cache.Open_File(Utf8String(TESTDATA_PATH) + "/audio/pcm1644m.wav");
    EXPECT_NE(data, nullptr);
}
#endif