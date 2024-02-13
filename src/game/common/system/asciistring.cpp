/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for handling strings that have a series of bytes as a code point.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "asciistring.h"
#include "endiantype.h"
#include "errorcodes.h"
#include "memdynalloc.h"
#include "unicodestring.h"
#include <captainslog.h>
#include <cctype>
#include <cstdio>
#if defined PLATFORM_WINDOWS
#include <utf.h>
#endif

Utf8String const Utf8String::s_emptyString(nullptr);

/**
 * Default constructor added for convenience
 */
Utf8String::Utf8String() : m_data(nullptr) {}

/**
 * Initializes this string with an existing string (copy) and increments reference count.
 */
Utf8String::Utf8String(Utf8String const &string) : m_data(string.m_data)
{
    if (m_data != nullptr) {
        m_data->Inc_Ref_Count();
    }
}

/**
 * Initializes this string with a reference to the start of a char array.
 */
Utf8String::Utf8String(const char *s) : m_data(nullptr)
{
    if (s != nullptr) {
        // Get length of the string that was passed
        const size_type len = static_cast<size_type>(strlen(s));
        if (len > 0) {
            // Allocate a buffer
            Ensure_Unique_Buffer_Of_Size(len + 1, false, s);
        }
    }
}

/**
 * A utility method to test nullptr on string content and log if that happens
 */
const char *Utf8String::Peek() const
{
    captainslog_dbgassert(m_data != nullptr, "null string ptr");

    return m_data->Peek();
}

/**
 * A utility method to test nullptr on string content and log if that happens
 */
char *Utf8String::Peek()
{
    captainslog_dbgassert(m_data != nullptr, "null string ptr");

    return m_data->Peek();
}

/**
 * Internal shorthand to decrement the reference count and release buffer if 0
 */
void Utf8String::Release_Buffer()
{
    Validate();

    if (m_data != nullptr) {
        m_data->Dec_Ref_Count();
        if (m_data->ref_count == 0) {
            Free_Bytes();
        }
        m_data = nullptr;
    }

    Validate();
}

/**
 * Frees the allocated memory for this string. Thanks for all the fish!
 */
void Utf8String::Free_Bytes()
{
    g_dynamicMemoryAllocator->Free_Bytes(m_data);
}

/**
 * Allocates a memory buffer for a string's content.
 */
void Utf8String::Ensure_Unique_Buffer_Of_Size(
    size_type chars_needed, bool keep_data, const char *str_to_cpy, const char *str_to_cat)
{
    Validate();

    if (m_data != nullptr && m_data->ref_count == 1 && m_data->num_chars_allocated >= chars_needed) {
        if (str_to_cpy != nullptr) {
            // #BUGFIX Originally uses strcpy here. Use memmove to support overlaps gracefully.
            captainslog_dbgassert(strlen(str_to_cpy) == chars_needed - 1, "Length does not match");
            memmove(Peek(), str_to_cpy, chars_needed * sizeof(value_type));
        }

        if (str_to_cat != nullptr) {
            strcat(Peek(), str_to_cat);
        }

    } else {
        const int required_size = chars_needed + sizeof(AsciiStringData);

        captainslog_relassert(required_size <= MAX_LEN, CODE_02, "Size exceeds max len");

        const int alloc_size = g_dynamicMemoryAllocator->Get_Actual_Allocation_Size(required_size);
        AsciiStringData *new_data =
            reinterpret_cast<AsciiStringData *>(g_dynamicMemoryAllocator->Allocate_Bytes_No_Zero(alloc_size));

        new_data->ref_count = 1;
        new_data->num_chars_allocated = alloc_size - sizeof(AsciiStringData);
#ifdef GAME_DEBUG_STRUCTS
        new_data->debug_ptr = new_data->Peek();
#endif

        if (m_data != nullptr && keep_data) {
            strcpy(new_data->Peek(), Peek());
        } else {
            *new_data->Peek() = '\0';
        }

        if (str_to_cpy != nullptr) {
            strcpy(new_data->Peek(), str_to_cpy);
        }

        if (str_to_cat != nullptr) {
            strcat(new_data->Peek(), str_to_cat);
        }

        Release_Buffer();
        m_data = new_data;
        Validate();
    }
}

