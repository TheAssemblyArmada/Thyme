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
#include "assetmgr.h"
#include "bmp2d.h"
#include "mainfrm.h"
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

CDataTreeView::CDataTreeView() : m_restrictAnims(true), m_categoryTreeItems{}
{
    m_imageListIDs[0] = -1;
    m_imageListIDs[1] = -1;
    m_imageListIDs[2] = -1;
    m_imageListIDs[3] = -1;
    m_imageListIDs[4] = -1;
    m_imageListIDs[5] = -1;
    m_imageListIDs[6] = -1;
    m_imageListIDs[7] = -1;
    m_imageListIDs[8] = -1;
    m_imageListIDs[9] = -1;
    m_imageListIDs[10] = -1;
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
        m_imageListIDs[0] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ANIMATION)));
        m_imageListIDs[1] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ANIMSPEED)));
        m_imageListIDs[2] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_TEXTURE)));
        m_imageListIDs[3] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_MESH)));
        m_imageListIDs[4] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_MATERIAL)));
        m_imageListIDs[5] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDD_HEIRARCHY)));
        m_imageListIDs[8] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_HEIRARCHY)));
        m_imageListIDs[6] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_HEIRARCHY)));
        m_imageListIDs[9] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_HEIRARCHY)));
        m_imageListIDs[7] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_PRIMITIVE)));
        m_imageListIDs[10] = il.Replace(-1, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_SOUND)));
        GetTreeCtrl().SetImageList(&il, 0);
        il.Detach();
        AddCategories();
        return 0;
    }

    return result;
}

void CDataTreeView::OnSelectTree(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMTREEVIEW *nm = (NMTREEVIEW *)pNMHDR;
    Select(nm->itemNew.hItem);
    *pResult = 0;
}

