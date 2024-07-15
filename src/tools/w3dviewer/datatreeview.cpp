/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View data tree view
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "datatreeview.h"
#include "assetinfo.h"
#include "assetmgr.h"
#include "bmp2d.h"
#include "hanim.h"
#include "mainfrm.h"
#include "part_emt.h"
#include "rendobj.h"
#include "resource.h"
#include "texture.h"
#include "utils.h"
#include "viewerscene.h"
#include "w3dviewdoc.h"

IMPLEMENT_DYNCREATE(CDataTreeView, CTreeView)

// clang-format off
BEGIN_MESSAGE_MAP(CDataTreeView, CTreeView)
    ON_WM_CREATE()
    ON_NOTIFY_REFLECT(TVN_SELCHANGED, &OnSelectTree)
    ON_NOTIFY_REFLECT(TVN_DELETEITEM, &OnDeleteItem)
    ON_NOTIFY_REFLECT(NM_DBLCLK, &OnDblClk)
END_MESSAGE_MAP()
// clang-format on

CDataTreeView::CDataTreeView() : m_categoryTreeItems{}, m_restrictAnims(true)
{
    m_imageListIDs[ICON_ANIMATION] = -1;
    m_imageListIDs[ICON_ANIMSPEED] = -1;
    m_imageListIDs[ICON_TEXTURE] = -1;
    m_imageListIDs[ICON_MESH] = -1;
    m_imageListIDs[ICON_MATERIAL] = -1;
    m_imageListIDs[ICON_HLOD] = -1;
    m_imageListIDs[ICON_EMITTER] = -1;
    m_imageListIDs[ICON_PRIMITIVE] = -1;
    m_imageListIDs[ICON_AGGREGATE] = -1;
    m_imageListIDs[ICON_HIERARCHY] = -1;
    m_imageListIDs[ICON_SOUND] = -1;
}

BOOL CDataTreeView::PreCreateWindow(CREATESTRUCT &cs)
{
    cs.style |= (TVS_SHOWSELALWAYS | TVS_LINESATROOT | TVS_HASLINES | TVS_HASBUTTONS);
    return CTreeView::PreCreateWindow(cs);
}

void CDataTreeView::OnInitialUpdate()
{
    CTreeView::OnInitialUpdate();
}

int CDataTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    int result = CTreeView::OnCreate(lpCreateStruct);

    if (result != -1) {
        CImageList il;
        il.Create(16, 18, ILC_MASK, 5, 10);
        m_imageListIDs[ICON_ANIMATION] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ANIMATION)));
        m_imageListIDs[ICON_ANIMSPEED] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ANIMSPEED)));
        m_imageListIDs[ICON_TEXTURE] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_TEXTURE)));
        m_imageListIDs[ICON_MESH] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_MESH)));
        m_imageListIDs[ICON_MATERIAL] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_MATERIAL)));
        m_imageListIDs[ICON_HLOD] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_HLOD)));
        m_imageListIDs[ICON_AGGREGATE] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_HIERARCHY)));
        m_imageListIDs[ICON_EMITTER] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_HIERARCHY)));
        m_imageListIDs[ICON_HIERARCHY] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_HIERARCHY)));
        m_imageListIDs[ICON_PRIMITIVE] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_PRIMITIVE)));
        m_imageListIDs[ICON_SOUND] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_SOUND)));
        GetTreeCtrl().SetImageList(&il, 0);
        il.Detach();
        AddCategories();
        return 0;
    }

    return result;
}

void CDataTreeView::OnSelectTree(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMTREEVIEW *nm = reinterpret_cast<NMTREEVIEW *>(pNMHDR);
    Select(nm->itemNew.hItem);
    *pResult = 0;
}

void CDataTreeView::OnDeleteItem(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMTREEVIEW *nm = reinterpret_cast<NMTREEVIEW *>(pNMHDR);
    AssetInfoClass *info = reinterpret_cast<AssetInfoClass *>(nm->itemOld.lParam);

    if (info != nullptr) {
        if (info->m_type == ASSET_TYPE_TEXTURE) {
            Ref_Ptr_Release(info->m_texture);
        }

        delete info;
    }

    GetTreeCtrl().SetItem(nm->itemOld.hItem, TVIF_PARAM, nullptr, 0, 0, 0, 0, 0);
    *pResult = 0;
}

