////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: MAIN.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: User entry point.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#include "main.h"
#include "cpudetect.h"
#include "hooker.h"
#include "hookcrt.h"
#include "critsection.h"
#include "gamemain.h"
#include "gamememory.h"
#include "mempool.h"
#include "minmax.h"
#include "unicodestring.h"
#include "version.h"
#include "gamedebug.h"

//
// Some Critical Sections to dole out to the functions that can use them.
//
SimpleCriticalSectionClass critSec1;
SimpleCriticalSectionClass critSec2;
SimpleCriticalSectionClass critSec3;

//
// Pointers to functions yet to be implemented.
//
#define Exception_Handler_Ptr Make_Function_Ptr<void, unsigned int, struct _EXCEPTION_POINTERS *>(0x00416490)
#define WinProc_Ptr Make_StdCall_Ptr<LRESULT, HWND, UINT, WPARAM, LPARAM>(0x00401000)

//
// Some globals, replace exe versions with own once all code that uses them is implemented.
//
#define GameIsWindowed (Make_Global<bool>(0x00A27B0C))
#define GameNotFullscreen (Make_Global<bool>(0x009C0ACC))
#define CreatingWindow (Make_Global<bool>(0x00A27B1C))
#define SplashImage (Make_Global<HGDIOBJ>(0x00A27B20))
#define ApplicationHInstance (Make_Global<HINSTANCE>(0x00A27B04))

//
// New globals for extra features
//
int const c_invalidPos = -1000000;
int g_xPos = c_invalidPos;
int g_yPos = c_invalidPos;
bool g_noBorder = false;

#ifdef PLATFORM_WINDOWS
// Taken from http://alter.org.ua/docs/win/args/
// Needed while we still have WinMain.
PCHAR *CommandLineToArgvA(PCHAR CmdLine, int* _argc)
{
    PCHAR* argv;
    PCHAR  _argv;
    ULONG   len;
    ULONG   argc;
    CHAR   a;
    ULONG   i, j;

    BOOLEAN  in_QM;
    BOOLEAN  in_TEXT;
    BOOLEAN  in_SPACE;

    len = strlen(CmdLine);
    i = ((len + 2) / 2) * sizeof(PVOID) + sizeof(PVOID);

    argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
        i + (len + 2) * sizeof(CHAR));

    _argv = (PCHAR)(((PUCHAR)argv) + i);

    argc = 0;
    argv[argc] = _argv;
    in_QM = FALSE;
    in_TEXT = FALSE;
    in_SPACE = TRUE;
    i = 0;
    j = 0;

    while ( a = CmdLine[i] ) {
        if ( in_QM ) {
            if ( a == '\"' ) {
                in_QM = FALSE;
            } else {
                _argv[j] = a;
                j++;
            }
        } else {
            switch ( a ) {
                case '\"':
                    in_QM = TRUE;
                    in_TEXT = TRUE;
                    if ( in_SPACE ) {
                        argv[argc] = _argv + j;
                        argc++;
                    }
                    in_SPACE = FALSE;
                    break;
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    if ( in_TEXT ) {
                        _argv[j] = '\0';
                        j++;
                    }
                    in_TEXT = FALSE;
                    in_SPACE = TRUE;
                    break;
                default:
                    in_TEXT = TRUE;
                    if ( in_SPACE ) {
                        argv[argc] = _argv + j;
                        argc++;
                    }
                    _argv[j] = a;
                    j++;
                    in_SPACE = FALSE;
                    break;
            }
        }
        i++;
    }
    _argv[j] = '\0';
    argv[argc] = NULL;

    (*_argc) = argc;
    return argv;
}
#endif

