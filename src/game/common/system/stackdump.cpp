/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Functions for creating dumps of information on program crash.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "stackdump.h"
#include "cpudetect.h"
#include "main.h"
#include <captainslog.h>
#include <cinttypes>
#include <stdio.h>

#ifndef GAME_DLL
Utf8String g_exceptionFileBuffer;
#endif

#ifdef PLATFORM_WINDOWS

#if defined PROCESSOR_X86_64
#define PRIPTRSIZE "016"
#elif defined PROCESSOR_X86
#define PRIPTRSIZE "08"
#else
#define PRIPTRSIZE
#endif

#define STACK_SYMNAME_MAX 512
#define STACK_DEPTH_MAX 30

const char *g_errorCodes[22] = {
    "Error code: EXCEPTION_ACCESS_VIOLATION\r\r\nDescription: The thread tried to read from or write to a virtual address "
    "for which it does not have the appropriate access.",
    "Error code: EXCEPTION_ARRAY_BOUNDS_EXCEEDED\r\r\nDescription: The thread tried to access an array element that is out "
    "of bounds and the underlying hardware supports bounds checking.",
    "Error code: EXCEPTION_BREAKPOINT\r\r\nDescription: A breakpoint was encountered.",
    "Error code: EXCEPTION_DATATYPE_MISALIGNMENT\r\r\nDescription: The thread tried to read or write data that is "
    "misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte "
    "boundaries; 32-bit values on 4-byte boundaries, and so on.",
    "Error code: EXCEPTION_FLT_DENORMAL_OPERAND\r\r\nDescription: One of the operands in a floating-point operation is "
    "denormal. A denormal value is one that is too small to represent as a standard floating-point value.",
    "Error code: EXCEPTION_FLT_DIVIDE_BY_ZERO\r\r\nDescription: The thread tried to divide a floating-point value by a "
    "floating-point divisor of zero.",
    "Error code: EXCEPTION_FLT_INEXACT_RESULT\r\r\nDescription: The result of a floating-point operation cannot be "
    "represented exactly as a decimal fraction.",
    "Error code: EXCEPTION_FLT_INVALID_OPERATION\r\r\nDescription: Some strange unknown floating point operation was "
    "attempted.",
    "Error code: EXCEPTION_FLT_OVERFLOW\r\r\nDescription: The exponent of a floating-point operation is greater than the "
    "magnitude allowed by the corresponding type.",
    "Error code: EXCEPTION_FLT_STACK_CHECK\r\r\nDescription: The stack overflowed or underflowed as the result of a "
    "floating-point operation.",
    "Error code: EXCEPTION_FLT_UNDERFLOW\r\r\nDescription:\tThe exponent of a floating-point operation is less than the "
    "magnitude allowed by the corresponding type.",
    "Error code: EXCEPTION_ILLEGAL_INSTRUCTION\r\r\nDescription:\tThe thread tried to execute an invalid instruction.",
    "Error code: EXCEPTION_IN_PAGE_ERROR\r\r\nDescription:\tThe thread tried to access a page that was not present, and the "
    "system was unable to load the page. For example, this exception might occur if a network connection is lost while "
    "running a program over the network.",
    "Error code: EXCEPTION_INT_DIVIDE_BY_ZERO\r\r\nDescription: The thread tried to divide an integer value by an integer "
    "divisor of zero.",
    "Error code: EXCEPTION_INT_OVERFLOW\r\r\nDescription: The result of an integer operation caused a carry out of the most "
    "significant bit of the result.",
    "Error code: EXCEPTION_INVALID_DISPOSITION\r\r\nDescription: An exception handler returned an invalid disposition to "
    "the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception.",
    "Error code: EXCEPTION_NONCONTINUABLE_EXCEPTION\r\r\nDescription: The thread tried to continue execution after a "
    "noncontinuable exception occurred.",
    "Error code: EXCEPTION_PRIV_INSTRUCTION\r\r\nDescription: The thread tried to execute an instruction whose operation is "
    "not allowed in the current machine mode.",
    "Error code: EXCEPTION_SINGLE_STEP\r\r\nDescription: A trace trap or other single-instruction mechanism signaled that "
    "one instruction has been executed.",
    "Error code: EXCEPTION_STACK_OVERFLOW\r\r\nDescription: The thread used up its stack.",
    "Error code: ?????\r\r\nDescription: Unknown exception.",
};

