/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View utillity functions
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "utils.h"
#include "mainfrm.h"
#include "w3dviewdoc.h"

CString GetFilePath(const char *name)
{
    char path[MAX_PATH];
    lstrcpy(path, name);
    char *c = strrchr(path, '\\');

    if (c) {
        *c = 0;
    }

    return path;
}

CString GetFilenameFromPath(const char *path)
{
    const char *c = strrchr(path, '\\');

    if (c) {
        return c + 1;
    } else {
        return path;
    }
}

CGraphicView *GetCurrentGraphicView()
{
    CMainFrame *frame = (CMainFrame *)AfxGetMainWnd();

    if (frame != nullptr) {
        CW3DViewDoc *document = (CW3DViewDoc *)frame->GetActiveDocument();
        
        if (document != nullptr) {
            return document->GetGraphicView();
        } else {
            return nullptr;
        }
    }
}