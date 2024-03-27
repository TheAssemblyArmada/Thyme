/**
 * @file
 *
 * @author DuncansPumpkin
 *
 * @brief W3D Mouse Input interface.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"
#include "image.h"
#include "rendobj.h"
#include "surfaceclass.h"
#include "texture.h"
#include "thread.h"
#include "win32mouse.h"

class MouseThreadClass : public ThreadClass
{
public:
    MouseThreadClass(const char *thread_name) : ThreadClass(thread_name, nullptr) {}
    virtual ~MouseThreadClass() {}

    virtual void Thread_Function() override;
};

class W3DMouse final : public Win32Mouse
{
public:
    W3DMouse();

    virtual ~W3DMouse() override;

    virtual void Init() override;
    virtual void Reset() override { Win32Mouse::Reset(); }
    virtual void Draw() override;
    virtual void Set_Cursor(MouseCursor cursor) override;
    virtual void Set_Redraw_Mode(RedrawMode mode) override;

    static void Set_Is_Drawing(bool isDrawing) { s_mouseThreadIsDrawing = isDrawing; }

    static constexpr size_t MAX_FRAMES = 21;

private:
    void Free_W3D_Assets();
    void Init_W3D_Assets();
    void Free_D3D_Assets();
    void Init_D3D_Assets();
    void Release_D3D_Cursor_Texture(MouseCursor cursor);
    bool Load_D3D_Cursor_Textures(MouseCursor cursor);
    void Init_Polygon_Assets();
    void Set_Cursor_Direction(MouseCursor cursor);

    MouseCursor m_currentD3DCursor;
    SurfaceClass *m_D3DCursorSurfaces[MAX_FRAMES];
    int32_t m_hotSpotX;
    int32_t m_hotSpotY;
    int32_t m_D3DCursorSurfaceCount;
    float m_unk4;
    int32_t m_unk5;
    int32_t m_D3DCursorLastDrawn;
    float m_unk7; // refersh rate related fps * 0.001
    bool m_setWinCursor;
    CameraClass *m_camera;
    MouseCursor m_currentW3DCursor;
    MouseCursor m_currentPolyCursor;

#ifdef GAME_DLL
    static MouseThreadClass &s_mouseThread;
    static bool &s_mouseThreadIsDrawing;
    static ARRAY_DEC(HAnimClass *, s_W3DMouseAssets1, CURSOR_COUNT);
    static ARRAY_DEC(RenderObjClass *, s_W3DMouseAssets2, CURSOR_COUNT);
    static ARRAY2D_DEC(TextureBaseClass *, s_D3DMouseAssets, CURSOR_COUNT, MAX_FRAMES); // TODO unsure on type
    static ARRAY_DEC(Image *, s_PolyMouseAssets, CURSOR_COUNT);
#else
    static MouseThreadClass s_mouseThread;
    static bool s_mouseThreadIsDrawing;
    static HAnimClass *s_W3DMouseAssets1[CURSOR_COUNT];
    static RenderObjClass *s_W3DMouseAssets2[CURSOR_COUNT];
    static TextureBaseClass *s_D3DMouseAssets[CURSOR_COUNT][MAX_FRAMES]; // TODO unsure on type
    static Image *s_PolyMouseAssets[CURSOR_COUNT];
#endif
};