void CDataTreeView::OnDblClk(NMHDR *pNMHDR, LRESULT *pResult)
{
    *pResult = 0;
}

void CDataTreeView::AddRenderObjects()
{
    SetRedraw(FALSE);
    RenderObjIterator *iter = W3DAssetManager::Get_Instance()->Create_Render_Obj_Iterator();

    if (iter != nullptr) {
        for (iter->First(); !iter->Is_Done(); iter->Next()) {
            const char *name = iter->Current_Item_Name();

            if (W3DAssetManager::Get_Instance()->Render_Obj_Exists(name)) {
                HTREEITEM category;
                int image;
                AssetType type;

                switch (iter->Current_Item_Class_ID()) {
                    case RenderObjClass::CLASSID_MESH:
                        category = m_categoryTreeItems[CATEGORY_MESH];
                        image = m_imageListIDs[ICON_MESH];
                        type = ASSET_TYPE_MESH;

                        if (HasBaseModelName(name)) {
                            category = m_categoryTreeItems[CATEGORY_AGGREGATE];
                            image = m_imageListIDs[ICON_AGGREGATE];
                            type = ASSET_TYPE_AGGREGATE;
                        }

                        if (FindItemByName(category, name) == nullptr) {
                            HTREEITEM newitem = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
                                name,
                                image,
                                image,
                                0,
                                0,
                                0,
                                category,
                                TVI_SORT);

                            AssetInfoClass *info = new AssetInfoClass(name, type, nullptr, nullptr);
                            GetTreeCtrl().SetItem(newitem, TVIF_PARAM, nullptr, 0, 0, 0, 0, (LPARAM)info);
                        }

                        break;
                    case RenderObjClass::CLASSID_PARTICLEEMITTER:
                        category = m_categoryTreeItems[CATEGORY_EMITTER];
                        image = m_imageListIDs[ICON_EMITTER];
                        type = ASSET_TYPE_PARTICLEEMITTER;

                        if (HasBaseModelName(name)) {
                            category = m_categoryTreeItems[CATEGORY_AGGREGATE];
                            image = m_imageListIDs[ICON_AGGREGATE];
                            type = ASSET_TYPE_AGGREGATE;
                        }

                        if (FindItemByName(category, name) == nullptr) {
                            HTREEITEM newitem = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
                                name,
                                image,
                                image,
                                0,
                                0,
                                0,
                                category,
                                TVI_SORT);

                            AssetInfoClass *info = new AssetInfoClass(name, type, nullptr, nullptr);
                            GetTreeCtrl().SetItem(newitem, TVIF_PARAM, nullptr, 0, 0, 0, 0, (LPARAM)info);
                        }

                        break;
                    case RenderObjClass::CLASSID_HLOD:
                        category = m_categoryTreeItems[CATEGORY_HIERARCHY];
                        image = m_imageListIDs[ICON_HIERARCHY];
                        type = ASSET_TYPE_HIERARCHY;

                        if (HasMultipleLODs(name)) {
                            category = m_categoryTreeItems[CATEGORY_HLOD];
                            image = m_imageListIDs[ICON_HLOD];
                            type = ASSET_TYPE_HLOD;
                        }

                        if (HasBaseModelName(name)) {
                            category = m_categoryTreeItems[CATEGORY_AGGREGATE];
                            image = m_imageListIDs[ICON_AGGREGATE];
                            type = ASSET_TYPE_AGGREGATE;
                        }

                        if (FindItemByName(category, name) == nullptr) {
                            HTREEITEM newitem = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
                                name,
                                image,
                                image,
                                0,
                                0,
                                0,
                                category,
                                TVI_SORT);

                            AssetInfoClass *info = new AssetInfoClass(name, type, nullptr, nullptr);
                            GetTreeCtrl().SetItem(newitem, TVIF_PARAM, nullptr, 0, 0, 0, 0, (LPARAM)info);
                        }

                        break;
                    default:
                        break;
                }
            }
        }

        delete iter;
    }

    AddAnimations();
    AddTextures();
    SetRedraw(TRUE);
    InvalidateRect(nullptr, FALSE);
    UpdateWindow();
}

