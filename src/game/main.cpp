/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Entry point and associated low level init code.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "main.h"
#include "audiomanager.h"
#include "copyprotect.h"
#include "cpudetect.h"
#include "crashhandler.h"
#include "critsection.h"
#include "gameengine.h"
#include "gamemain.h"
#include "gamememory.h"
#include "gitverinfo.h"
#include "imemanager.h"
#include "keyboard.h"
#include "memdynalloc.h"
#include "mempool.h"
#include "messagestream.h"
#include "stackdump.h"
#include "unicodestring.h"
#include "version.h"
#include "w3ddisplay.h"
#include "win32compat.h"
#include "win32mouse.h"
#include <algorithm>
#include <captainslog.h>
#include <cstdio>

using std::remove;
using std::rename;

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#ifdef BUILD_WITH_SDL2
#include <SDL.h>
#endif

#ifdef PLATFORM_WINDOWS
#include <direct.h>
#include <shellapi.h>
#include <windowsx.h>
#include <wingdi.h>
#include <winuser.h>

#ifndef GAME_DLL
// This pragma makes windows use the normal main function for GUI applications.
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif
#endif

#ifdef GAME_DLL
#include "hookcrt.h"
// Some globals, replace exe versions with own once all code that uses them is implemented.
extern bool &g_gameIsWindowed;
extern bool &g_gameNotFullscreen;
extern bool &g_gameActive;
extern bool &g_creatingWindow;
extern HGDIOBJ &g_splashImage;
extern HINSTANCE &g_applicationHInstance;
#else
#ifdef PLATFORM_WINDOWS
HWND g_applicationHWnd;
HGDIOBJ g_splashImage;
HINSTANCE g_applicationHInstance;
#endif
unsigned g_theMessageTime = 0;
bool g_gameIsWindowed;
bool g_gameNotFullscreen;
bool g_gameActive;
bool g_creatingWindow;
#endif

// Some Critical Sections to dole out to the functions that can use them.
SimpleCriticalSectionClass critSec1;
SimpleCriticalSectionClass critSec2;
SimpleCriticalSectionClass critSec3;

// New globals for extra features
int const c_invalidPos = -1000000;
int g_xPos = c_invalidPos;
int g_yPos = c_invalidPos;
bool g_noBorder = false;

/**
 * @brief Sets the working directory, some code currently expects data to be in this directory.
 */
inline void Set_Working_Directory()
{
#if defined(PLATFORM_WINDOWS)
    char path[MAX_PATH];

    GetModuleFileNameA(GetModuleHandleA(nullptr), path, sizeof(path));

    for (char *i = &path[strlen(path)]; i != path; --i) {
        if (*i == '\\' || *i == '/') {
            *i = '\0';
            break;
        }
    }

    SetCurrentDirectoryA(path);

#elif defined(PLATFORM_LINUX) // posix otherwise, really just linux currently
    // TODO /proc/curproc/file for FreeBSD /proc/self/path/a.out Solaris
    char path[PATH_MAX];
    readlink("/proc/self/exe", path, PATH_MAX);
    chdir(dirname(path));

#elif defined(PLATFORM_OSX) // osx otherwise
    char path[PATH_MAX];
    int size = PATH_MAX;
    _NSGetExecutablePath(path, &size);
    chdir(dirname(path));

#else //
#error Platform not supported for Set_Working_Directory()!
#endif // PLATFORM_WINDOWS
}

constexpr int WIN_WIDTH = 800;
constexpr int WIN_HEIGHT = 600;
constexpr const char *WIN_TITLE = "Thyme RTS Engine";
constexpr const char *WIN_SPLASH = "Install_Final.bmp";

/**
 * @brief Check the command line for early startup related flags.
 */
void Check_Windowed(int argc, char *argv[])
{
#ifdef PLATFORM_WINDOWS
    RECT Res;
    GetWindowRect(GetDesktopWindow(), &Res);
#endif
    for (int i = 0; i < argc && i < 20; ++i) {
        // DEBUG_LOG("Argument %d was %s\n", i, argv[i]);

        if (strcasecmp(argv[i], "-win") == 0) {
            g_gameIsWindowed = true;
        }

        if (strcasecmp(argv[i], "-noBorder") == 0) {
            g_noBorder = true;
        }

        if (strcasecmp(argv[i], "-xpos") == 0) {
            ++i;
            g_xPos = atoi(argv[i]);
#ifdef PLATFORM_WINDOWS
            g_xPos = std::clamp(g_xPos, 0, (int)(Res.right - WIN_WIDTH)); // Prevent negative values
#endif
        }

        if (strcasecmp(argv[i], "-ypos") == 0) {
            ++i;
            g_yPos = atoi(argv[i]);
#ifdef PLATFORM_WINDOWS
            g_yPos = std::clamp(g_yPos, 0, (int)(Res.bottom - WIN_HEIGHT)); // Prevent negative values
#endif
        }
    }
}