unsigned int g_errorValues[] = { 0xC0000005,
    0xC000008C,
    0x80000003,
    0x80000002,
    0xC000008D,
    0xC000008E,
    0xC000008F,
    0xC0000090,
    0xC0000091,
    0xC0000092,
    0xC0000093,
    0xC000001D,
    0xC0000006,
    0xC0000094,
    0xC0000095,
    0xC0000026,
    0xC0000025,
    0xC0000096,
    0x80000004,
    0xC00000FD,
    0xFFFFFFFF };

static bool g_symbolInit;
static CONTEXT g_stackContext;

#if defined PROCESSOR_X86_64
static BOOL(__stdcall *SymCleanupPtr)(HANDLE);
static BOOL(__stdcall *SymGetSymFromAddrPtr)(HANDLE, DWORD64, PDWORD64, PIMAGEHLP_SYMBOL64);
static BOOL(__stdcall *SymInitializePtr)(HANDLE, PCSTR, BOOL);
static DWORD64(__stdcall *SymLoadModulePtr)(HANDLE, HANDLE, PCSTR, PCSTR, DWORD64, DWORD);
static DWORD(__stdcall *SymSetOptionsPtr)(DWORD);
static BOOL(__stdcall *SymUnloadModulePtr)(HANDLE, DWORD64);
static BOOL(__stdcall *StackWalkPtr)(DWORD,
    HANDLE,
    HANDLE,
    LPSTACKFRAME,
    PVOID,
    PREAD_PROCESS_MEMORY_ROUTINE,
    PFUNCTION_TABLE_ACCESS_ROUTINE,
    PGET_MODULE_BASE_ROUTINE,
    PTRANSLATE_ADDRESS_ROUTINE);
static PVOID(__stdcall *SymFunctionTableAccessPtr)(HANDLE, DWORD64);
static BOOL(__stdcall *SymGetLineFromAddrPtr)(HANDLE, DWORD64, PDWORD, PIMAGEHLP_LINE64);
static DWORD64(__stdcall *SymGetModuleBasePtr)(HANDLE, DWORD64);
#elif defined PROCESSOR_X86
static BOOL(__stdcall *SymCleanupPtr)(HANDLE);
static BOOL(__stdcall *SymGetSymFromAddrPtr)(HANDLE, DWORD, PDWORD, PIMAGEHLP_SYMBOL);
static BOOL(__stdcall *SymInitializePtr)(HANDLE, PCSTR, BOOL);
static DWORD(__stdcall *SymLoadModulePtr)(HANDLE, HANDLE, PCSTR, PCSTR, DWORD, DWORD);
static DWORD(__stdcall *SymSetOptionsPtr)(DWORD);
static BOOL(__stdcall *SymUnloadModulePtr)(HANDLE, DWORD);
static BOOL(__stdcall *StackWalkPtr)(DWORD,
    HANDLE,
    HANDLE,
    LPSTACKFRAME,
    PVOID,
    PREAD_PROCESS_MEMORY_ROUTINE,
    PFUNCTION_TABLE_ACCESS_ROUTINE,
    PGET_MODULE_BASE_ROUTINE,
    PTRANSLATE_ADDRESS_ROUTINE);
static PVOID(__stdcall *SymFunctionTableAccessPtr)(HANDLE, DWORD);
static BOOL(__stdcall *SymGetLineFromAddrPtr)(HANDLE, DWORD, PDWORD, PIMAGEHLP_LINE);
static DWORD(__stdcall *SymGetModuleBasePtr)(HANDLE, DWORD);
#endif

