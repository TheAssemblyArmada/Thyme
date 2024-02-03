/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Language Filter
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "languagefilter.h"
#include "filesystem.h"

#ifndef GAME_DLL
LanguageFilter *g_theLanguageFilter;
#else
#include "hooker.h"
#endif

LanguageFilter::LanguageFilter() {}

LanguageFilter::~LanguageFilter()
{
    m_wordList.clear();
}

void LanguageFilter::Init()
{
    m_wordList.clear();

    File *file = g_theFileSystem->Open_File("langdata.dat", File::READ | File::BINARY);

    if (file != nullptr) {
        unichar_t buf[128];

        while (Read_Word(file, buf)) {
            int len = u_strlen(buf);

            if (len != 0) {
                for (int i = 0; i < len; i++) {
                    buf[i] ^= 0x5555;
                }

                Utf16String word(buf);
                Un_Haxor(word);
                m_wordList[word] = true;
            }
        }

        file->Close();
    }
}

void LanguageFilter::Reset()
{
    Init();
}

void LanguageFilter::Update() {}

void LanguageFilter::Un_Haxor(Utf16String &word)
{
    int length = word.Get_Length();
    Utf16String str(U_CHAR(""));

    for (int i = 0; i < length; i++) {
        unichar_t c = word.Get_Char(i);
        switch (c) {
            case U_CHAR('P'):
            case U_CHAR('p'):
                if (i + 1 < length && (word.Get_Char(i + 1) == U_CHAR('h') || word.Get_Char(i + 1) == U_CHAR('H'))) {
                    str.Concat(U_CHAR('f'));
                    i++;
                } else {
                    str.Concat(c);
                }

                break;
            case U_CHAR('1'):
                str.Concat(U_CHAR('l'));
                break;
            case U_CHAR('3'):
                str.Concat(U_CHAR('e'));
                break;
            case U_CHAR('4'):
                str.Concat(U_CHAR('a'));
                break;
            case U_CHAR('5'):
                str.Concat(U_CHAR('s'));
                break;
            case U_CHAR('6'):
                str.Concat(U_CHAR('b'));
                break;
            case U_CHAR('7'):
                str.Concat(U_CHAR('t'));
                break;
            case U_CHAR('0'):
                str.Concat(U_CHAR('o'));
                break;
            case U_CHAR('@'):
                str.Concat(U_CHAR('a'));
                break;
            case U_CHAR('$'):
                str.Concat(U_CHAR('s'));
                break;
            case U_CHAR('+'):
                str.Concat(U_CHAR('t'));
                break;
            default:
                if (!u_strrchr(U_CHAR("-_*'\""), c)) {
                    str.Concat(c);
                }

                break;
        }
    }

    word.Set(str);
}

bool LanguageFilter::Read_Word(File *file, unichar_t *buf)
{
    int count = 0;
    unichar_t c;
    int ret = file->Read(&c, sizeof(c));

    if (ret == -1 || ret == 0) {
        buf[0] = U_CHAR('\0');
    } else {
        buf[0] = c;

        while (buf[count] != U_CHAR(' ')) {
            count++;
            ret = file->Read(&c, sizeof(c));

            if (ret == -1 || ret == 0) {
                c = 0xFFFF;
            }

            if (c == 0xFFFF || c == U_CHAR(' ')) {
                buf[count] = U_CHAR('\0');

                if (c == 0xFFFF) {
                    return false;
                }

                return true;
            }

            buf[count] = c;
        }

        return true;
    }

    return false;
}

void LanguageFilter::Filter_Line(Utf16String &line)
{
#ifdef GAME_DLL
    Call_Method<void, LanguageFilter>(PICK_ADDRESS(0x00500E30, 0x008DAE87), this);
#endif
}

LanguageFilter *Create_Language_Filter()
{
    return new LanguageFilter;
}