void CDataTreeView::AddCategories()
{
    m_categoryTreeItems[CATEGORY_MATERIAL] = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
        "Materials",
        m_imageListIDs[ICON_MATERIAL],
        m_imageListIDs[ICON_MATERIAL],
        0,
        0,
        0,
        TVI_ROOT,
        TVI_LAST);

    m_categoryTreeItems[CATEGORY_MESH] = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
        "Mesh",
        m_imageListIDs[ICON_MESH],
        m_imageListIDs[ICON_MESH],
        0,
        0,
        0,
        TVI_ROOT,
        TVI_LAST);

    m_categoryTreeItems[CATEGORY_HIERARCHY] = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
        "Hierarchy",
        m_imageListIDs[ICON_HIERARCHY],
        m_imageListIDs[ICON_HIERARCHY],
        0,
        0,
        0,
        TVI_ROOT,
        TVI_LAST);

    m_categoryTreeItems[CATEGORY_HLOD] = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
        "H-LOD",
        m_imageListIDs[ICON_HLOD],
        m_imageListIDs[ICON_HLOD],
        0,
        0,
        0,
        TVI_ROOT,
        TVI_LAST);

    m_categoryTreeItems[CATEGORY_COLLECTION] = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
        "Mesh Collection",
        m_imageListIDs[ICON_MESH],
        m_imageListIDs[ICON_MESH],
        0,
        0,
        0,
        TVI_ROOT,
        TVI_LAST);

    m_categoryTreeItems[CATEGORY_AGGREGATE] = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
        "Aggregate",
        m_imageListIDs[ICON_AGGREGATE],
        m_imageListIDs[ICON_AGGREGATE],
        0,
        0,
        0,
        TVI_ROOT,
        TVI_LAST);

    m_categoryTreeItems[CATEGORY_EMITTER] = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
        "Emitter",
        m_imageListIDs[ICON_EMITTER],
        m_imageListIDs[ICON_EMITTER],
        0,
        0,
        0,
        TVI_ROOT,
        TVI_LAST);

    m_categoryTreeItems[CATEGORY_PRIMITIVES] = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
        "Primitives",
        m_imageListIDs[ICON_PRIMITIVE],
        m_imageListIDs[ICON_PRIMITIVE],
        0,
        0,
        0,
        TVI_ROOT,
        TVI_LAST);

    m_categoryTreeItems[CATEGORY_SOUNDS] = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
        "Sounds",
        m_imageListIDs[ICON_SOUND],
        m_imageListIDs[ICON_SOUND],
        0,
        0,
        0,
        TVI_ROOT,
        TVI_LAST);
}

void CDataTreeView::Select(HTREEITEM item)
{
    if (item == nullptr) {
        item = GetTreeCtrl().GetSelectedItem();
    }

    if (item != nullptr) {
        AssetInfoClass *info = reinterpret_cast<AssetInfoClass *>(GetTreeCtrl().GetItemData(item));

        if (info != nullptr) {
            CW3DViewDoc *document = static_cast<CW3DViewDoc *>(m_pDocument);

            if (document != nullptr) {
                switch (info->m_type) {
                    case ASSET_TYPE_ANIMATION:
                    case ASSET_TYPE_MORPH: {
                        HTREEITEM parent = GetTreeCtrl().GetParentItem(item);

                        if (parent != nullptr) {
                            RenderObjClass *robj = GetRenderObj(parent);
                            document->SetAnimationByName(robj, info->m_name, true, true);
                            Ref_Ptr_Release(robj);
                        }

                    } break;
                    case ASSET_TYPE_PARTICLEEMITTER: {
                        RenderObjClass *robj = GetRenderObj(item);
                        document->SetParticleEmitter(static_cast<ParticleEmitterClass *>(robj), true, true);
                        Ref_Ptr_Release(robj);
                    } break;
                    default: {
                        RenderObjClass *robj = GetRenderObj(item);
                        document->SetRenderObject(robj, true, true, false);
                        Ref_Ptr_Release(robj);
                    } break;
                }

                CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());

                if (frame != nullptr) {
                    frame->UpdateMenus(info->m_type);
                }
            }

            return;
        }
    }

    CW3DViewDoc *document = static_cast<CW3DViewDoc *>(m_pDocument);

    if (document != nullptr) {
        document->SetRenderObject(nullptr, true, true, false);
        CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());

        if (frame != nullptr) {
            frame->UpdateMenus(ASSET_TYPE_NONE);
        }
    }
}

