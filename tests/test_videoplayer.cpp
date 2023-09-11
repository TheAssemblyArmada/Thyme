/**
 * @file
 *
 * @author feliwir
 *
 * @brief Set of tests to validate videoplayer implementations
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include <gtest/gtest.h>
#include <swvideobuffer.h>
#include <videostream.h>
#include <win32localfilesystem.h>
#ifdef BUILD_WITH_FFMPEG
#include <ffmpegvideoplayer.h>
#endif
#ifdef BUILD_WITH_OPENAL
#include <alaudiomanager.h>
#include <alaudiostream.h>
#endif

extern LocalFileSystem *g_theLocalFileSystem;

#ifdef BUILD_WITH_FFMPEG
TEST(video, ffmpegvideoplayer)
{
    g_theLocalFileSystem = new Win32LocalFileSystem;
    Thyme::FFmpegVideoPlayer player;
    // Create an audio manager for audio playback
#ifdef BUILD_WITH_OPENAL
    g_theAudio = new Thyme::ALAudioManager();
    g_theAudio->Open_Device();
    player.Initialise_FFmpeg_With_OpenAL();
#endif

    auto filepath = Utf8String(TESTDATA_PATH) + "/video/video.bik";
    auto file = g_theLocalFileSystem->Open_File(filepath.Str(), File::BINARY | File::READ);
    auto stream = player.Create_Stream(file);
    ASSERT_NE(stream, nullptr);
    EXPECT_EQ(stream->Frame_Index(), 0);
    EXPECT_EQ(stream->Frame_Count(), 71);
    EXPECT_EQ(stream->Height(), 120);
    EXPECT_EQ(stream->Width(), 96);

    const int count = stream->Frame_Count();
    VideoBuffer *buffer = new Thyme::SWVideoBuffer(W3DVideoBuffer::TYPE_X8R8G8B8);
    ASSERT_TRUE(buffer->Allocate(stream->Width(), stream->Height()));
    for (int idx = stream->Frame_Index(); idx < count;) {
        player.Update();
        if (stream->Is_Frame_Ready()) {
            EXPECT_EQ(stream->Frame_Index(), idx);
            stream->Render_Frame(buffer);
            stream->Next_Frame();
            idx++;
        } else {
            rts::Sleep_Ms(1);
        }
    }

    delete buffer;
    delete stream;
    delete g_theAudio;
    delete g_theLocalFileSystem;
}
#endif
