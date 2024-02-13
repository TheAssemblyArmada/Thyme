/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Another string class.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "wwstring.h"
#include <cstdio>
#if defined PLATFORM_WINDOWS
#include <utf.h>
#endif

#ifndef GAME_DLL
FastCriticalSectionClass StringClass::m_mutex;
char StringClass::m_nullChar = '\0';
char *StringClass::m_emptyString = &m_nullChar;
unsigned StringClass::m_reserveMask = 0;
#endif
char StringClass::m_tempStrings[StringClass::MAX_TEMP_STRING][StringClass::MAX_TEMP_BYTES];

void StringClass::Get_String(size_t length, bool is_temp)
{
    if (!is_temp && length == 0) {
        m_buffer = m_emptyString;
        return;
    }

    char *string = nullptr;

    if (is_temp && length <= MAX_TEMP_LEN && m_reserveMask != ALL_TEMP_STRINGS_USED_MASK) {
        FastCriticalSectionClass::LockClass m(m_mutex);

        for (int i = 0; i < MAX_TEMP_STRING; ++i) {
            unsigned mask = 1 << i;

            if (!(m_reserveMask & mask)) {
                m_reserveMask |= mask;
                string = m_tempStrings[i] + sizeof(HEADER);

                Set_Buffer_And_Allocated_Length(string, MAX_TEMP_LEN);
                break;
            }
        }
    }

    if (string == nullptr) {
        if (length > 0) {
            Set_Buffer_And_Allocated_Length(Allocate_Buffer(length), length);
        } else {
            Free_String();
        }
    }
}

void StringClass::Resize(int new_len)
{
    int allocated_len = Get_Allocated_Length();

    if (new_len > allocated_len) {
        char *new_buffer = Allocate_Buffer(new_len);
        strcpy(new_buffer, m_buffer);
        Set_Buffer_And_Allocated_Length(new_buffer, new_len);
    }
}

void StringClass::Uninitialised_Grow(size_t new_len)
{
    size_t allocated_len = Get_Allocated_Length();

    if (new_len > allocated_len) {
        char *new_buffer = Allocate_Buffer(new_len);
        Set_Buffer_And_Allocated_Length(new_buffer, new_len);
    }

    Store_Length(0);
}

void StringClass::Free_String()
{
    if (m_buffer != m_emptyString) {
        intptr_t buffer_base = reinterpret_cast<intptr_t>(m_buffer - sizeof(StringClass::HEADER));
        intptr_t diff = buffer_base - reinterpret_cast<intptr_t>(m_tempStrings[0]);

        if (diff >= 0 && diff < MAX_TEMP_BYTES * MAX_TEMP_STRING) {
            m_buffer[0] = 0;
            FastCriticalSectionClass::LockClass m(m_mutex);

            intptr_t index = buffer_base / MAX_TEMP_BYTES;
            m_reserveMask &= ~(1 << index);
        } else {
            char *buffer = reinterpret_cast<char *>(buffer_base);
            delete[] buffer;
        }

        m_buffer = m_emptyString;
    }
}

int StringClass::Format_Args(const char *format, va_list &arg_list)
{
    char temp_buffer[512] = { 0 };
    int retval = vsnprintf(temp_buffer, 512, format, arg_list);

    *this = temp_buffer;

    return retval;
}

int StringClass::Format(const char *format, ...)
{
    va_list arg_list;
    va_start(arg_list, format);

    int retval = Format_Args(format, arg_list);

    va_end(arg_list);

    return retval;
}

bool StringClass::Copy_Wide(const unichar_t *source)
{
    if (source != nullptr) {
#if defined BUILD_WITH_ICU
        int32_t length;
        UErrorCode error = U_ZERO_ERROR;
        u_strToUTF8(nullptr, 0, &length, source, -1, &error);

        if (U_SUCCESS(error) && length > 0) {
            u_strToUTF8(Get_Buffer(length), length, nullptr, source, -1, &error);

            if (U_SUCCESS(error)) {
                return true;
            }
        }
#elif defined PLATFORM_WINDOWS
        BOOL unmapped;
        int length = WideCharToMultiByte(CP_UTF8, 0, source, -1, nullptr, 0, nullptr, &unmapped);

        if (length > 0) {
            WideCharToMultiByte(CP_UTF8, 0, source, -1, Get_Buffer(length), length, nullptr, nullptr);
            Store_Length(length - 1);
        }

        return !unmapped;
#endif
    }

    return false;
}