void SetLODLevel(RenderObjClass *robj)
{
    if (robj != nullptr) {
        for (int i = 0; i < robj->Get_Num_Sub_Objects(); i++) {
            RenderObjClass *o = robj->Get_Sub_Object(i);

            if (o != nullptr) {
                SetLODLevel(o);
                o->Release_Ref();
            }
        }

        if (robj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
            robj->Set_LOD_Level(robj->Get_LOD_Count() - 1);
        }
    }
}

RenderObjClass *CDataTreeView::GetRenderObj(HTREEITEM item)
{
    RenderObjClass *robj = nullptr;
    AssetInfoClass *info = reinterpret_cast<AssetInfoClass *>(GetTreeCtrl().GetItemData(item));

    if (info != nullptr) {
        Ref_Ptr_Set(robj, info->m_renderObj);

        if (robj == nullptr) {
            if (info->m_type == ASSET_TYPE_TEXTURE) {
                if (info->m_texture != nullptr) {
                    robj = new Bitmap2D(info->m_texture, 0.5f, 0.5f, true, false, false, true);
                }
            }

            if (robj == nullptr) {
                robj = W3DAssetManager::Get_Instance()->Create_Render_Obj(info->m_name);
            }
        }

        if (robj != nullptr) {
            CW3DViewDoc *document = GetCurrentDocument();

            if (!document->m_scene->Get_Auto_Switch_LOD()) {
                SetLODLevel(robj);
            }
        }
    }

    return robj;
}

void CDataTreeView::AddTextures()
{
    HashTemplateIterator<StringClass, TextureClass *> textureIter(W3DAssetManager::Get_Instance()->Texture_Hash());

    for (textureIter.First(); !textureIter.Is_Done(); textureIter.Next()) {
        TextureClass *texture = textureIter.Peek_Value();

        if (FindItemByName(m_categoryTreeItems[CATEGORY_MATERIAL], texture->Get_Name()) == nullptr) {
            HTREEITEM newitem = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
                texture->Get_Name(),
                m_imageListIDs[ICON_MATERIAL],
                m_imageListIDs[ICON_MATERIAL],
                0,
                0,
                0,
                m_categoryTreeItems[CATEGORY_MATERIAL],
                TVI_SORT);

            texture->Add_Ref();
            AssetInfoClass *info = new AssetInfoClass(texture->Get_Name(), ASSET_TYPE_TEXTURE, nullptr, texture);
            GetTreeCtrl().SetItem(newitem, TVIF_PARAM, nullptr, 0, 0, 0, 0, (LPARAM)info);
        }
    }
}

void CDataTreeView::AddAnimations()
{
    AssetIterator *iterator = W3DAssetManager::Get_Instance()->Create_HAnim_Iterator();

    if (iterator != nullptr) {
        for (iterator->First(); !iterator->Is_Done(); iterator->Next()) {
            HAnimClass *anim = W3DAssetManager::Get_Instance()->Get_HAnim(iterator->Current_Item_Name());

            for (HTREEITEM i = FindFirstItem(m_categoryTreeItems[CATEGORY_HIERARCHY], anim->Get_HName()); i != nullptr;
                 i = FindNextItem(i, anim->Get_HName())) {
                if (FindItemByName(i, iterator->Current_Item_Name()) == nullptr) {
                    HTREEITEM newitem = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
                        iterator->Current_Item_Name(),
                        m_imageListIDs[ICON_ANIMATION],
                        m_imageListIDs[ICON_ANIMATION],
                        0,
                        0,
                        0,
                        i,
                        TVI_SORT);

                    AssetInfoClass *info =
                        new AssetInfoClass(iterator->Current_Item_Name(), ASSET_TYPE_ANIMATION, nullptr, nullptr);
                    GetTreeCtrl().SetItem(newitem, TVIF_PARAM, nullptr, 0, 0, 0, 0, (LPARAM)info);
                }
            }

            for (HTREEITEM i = FindFirstItem(m_categoryTreeItems[CATEGORY_AGGREGATE], anim->Get_HName()); i != nullptr;
                 i = FindNextItem(i, anim->Get_HName())) {
                if (FindItemByName(i, iterator->Current_Item_Name()) == nullptr) {
                    HTREEITEM newitem = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
                        iterator->Current_Item_Name(),
                        m_imageListIDs[ICON_ANIMATION],
                        m_imageListIDs[ICON_ANIMATION],
                        0,
                        0,
                        0,
                        i,
                        TVI_SORT);

                    AssetInfoClass *info =
                        new AssetInfoClass(iterator->Current_Item_Name(), ASSET_TYPE_ANIMATION, nullptr, nullptr);
                    GetTreeCtrl().SetItem(newitem, TVIF_PARAM, nullptr, 0, 0, 0, 0, (LPARAM)info);
                }
            }

            for (HTREEITEM i = FindFirstItem(m_categoryTreeItems[CATEGORY_HLOD], anim->Get_HName()); i != nullptr;
                 i = FindNextItem(i, anim->Get_HName())) {
                if (FindItemByName(i, iterator->Current_Item_Name()) == nullptr) {
                    HTREEITEM newitem = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
                        iterator->Current_Item_Name(),
                        m_imageListIDs[ICON_ANIMATION],
                        m_imageListIDs[ICON_ANIMATION],
                        0,
                        0,
                        0,
                        i,
                        TVI_SORT);

                    AssetInfoClass *info =
                        new AssetInfoClass(iterator->Current_Item_Name(), ASSET_TYPE_ANIMATION, nullptr, nullptr);
                    GetTreeCtrl().SetItem(newitem, TVIF_PARAM, nullptr, 0, 0, 0, 0, (LPARAM)info);
                }
            }
        }
    }
}

