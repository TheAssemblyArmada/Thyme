/**
 * @file
 *
 * @author OmniBlade
 * @author tomsons26
 *
 * @brief Missing asset handling.
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
#include "hash.h"
#include "w3dtypes.h"
#include "wwstring.h"

class MissingTextureClass : public HashableClass
{
public:
    MissingTextureClass(const char *name) : m_name(name) {}
    virtual ~MissingTextureClass(){};
    virtual const char *Get_Key() override { return m_name.Peek_Buffer(); };

private:
    StringClass m_name;
};

typedef MissingTextureClass MissingAnimClass;
typedef MissingTextureClass MissingGeomClass;

class MissingTexture
{
public:
    static w3dtexture_t Get_Missing_Texture();
    static void Init();
    static void Deinit();
    static w3dsurface_t Create_Missing_Surface();

private:
#ifdef GAME_DLL
    static w3dtexture_t &s_missingTexture;
#else
    static w3dtexture_t s_missingTexture;
#endif
};

#ifdef GAME_DLL
#include "hooker.h"
#endif
