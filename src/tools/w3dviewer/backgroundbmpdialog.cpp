/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View Background Bitmap Dialog
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "backgroundbmpdialog.h"
#include "resource.h"
#include "utils.h"
#include "w3dviewdoc.h"

// clang-format off
BEGIN_MESSAGE_MAP(CBackgroundBMPDialog, CDialog)
    ON_COMMAND(IDABORT, OnBrowse)
END_MESSAGE_MAP()
// clang-format on

CBackgroundBMPDialog::CBackgroundBMPDialog(CWnd *pParentWnd) : CDialog(IDD_BACKGROUNDBITMAP, pParentWnd) {}

BOOL CBackgroundBMPDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    PositionWindow(this->m_hWnd);
    CW3DViewDoc *doc = GetCurrentDocument();

    if (doc != nullptr) {
        SetDlgItemText(IDC_FILENAME, doc->m_backgroundBitmapName);
    }

    return TRUE;
}

void CBackgroundBMPDialog::OnOK()
{
    CW3DViewDoc *doc = GetCurrentDocument();

    if (doc != nullptr) {
        CString str;
        const char *c;

        if (GetDlgItemText(IDC_FILENAME, str) <= 0) {
            c = nullptr;
        } else {
            c = str;
        }

        doc->CreateBackgroundBitmap(c);
    }

    CDialog::OnOK();
}

void CBackgroundBMPDialog::OnBrowse()
{
    CW3DViewDoc *doc = GetCurrentDocument();

    if (doc != nullptr) {
        CFileDialog dlg(TRUE,
            ".tga",
            doc->m_backgroundBitmapName,
            OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
            "Targa files (*.tga)|*.tga||",
            this);

        if (dlg.DoModal() == IDOK) {
            SetDlgItemText(IDC_FILENAME, dlg.GetPathName());
        }
    }
}