HTREEITEM CDataTreeView::FindItemByName(HTREEITEM item, const char *name)
{
    HTREEITEM item1 = nullptr;
    HTREEITEM item2 = nullptr;
    HTREEITEM item3 = GetTreeCtrl().GetChildItem(item);

    if (item3 != nullptr) {
        while (item1 == nullptr) {
            if (lstrcmp(GetTreeCtrl().GetItemText(item3), name) == 0) {
                item2 = item3;
            }

            item3 = GetTreeCtrl().GetNextSiblingItem(item3);

            if (item3 == nullptr) {
                return item2;
            }

            item1 = item2;
        }
    }

    return item1;
}

HTREEITEM CDataTreeView::FindNextItem(HTREEITEM item, const char *name)
{
    HTREEITEM found = nullptr;

    for (HTREEITEM i = GetTreeCtrl().GetNextSiblingItem(item); i != nullptr; i = GetTreeCtrl().GetNextSiblingItem(i)) {
        if (found != nullptr) {
            break;
        }

        AssetInfoClass *info = reinterpret_cast<AssetInfoClass *>(GetTreeCtrl().GetItemData(i));

        if (!m_restrictAnims || info != nullptr && !lstrcmpi(info->m_hierarchyName, name)) {
            found = i;
        }
    }

    return found;
}

HTREEITEM CDataTreeView::FindFirstItem(HTREEITEM item, const char *name)
{
    HTREEITEM found = nullptr;
    for (HTREEITEM i = GetTreeCtrl().GetChildItem(item); i != nullptr; i = GetTreeCtrl().GetNextSiblingItem(i)) {
        if (found != nullptr) {
            break;
        }

        AssetInfoClass *info = reinterpret_cast<AssetInfoClass *>(GetTreeCtrl().GetItemData(i));

        if (!m_restrictAnims || info != nullptr && !lstrcmpi(info->m_hierarchyName, name)) {
            found = i;
        }
    }

    return found;
}

void CDataTreeView::SelectNext()
{
    HTREEITEM selected = GetTreeCtrl().GetSelectedItem();

    if (selected != nullptr) {
        HTREEITEM next = GetTreeCtrl().GetNextSiblingItem(selected);

        if (next != nullptr) {
            GetTreeCtrl().SelectItem(next);
        }
    }
}

void CDataTreeView::SelectPrev()
{
    HTREEITEM selected = GetTreeCtrl().GetSelectedItem();

    if (selected != nullptr) {
        HTREEITEM prev = GetTreeCtrl().GetPrevSiblingItem(selected);

        if (prev != nullptr) {
            GetTreeCtrl().SelectItem(prev);
        }
    }
}

void CDataTreeView::RestrictAnims(bool enable)
{
    if (m_restrictAnims != enable) {
        m_restrictAnims = enable;
        GetTreeCtrl().DeleteItem(TVI_ROOT);
        AddCategories();
        AddRenderObjects();
    }
}

