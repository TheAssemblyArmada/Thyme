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

    auto filepath = Utf8String(TESTDATA_PATH) + "/audio/pcm1644m.wav";
    void *data = cache.Open_File(filepath);
    // We expect this to fail since our cache size is 0
    EXPECT_EQ(data, nullptr);

    // Use something that can hold our file
    cache.Set_Max_Size(0xFFFFF);
    data = cache.Open_File(filepath);
    EXPECT_NE(data, nullptr);
    auto cache_size = cache.Get_Current_Size();
    EXPECT_NE(cache_size, 0);

    // Opening the same file a second time shouldn't increase cachesize
    data = cache.Open_File(filepath);
    EXPECT_EQ(cache_size, cache.Get_Current_Size());

    // Close the file twice, since we opened it twice
    cache.Close_File(data);
    cache.Close_File(data);

    EXPECT_EQ(cache.Free_Space(), cache_size);
    EXPECT_EQ(cache.Get_Current_Size(), 0);

    delete g_theLocalFileSystem;
}
#endif
