/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief
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
#include "w3dmpo.h"

struct PivotClass;

class HTreeClass : public W3DMPO
{
    IMPLEMENT_W3D_POOL(HTreeClass)
private:
    char m_name[16];
    int m_numPivots;
    PivotClass *m_pivot;
    float m_scaleFactor;

public:
    virtual ~HTreeClass();
    const char *Get_Name() const { return m_name; }
};
