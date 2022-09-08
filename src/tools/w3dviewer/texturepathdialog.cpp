/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View Texture Path Dialog
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "texturepathdialog.h"
#include "directorydialog.h"
#include "resource.h"
#include "utils.h"
#include "w3dviewdoc.h"

// clang-format off
BEGIN_MESSAGE_MAP(TexturePathDialogClass, CDialog)
    ON_COMMAND(IDC_BROWSE1, OnBrowse1)
    ON_COMMAND(IDC_BROWSE2, OnBrowse2)
END_MESSAGE_MAP()
// clang-format on

TexturePathDialogClass::TexturePathDialogClass(CWnd *pParentWnd) : CDialog(IDD_TEXTUREDIR, pParentWnd) {}

BOOL TexturePathDialogClass::OnInitDialog()
{
    CDialog::OnInitDialog();
    CW3DViewDoc *doc = GetCurrentDocument();
    SetDlgItemText(IDC_PATH1, doc->m_texturePath1);
    SetDlgItemText(IDC_PATH2, doc->m_texturePath2);
    return TRUE;
}

void TexturePathDialogClass::OnOK()
{
    CString path1;
    CString path2;
    GetDlgItemText(IDC_PATH1, path1);
    GetDlgItemText(IDC_PATH2, path2);
    CW3DViewDoc *doc = GetCurrentDocument();
    doc->SetTexturePath1(path1);
    doc->SetTexturePath2(path2);
    CDialog::OnOK();
}

void TexturePathDialogClass::OnBrowse1()
{
    CString oldpath;
    GetDlgItemText(IDC_PATH1, oldpath);
    CString newpath;
    if (ChooseDirectory(m_hWnd, oldpath, newpath)) {
        SetDlgItemText(IDC_PATH1, newpath);
    }
}

void TexturePathDialogClass::OnBrowse2()
{
    CString oldpath;
    GetDlgItemText(IDC_PATH2, oldpath);
    CString newpath;
    if (ChooseDirectory(m_hWnd, oldpath, newpath)) {
        SetDlgItemText(IDC_PATH2, newpath);
    }
}