void CDataTreeView::RenameItem(const char *newname, const char *oldname, AssetType type)
{
    if (newname != nullptr && oldname != nullptr) {
        SetRedraw(false);
        HTREEITEM item = nullptr;

        switch (type) {
            case ASSET_TYPE_MESH:
                item = m_categoryTreeItems[CATEGORY_MESH];
                break;
            case ASSET_TYPE_AGGREGATE:
                item = m_categoryTreeItems[CATEGORY_AGGREGATE];
                break;
            case ASSET_TYPE_HLOD:
                item = m_categoryTreeItems[CATEGORY_HLOD];
                break;
            case ASSET_TYPE_PARTICLEEMITTER:
                item = m_categoryTreeItems[CATEGORY_EMITTER];
                break;
            case ASSET_TYPE_PRIMITIVE:
                item = m_categoryTreeItems[CATEGORY_PRIMITIVES];
                break;
            case ASSET_TYPE_SOUND:
                item = m_categoryTreeItems[CATEGORY_SOUNDS];
                break;
            default:
                break;
        }

        HTREEITEM newitem = FindItemByName(item, oldname);

        if (newitem != nullptr) {
            GetTreeCtrl().SetItem(newitem, TVIF_TEXT, newname, 0, 0, 0, 0, 0);
            AssetInfoClass *info = reinterpret_cast<AssetInfoClass *>(GetTreeCtrl().GetItemData(newitem));

            if (info != nullptr) {
                info->m_name = newname;
            }
        } else {
            AddItem(newname, type, true);
        }

        SetRedraw();
        InvalidateRect(nullptr, FALSE);
        UpdateWindow();
    }
}

void CDataTreeView::AddItem(const char *name, AssetType type, bool select)
{
    if (name != nullptr) {
        SetRedraw(false);
        HTREEITEM item = nullptr;
        int image = 0;

        switch (type) {
            case ASSET_TYPE_MESH:
                item = m_categoryTreeItems[CATEGORY_MESH];
                image = m_imageListIDs[ICON_MESH];
                break;
            case ASSET_TYPE_AGGREGATE:
                item = m_categoryTreeItems[CATEGORY_AGGREGATE];
                image = m_imageListIDs[ICON_AGGREGATE];
                break;
            case ASSET_TYPE_HLOD:
                item = m_categoryTreeItems[CATEGORY_HLOD];
                image = m_imageListIDs[ICON_HLOD];
                break;
            case ASSET_TYPE_PARTICLEEMITTER:
                item = m_categoryTreeItems[CATEGORY_EMITTER];
                image = m_imageListIDs[ICON_EMITTER];
                break;
            case ASSET_TYPE_PRIMITIVE:
                item = m_categoryTreeItems[CATEGORY_PRIMITIVES];
                image = m_imageListIDs[ICON_PRIMITIVE];
                break;
            case ASSET_TYPE_SOUND:
                item = m_categoryTreeItems[CATEGORY_SOUNDS];
                image = m_imageListIDs[ICON_SOUND];
                break;
            default:
                break;
        }

        HTREEITEM newitem = nullptr;

        for (HTREEITEM i = GetTreeCtrl().GetChildItem(item); i != nullptr; i = GetTreeCtrl().GetNextSiblingItem(i)) {
            if (newitem) {
                break;
            }

            if (lstrcmp(GetTreeCtrl().GetItemText(i), name) == 0) {
                newitem = i;
            }
        }

        if (newitem == nullptr) {
            newitem = GetTreeCtrl().InsertItem(
                TVIF_SELECTEDIMAGE | TVIF_IMAGE | TVIF_TEXT, name, image, image, 0, 0, 0, item, TVI_SORT);
            AssetInfoClass *info = new AssetInfoClass(name, type, nullptr, nullptr);
            GetTreeCtrl().SetItem(newitem, TVIF_PARAM, 0, 0, 0, 0, 0, reinterpret_cast<LPARAM>(info));

            if (info->m_hierarchyName.GetLength()) {
                AddAnimationsForItem(item);
            }
        }

        if (select) {
            GetTreeCtrl().Select(newitem, TVGN_CARET);
            GetTreeCtrl().EnsureVisible(newitem);
        }

        SetRedraw();
        InvalidateRect(nullptr, FALSE);
        UpdateWindow();
    }
}

