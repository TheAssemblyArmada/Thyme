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
#include "assettypes.h"
#include "vector.h"

class RenderObjClass;

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
    void AddItem(const char *name, AssetType type, bool select);
    HTREEITEM FindItemByName(HTREEITEM item, const char *name);
    HTREEITEM FindNextItem(HTREEITEM item, const char *name);
    HTREEITEM FindFirstItem(HTREEITEM item, const char *name);
    void Select(HTREEITEM item);
    const char *GetSelectedItemName();
    int GetSelectedItemType();
    void GetRenderObjectList(DynamicVectorClass<CString> &vector, HTREEITEM item);
    RenderObjClass *GetRenderObj(HTREEITEM item);
    void RenameItem(const char *newname, const char *oldname, AssetType type);
    void SelectNext();
    void SelectPrev();
    void RefreshRenderObjects();
    void RestrictAnims(bool enable);

protected:
    CDataTreeView();
    DECLARE_DYNCREATE(CDataTreeView)
    DECLARE_MESSAGE_MAP()

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSelectTree(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDeleteItem(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDblClk(NMHDR *pNMHDR, LRESULT *pResult);

public:
    enum Categories
    {
        CATEGORY_MATERIAL,
        CATEGORY_MESH,
        CATEGORY_AGGREGATE,
        CATEGORY_HLOD,
        CATEGORY_COLLECTION,
        CATEGORY_EMITTER,
        CATEGORY_PRIMITIVES,
        CATEGORY_HIERARCHY,
        CATEGORY_SOUNDS,
        CATEGORY_COUNT,
    };

    enum Icons
    {
        ICON_ANIMATION,
        ICON_ANIMSPEED,
        ICON_TEXTURE,
        ICON_MESH,
        ICON_MATERIAL,
        ICON_HLOD,
        ICON_EMITTER,
        ICON_PRIMITIVE,
        ICON_AGGREGATE,
        ICON_HIERARCHY,
        ICON_SOUND,
        ICON_COUNT,
    };

    HTREEITEM m_categoryTreeItems[CATEGORY_COUNT];
    int m_imageListIDs[ICON_COUNT];
    bool m_restrictAnims;
};

void SetLODLevel(RenderObjClass *robj);