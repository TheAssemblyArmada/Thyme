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
#include <audioeventrts.h>
#include <audiomanager.h>
#include <gtest/gtest.h>
#include <win32localfilesystem.h>
#ifdef BUILD_WITH_OPENAL
#include <alaudiomanager.h>
#endif
#include <chrono>
#include <thread>

extern LocalFileSystem *g_theLocalFileSystem;

class TestAudioEventInfo : public AudioEventInfo
{
public:
    TestAudioEventInfo()
    {
        m_eventName = "testevent";
        m_filename = Utf8String(TESTDATA_PATH) + "/audio/pcm1644m.wav";
        m_eventType = EVENT_SOUND;
    }
};

#ifdef BUILD_WITH_OPENAL
void test_audiomanager(Thyme::ALAudioManager &mngr)
{
    using namespace std::chrono_literals;

    auto empty = mngr.Get_Empty_Audio_Event();
    EXPECT_FALSE(mngr.Is_Playing_Already(empty));
    EXPECT_FALSE(mngr.Is_Valid_Audio_Event(empty));
    EXPECT_FALSE(mngr.Is_Music_Playing());

    auto filepath = Utf8String(TESTDATA_PATH) + "/audio/pcm1644m.wav";
    EXPECT_FLOAT_EQ(mngr.Get_File_Length_MS(filepath), 6687.28f);

    // Init would be the correct call, but we don't have any INIs we can load
    mngr.Open_Device();
    if (!mngr.Is_Device_Open())
        return;

    static int playing_handle = 0;
    TestAudioEventInfo test_info;
    AudioEventRTS *ev = new AudioEventRTS(Utf8String("testevent"));
    ev->Set_Event_Info_With_Filename(&test_info);
    ev->Set_Playing_Handle(playing_handle++);
    ev->Set_Next_Play_Portion(1);
    mngr.friend_Force_Play_Audio_Event(ev);
    EXPECT_TRUE(mngr.Is_Currently_Playing(ev->Get_Playing_Handle()));

    while (mngr.Is_Currently_Playing(ev->Get_Playing_Handle())) {
        std::this_thread::sleep_for(200ms);
        mngr.Process_Playing_List();
    }

    EXPECT_FALSE(mngr.Is_Currently_Playing(ev->Get_Playing_Handle()));

    mngr.Close_Device();
}

TEST(audio, alaudiomanager)
{
    g_theLocalFileSystem = new Win32LocalFileSystem;
    Thyme::ALAudioManager mngr;
    mngr.Set_Cache_Max_Size(0xFFFFFF);

    test_audiomanager(mngr);

    delete g_theLocalFileSystem;
}
#endif
