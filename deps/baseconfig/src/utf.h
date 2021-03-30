/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Wrappers around unicode strings to do back and forward conversions on windows between UTF-8 and UTF-16.
 *
 * @copyright Baseconfig is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#ifndef BASE_UTF_H
#define BASE_UTF_H

// Only useful on windows when building a C++ project
#if defined _WIN32 && defined __cplusplus
#ifdef __WATCOMC__
#include <windows.h>
#else
#include <stringapiset.h>
#endif
#include <wchar.h>

#ifndef BASE_ALWAYS_H
#error This file should only be included after baseconfigs always.h
#endif

/**
 *  @brief Conversion class for interaction between UTF8 APIs and the WINAPI.
 * 
 *  This class is intended to be used as a wrapper for calling WINAPI unicode
 *  functions with UTF8 formatted strings to make them behave as if they took
 *  char* rather than wchar_t* while still allowing access to the full unicode
 *  character set.
 * 
 *  E.g Win32APIFuncW(UTF8To16(utf8_string));
 * 
 *  The class will act as a temporary here, doing the memory allocation and
 *  conversion for you on the fly with the destructor cleaning up after the
 *  function call when it goes out of scope.
 */
class UTF8To16
{
public:
    UTF8To16(const char *utf8) : m_buffer(nullptr)
    {
        int size = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
        m_buffer = new wchar_t[size];
        MultiByteToWideChar(CP_UTF8, 0, utf8, -1, m_buffer, size);
    }

    ~UTF8To16()
    {
        delete[] m_buffer;
        m_buffer = nullptr;
    }

    operator const wchar_t *() const { return m_buffer; }

private:
    wchar_t *m_buffer;
};

/**
 *  @brief Conversion class for interaction between UTF8 APIs and the WINAPI.
 * 
 *  This class is intended to be used as a wrapper for interpreting unicode
 *  strings provided by the WINAPI as UTF8 strings.
 * 
 *  E.g UTF8APIFunc(UTF16To8(utf16_string));
 * 
 *  The class will act as a temporary here, doing the memory allocation and
 *  conversion for you on the fly with the destructor cleaning up after the
 *  function call when it goes out of scope.
 */
class UTF16To8
{
public:
    UTF16To8(const wchar_t *utf16) : m_buffer(nullptr)
    {
        int size = WideCharToMultiByte(CP_UTF8, 0, utf16, -1, nullptr, 0, nullptr, nullptr);
        m_buffer = new char[size];
        WideCharToMultiByte(CP_UTF8, 0, utf16, -1, m_buffer, size, nullptr, nullptr);
    }

    ~UTF16To8()
    {
        delete[] m_buffer;
        m_buffer = nullptr;
    }

    operator const char *() const { return m_buffer; }

private:
    char *m_buffer;
};
#endif // _WIN32

#endif// BASE_UTF_H