#if defined PLATFORM_WINDOWS && defined GAME_DLL
// zh: 0x00401000
static LRESULT __stdcall Wnd_Proc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param)
{
    if (g_theIMEManager != nullptr && g_theIMEManager->Service_IME_Message(window_handle, message, w_param, l_param)) {
        return g_theIMEManager->Result();
    }

    CopyProtect::checkForMessage(message, l_param);

    switch (message) {
        case WM_SETCURSOR:
            if (g_theWin32Mouse != nullptr) {
                g_theWin32Mouse->Set_Cursor(g_theWin32Mouse->Get_Mouse_Cursor());
            }
            return TRUE;

        case WM_SIZE:
            if (!g_creatingWindow) {
                g_gameNotFullscreen = false;
            }
            break;

        case WM_ACTIVATE:
            switch (LOWORD(w_param)) {
                case WA_INACTIVE:
                    ClipCursor(nullptr);
                    if (g_theAudio != nullptr) {
                        g_theAudio->Lose_Focus();
                    }
                    break;
                default:
                case WA_ACTIVE:
                case WA_CLICKACTIVE:
                    if (g_theMouse != nullptr) {
                        g_theMouse->Set_Mouse_Limits();
                    }
                    if (g_theAudio != nullptr) {
                        g_theAudio->Regain_Focus();
                    }
                    break;
            }
            break;

        case WM_SETFOCUS:
            if (g_theKeyboard != nullptr) {
                g_theKeyboard->Reset_Keys();
            }
            if (g_theWin32Mouse != nullptr) {
                g_theWin32Mouse->Set_Lost_Focus(false);
            }
            break;

        case WM_KILLFOCUS:
            if (g_theKeyboard != nullptr) {
                g_theKeyboard->Reset_Keys();
            }
            if (g_theWin32Mouse != nullptr) {
                g_theWin32Mouse->Set_Lost_Focus(true);
            }
            break;

        case WM_PAINT:
            if (g_gameNotFullscreen) {
                PAINTSTRUCT ps{};
                auto hDC = BeginPaint(window_handle, &ps);
                if (g_splashImage != nullptr) {
                    const auto old_dc = SaveDC(hDC);
                    auto compat_dc = CreateCompatibleDC(hDC);
                    auto ha = SelectObject(compat_dc, g_splashImage);
                    BitBlt(hDC, 0, 0, 800, 600, compat_dc, 0, 0, SRCCOPY);
                    SelectObject(compat_dc, ha);
                    DeleteDC(compat_dc);
                    RestoreDC(hDC, old_dc);
                }
                EndPaint(window_handle, &ps);
                return 0; // BUGFIX: vanilla returned 1 WinAPI says return 0
            }
            break;

        case WM_CLOSE:
            if (g_theGameEngine->Get_Quitting()) {
                if (g_theKeyboard != nullptr) {
                    g_theKeyboard->Reset_Keys();
                }
                if (g_theWin32Mouse != nullptr) {
                    g_theWin32Mouse->Set_Lost_Focus(false);
                }
                break;
            }
            g_theMessageStream->Append_Message(GameMessage::MessageType::MSG_META_DEMO_INSTANT_QUIT);
            return 0;

        case WM_QUERYENDSESSION:
            g_theMessageStream->Append_Message(GameMessage::MessageType::MSG_META_DEMO_INSTANT_QUIT);
            return 0;

        case WM_ERASEBKGND:
            if (!g_gameNotFullscreen) {
                return 1;
            }
            break;

        case WM_ACTIVATEAPP: {
            const bool activate_state = w_param == TRUE;
            if (activate_state == g_gameActive) {
                return 0;
            }
            g_gameActive = activate_state;
            if (g_theGameEngine != nullptr) {
                g_theGameEngine->Set_Is_Active(activate_state);
            }
            Reset_D3D_Device(activate_state);
            if (activate_state && g_theWin32Mouse != nullptr) {
                g_theWin32Mouse->Set_Cursor(g_theWin32Mouse->Get_Mouse_Cursor());
            }
            return 0;
        }
        case WM_NCHITTEST:
            if (!g_gameIsWindowed) {
                return HTCLIENT;
            }
            break;

        case WM_KEYDOWN:
            if (w_param == VK_ESCAPE) {
                PostQuitMessage(EXIT_SUCCESS);
            }
            return 0;

        case WM_SYSCOMMAND:
            switch (w_param) {
                case SC_MAXIMIZE:
                case SC_SIZE:
                case SC_MOVE:
                case SC_KEYMENU:
                case SC_MONITORPOWER:
                    if (!g_gameIsWindowed) {
                        return 0; // BUGFIX: vanilla returned 1 WinAPI says return 0
                    }
            }
            break;

        case WM_MOUSEMOVE: {
            RECT window_rect{};
            GetClientRect(g_applicationHWnd, &window_rect);
            POINT mouse_position{ GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param) };
            if (mouse_position.x < window_rect.left || mouse_position.x > window_rect.right
                || mouse_position.y < window_rect.top || mouse_position.y > window_rect.bottom) {
                return 0;
            }
            if (g_theWin32Mouse != nullptr) {
                g_theWin32Mouse->Add_Win32_Event(WM_MOUSEMOVE, w_param, l_param, g_theMessageTime);
            }
            return 0;
        }
        case WM_MOUSEWHEEL: {
            RECT window_rect{};
            // Note that this is different to MOUSEMOVE in that its the WindowRect
            GetWindowRect(g_applicationHWnd, &window_rect);
            POINT mouse_position{ GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param) };
            if (mouse_position.x < window_rect.left || mouse_position.x > window_rect.right
                || mouse_position.y < window_rect.top || mouse_position.y > window_rect.bottom) {
                return 0;
            }
            if (g_theWin32Mouse != nullptr) {
                g_theWin32Mouse->Add_Win32_Event(message, w_param, l_param, g_theMessageTime);
            }
            return 0;
        }
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK:
            if (g_theWin32Mouse != nullptr) {
                g_theWin32Mouse->Add_Win32_Event(message, w_param, l_param, g_theMessageTime);
            }
            return 0;

        case WM_POWERBROADCAST:
            switch (w_param) {
                case PBT_APMQUERYSUSPEND:
                    return TRUE;
                case PBT_APMRESUMESUSPEND:
                    return TRUE;
                default:
                    break;
            }
            break;
    }
    return DefWindowProcA(window_handle, message, w_param, l_param);
}
#endif

