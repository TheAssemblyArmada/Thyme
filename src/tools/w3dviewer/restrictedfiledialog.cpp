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
#include "restrictedfiledialog.h"
#include "utils.h"
#include <dlgs.h>

IMPLEMENT_DYNAMIC(RestrictedFileDialogClass, CFileDialog)

// clang-format off
BEGIN_MESSAGE_MAP(RestrictedFileDialogClass, CFileDialog)
END_MESSAGE_MAP()
// clang-format on

RestrictedFileDialogClass::RestrictedFileDialogClass(BOOL bOpenFileDialog,
    LPCTSTR lpszDefExt,
    LPCTSTR lpszFileName,
    DWORD dwFlags,
    LPCTSTR lpszFilter,
    CWnd *pParentWnd,
    DWORD dwSize,
    BOOL bVistaStyle) :
    CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
    m_filename = lpszFileName;
}

BOOL RestrictedFileDialogClass::OnFileNameOK()
{
    const char *str = m_filename;
    ::SendMessage(::GetParent(m_hWnd), CDM_SETCONTROLTEXT, edt1, reinterpret_cast<LPARAM>(str));
    CString path = GetFilePath(m_ofn.lpstrFile);
    path += "\\";
    path += m_filename;
    strcpy(m_ofn.lpstrFile, path);
    strcpy(m_ofn.lpstrFileTitle, m_filename);
    return FALSE;
}

void RestrictedFileDialogClass::OnInitDone()
{
    ::EnableWindow(::GetDlgItem(::GetParent(m_hWnd), edt1), FALSE);
    ::EnableWindow(::GetDlgItem(::GetParent(m_hWnd), cmb1), FALSE);
}

void RestrictedFileDialogClass::OnFileNameChange()
{
    const char *str = m_filename;
    ::SendMessage(::GetParent(m_hWnd), CDM_SETCONTROLTEXT, edt1, reinterpret_cast<LPARAM>(str));
}