static void Init_DbgHelp()
{
#if defined PROCESSOR_X86_64
    static const char *_sym_functions[] = { "SymCleanup",
        "SymGetSymFromAddr64",
        "SymInitialize",
        "SymLoadModule64",
        "SymSetOptions",
        "SymUnloadModule64",
        "StackWalk",
        "SymFunctionTableAccess64",
        "SymGetLineFromAddr64",
        "SymGetModuleBase64" };
#elif defined PROCESSOR_X86
    static const char *_sym_functions[] = { "SymCleanup",
        "SymGetSymFromAddr",
        "SymInitialize",
        "SymLoadModule",
        "SymSetOptions",
        "SymUnloadModule",
        "StackWalk",
        "SymFunctionTableAccess",
        "SymGetLineFromAddr",
        "SymGetModuleBase" };
#endif

    static FARPROC *_sym_pointers[] = { (FARPROC *)&SymCleanupPtr,
        (FARPROC *)&SymGetSymFromAddrPtr,
        (FARPROC *)&SymInitializePtr,
        (FARPROC *)&SymLoadModulePtr,
        (FARPROC *)&SymSetOptionsPtr,
        (FARPROC *)&SymUnloadModulePtr,
        (FARPROC *)&StackWalkPtr,
        (FARPROC *)&SymFunctionTableAccessPtr,
        (FARPROC *)&SymGetLineFromAddrPtr,
        (FARPROC *)&SymGetModuleBasePtr };

    static bool _initialised = false;

    if (_initialised) {
        return;
    }

    _initialised = true;

    HMODULE dll_handle = LoadLibraryA("dbghelp.dll");

    if (dll_handle) {
        for (int i = 0; i < ARRAY_SIZE(_sym_pointers); ++i) {
            *_sym_pointers[i] = GetProcAddress(dll_handle, _sym_functions[i]);

            if (*_sym_pointers[i] == nullptr) {
                captainslog_debug("Exception Handler: Unable to load %s from dbghelp.dll.", _sym_functions[i]);
            }
        }
    } else {
        captainslog_debug("Exception Handler: Unable to load dbghelp.dll.");
    }
}

static void Get_Function_Details(void *pointer, char *name, char *filename, unsigned *linenumber, uintptr_t *address)
{
    Init_Symbol_Info();
    if (name) {
        strcpy(name, "<Unknown>");
    }

    if (filename) {
        strcpy(filename, "<Unknown>");
    }

    if (linenumber) {
        *linenumber = (unsigned)-1;
    }

    if (address) {
        *address = (uintptr_t)-1;
    }

    unsigned displacement = 0;

    HANDLE process = GetCurrentProcess();
    char symbol_buffer[sizeof(IMAGEHLP_SYMBOL) + STACK_SYMNAME_MAX];
    IMAGEHLP_SYMBOL *const symbol_bufferp = reinterpret_cast<IMAGEHLP_SYMBOL *>(symbol_buffer);
    memset(symbol_buffer, 0, sizeof(symbol_buffer));
    symbol_bufferp->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL) + STACK_SYMNAME_MAX;
    symbol_bufferp->MaxNameLength = STACK_SYMNAME_MAX;

    if (SymGetSymFromAddrPtr != nullptr
        && SymGetSymFromAddrPtr(
            process, reinterpret_cast<DWORD_PTR>(pointer), reinterpret_cast<PDWORD_PTR>(&displacement), symbol_bufferp)) {
        if (name != nullptr) {
            strcpy(name, symbol_bufferp->Name);
            strcat(name, "();");
        }

        if (SymGetLineFromAddrPtr != nullptr) {
            IMAGEHLP_LINE line;
            line.Key = 0;
            line.LineNumber = 0;
            line.SizeOfStruct = sizeof(line);
            line.FileName = 0;
            line.Address = 0;

            if (SymGetLineFromAddrPtr(
                    process, reinterpret_cast<DWORD_PTR>(pointer), reinterpret_cast<PDWORD>(&displacement), &line)) {
                if (filename != nullptr) {
                    strcpy(filename, line.FileName);
                }

                if (linenumber != nullptr) {
                    *linenumber = line.LineNumber;
                }

                if (address != nullptr) {
                    *address = line.Address;
                }
            }
        }
    }
}

static void Write_Stack_Line(void *address, void(__cdecl *callback)(const char *))
{
    static char filename[STACK_SYMNAME_MAX];
    static char funcname[PATH_MAX];
    static char dest[260];

    uintptr_t addr;
    unsigned line;

    Get_Function_Details(address, funcname, filename, &line, &addr);
    sprintf(dest, "  %s(%d) : %s 0x%" PRIPTRSIZE PRIXPTR "\n", filename, line, funcname, (uintptr_t)address);
    callback(dest);
}