void Create_Window()
{
#if defined PLATFORM_WINDOWS && defined GAME_DLL
    WNDCLASSA WndClass;
    RECT Rect;
    HINSTANCE app_hinstance = GetModuleHandle(nullptr);
    STARTUPINFOA sinfo;
    int show_cmd;

    sinfo.dwFlags = 0;
    GetStartupInfoA(&sinfo);

    if (sinfo.dwFlags & STARTF_USESHOWWINDOW) {
        show_cmd = sinfo.wShowWindow;
    } else {
        show_cmd = SW_SHOWDEFAULT;
    }

    g_splashImage = LoadImageA(app_hinstance, WIN_SPLASH, 0, 0, 0, LR_LOADFROMFILE | LR_SHARED);
    bool is_windowed = g_gameIsWindowed;

    WndClass.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
    WndClass.lpfnWndProc = &Wnd_Proc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = app_hinstance;
    WndClass.hIcon = LoadIconA(app_hinstance, (LPCSTR)0x66);
    WndClass.hCursor = 0;
    WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    WndClass.lpszMenuName = nullptr;
    WndClass.lpszClassName = "Game Window";

    RegisterClassA(&WndClass);

    Rect.right = WIN_WIDTH;
    Rect.bottom = WIN_HEIGHT;
    Rect.left = 0;
    Rect.top = 0;

    uint32_t style = WS_POPUP | WS_VISIBLE | WS_EX_LAYERED;

    if (is_windowed && !g_noBorder) {
        style |= WS_BORDER | WS_EX_LAYOUTRTL;
    } else {
        style |= WS_EX_TOPMOST;
    }

    // DEBUG_LOG("Setting up window style of %08x\n", style);

    AdjustWindowRect(&Rect, style, 0);

    g_creatingWindow = true;

    // 0x90C80000
    // WS_POPUP | WS_VISIBLE | WS_BORDER | WS_EX_LAYOUTRTL | WS_EX_LAYERED

    // 0x90080008
    // WS_POPUP | WS_VISIBLE | WS_EX_LAYERED | WS_EX_TOPMOST

    if (g_xPos == c_invalidPos) {
        g_xPos = GetSystemMetrics(SM_CXSCREEN) / 2 - (WIN_WIDTH / 2);
    }

    if (g_yPos == c_invalidPos) {
        g_yPos = GetSystemMetrics(SM_CYSCREEN) / 2 - (WIN_HEIGHT / 2);
    }

    HWND app_hwnd = CreateWindowExA(0,
        "Game Window",
        WIN_TITLE,
        style,
        g_xPos,
        g_yPos,
        Rect.right - Rect.left,
        Rect.bottom - Rect.top,
        0,
        0,
        app_hinstance,
        0);

    if (is_windowed) {
        SetWindowPos(app_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    } else {
        SetWindowPos(app_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }

    SetFocus(app_hwnd);
    SetForegroundWindow(app_hwnd);
    ShowWindow(app_hwnd, show_cmd);
    UpdateWindow(app_hwnd);

    g_applicationHInstance = app_hinstance;
    g_applicationHWnd = app_hwnd;

    g_creatingWindow = false;

    if (!is_windowed) {
        g_gameNotFullscreen = false;
    }

    if (g_splashImage) {
        DeleteObject(g_splashImage);
        g_splashImage = 0;
    }
#endif
}

#ifdef BUILD_WITH_SDL2
namespace Thyme
{
bool Create_Window_SDL2()
{
    g_applicationWindow = nullptr;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        captainslog_error("SDL could not initialize!");
        return false;
    }

    SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "0");

    g_creatingWindow = true;

    bool is_windowed = g_gameIsWindowed;
    g_applicationWindow = SDL_CreateWindow(WIN_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIN_WIDTH,
        WIN_HEIGHT,
        is_windowed ? SDL_WINDOW_HIDDEN : SDL_WINDOW_HIDDEN | SDL_WINDOW_FULLSCREEN);

    if (g_applicationWindow == nullptr) {
        captainslog_error("SDL could not create window: %s", SDL_GetError());
        return false;
    }

    // Load the splashscreen
    SDL_Surface *splashSurface = SDL_LoadBMP(WIN_SPLASH);
    if (splashSurface == nullptr) {
        captainslog_error("SDL could not load splashscreen image: %s", SDL_GetError());
        return false;
    }

    // Get the window surface
    SDL_Surface *winSurface = SDL_GetWindowSurface(g_applicationWindow);
    if (winSurface == nullptr) {
        captainslog_error("SDL could not get window surface: %s", SDL_GetError());
        return false;
    }

    // Focus & show the window
    SDL_RaiseWindow(g_applicationWindow);
    SDL_ShowWindow(g_applicationWindow);

    // Blit the splashscreen to the window
    if (SDL_BlitSurface(splashSurface, NULL, winSurface, NULL) < 0) {
        captainslog_error("SDL could not blit splashscreen image: %s", SDL_GetError());
        return false;
    }
    if (SDL_UpdateWindowSurface(g_applicationWindow) < 0) {
        captainslog_error("SDL could not update window surface: %s", SDL_GetError());
        return false;
    }

    g_creatingWindow = false;
#ifdef PLATFORM_WINDOWS
    // Get module handle when on windows
    HINSTANCE app_hinstance = GetModuleHandle(nullptr);
    g_applicationHInstance = app_hinstance;
    // Get native window handle
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(g_applicationWindow, &wmInfo);
    g_applicationHWnd = wmInfo.info.win.window;
#endif
    return true;
}

void Destroy_Window_SDL2()
{
    if (g_applicationWindow)
        SDL_DestroyWindow(g_applicationWindow);

    g_applicationWindow = nullptr;
    SDL_Quit();
}
} // namespace Thyme
#endif