/**
 * Gets the length of the string
 */
Utf8String::size_type Utf8String::Get_Length() const
{
    if (m_data != nullptr) {
        const size_type len = static_cast<size_type>(strlen(Str()));
        captainslog_dbgassert(len > 0, "length of string is less than or equal to 0.");

        return len;
    }

    return 0;
}

char Utf8String::Get_Char(size_type index) const
{
    captainslog_dbgassert(index >= 0, "Index must be equal or larger than 0.");
    captainslog_dbgassert(index < Get_Length(), "Index must be smaller than length.");

    return Peek()[index];
}

/**
 * This is effectively ToString() It is in original code, but never called.
 */
const char *Utf8String::Str() const
{
    static char const TheNullChr[4] = "";

    if (m_data != nullptr) {
        return Peek();
    }

    return TheNullChr;
}

/**
 * Generates a buffer with the requested length for reading and returns it.
 */
char *Utf8String::Get_Buffer_For_Read(size_type len)
{
    Ensure_Unique_Buffer_Of_Size(len + 1, false, nullptr, nullptr);
    return Peek();
}

/**
 * Set this string content to a new string
 */
void Utf8String::Set(const char *str)
{
    if (m_data == nullptr || str != m_data->Peek()) {
        const size_type len = str ? static_cast<size_type>(strlen(str)) : 0;

        if (len != 0) {
            Ensure_Unique_Buffer_Of_Size(len + 1, false, str, nullptr);
        } else {
            Release_Buffer();
        }
    }
}

/**
 * A convenience version of Set. Not found in original code.
 */
void Utf8String::Set(Utf8String const &string)
{
    if (&string != this) {
        Release_Buffer();
        m_data = string.m_data;

        if (m_data != nullptr) {
            m_data->Inc_Ref_Count();
        }
    }
}

/**
 * Converts a Utf16 string to Utf8
 */
void Utf8String::Translate_Internal(const unichar_t *utf16_string, const size_type utf16_len)
{
    Release_Buffer();

#if defined BUILD_WITH_ICU
    // Use ICU converters.
    if (utf16_len > 0) {
        int32_t utf8_len;
        UErrorCode error = U_ZERO_ERROR;
        // Get utf8 string length.
        u_strToUTF8(nullptr, 0, &utf8_len, utf16_string, utf16_len, &error);

        if (U_SUCCESS(error) && utf8_len > 0) {
            // Allocate and fill new utf8 string.
            char *utf8_buffer = Get_Buffer_For_Read(utf8_len);
            u_strToUTF8(utf8_buffer, utf8_len, nullptr, utf16_string, utf16_len, &error);

            if (U_FAILURE(error)) {
                Clear();
            } else {
                // Add null terminator manually.
                utf8_buffer[utf8_len] = '\0';
            }
        }
    }
#elif defined PLATFORM_WINDOWS
    // Use WIN32 API converters.
    if (utf16_len > 0) {
        // Get utf8 string length.
        const size_type utf8_len = WideCharToMultiByte(CP_UTF8, 0, utf16_string, utf16_len, nullptr, 0, nullptr, nullptr);

        if (utf8_len > 0) {
            // Allocate and fill new utf8 string.
            char *utf8_buffer = Get_Buffer_For_Read(utf8_len);
            WideCharToMultiByte(CP_UTF8, 0, utf16_string, utf16_len, utf8_buffer, utf8_len, nullptr, nullptr);

            // Add null terminator manually.
            utf8_buffer[utf8_len] = '\0';
        }
    }
#else
    // Naive copy, this is what the original does.
    for (size_type i = 0; i < utf16_len; ++i) {
        unichar_t u = utf16_string[i];
        // FEATURE: Append ? character for non ASCII characters
        if (u > 127)
            u = U_CHAR('?');
        Concat(static_cast<char>(u));
    }
#endif
}

void Utf8String::Translate(Utf16String const &utf16_string)
{
    Translate_Internal(utf16_string.Str(), utf16_string.Get_Length());
}

void Utf8String::Translate(const unichar_t *utf16_string)
{
    const size_type utf16_len = static_cast<size_type>(u_strlen(utf16_string));
    Translate_Internal(utf16_string, utf16_len);
}

