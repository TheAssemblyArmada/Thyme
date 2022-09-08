/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View Add to lineup dialog
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

class ViewerSceneClass;

class CAddToLineupDialog : public CDialog
{
public:
    CAddToLineupDialog(ViewerSceneClass *scene, CWnd *pParentWnd);
    virtual ~CAddToLineupDialog() override {}
    virtual void DoDataExchange(CDataExchange *pDX) override;
    virtual BOOL OnInitDialog() override;
    virtual void OnOK() override;

    CString Get_Name() const { return m_objectName; }

private:
    DECLARE_MESSAGE_MAP();

    CString m_objectName;
    ViewerSceneClass *m_scene;
};
