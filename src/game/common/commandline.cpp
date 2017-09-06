/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief Command line argument parser.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "commandline.h"
#include "archivefilesystem.h"
#include "globaldata.h"
#include "localfilesystem.h"
#include "version.h"
#include <cstring>
#include <sys/stat.h>

typedef int (*cmdparse_t)(char **, int);

struct CmdParseStruct
{
    const char *argument;
    cmdparse_t handler;
};

int Parse_Win(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_windowed = true;
    }

    return 1;
}

int Parse_Full_Screen(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_windowed = false;
    }

    return 1;
}

int Parse_Full_Version(char **argv, int argc)
{
    if (g_theVersion != nullptr && argc > 1) {
        g_theVersion->Set_Use_Full_Version(atoi(argv[1]) != 0);

        return 2;
    }

    return 1;
}

int Parse_XRes(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr && argc > 1) {
        g_theWriteableGlobalData->m_xResolution = atoi(argv[1]);

        return 2;
    }

    return 1;
}

int Parse_YRes(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr && argc > 1) {
        g_theWriteableGlobalData->m_yResolution = atoi(argv[1]);

        return 2;
    }

    return 1;
}

int Parse_Script_Debug(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_scriptDebug = true;
        g_theWriteableGlobalData->m_winCursors = true;
    }

    return 1;
}

int Parse_Particle_Edit(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_particleEdit = true;
        g_theWriteableGlobalData->m_winCursors = true;
        g_theWriteableGlobalData->m_windowed = true;
    }

    return 1;
}

int Parse_No_Shell_Map(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_shellMapOn = false;
    }

    return 1;
}

int Parse_No_Shaders(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_chipsetType = 1;
    }

    return 1;
}

int Parse_Quick_Start(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_playSizzle = false;
        g_theWriteableGlobalData->m_shellMapOn = false;
        g_theWriteableGlobalData->m_animateWindows = false;
    }

    return 1;
}

int Parse_Play_Stats(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr && argc > 1) {
        g_theWriteableGlobalData->m_playerStats = atoi(argv[1]);

        return 2;
    }

    return 1;
}

int Parse_Mod(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr && argc > 1) {
        AsciiString path = argv[1];

        // If its not an absolute path, make it relative to user data dir.
        if (!strchr(path.Str(), ':') && !path.Starts_With("/") && !path.Starts_With("\\")) {
            path.Format("%s%s", g_theWriteableGlobalData->m_userDataDirectory.Str(), argv[1]);
        }

        // Check if it exists
        if (g_theLocalFileSystem->Does_File_Exist(path.Str())) {
            struct stat st;

            // Check if its a directory or file and handle accordingly
            if (stat(path.Str(), &st) == 0) {
                if ((st.st_mode & S_IFDIR)) {
                    if (!path.Ends_With("/") && !path.Ends_With("\\")) {
                        path.Concat("/");
                    }

                    g_theWriteableGlobalData->m_userModDirectory = path;
                } else {
                    g_theWriteableGlobalData->m_userModFile = path;
                }
            }
        }

        return 2;
    }

    return 1;
}

// Parses the command line passed to the executable via argc and argv.
void Parse_Command_Line(int argc, char *argv[])
{
    // Arguments we support and the functions to parse them.
    CmdParseStruct _params[] = {
        { "-noshellmap", &Parse_No_Shell_Map },
        { "-win", &Parse_Win },
        { "-xres", &Parse_XRes },
        { "-yres", &Parse_YRes },
        { "-fullscreen", &Parse_Full_Screen },
        { "-fullVersion", &Parse_Full_Version },
        { "-particleEdit", &Parse_Particle_Edit },
        { "-scriptDebug", &Parse_Script_Debug },
        { "-playStats", &Parse_Play_Stats },
        { "-mod", &Parse_Mod },
        { "-noshaders", &Parse_No_Shaders },
        { "-quickstart", &Parse_Quick_Start }
    };

    // Starting with argument 1 (0 being the name of the binary in most cases)
    // compare the argument against the list of argument handlers and call
    // it if a match is found.
    int arg = 1;

    while (arg < argc) {
        bool parsed = false;

        for (unsigned int i = 0; i < ARRAY_SIZE(_params); ++i) {
            if (strlen(_params[i].argument) == strlen(argv[arg])
                && strncasecmp(argv[arg], _params[i].argument, strlen(_params[i].argument)) == 0) {
                arg += _params[i].handler(&argv[arg], argc - arg);
                parsed = true;
                break;
            }
        }

        if (!parsed) {
            ++arg;
        }
    }

    // Loads any mod big files that were specified on the command line.
    g_theArchiveFileSystem->Load_Mods();
}