//
// Set the working directory to be the location of the exe. Game currently
// relies on this for some file open operations.
//
inline void Set_Working_Directory(void)
{
#if defined(PLATFORM_WINDOWS)
    char path[MAX_PATH];

    GetModuleFileName(GetModuleHandle(nullptr), path, sizeof(path));

    for (char *i = &path[strlen(path)]; i != path; --i ) {
        if ( *i == '\\' || *i == '/' ) {
            *i = '\0';
            break;
        }
    }

    SetCurrentDirectory(path);

#elif defined(PLATFORM_LINUX) //posix otherwise, really just linux currently
    //TODO /proc/curproc/file for FreeBSD /proc/self/path/a.out Solaris
    char path[PATH_MAX];
    readlink("/proc/self/exe", path, PATH_MAX);
    chdir(dirname(path));

#elif defined(PLATFORM_OSX) //osx otherwise
    char path[PATH_MAX];
    int size = PATH_MAX;
    _NSGetExecutablePath(path, &size);
    chdir(dirname(path));

#else //
    COMPILER_ERROR("Platform not supported for Set_Working_Directory()!");
#endif // PLATFORM_WINDOWS
}

//
// Check the command line for the -win parameter. We need to know it
// earlier than the full command line parse. Also parses any arguments
// for the position of the window.
//
void Check_Windowed(int argc, char *argv[])
{
    RECT Res;
    GetWindowRect(GetDesktopWindow(), &Res);

    for ( int i = 0; i < argc && i < 20; ++i ) {
        //DEBUG_LOG("Argument %d was %s\n", i, argv[i]);

        if ( strcasecmp(argv[i], "-win") == 0 ) {
            GameIsWindowed = true;
        }

        if ( strcasecmp(argv[i], "-noBorder") == 0 ) {
            g_noBorder = true;
        }

        if ( strcasecmp(argv[i], "-xpos") == 0 ) {
            ++i;
            g_xPos = atoi(argv[i]);
            g_xPos = Clamp(g_xPos, 0, (int)(Res.right - 800));    // Prevent negative values
        }

        if ( strcasecmp(argv[i], "-ypos") == 0 ) {
            ++i;
            g_yPos = atoi(argv[i]);
            g_yPos = Clamp(g_yPos, 0, (int)(Res.bottom - 600));    // Prevent negative values
        }
    }
}

