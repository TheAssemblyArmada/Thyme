/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Recovers configuration information.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */

#include "registry.h"

AsciiString Get_Registry_Language()
{
    static AsciiString lang = "english";
    static bool retrieved = false;
    
    if(retrieved) {
        return lang;
    } else {
        Get_String_From_Registry("", "Language", lang);
        retrieved = true;
        
        return lang;
    }
}

void Get_String_From_Registry(AsciiString subkey, AsciiString value, AsciiString const &destination)
{
#ifndef THYME_STANDALONE
    Call_Function<void, AsciiString, AsciiString, AsciiString const &>(0x00498A80, subkey, value, destination);
#endif
}

void Get_String_From_Generals_Registry(AsciiString subkey, AsciiString value, AsciiString const &destination)
{
#ifndef THYME_STANDALONE
    Call_Function<void, AsciiString, AsciiString, AsciiString const &>(0x004988A0, subkey, value, destination);
#endif
}