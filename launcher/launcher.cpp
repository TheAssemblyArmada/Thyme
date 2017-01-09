////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: LAUNCHER.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: 
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
#include    <windows.h>
#include    <string>
#include    <stdio.h>

// Use WinMain and don't make a console window.
#pragma comment(linker, "/subsystem:windows /ENTRY:WinMainCRTStartup")

// Define the point to stall at while the DLL loads.
//#define EXE_ENTRY 0x00401700    // ZH WinMain
#define EXE_ENTRY 0x008E0778    // ZH real entry point

// Define the EXE to launch and the DLL to inject.
#define EXE_NAME "game.dat"     // ZH acutual binary (Original disc version only supported).
#define DLL_NAME "thyme.dll"    // Our reimplementation DLL.

char *Make_Args(const char *args)
{
    const char *argstart = args;
    static char argstring[_MAX_PATH * 2];
    
    // Stick the exe name on the front of the arg string.
    snprintf(argstring, sizeof(argstring), "%s %s", EXE_NAME, argstart);
	
    return argstring;
}

// Based on code from http://www.codeproject.com/Articles/4610/Three-Ways-to-Inject-Your-Code-into-Another-Proces
void Inject_Dll(const char *dllname, HANDLE hProcess)
{
    HANDLE hThread;
    char szLibPath[_MAX_PATH];  // Buffer to hold the name of the DLL (including full path!)
    void *pLibRemote;   // The address (in the remote process) where szLibPath will be copied to.
    DWORD hLibModule;   // Base address of loaded module.
    HMODULE hKernel32 = GetModuleHandleA("Kernel32");   //For the LoadLibraryA func.
    
    GetFullPathNameA(dllname, _MAX_PATH, szLibPath, NULL);
    
    // 1. Allocate memory in the remote process for szLibPath
    // 2. Write szLibPath to the allocated memory
    pLibRemote = VirtualAllocEx(hProcess, NULL, sizeof(szLibPath), MEM_COMMIT, PAGE_READWRITE);
    
    WriteProcessMemory(hProcess, pLibRemote, (void*)szLibPath, sizeof(szLibPath), NULL);
    
    // Load "dll" into the remote process by passing LoadLibraryA as the function
    // to run as a thread with CreateRemoteThread. Pass copied name of DLL as
    // the arguments to the function.
    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryA"), pLibRemote, 0, NULL);
    
    // Wait for the DLL to load and return.
    WaitForSingleObject(hThread, INFINITE);

    // Get handle of the loaded module
    GetExitCodeThread(hThread, &hLibModule);

    // Clean up
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pLibRemote, sizeof(szLibPath), MEM_RELEASE);
}

// Based on code snippet from https://opcode0x90.wordpress.com/2011/01/15/injecting-dll-into-process-on-load/
void Inject_Loader(const char *path, const char *dllname, char *args)
{
    STARTUPINFOA StartupInfo = {0};
    PROCESS_INFORMATION ProcessInformation;
    DWORD oldProtect;
    DWORD oldProtect2;
    char oldBytes[2];
    char checkBytes[2];
    static const char patchBytes[2] = { '\xEB', '\xFE' }; // JMP $-2
    SIZE_T memwritten;
    SIZE_T memread;

    // initialize the structures
    StartupInfo.cb = sizeof(StartupInfo);
 
    // attempt to load the specified target in suspended state
    if ( CreateProcessA(
        path,
        args,
        NULL,
        NULL,
        FALSE,
        CREATE_SUSPENDED,
        NULL,
        NULL,
        &StartupInfo,
        &ProcessInformation
        ) 
    ) {
        HANDLE hProcess(ProcessInformation.hProcess);

        // wait for the process to done
        try {
            // locate the entry point
            LPVOID entry = (LPVOID)EXE_ENTRY;

            // patch the entry point with infinite loop
            VirtualProtectEx(hProcess, entry, 2, PAGE_EXECUTE_READWRITE, &oldProtect);
            ReadProcessMemory(hProcess, entry, oldBytes, 2, &memread);
            WriteProcessMemory(hProcess, entry, patchBytes, 2, &memwritten);
            VirtualProtectEx(hProcess, entry, 2, oldProtect, &oldProtect2);

            // resume the main thread
            ResumeThread(ProcessInformation.hThread);

            // wait until the thread stuck at entry point
            CONTEXT context;
            
            for ( unsigned int i = 0; i < 50 && context.Eip != (DWORD)entry; ++i ) {
                // patience.
                Sleep(100);

                // read the thread context
                context.ContextFlags = CONTEXT_CONTROL;
                GetThreadContext(ProcessInformation.hThread, &context);
            }
            
            if ( context.Eip != (DWORD)entry ) {
                // wait timed out, we never got to the entry point :/
                throw "entry point blockade timed out";
            }

            // inject DLL payload into remote process
			Inject_Dll(dllname, hProcess);

            // pause and restore original entry point unless DLL init overwrote
            // it already.
            SuspendThread(ProcessInformation.hThread);
            VirtualProtectEx(hProcess, entry, 2, PAGE_EXECUTE_READWRITE, &oldProtect);
            ReadProcessMemory(hProcess, entry, checkBytes, 2, &memread);
            
			// Check entry point is still patched to infinite loop. We don't
            // want to mess up any patching the DLL did.
            if ( memcmp(checkBytes, patchBytes, 2) == 0 ) {
                WriteProcessMemory(hProcess, entry, oldBytes, 2, &memwritten);
            }
            
            VirtualProtectEx(hProcess, entry, 2, oldProtect, &oldProtect2);
            
            MessageBox(NULL, "Attach debugger or continue.", "game.dat Debug Time!", MB_OK|MB_SERVICE_NOTIFICATION);
            
            // you are ready to go
            ResumeThread(ProcessInformation.hThread);
        } catch ( ... ) {
            // terminate the newly spawned process
            TerminateProcess(hProcess, -1);

            // rethrow the exception to top-level handler
            throw;
        }
    } else {
        // are you sure this is a valid target ?
        throw "unable to load the specified executable";
    }
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Inject_Loader(EXE_NAME, DLL_NAME, Make_Args(lpCmdLine));
    
    return 0;
}