/**
 * Convenience concat for 1 char. Not in original code.
 */
void Utf8String::Concat(char c)
{
    char str[2];

    str[0] = c;
    str[1] = '\0';
    Concat(str);
}

/**
 * The original concat as it was in original code.
 */
void Utf8String::Concat(const char *s)
{
    const size_type add_len = static_cast<size_type>(strlen(s));

    if (add_len > 0) {
        if (m_data != nullptr) {
            const size_type cur_len = strlen(Peek());
            Ensure_Unique_Buffer_Of_Size(cur_len + add_len + 1, true, nullptr, s);
        } else {
            Set(s);
        }
    }
}

/**
 * Trims this string to remove all leading and trailing spaces
 */
void Utf8String::Trim()
{
    // No string, no Trim.
    if (m_data == nullptr) {
        return;
    }

    char *str = Peek();

    // Find first none space in string if not the first.
    for (char i = *str; i != '\0'; i = *(++str)) {
        if (!isspace(i)) {
            break;
        }
    }

    // If we had some spaces and moved Str, Set string to that position.
    if (str != Peek()) {
        Set(str);
    }

    // Oops, Set call broke the string.
    if (m_data == nullptr) {
        return;
    }

    const size_type len = static_cast<size_type>(strlen(Peek()));

    for (size_type i = len - 1; i >= 0; --i) {
        if (!isspace(Get_Char(i))) {
            break;
        }

        Remove_Last_Char();
    }
}

/**
 * Converts this string to lower case
 */
void Utf8String::To_Lower()
{
    // Size specifically matches original code for compatibility.
    char buf[MAX_TO_LOWER_BUF_LEN];

    if (m_data == nullptr) {
        return;
    }

    strcpy(buf, Peek());

    for (char *c = buf; *c != '\0'; ++c) {
        *c = tolower(*c);
    }

    Set(buf);
}

/**
 * @brief Convert any windows path separators to posix ('\' to '/').
 */
Utf8String Utf8String::Posix_Path() const
{
    char buf[MAX_FORMAT_BUF_LEN];

    if (m_data == nullptr) {
        return s_emptyString;
    }

    strcpy(buf, Peek());

    for (char *c = buf; *c != '\0'; ++c) {
        if (*c == '\\') {
            *c = '/';
        }
    }

    return buf;
}

/**
 * @brief Convert any posix path separators to windows ('/' to '\').
 */
Utf8String Utf8String::Windows_Path() const
{
    char buf[MAX_FORMAT_BUF_LEN];

    if (m_data == nullptr) {
        return s_emptyString;
    }

    strcpy(buf, Peek());

    for (char *c = buf; *c != '\0'; ++c) {
        if (*c == '/') {
            *c = '\\';
        }
    }

    return buf;
}

/**
 * Removes the last character from a string
 */
void Utf8String::Remove_Last_Char()
{
    if (m_data == nullptr) {
        return;
    }

    const size_type len = static_cast<size_type>(strlen(Peek()));

    if (len > 0) {
        Ensure_Unique_Buffer_Of_Size(len + 1, true);
        Peek()[len - 1] = '\0';
    }
}

/**
 * Forms a string from parameters to print to console.
 */
void Utf8String::Format(const char *format, ...)
{
    va_list va;

    va_start(va, format);
    Format_VA(format, va);
    va_end(va);
}

/**
 * Forms a string from parameters to print to console.
 */
void Utf8String::Format(Utf8String format, ...)
{
    va_list va;

    va_start(va, format);
    Format_VA(format, va);
    va_end(va);
}

/**
 * Prints a string to console for a capped length.
 */
void Utf8String::Format_VA(const char *format, va_list args)
{
    char buf[MAX_FORMAT_BUF_LEN];

    const size_type res = vsnprintf(buf, ARRAY_SIZE(buf), format, args);
    captainslog_relassert(res > 0, 0xDEAD0002, "Unable to format buffer");

    Set(buf);
}

/**
 * Prints a string to console for a capped length. Format_VA variant for convenience. Not used in original code.
 */
void Utf8String::Format_VA(Utf8String &format, va_list args)
{
    char buf[MAX_FORMAT_BUF_LEN];
    const size_type res = vsnprintf(buf, ARRAY_SIZE(buf), format.Str(), args);
    captainslog_relassert(res > 0, 0xDEAD0002, "Unable to format buffer");

    Set(buf);
}

