/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Null Render Object
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "nullrobj.h"
#include <cstring>

using std::memcpy;
using std::strcpy;

Null3DObjClass::Null3DObjClass(char *name)
{
    strcpy(m_Name, name);
}

Null3DObjClass::Null3DObjClass(const Null3DObjClass &src)
{
    strcpy(m_Name, src.m_Name);
}

Null3DObjClass &Null3DObjClass::operator=(const Null3DObjClass &that)
{
    strcpy(m_Name, that.m_Name);
    RenderObjClass::operator=(that);
    return *this;
}

NullPrototypeClass::NullPrototypeClass()
{
    strcpy(m_definition.name, "NULL");
}

NullPrototypeClass::NullPrototypeClass(const W3dNullObjectStruct &def)
{
    m_definition = def;
}

PrototypeClass *NullLoaderClass::Load_W3D(ChunkLoadClass &cload)
{
    W3dNullObjectStruct header;
    cload.Read(&header, sizeof(header));
    return new NullPrototypeClass(header);
}