void Uninit_Symbol_Info()
{
    if (g_symbolInit) {
        g_symbolInit = false;

        if (SymCleanupPtr != nullptr) {
            SymCleanupPtr(GetCurrentProcess());
        }
    }
}

BOOL Init_Symbol_Info()
{
    char drive[10];
    char pathname[PATH_MAX + 1];
    char directory[PATH_MAX + 1];

    if (g_symbolInit) {
        return true;
    }

    g_symbolInit = true;
    atexit(Uninit_Symbol_Info);
    Init_DbgHelp();

    if (SymSetOptionsPtr != nullptr) {
        SymSetOptionsPtr(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_OMAP_FIND_NEAREST);
        HANDLE process = GetCurrentProcess();
        GetModuleFileNameA(0, pathname, PATH_MAX);
        _splitpath(pathname, drive, directory, 0, 0);
        sprintf(pathname, "%s:\\%s", drive, directory);
        strcat(pathname, ";.;");

        if (SymInitializePtr != nullptr && SymInitializePtr(process, pathname, 0)) {
            GetModuleFileNameA(0, pathname, PATH_MAX);

            if (SymLoadModulePtr != nullptr && SymLoadModulePtr(process, 0, pathname, 0, 0, 0)) {
                return true;
            }

            if (SymCleanupPtr != nullptr) {
                SymCleanupPtr(process);
            }
        }
    }

    return false;
}

void Make_Stack_Trace(
    uintptr_t myeip, uintptr_t myesp, uintptr_t myebp, int skip_frames, void(__cdecl *callback)(char const *))
{
    BOOL carry_on = true;
    STACKFRAME stack_frame;
    HANDLE thread = GetCurrentThread();
    HANDLE process = GetCurrentProcess();

    Init_DbgHelp();
    memset(&g_stackContext, 0, sizeof(g_stackContext));
    memset(&stack_frame, 0, sizeof(stack_frame));

    g_stackContext.ContextFlags = CONTEXT_FULL;

    stack_frame.AddrPC.Mode = AddrModeFlat;
    stack_frame.AddrStack.Mode = AddrModeFlat;
    stack_frame.AddrFrame.Mode = AddrModeFlat;
    stack_frame.AddrPC.Offset = myeip;
    stack_frame.AddrStack.Offset = myesp;
    stack_frame.AddrFrame.Offset = myebp;

    callback("Call Stack:\n");

    if (StackWalkPtr != nullptr) {
        while (skip_frames-- != 0) {
            carry_on = StackWalkPtr(IMAGE_FILE_MACHINE_I386,
                process,
                thread,
                &stack_frame,
                nullptr,
                nullptr,
                SymFunctionTableAccessPtr,
                SymGetModuleBasePtr,
                nullptr);

            if (!carry_on) {
                break;
            }
        }

        for (int i = 0; carry_on && i < STACK_DEPTH_MAX; ++i) {
            carry_on = StackWalkPtr(IMAGE_FILE_MACHINE_I386,
                process,
                thread,
                &stack_frame,
                nullptr,
                nullptr,
                SymFunctionTableAccessPtr,
                SymGetModuleBasePtr,
                nullptr);

            if (carry_on) {
                Write_Stack_Line((void *)stack_frame.AddrPC.Offset, callback);
            }
        }
    }
}

void __cdecl Stack_Dump_Handler(const char *data)
{
    g_exceptionFileBuffer += data;
}

