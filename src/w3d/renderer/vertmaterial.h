/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Vertex Materials
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
#include "refcount.h"
#include "w3dmpo.h"
#include "w3dtypes.h"
#include "wwstring.h"
class TextureMapperClass;
class VertexMaterialClass : public W3DMPO, public RefCountClass
{
public:
    virtual ~VertexMaterialClass();
    void Apply() const;
    static void Init();
    static void Shutdown();
    static void Apply_Null();

private:
    w3dmat_t *m_material;
    unsigned int m_flags;
    unsigned int m_ambientColorSource;
    unsigned int m_emissiveColorSource;
    unsigned int m_diffuseColorSource;
    StringClass m_name;
    TextureMapperClass *m_mapper[8];
    unsigned int m_UVSource[8];
    unsigned int m_uniqueID;
    unsigned long m_CRC;
    bool m_CRCDirty;
    bool m_useLighting;
};
