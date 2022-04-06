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
#pragma once
#include "w3dafx.h"
#include "vector.h"

class RenderObjClass;
class TextureClass;

class AssetInfoClass
{
public:
    AssetInfoClass(const char *name, int type, RenderObjClass *robj, TextureClass *texture);
    virtual ~AssetInfoClass();
    void GetHeirarchyName();

public:
    CString m_name;
    CString m_heirarchyName;
    CString m_unk1;
    CString m_unk2;
    int m_type;
    TextureClass *m_texture;
    RenderObjClass *m_renderObj;
};

class CDataTreeView : public CTreeView
{
public:
    virtual ~CDataTreeView() override {}
    virtual BOOL PreCreateWindow(CREATESTRUCT &cs) override;
    virtual void OnInitialUpdate() override;
    virtual void OnDraw(CDC *pDC) override {}

    void AddCategories();
    void AddTextures();
    void AddRenderObjects();
    void AddAnimations();
    void AddAnimationsForItem(HTREEITEM item);
    void AddItem(const char *name, int category, bool select);
    HTREEITEM FindItemByName(HTREEITEM item, const char *name);
    HTREEITEM FindNextItem(HTREEITEM item, const char *name);
    HTREEITEM FindFirstItem(HTREEITEM item, const char *name);
    void Select(HTREEITEM item);
    const char *GetSelectedItemName();
    int GetSelectedItemType();
    void GetRenderObjectList(DynamicVectorClass<CString> &vector, HTREEITEM item);
    RenderObjClass *GetRenderObj(HTREEITEM item);
    void RenameItem(const char *oldname, const char *newname, int category);
    void SelectNext();
    void SelectPrev();
    void RefreshRenderObjects();
    void RestrictAnims(bool restrict);

protected:
    CDataTreeView();
    DECLARE_DYNCREATE(CDataTreeView)
    DECLARE_MESSAGE_MAP()

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSelectTree(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDeleteItem(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDblClk(NMHDR *pNMHDR, LRESULT *pResult);

    CTreeView base;
    HTREEITEM m_categoryTreeItems[9];
    int m_imageListIDs[11];
    bool m_restrictAnims;
};
