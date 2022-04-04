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
#include "assetinfo.h"
#include "assetmgr.h"

AssetInfoClass::AssetInfoClass(const char *name, int type, RenderObjClass *robj, TextureClass *texture) :
    m_name(name), m_type(type), m_texture(texture), m_renderObj(nullptr)
{
    Ref_Ptr_Set(m_renderObj, robj);
    GetHeirarchyName();
}

AssetInfoClass::~AssetInfoClass()
{
    Ref_Ptr_Release(m_renderObj);
}

void AssetInfoClass::GetHeirarchyName()
{
    if (m_type != 0) {
        RenderObjClass *robj = m_renderObj;
        if (robj != nullptr) {
            robj->Add_Ref();
        } else {
            robj = W3DAssetManager::Get_Instance()->Create_Render_Obj(m_name);

            if (robj == nullptr) {
                return;
            }
        }

        const HTreeClass *tree = robj->Get_HTree();

        if (tree != nullptr) {
            m_heirarchyName = tree->Get_Name();
        }

        robj->Release_Ref();
    }
}