void Create_Window()
{
    WNDCLASSA WndClass;
    RECT Rect;
    HINSTANCE app_hinstance = GetModuleHandle(nullptr);
    STARTUPINFOA sinfo;
    int show_cmd;

    sinfo.dwFlags = 0;
    GetStartupInfoA(&sinfo);

    if ( sinfo.dwFlags & STARTF_USESHOWWINDOW ) {
        show_cmd = sinfo.wShowWindow;
    } else {
        show_cmd = SW_SHOWDEFAULT;
    }

    SplashImage = LoadImageA(app_hinstance, "Install_Final.bmp", 0, 0, 0, LR_LOADFROMFILE | LR_SHARED);
    bool is_windowed = GameIsWindowed;

    WndClass.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
    WndClass.lpfnWndProc = WinProc_Ptr;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = app_hinstance;
    WndClass.hIcon = LoadIconA(app_hinstance, (LPCSTR)0x66);
    WndClass.hCursor = 0;
    WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    WndClass.lpszMenuName = nullptr;
    WndClass.lpszClassName = "Game Window";

    RegisterClassA(&WndClass);

    Rect.right = 800;
    Rect.bottom = 600;
    Rect.left = 0;
    Rect.top = 0;

    uint32_t style = WS_POPUP | WS_VISIBLE | WS_EX_LAYERED;

    if ( is_windowed && !g_noBorder ) {
        style |= WS_BORDER | WS_EX_LAYOUTRTL;
    } else {
        style |= WS_EX_TOPMOST;
    }

    //DEBUG_LOG("Setting up window style of %08x\n", style);

    AdjustWindowRect(&Rect, style, 0);

    CreatingWindow = true;

    // 0x90C80000
    // WS_POPUP | WS_VISIBLE | WS_BORDER | WS_EX_LAYOUTRTL | WS_EX_LAYERED

    // 0x90080008
    // WS_POPUP | WS_VISIBLE | WS_EX_LAYERED | WS_EX_TOPMOST

    if ( g_xPos == c_invalidPos ) {
        g_xPos = GetSystemMetrics(SM_CXSCREEN) / 2 - 400;
    }

    if ( g_yPos == c_invalidPos ) {
        g_yPos = GetSystemMetrics(SM_CYSCREEN) / 2 - 300;
    }

    HWND app_hwnd = CreateWindowExA(
        0,
        "Game Window",
        "Thyme RTS Engine",
        style,
        g_xPos,
        g_yPos,
        Rect.right - Rect.left,
        Rect.bottom - Rect.top,
        0,
        0,
        app_hinstance,
        0
    );

    if ( is_windowed ) {
        SetWindowPos(app_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    } else {
        SetWindowPos(app_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }

    SetFocus(app_hwnd);
    SetForegroundWindow(app_hwnd);
    ShowWindow(app_hwnd, show_cmd);
    UpdateWindow(app_hwnd);

    ApplicationHInstance = app_hinstance;
    ApplicationHWnd = app_hwnd;

    CreatingWindow = false;

    if ( !is_windowed ) {
        GameNotFullscreen = false;
    }

    if ( SplashImage ) {
        DeleteObject(SplashImage);
        SplashImage = 0;
    }
}

// This will eventually be replaced by a standard int main(int arc, char *argv[]) function
int __stdcall Main_Func(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DEBUG_INIT(DEBUG_LOG_TO_FILE);
    //DEBUG_LOG("Running main().\n");

#ifdef PLATFORM_WINDOWS
    // Set the exception handler to the one provided by the EXE.
    // Only works on MSVC and only for SEH exceptions.
    crt_set_se_translator(Exception_Handler_Ptr);
#endif

    // Assign some critical sections for code sensitive to threaded calls.
    UnicodeStringCriticalSection = &critSec1;
    DmaCriticalSection = &critSec2;
    MemoryPoolCriticalSection = &critSec3;

    // Set working directory to the exe directory.
    //DEBUG_LOG("Setting working directory.\n");
    Set_Working_Directory();

    // Check command line for -win
#ifdef PLATFORM_WINDOWS
    int argc;
    char **argv = CommandLineToArgvA(GetCommandLineA(), &argc);
#endif
    Check_Windowed(argc, argv);

    // Create the window
    //DEBUG_LOG("Creating Window.\n");
    Create_Window();

    //DEBUG_LOG("Initialising memory manager.\n");
    Init_Memory_Manager();

    // Use of some of the version strings to use the git commit and branch stuff.
    //DEBUG_LOG("Initialising Version manager.\n");
    g_theVersion = new Version;
    g_theVersion->Set_Version(
        0,                          // Major
        1,                          // Minor
        0,                          // Patch
        THYME_COMMIT_COUNT,         // Internal build number
        THYME_BRANCH,               // Git branch, was "location" in original build system
        THYME_COMMIT_SHA1_SHORT,    // Git commit, was "user" in original build system
        __TIME__,                   // Build time
        __DATE__                    // Build date
    );

    // Make pretty log header for debug logging builds.
    DEBUG_LOG("================================================================================\n");
    DEBUG_LOG("Thyme Version: %s\n", g_theVersion->Get_Ascii_Version().Str());
    DEBUG_LOG("Build date: %s\n", g_theVersion->Get_Ascii_Build_Time().Str());
    DEBUG_LOG("Build branch: %s\n", g_theVersion->Get_Ascii_Branch().Str());
    DEBUG_LOG("Build commit: %s\n", g_theVersion->Get_Ascii_Commit_Hash().Str());
    DEBUG_LOG("Processor: %s\n", CPUDetectClass::Get_Processor_String());
    DEBUG_LOG("Physical Memory: %llu MiB.\n", CPUDetectClass::Get_Total_Physical_Memory() / (1024 * 1024 + 1));
    //DEBUG_LOG(CPUDetectClass::Get_Processor_Log());
    DEBUG_LOG("================================================================================\n");

    DEBUG_LOG("About to run Game_Main\n");
    Game_Main(argc, argv);
    DEBUG_LOG("Game shutting down.\n");

    delete g_theVersion;
    g_theVersion = nullptr;

    Shutdown_Memory_Manager();

    UnicodeStringCriticalSection = nullptr;
    DmaCriticalSection = nullptr;
    MemoryPoolCriticalSection = nullptr;

    DEBUG_STOP();

    return 0;
}