void CDataTreeView::OnDeleteItem(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMTREEVIEW *nm = (NMTREEVIEW *)pNMHDR;
    AssetInfoClass *info = (AssetInfoClass *)nm->itemOld.lParam;

    if (info != nullptr) {
        if (info->m_type == 0) {
            Ref_Ptr_Release(info->m_texture);
        }

        delete info;
    }

    GetTreeCtrl().SetItem(nm->itemOld.hItem, TVIF_PARAM, 0, 0, 0, 0, 0, 0);
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
        iter->First();

        while (!iter->Is_Done()) {
            const char *name = iter->Current_Item_Name();

            if (W3DAssetManager::Get_Instance()->Render_Obj_Exists(name)) {
                HTREEITEM category;
                int image;
                int type;
                HTREEITEM item2;
                HTREEITEM item;

                switch (iter->Current_Item_Class_ID()) {
                    case RenderObjClass::CLASSID_MESH:
                        category = m_categoryTreeItems[1];
                        image = m_imageListIDs[3];
                        type = 1;

                        if (HasBaseModelName(name)) {
                            category = m_categoryTreeItems[2];
                            image = m_imageListIDs[8];
                            type = 3;
                        }

                        item2 = 0;
                        item = GetTreeCtrl().GetNextItem(category, TVGN_CHILD);

                        if (item == nullptr) {
                            goto l2;
                        }

                        break;
                    case RenderObjClass::CLASSID_PARTICLEEMITTER:
                        category = m_categoryTreeItems[5];
                        image = m_imageListIDs[6];
                        type = 7;

                        if (HasBaseModelName(name)) {
                            category = m_categoryTreeItems[2];
                            image = m_imageListIDs[8];
                            type = 3;
                        }

                        item2 = 0;
                        item = GetTreeCtrl().GetNextItem(category, TVGN_CHILD);

                        if (item == nullptr) {
                            goto l2;
                        }

                        break;
                    case RenderObjClass::CLASSID_HLOD:
                        category = m_categoryTreeItems[7];
                        image = m_imageListIDs[9];
                        type = 2;

                        if (HasMultipleLODs(name)) {
                            category = m_categoryTreeItems[3];
                            image = m_imageListIDs[5];
                            type = 4;
                        }

                        if (HasBaseModelName(name)) {
                            category = m_categoryTreeItems[2];
                            image = m_imageListIDs[8];
                            type = 3;
                        }

                        item2 = 0;
                        item = GetTreeCtrl().GetNextItem(category, TVGN_CHILD);

                        if (item == nullptr) {
                            goto l2;
                        }

                        break;
                    default:
                        goto l3;
                }

                while (item2 == nullptr) {
                    if (lstrcmp(GetTreeCtrl().GetItemText(item), name) == 0) {
                        item2 = item;
                    }

                    item = GetTreeCtrl().GetNextItem(item, TVGN_NEXT);

                    if (item == nullptr) {
                        if (item2 == nullptr) {
                        l2:
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
                            GetTreeCtrl().SetItem(newitem, TVIF_PARAM, 0, 0, 0, 0, 0, (LPARAM)info);
                        }

                        break;
                    }
                }
            }

        l3:
            iter->Next();
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
    m_categoryTreeItems[0] = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
        "Materials",
        m_imageListIDs[4],
        m_imageListIDs[4],
        0,
        0,
        0,
        TVI_ROOT,
        TVI_LAST);

    m_categoryTreeItems[1] = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
        "Mesh",
        m_imageListIDs[3],
        m_imageListIDs[3],
        0,
        0,
        0,
        TVI_ROOT,
        TVI_LAST);

    m_categoryTreeItems[7] = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
        "Hierarchy",
        m_imageListIDs[9],
        m_imageListIDs[9],
        0,
        0,
        0,
        TVI_ROOT,
        TVI_LAST);

    m_categoryTreeItems[3] = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
        "H-LOD",
        m_imageListIDs[5],
        m_imageListIDs[5],
        0,
        0,
        0,
        TVI_ROOT,
        TVI_LAST);

    m_categoryTreeItems[4] = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
        "Mesh Collection",
        m_imageListIDs[3],
        m_imageListIDs[3],
        0,
        0,
        0,
        TVI_ROOT,
        TVI_LAST);

    m_categoryTreeItems[2] = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
        "Aggregate",
        m_imageListIDs[8],
        m_imageListIDs[8],
        0,
        0,
        0,
        TVI_ROOT,
        TVI_LAST);

    m_categoryTreeItems[5] = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
        "Emitter",
        m_imageListIDs[6],
        m_imageListIDs[6],
        0,
        0,
        0,
        TVI_ROOT,
        TVI_LAST);

    m_categoryTreeItems[5] = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
        "Primatives",
        m_imageListIDs[7],
        m_imageListIDs[7],
        0,
        0,
        0,
        TVI_ROOT,
        TVI_LAST);

    m_categoryTreeItems[8] = GetTreeCtrl().InsertItem(TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_IMAGE,
        "Sounds",
        m_imageListIDs[10],
        m_imageListIDs[10],
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
        AssetInfoClass *info = (AssetInfoClass *)GetTreeCtrl().GetItemData(item);

        if (info != nullptr) {
            CW3DViewDoc *document = ((CW3DViewDoc *)m_pDocument);

            if (document != nullptr) {
                switch (info->m_type) {
                    case 5:
                    case 8: {
                        HTREEITEM parent = GetTreeCtrl().GetParentItem(item);

                        if (parent != nullptr) {
                            RenderObjClass *robj = GetRenderObj(parent);
                            document->SetAnimationByName(robj, info->m_name, true, true);
                            Ref_Ptr_Release(robj);
                        }

                    } break;
                    case 7: {
                        RenderObjClass *robj = GetRenderObj(item);
                        document->SetParticleEmitter((ParticleEmitterClass *)robj, true, true);
                        Ref_Ptr_Release(robj);
                    } break;
                    default: {
                        RenderObjClass *robj = GetRenderObj(item);
                        document->SetRenderObject(robj, true, true, false);
                        Ref_Ptr_Release(robj);
                    } break;
                }

                CMainFrame *frame = ((CMainFrame *)AfxGetMainWnd());

                if (frame != nullptr) {
                    frame->UpdateMenus(info->m_type);

                    if (info->m_type == 3) {
                        frame->UpdateEmitterMenu();
                    } else {
                        EnableMenuItem(GetSubMenu(::GetMenu(frame->m_hWnd), 3), 3, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);

                        while (RemoveMenu(frame->m_subMenu, 0, MF_BYPOSITION)) {
                        }
                    }
                }
            }
        }

        return;
    }

    CW3DViewDoc *document = ((CW3DViewDoc *)m_pDocument);

    if (document != nullptr) {
        document->SetRenderObject(nullptr, true, true, false);
        CMainFrame *frame = ((CMainFrame *)AfxGetMainWnd());

        if (frame != nullptr) {
            frame->UpdateMenus(-1);
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
    AssetInfoClass *info = (AssetInfoClass *)GetTreeCtrl().GetItemData(item);

    if (info != nullptr) {
        Ref_Ptr_Set(robj, info->m_renderObj);

        if (robj == nullptr) {
            if (info->m_type == 0) {
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
    // TODO
}

void CDataTreeView::AddAnimations()
{
    // TODO
}

#if 0
void CDataTreeView::AddAnimationsForItem(HTREEITEM item)
{
    // TODO
}

void CDataTreeView::AddItem(const char *name, int category, bool select)
{
    // TODO
}

HTREEITEM CDataTreeView::FindItemByName(HTREEITEM item, const char *name)
{
    // TODO
}

HTREEITEM CDataTreeView::FindNextItem(HTREEITEM item, const char *name)
{
    // TODO
}

HTREEITEM CDataTreeView::FindFirstItem(HTREEITEM item, const char *name)
{
    // TODO
}

const char *CDataTreeView::GetSelectedItemName()
{
    // TODO
}

int CDataTreeView::GetSelectedItemType()
{
    // TODO
}

void CDataTreeView::GetRenderObjectList(DynamicVectorClass<CString> &vector, HTREEITEM item)
{
    // TODO
}

void CDataTreeView::RenameItem(const char *oldname, const char *newname, int category)
{
    // TODO
}

void CDataTreeView::SelectNext()
{
    // TODO
}

void CDataTreeView::SelectPrev()
{
    // TODO
}

void CDataTreeView::RefreshRenderObjects()
{
    // TODO
}

void CDataTreeView::RestrictAnims(bool restrict)
{
    // TODO
}
#endif
