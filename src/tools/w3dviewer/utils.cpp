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
#include "assetmgr.h"
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
        }
    }

    return nullptr;
}

bool HasBaseModelName(const char *name)
{
    bool has = false;

    RenderObjClass *robj = W3DAssetManager::Get_Instance()->Create_Render_Obj(name);

    if (robj != nullptr) {
        if (robj->Get_Base_Model_Name() != nullptr) {
            has = true;
        }

        robj->Release_Ref();
    }

    return has;
}

bool HasMultipleLODs(const char *name)
{
    bool has = false;

    RenderObjClass *robj = W3DAssetManager::Get_Instance()->Create_Render_Obj(name);

    if (robj != nullptr) {
        if (robj->Class_ID() == RenderObjClass::CLASSID_HLOD && robj->Get_LOD_Count() > 1) {
            has = true;
        }

        robj->Release_Ref();
    }

    return has;
}

CW3DViewDoc *GetCurrentDocument()
{
    CMainFrame *frame = (CMainFrame *)AfxGetMainWnd();

    if (frame != nullptr) {
        CW3DViewDoc *document = (CW3DViewDoc *)frame->GetActiveDocument();

        if (document != nullptr) {
            return document;
        }
    }

    return nullptr;
}
