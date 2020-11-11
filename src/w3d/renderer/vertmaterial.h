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
#include "vector3.h"
#include "w3dmpo.h"
#include "w3dtypes.h"
#include "wwstring.h"
#ifdef BUILD_WITH_D3D8
struct DynD3DMATERIAL8 : public W3DMPO, public D3DMATERIAL8
{
    IMPLEMENT_W3D_POOL(DynD3DMATERIAL8);
};
#endif
class DX8Wrapper;
class TextureMapperClass;
class VertexMaterialClass : public W3DMPO, public RefCountClass
{
    friend DX8Wrapper;

public:
    enum ColorSourceType
    {
        MATERIAL = 0,
        COLOR1,
        COLOR2,
    };

    virtual ~VertexMaterialClass();
    static void Init();
    static void Shutdown();

    unsigned long Get_CRC() const
    {
        if (m_CRCDirty) {
            m_CRC = Compute_CRC();
            m_CRCDirty = false;
        }

        return m_CRC;
    }

    float Get_Opacity() const;
    void Set_Opacity(float o);

    void Get_Diffuse(Vector3 *set_color) const;
    void Set_Diffuse(float r, float g, float b);
    TextureMapperClass *Peek_Mapper(int stage = 0) { return m_mapper[stage]; }

protected:
#ifdef BUILD_WITH_D3D8
    DynD3DMATERIAL8 *m_material;
#else
    w3dmat_t *m_material;
#endif
    unsigned int m_flags;
    unsigned int m_ambientColorSource;
    unsigned int m_emissiveColorSource;
    unsigned int m_diffuseColorSource;
    StringClass m_name;
    TextureMapperClass *m_mapper[8];
    unsigned int m_UVSource[8];
    unsigned int m_uniqueID;
    mutable unsigned long m_CRC;
    mutable bool m_CRCDirty;
    bool m_useLighting;

private:
    void Apply() const;
    static void Apply_Null();
    unsigned long Compute_CRC(void) const;
};
