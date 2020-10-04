/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Setup the crash handling functions to generate crash reports.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "crashhandler.h"
#include "crashwrapper.h"
#include <cstdlib>
#include <gitverinfo.h>

#ifdef BUILD_WITH_CRASHPAD
#include <client/crash_report_database.h>
#include <client/crashpad_client.h>
#include <client/settings.h>
#endif

#if defined _WIN32 && defined GAME_DLL
#include "hookcrt.h"
#include "stackdump.h"
#endif

using std::getenv;

#if defined BUILD_WITH_CRASHPAD && defined _WIN32
void __cdecl Crashpad_Dump_Exception_Info(unsigned int u, struct _EXCEPTION_POINTERS *e_info)
{
    crashpad::CrashpadClient::DumpAndCrash(e_info);
}
#endif

bool Setup_Crash_Handler()
{
#ifdef BUILD_WITH_CRASHPAD
#ifdef _WIN32
    std::wstring homedir;
    std::wstring handler_path = L"thymecrashhandler.exe";
    homedir += _wgetenv(L"USERPROFILE");
    homedir += L"\\Documents\\Command and Conquer Generals Zero Hour Data\\CrashLogs";
#else
    std::string homedir;
    std::string handler_path = "thymecrashhandler";
    homedir += getenv("HOME");
    homedir += "/Documents/Command and Conquer Generals Zero Hour Data/CrashLogs";
#endif

    CrashPrefWrapper prefs;

    // Cache directory that will store crashpad information and minidumps
    base::FilePath database(homedir);
    // Path to the out-of-process handler executable
    base::FilePath handler(handler_path);
    // URL used to submit minidumps to
    std::string url = prefs.Upload_Allowed() ? prefs.Get_Upload_URL() : "";
    // Optional annotations passed via --annotations to the handler
    std::map<std::string, std::string> annotations = { { "commit", g_gitSHA1 } };
    // Optional arguments to pass to the handler
    std::vector<std::string> arguments;

    std::unique_ptr<crashpad::CrashReportDatabase> db = crashpad::CrashReportDatabase::Initialize(database);

    if (db != nullptr && db->GetSettings() != nullptr) {
        db->GetSettings()->SetUploadsEnabled(prefs.Upload_Allowed());
    }

    crashpad::CrashpadClient client;

    bool success = client.StartHandler(handler,
        database,
        database,
        url,
        annotations,
        arguments,
        /* restartable */ true,
        /* asynchronous_start */ false);

    // This will force it to dump from the original binary.
#if defined _WIN32 && defined GAME_DLL
    crt_set_se_translator(Crashpad_Dump_Exception_Info);
#endif

    return success;
#elif defined _WIN32 && defined GAME_DLL
    // Set the exception handler to the one provided by the EXE.
    // Only works on MSVC and only for SEH exceptions.
    // crt_set_se_translator(Make_Function_Ptr<void, unsigned int, struct _EXCEPTION_POINTERS *>(0x00416490));
    crt_set_se_translator(Dump_Exception_Info);
    //_set_se_translator(Dump_Exception_Info);

    return true;
#else
    return false;
#endif
}
