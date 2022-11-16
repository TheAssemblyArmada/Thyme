#include <captainslog.h>
#include <gtest/gtest.h>
#include <iostream>

#include "gamememory.h"

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    captains_settings_t captains_settings = { 0 };
    captains_settings.level = LOGLEVEL_DEBUG;
    captains_settings.console = true;
    captainslog_init(&captains_settings);

    Init_Memory_Manager();

    int result = RUN_ALL_TESTS();

    Shutdown_Memory_Manager();

    captainslog_deinit();

    return result;
}
