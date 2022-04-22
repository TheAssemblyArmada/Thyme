/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View asset info
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once
#include "w3dafx.h"
#include "assettypes.h"

class RenderObjClass;
class TextureClass;

class AssetInfoClass
{
public:
    AssetInfoClass(const char *name, AssetType type, RenderObjClass *robj, TextureClass *texture);
    virtual ~AssetInfoClass();
    void GetHierarchyName();

public:
    CString m_name;
    CString m_hierarchyName;
    CString m_unk1;
    CString m_unk2;
    AssetType m_type;
    TextureClass *m_texture;
    RenderObjClass *m_renderObj;
};