void CDataTreeView::AddAnimationsForItem(HTREEITEM item)
{
    AssetInfoClass *info = reinterpret_cast<AssetInfoClass *>(GetTreeCtrl().GetItemData(item));
    AssetIterator *iterator = W3DAssetManager::Get_Instance()->Create_HAnim_Iterator();

    if (iterator != nullptr) {
        for (iterator->First(); !iterator->Is_Done(); iterator->Next()) {
            const char *name = iterator->Current_Item_Name();
            HAnimClass *anim = W3DAssetManager::Get_Instance()->Get_HAnim(name);

            if (anim != nullptr) {
                if (lstrcmp(info->m_hierarchyName, anim->Get_HName()) == 0 && !FindItemByName(item, name)) {
                    HTREEITEM newitem = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_IMAGE | TVIF_TEXT,
                        name,
                        m_imageListIDs[0],
                        m_imageListIDs[0],
                        0,
                        0,
                        0,
                        item,
                        TVI_SORT);
                    AssetInfoClass *info2 = new AssetInfoClass(name, ASSET_TYPE_ANIMATION, nullptr, nullptr);
                    GetTreeCtrl().SetItem(newitem, TVIF_PARAM, 0, 0, 0, 0, 0, reinterpret_cast<LPARAM>(info2));
                }

                anim->Release_Ref();
            }
        }
    }
}

const char *CDataTreeView::GetSelectedItemName()
{
    HTREEITEM item = GetTreeCtrl().GetSelectedItem();

    if (item != nullptr) {
        AssetInfoClass *info = reinterpret_cast<AssetInfoClass *>(GetTreeCtrl().GetItemData(item));

        if (info != nullptr) {
            return info->m_name;
        }
    }

    return nullptr;
}

int CDataTreeView::GetSelectedItemType()
{
    HTREEITEM item = GetTreeCtrl().GetSelectedItem();

    if (item != nullptr) {
        AssetInfoClass *info = reinterpret_cast<AssetInfoClass *>(GetTreeCtrl().GetItemData(item));

        if (info != nullptr) {
            return info->m_type;
        }
    }

    return -1;
}

void CDataTreeView::GetRenderObjectList(DynamicVectorClass<CString> &vector, HTREEITEM item)
{
    for (HTREEITEM i = GetTreeCtrl().GetChildItem(item); i != nullptr; i = GetTreeCtrl().GetNextSiblingItem(i)) {
        AssetInfoClass *info = reinterpret_cast<AssetInfoClass *>(GetTreeCtrl().GetItemData(i));

        if (info != nullptr) {
            if (info->m_type != ASSET_TYPE_ANIMATION && info->m_type != ASSET_TYPE_MORPH
                && info->m_type != ASSET_TYPE_TEXTURE) {
                CString str = GetTreeCtrl().GetItemText(i);
                vector.Add(str);
            }
        }

        if (GetTreeCtrl().ItemHasChildren(i)) {
            GetRenderObjectList(vector, i);
        }
    }
}

void CDataTreeView::RefreshRenderObjects()
{
    for (HTREEITEM i = GetTreeCtrl().GetChildItem(m_categoryTreeItems[CATEGORY_COLLECTION]); i != nullptr;
         i = GetTreeCtrl().GetNextSiblingItem(i)) {
        AssetInfoClass *info = reinterpret_cast<AssetInfoClass *>(GetTreeCtrl().GetItemData(i));

        if (info != nullptr) {
            W3DAssetManager::Get_Instance()->Remove_Prototype(info->m_name);
        }
    }

    for (HTREEITEM i = GetTreeCtrl().GetChildItem(m_categoryTreeItems[CATEGORY_HIERARCHY]); i != nullptr;
         i = GetTreeCtrl().GetNextSiblingItem(i)) {
        AssetInfoClass *info = reinterpret_cast<AssetInfoClass *>(GetTreeCtrl().GetItemData(i));

        if (info != nullptr) {
            W3DAssetManager::Get_Instance()->Remove_Prototype(info->m_name);
        }
    }

    for (HTREEITEM i = GetTreeCtrl().GetChildItem(m_categoryTreeItems[CATEGORY_MESH]); i != nullptr;
         i = GetTreeCtrl().GetNextSiblingItem(i)) {
        AssetInfoClass *info = reinterpret_cast<AssetInfoClass *>(GetTreeCtrl().GetItemData(i));

        if (info != nullptr) {
            W3DAssetManager::Get_Instance()->Remove_Prototype(info->m_name);
        }
    }
}
