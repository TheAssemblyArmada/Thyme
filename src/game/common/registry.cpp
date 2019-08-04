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

Utf8String Get_Registry_Language()
{
    static Utf8String lang = "english";
    static bool retrieved = false;
    
    if(retrieved) {
        return lang;
    } else {
        Get_String_From_Registry("", "Language", lang);
        retrieved = true;
        
        return lang;
    }
}

void Get_String_From_Registry(Utf8String subkey, Utf8String value, Utf8String const &destination)
{
#ifdef GAME_DLL
    Call_Function<void, Utf8String, Utf8String, Utf8String const &>(0x00498A80, subkey, value, destination);
#endif
}

void Get_String_From_Generals_Registry(Utf8String subkey, Utf8String value, Utf8String const &destination)
{
#ifdef GAME_DLL
    Call_Function<void, Utf8String, Utf8String, Utf8String const &>(0x004988A0, subkey, value, destination);
#endif
}