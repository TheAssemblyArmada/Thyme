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
#include "vertmaterial.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

void VertexMaterialClass::Init()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x008194F0, 0x004E6590));
#endif
}

void VertexMaterialClass::Shutdown()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x008195B0, 0x004E6650));
#endif
}

void VertexMaterialClass::Apply() const
{
#ifdef GAME_DLL
    Call_Method<void, const VertexMaterialClass>(PICK_ADDRESS(0x008193A0, 0x004E63A0), this);
#endif
}

void VertexMaterialClass::Apply_Null()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x00819460, 0x004E64B0));
#endif
}

float VertexMaterialClass::Get_Opacity() const
{
#ifdef BUILD_WITH_D3D8
    return m_material->Diffuse.a;
#else
    return 0;
#endif
}

void VertexMaterialClass::Get_Diffuse(Vector3 *set_color) const
{
#ifdef BUILD_WITH_D3D8
    set_color->X = m_material->Diffuse.r;
    set_color->Y = m_material->Diffuse.g;
    set_color->Z = m_material->Diffuse.b;
#endif
}

unsigned long VertexMaterialClass::Compute_CRC() const
{
#ifdef GAME_DLL
    return Call_Method<unsigned long, const VertexMaterialClass>(PICK_ADDRESS(0x00817750, 0x004E4400), this);
#else
    return 0;
#endif
}

void VertexMaterialClass::Set_Diffuse(float r, float g, float b)
{
#ifdef BUILD_WITH_D3D8
    m_CRCDirty = true;
    m_material->Diffuse.r = r;
    m_material->Diffuse.g = g;
    m_material->Diffuse.b = b;
#endif
}

void VertexMaterialClass::Set_Opacity(float o)
{
#ifdef BUILD_WITH_D3D8
    m_CRCDirty = true;
    m_material->Diffuse.a = o;
#endif
}
