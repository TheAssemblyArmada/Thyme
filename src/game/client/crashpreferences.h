/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class holding preferences for crash handling and uploading.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"
#include "userpreferences.h"

class CrashPreferences : public UserPreferences
{
public:
    CrashPreferences();
    virtual ~CrashPreferences() {}

    virtual bool Load(Utf8String filename) override;

    Utf8String Get_Upload_URL();
    bool Upload_Allowed();
    void Set_Upload_URL(const Utf8String &url);
    void Set_Allow_Upload(bool allowed);
};