/**
 *  Checks if string starts with *p
 */
bool Utf8String::Starts_With(const char *p) const
{
    if (*p == '\0') {
        return true;
    }
    // early out if our string is shorter than the input one
    const size_type thislen = m_data != nullptr ? static_cast<size_type>(strlen(Peek())) : 0;
    const size_type thatlen = static_cast<size_type>(strlen(p));

    if (thislen < thatlen) {
        return false;
    }

    return strncmp(Peek(), p, thatlen) == 0;
}

/**
 *  Checks if string ends with *p
 */
bool Utf8String::Ends_With(const char *p) const
{
    if (*p == '\0') {
        return true;
    }
    // early out if our string is shorter than the input one
    const size_type thislen = m_data != nullptr ? static_cast<size_type>(strlen(Peek())) : 0;
    const size_type thatlen = static_cast<size_type>(strlen(p));

    if (thislen < thatlen) {
        return false;
    }
    // compare strings
    return strncmp(Peek() + thislen - thatlen, p, thatlen) == 0;
}

/**
 *  Checks if string starts with *p, not case sensitive
 */
bool Utf8String::Starts_With_No_Case(const char *p) const
{
    if (*p == '\0') {
        return true;
    }
    // early out if our string is shorter than the input one
    const size_type thislen = Get_Length();
    const size_type thatlen = static_cast<size_type>(strlen(p));

    if (thislen < thatlen) {
        return false;
    }

    return strncasecmp(Peek(), p, thatlen) == 0;
}

/**
 *  Checks if string ends with *p, not case sensitive
 */
bool Utf8String::Ends_With_No_Case(const char *p) const
{
    if (*p == '\0') {
        return true;
    }

    const size_type thislen = m_data != nullptr ? static_cast<size_type>(strlen(Peek())) : 0;
    const size_type thatlen = static_cast<size_type>(strlen(p));

    if (thislen < thatlen) {
        return false;
    }

    return strncasecmp(Peek() + thislen - thatlen, p, thatlen) == 0;
}

/**
 * Find the next tokens and loads *tok with the string between these token
 * The string this is used on is consumed in the process. (a primitive string split)
 */
bool Utf8String::Next_Token(Utf8String *tok, const char *delims)
{
    if (m_data == nullptr) {
        return false;
    }

    if (*Peek() == '\0' || this == tok) {
        return false;
    }

    //
    // If no separators provided, default to white space.
    //
    if (delims == nullptr) {
        delims = " \n\r\t";
    }

    char *start = Peek();

    //
    // Find next instance of token or end of string
    //
    for (char c = *start; c != '\0'; c = *(++start)) {
        if (strchr(delims, c) == nullptr) {
            break;
        }
    }

    if (*start == '\0') {
        Release_Buffer();
        tok->Release_Buffer();

        return false;
    }

    char *end = start;

    //
    // Find next instance of token or end of string.
    //
    for (char c = *end; c != '\0'; c = *(++end)) {
        if (strchr(delims, c) != nullptr) {
            break;
        }
    }

    if (end <= start) {
        Release_Buffer();
        tok->Release_Buffer();

        return false;
    }

    //
    // Copy found region into provided Utf8String, then move this string
    // to start of next section.
    //
    char *tokstr = tok->Get_Buffer_For_Read(end - start + 1);
    memcpy(tokstr, start, end - start);
    tokstr[end - start] = '\0';
    Set(end);

    return true;
}

void Utf8String::Validate()
{
    // TODO, does not seem to be implemented anywhere? It is called though...
}

#ifdef GAME_DEBUG
void Utf8String::Debug_Ignore_Leaks()
{
    // TODO, does not seem to be implemented anywhere? It is called though...
}
#endif // GAME_DEBUG

#ifdef GAME_DLL
#include <new>
Utf8String *Utf8String::Hook_Ctor1(const char *s)
{
    return new (this) Utf8String(s);
}
Utf8String *Utf8String::Hook_Ctor2(Utf8String const &string)
{
    return new (this) Utf8String(string);
}
#endif // GAME_DLL
