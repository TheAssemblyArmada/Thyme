/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View resitrcted file dialog
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

class RestrictedFileDialogClass : public CFileDialog
{
public:
    RestrictedFileDialogClass(BOOL bOpenFileDialog,
        LPCTSTR lpszDefExt = NULL,
        LPCTSTR lpszFileName = NULL,
        DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        LPCTSTR lpszFilter = NULL,
        CWnd *pParentWnd = NULL,
        DWORD dwSize = 0,
        BOOL bVistaStyle = TRUE);
    virtual ~RestrictedFileDialogClass() override {}

private:
    DECLARE_DYNAMIC(RestrictedFileDialogClass)
    DECLARE_MESSAGE_MAP()

    virtual BOOL OnFileNameOK() override;
    virtual void OnInitDone() override;
    virtual void OnFileNameChange() override;

    CString m_filename;
};