/**
 * @brief Entry point for the game engine.
 */
int main(int argc, char **argv)
{
    // Windows main can't take arguments as UTF8, so we need to overwrite them with the correct content.
    Handle_Win32_Args(&argc, &argv);
    Handle_Win32_Console();

    // Install crash handler
    bool crashes_handled = Setup_Crash_Handler();

    const char *logfile = nullptr;
#if LOGGING_LEVEL != LOGLEVEL_NONE
    char dirbuf[PATH_MAX];
    char curbuf[PATH_MAX];
    char prevbuf[PATH_MAX];

#ifdef PLATFORM_WINDOWS
    char *tmp = getenv("USERPROFILE");

    if (tmp != NULL) {
        strcpy(dirbuf, tmp);
        strcat(dirbuf, "\\Documents\\Command and Conquer Generals Zero Hour Data");
        mkdir(dirbuf);
        strcat(dirbuf, "\\");
    } else {
        GetModuleFileNameA(0, dirbuf, sizeof(dirbuf));

        // Get the path to the executable minus the actual filename.
        for (char *i = &dirbuf[strlen(dirbuf)]; i != dirbuf; --i) {
            if (*i == '\\' || *i == '/') {
                *i = '\0';
                break;
            }
        }
    }
#else
    char *homedir = getenv("HOME");
    if (homedir != nullptr) {
        strcpy(dirbuf, homedir);
    }

    if (homedir == nullptr) {
        homedir = getpwuid(getuid())->pw_dir;
        if (homedir != nullptr) {
            strcpy(dirbuf, homedir);
        }
    }

    if (homedir != nullptr) {
        strcat(dirbuf, "/Command and Conquer Generals Zero Hour Data");
    }
#endif
    const char *prefix = "";
    strcpy(prevbuf, dirbuf);
    strcat(prevbuf, prefix);
    strcat(prevbuf, "ThymeDebugLogPrev.txt");
    strcpy(curbuf, dirbuf);
    strcat(curbuf, prefix);
    strcat(curbuf, "ThymeDebugLogFile.txt");
    remove(prevbuf);
    rename(curbuf, prevbuf);
    logfile = curbuf;
#endif
    captains_settings_t captains_settings = { 0 };
    captains_settings.level = LOGLEVEL_DEBUG;
    captains_settings.filename = logfile;
    captains_settings.console = true;
    captains_settings.print_file = true;
    captainslog_init(&captains_settings);

    // Assign some critical sections for code sensitive to threaded calls.
    g_unicodeStringCriticalSection = &critSec1;
    g_dmaCriticalSection = &critSec2;
    g_memoryPoolCriticalSection = &critSec3;

    // Set working directory to the exe directory.
    // DEBUG_LOG("Setting working directory.\n");
    Set_Working_Directory();

    Check_Windowed(argc, argv);

    // Create the window
    captainslog_info("Creating window");
#ifdef BUILD_WITH_SDL2
    if (!Thyme::Create_Window_SDL2())
        return 1;
#else
    Create_Window();
#endif

    // DEBUG_LOG("Initialising memory manager.\n");
    Init_Memory_Manager();

    // Use of some of the version strings to use the git commit and branch stuff.
    // DEBUG_LOG("Initialising Version manager.\n");
    g_theVersion = new Version;
    g_theVersion->Set_Version(g_majorVersion, // Major
        g_minorVersion, // Minor
        g_patchVersion, // Patch
        g_gitCommitCount, // Internal build number
        g_gitBranch, // Git branch, was "location" in original build system
        g_gitShortSHA1, // Git commit, was "user" in original build system
        g_gitCommitTime, // Commit time
        g_gitCommitDate // Commit date
    );

    // Make pretty log header for debug logging builds.
    captainslog_line("================================================================================");
    captainslog_line("Thyme Version: %s", g_theVersion->Get_Ascii_Version().Str());
    captainslog_line("Build date: %s", g_theVersion->Get_Ascii_Build_Time().Str());
    captainslog_line("Build branch: %s", g_theVersion->Get_Ascii_Branch().Str());
    captainslog_line("Build commit: %s", g_theVersion->Get_Ascii_Commit_Hash().Str());
    // captainslog_line("Processor: %s\n", CPUDetectClass::Get_Processor_String());
    // captainslog_line("Physical Memory: %llu MiB.\n", CPUDetectClass::Get_Total_Physical_Memory() / (1024 * 1024 + 1));
    captainslog_line(CPUDetectClass::Get_Processor_Log());
    captainslog_line("================================================================================");

    if (!crashes_handled) {
        captainslog_warn("Crash handler failed to install correctly, crash logs will not be generated.");
    }

    captainslog_info("About to run Game_Main");
    Game_Main(argc, argv);
    captainslog_info("Game shutting down.");

    delete g_theVersion;
    g_theVersion = nullptr;

    Shutdown_Memory_Manager();

#ifdef BUILD_WITH_SDL2
    Thyme::Destroy_Window_SDL2();
#endif

    g_unicodeStringCriticalSection = nullptr;
    g_dmaCriticalSection = nullptr;
    g_memoryPoolCriticalSection = nullptr;

    captainslog_deinit();

    return 0;
}

#ifdef GAME_DLL
/**
 * @brief Wrapper for main to hook original entry point
 */
int __stdcall Main_Func(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Windows code will replace the arguments to main anyway so it doesn't matter what we do here.
    int ret = main(0, nullptr);

    return ret;
}
#endif