void __cdecl Dump_Exception_Info(unsigned int u, struct _EXCEPTION_POINTERS *e_info)
{
    Utf8String tmp;
    g_exceptionFileBuffer.Clear();
    unsigned int e_code = e_info->ExceptionRecord->ExceptionCode;

    if (e_code == EXCEPTION_ACCESS_VIOLATION) {
        tmp = "Exception is access violation\n";
    } else {
        tmp.Format("Exception code is %x\n", e_code);
    }

    g_exceptionFileBuffer += tmp;
#ifndef BUILD_EDITOR
    tmp.Format("main function at %" PRIPTRSIZE PRIXPTR "\n", main);
    g_exceptionFileBuffer += tmp;
#endif

    int error_index = 0;

    while (g_errorValues[error_index] != 0xFFFFFFFF) {
        if (g_errorValues[error_index] == e_code) {
            break;
        }

        ++error_index;
    }

    tmp.Format("%s\n", g_errorCodes[error_index]);
    g_exceptionFileBuffer += tmp;

    if (e_code == EXCEPTION_ACCESS_VIOLATION) {
        // This checks the kind of access vioation.
        switch (e_info->ExceptionRecord->ExceptionInformation[0]) {
            case 0: // Read violation
                tmp.Format("Access address:%" PRIPTRSIZE PRIXPTR " was read from.\n",
                    e_info->ExceptionRecord->ExceptionInformation[1]);
                break;
            case 1:
                tmp.Format("Access address:%" PRIPTRSIZE PRIXPTR " was written to.\n",
                    e_info->ExceptionRecord->ExceptionInformation[1]);
                break;
            case 8:
                tmp.Format("Access address:%" PRIPTRSIZE PRIXPTR " DEP violation.\n",
                    e_info->ExceptionRecord->ExceptionInformation[1]);
                break;
            default:
                tmp.Format("Access address:%" PRIPTRSIZE PRIXPTR " Unknown violation.\n",
                    e_info->ExceptionRecord->ExceptionInformation[1]);
                break;
        }

        g_exceptionFileBuffer += tmp;
    }

    Init_Symbol_Info();

    CONTEXT *ctext = e_info->ContextRecord;
#if defined PROCESSOR_X86_64
    tmp.Format("Exception occurred at %" PRIPTRSIZE PRIXPTR "\n", ctext->Rip);
#elif defined PROCESSOR_X86
    tmp.Format("Exception occurred at %" PRIPTRSIZE PRIXPTR "\n", ctext->Eip);
#endif
    g_exceptionFileBuffer += tmp;
    g_exceptionFileBuffer += "\n";

#if defined PROCESSOR_X86_64
    Make_Stack_Trace(ctext->Rip, ctext->Rsp, ctext->Rbp, 0, Stack_Dump_Handler);
#elif defined PROCESSOR_X86
    Make_Stack_Trace(ctext->Eip, ctext->Esp, ctext->Ebp, 0, Stack_Dump_Handler);
#endif
    tmp.Format("\nCPU %s, %d Mhz, Vendor: %s\n",
        CPUDetectClass::Get_Processor_String(),
        CPUDetectClass::Get_Processor_Speed(),
        CPUDetectClass::Get_Processor_Manufacturer_Name());
    g_exceptionFileBuffer += tmp;
    g_exceptionFileBuffer += "\nDetails:\n";
    g_exceptionFileBuffer += "\nRegister dump...\n";

#if defined PROCESSOR_X86_64
    tmp.Format("Rip:%" PRIPTRSIZE PRIXPTR "\tRsp:%" PRIPTRSIZE PRIXPTR "\tRbp:%" PRIPTRSIZE PRIXPTR "\n",
        ctext->Rip,
        ctext->Rsp,
        ctext->Rbp);
#elif defined PROCESSOR_X86
    tmp.Format("Eip:%" PRIPTRSIZE PRIXPTR "\tEsp:%" PRIPTRSIZE PRIXPTR "\tEbp:%" PRIPTRSIZE PRIXPTR "\n",
        ctext->Eip,
        ctext->Esp,
        ctext->Ebp);
#endif
    g_exceptionFileBuffer += tmp;

#if defined PROCESSOR_X86_64
    tmp.Format("Rax:%" PRIPTRSIZE PRIXPTR "\tRbx:%" PRIPTRSIZE PRIXPTR "\tRcx:%" PRIPTRSIZE PRIXPTR "\n",
        ctext->Rax,
        ctext->Rbx,
        ctext->Rcx);
#elif defined PROCESSOR_X86
    tmp.Format("Eax:%" PRIPTRSIZE PRIXPTR "\tEbx:%" PRIPTRSIZE PRIXPTR "\tEcx:%" PRIPTRSIZE PRIXPTR "\n",
        ctext->Eax,
        ctext->Ebx,
        ctext->Ecx);
#endif
    g_exceptionFileBuffer += tmp;

#if defined PROCESSOR_X86_64
    tmp.Format("Rdx:%" PRIPTRSIZE PRIXPTR "\tRsi:%" PRIPTRSIZE PRIXPTR "\tRdi:%" PRIPTRSIZE PRIXPTR "\n",
        ctext->Rdx,
        ctext->Rsi,
        ctext->Rdi);
#elif defined PROCESSOR_X86
    tmp.Format("Edx:%" PRIPTRSIZE PRIXPTR "\tEsi:%" PRIPTRSIZE PRIXPTR "\tEdi:%" PRIPTRSIZE PRIXPTR "\n",
        ctext->Edx,
        ctext->Esi,
        ctext->Edi);
#endif
    g_exceptionFileBuffer += tmp;

#if defined PROCESSOR_X86_64
    tmp.Format("R8 :%" PRIPTRSIZE PRIXPTR "\tR9 :%" PRIPTRSIZE PRIXPTR "\tR10:%" PRIPTRSIZE PRIXPTR "\n",
        ctext->R8,
        ctext->R9,
        ctext->R10);
    g_exceptionFileBuffer += tmp;

    tmp.Format("R11:%" PRIPTRSIZE PRIXPTR "\tR12:%" PRIPTRSIZE PRIXPTR "\tR13:%" PRIPTRSIZE PRIXPTR "\n",
        ctext->R11,
        ctext->R12,
        ctext->R13);
    g_exceptionFileBuffer += tmp;

    tmp.Format("R14:%" PRIPTRSIZE PRIXPTR "\tR15:%" PRIPTRSIZE PRIXPTR "\n", ctext->R14, ctext->R15);
    g_exceptionFileBuffer += tmp;
#endif

    tmp.Format("EFlags:%08X \n", ctext->EFlags);
    g_exceptionFileBuffer += tmp;

    tmp.Format("CS:%04x  SS:%04x  DS:%04x  ES:%04x  FS:%04x  GS:%04x\n",
        ctext->SegCs,
        ctext->SegSs,
        ctext->SegDs,
        ctext->SegEs,
        ctext->SegFs,
        ctext->SegGs);
    g_exceptionFileBuffer += tmp;

    g_exceptionFileBuffer += "\nFloating point status\n";
#if defined PROCESSOR_X86_64
    tmp.Format("     Control word: %08x\n", ctext->FltSave.ControlWord);
    g_exceptionFileBuffer += tmp;
    tmp.Format("      Status word: %08x\n", ctext->FltSave.StatusWord);
    g_exceptionFileBuffer += tmp;
    tmp.Format("         Tag word: %08x\n", ctext->FltSave.TagWord);
    g_exceptionFileBuffer += tmp;
    tmp.Format("     Error Offset: %08x\n", ctext->FltSave.ErrorOffset);
    g_exceptionFileBuffer += tmp;
    tmp.Format("   Error Selector: %08x\n", ctext->FltSave.ErrorSelector);
    g_exceptionFileBuffer += tmp;
    tmp.Format("      Data Offset: %08x\n", ctext->FltSave.DataOffset);
    g_exceptionFileBuffer += tmp;
    tmp.Format("    Data Selector: %08x\n", ctext->FltSave.DataSelector);
    g_exceptionFileBuffer += tmp;
#elif defined PROCESSOR_X86
    tmp.Format("     Control word: %08x\n", ctext->FloatSave.ControlWord);
    g_exceptionFileBuffer += tmp;
    tmp.Format("      Status word: %08x\n", ctext->FloatSave.StatusWord);
    g_exceptionFileBuffer += tmp;
    tmp.Format("         Tag word: %08x\n", ctext->FloatSave.TagWord);
    g_exceptionFileBuffer += tmp;
    tmp.Format("     Error Offset: %08x\n", ctext->FloatSave.ErrorOffset);
    g_exceptionFileBuffer += tmp;
    tmp.Format("   Error Selector: %08x\n", ctext->FloatSave.ErrorSelector);
    g_exceptionFileBuffer += tmp;
    tmp.Format("      Data Offset: %08x\n", ctext->FloatSave.DataOffset);
    g_exceptionFileBuffer += tmp;
    tmp.Format("    Data Selector: %08x\n", ctext->FloatSave.DataSelector);
    g_exceptionFileBuffer += tmp;
#endif

#if defined PROCESSOR_X86
    for (int i = 0; i < 8; ++i) {
        tmp.Format("ST%d : ", i);
        g_exceptionFileBuffer += tmp;

        for (int j = 0; j < 10; ++j) {
            tmp.Format("%02X", ctext->FloatSave.RegisterArea[i * 10 + j]);
            g_exceptionFileBuffer += tmp;
        }

        tmp.Format("   %+#.17e\n", *reinterpret_cast<double *>(&ctext->FloatSave.RegisterArea[i * 10]));
        g_exceptionFileBuffer += tmp;
    }
#endif

    g_exceptionFileBuffer += "EIP bytes dump...\n";

    char bytestr[32];
    char scratch[512];
    uint8_t *eip_pointer;

#if defined PROCESSOR_X86_64
    sprintf(scratch, "\nBytes at CS:RIP (%" PRIPTRSIZE PRIXPTR ")  : ", ctext->Rip);
    eip_pointer = reinterpret_cast<uint8_t *>(ctext->Rip);
#elif defined PROCESSOR_X86
    sprintf(scratch, "\nBytes at CS:EIP (%" PRIPTRSIZE PRIXPTR ")  : ", ctext->Eip);
    eip_pointer = reinterpret_cast<uint8_t *>(ctext->Eip);
#endif

    for (int i = 32; i != 0; --i) {
        if (IsBadReadPtr(eip_pointer, 1)) {
            strlcat_tpl(scratch, "?? ");
        } else {
            sprintf(bytestr, "%02X ", *eip_pointer);
            strlcat_tpl(scratch, bytestr);
        }

        ++eip_pointer;
    }

    strcat(scratch, "\n");
    g_exceptionFileBuffer += scratch;
    g_exceptionFileBuffer += "\nStack dump (* indicates possible code address) :\n";

    uintptr_t *esp_pointer;
    scratch[0] = '\0';
#if defined PROCESSOR_X86_64
    esp_pointer = reinterpret_cast<uintptr_t *>(ctext->Rsp);
#elif defined PROCESSOR_X86
    esp_pointer = reinterpret_cast<uintptr_t *>(ctext->Esp);
#endif

    for (int i = 0; i < 2048; ++i) {
        // If we can't read the address we don't know where we are.
        if (IsBadReadPtr(esp_pointer, 4)) {
            snprintf(scratch, sizeof(scratch), "%" PRIPTRSIZE PRIXPTR ": ????????\n", (uintptr_t)esp_pointer);
            g_exceptionFileBuffer += scratch;
            ++esp_pointer;
            continue;
        }

        // If we aren't in code, then we don't know where we are.
        if (IsBadCodePtr((FARPROC)*esp_pointer)) {
            snprintf(scratch,
                sizeof(scratch),
                "%" PRIPTRSIZE PRIXPTR ": %" PRIPTRSIZE PRIXPTR " DATA_PTR\n",
                (uintptr_t)esp_pointer,
                *esp_pointer);
            g_exceptionFileBuffer += scratch;
            ++esp_pointer;
            continue;
        }

        snprintf(scratch,
            sizeof(scratch),
            "%" PRIPTRSIZE PRIXPTR ": %" PRIPTRSIZE PRIXPTR "",
            (uintptr_t)esp_pointer,
            *esp_pointer);
        if (SymGetSymFromAddrPtr != nullptr) {
#if defined PROCESSOR_X86_64
            IMAGEHLP_SYMBOL64 sym;
            sym.SizeOfStruct = 256;
            sym.MaxNameLength = 128;
            sym.Size = 0;
            sym.Address = *esp_pointer;
            DWORD64 disp;
#elif defined PROCESSOR_X86
            IMAGEHLP_SYMBOL sym;
            sym.SizeOfStruct = 256;
            sym.MaxNameLength = 128;
            sym.Size = 0;
            sym.Address = *esp_pointer;
            DWORD disp;
#endif
            if (SymGetSymFromAddrPtr(GetCurrentProcess(), *esp_pointer, &disp, &sym) != 0) {
                char sym_scratch[256];
                snprintf(sym_scratch, sizeof(sym_scratch), " - %s + %" PRIPTRSIZE PRIXPTR "", sym.Name, disp);
                strlcat_tpl(scratch, sym_scratch);
            } else {
                strlcat_tpl(scratch, " *");
            }
        } else {
            strlcat_tpl(scratch, " *");
        }

        strlcat_tpl(scratch, "\n");
        g_exceptionFileBuffer += scratch;
        ++esp_pointer;
    }
}

#endif
