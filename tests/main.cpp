#include <captainslog.h>
#include <gtest/gtest.h>
#include <iostream>

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    captains_settings_t captains_settings = { 0 };
    captains_settings.level = LOGLEVEL_DEBUG;
    captains_settings.console = true;
    captainslog_init(&captains_settings);

    return RUN_ALL_TESTS();
}