/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Waypoint Drawing Code
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
#include <new>

class RenderObjClass;
class SegmentedLineClass;
class TextureClass;
class RenderInfoClass;

class W3DWaypointBuffer
{
public:
    W3DWaypointBuffer();
    ~W3DWaypointBuffer();

    void Free_Waypoint_Buffers();
    void Set_Default_Line_Style();
    void Draw_Waypoints(RenderInfoClass &rinfo);

#ifdef GAME_DLL
    W3DWaypointBuffer *Hook_Ctor() { return new (this) W3DWaypointBuffer; }
    void Hook_Dtor() { W3DWaypointBuffer::~W3DWaypointBuffer(); }
#endif

private:
    RenderObjClass *m_renderObj;
    SegmentedLineClass *m_segLine;
    TextureClass *m_texture;
};
