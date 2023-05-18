/**
 * @file
 *
 * @author tomsons26
 *
 * @brief W3D Terrain Background
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dterrainbackground.h"

#include "baseheightmap.h"
#include "camera.h"
#include "dx8wrapper.h"
#include "rendobj.h"
#include "terraintex.h"
#include "texture.h"
#include "worldheightmap.h"

void W3DTerrainBackground::Set_Flip(WorldHeightMap *map)
{
    if (m_map != nullptr) {
        if (map != nullptr) {
            Ref_Ptr_Set(m_map, map);
        }

        if (m_allocated) {
            Set_Flip_Recursive(0, 0, m_pixelsPerCell);
        }
    }
}

void W3DTerrainBackground::Do_Partial_Update(IRegion2D const &region, WorldHeightMap *map, bool set_textures)
{
    if (m_map != nullptr) {
        if (map != nullptr) {
            Ref_Ptr_Set(m_map, map);
        }

        if (m_allocated) {
            Do_Tesselated_Update(region, map, set_textures);
        }
    }
}

bool W3DTerrainBackground::Advance_Left(ICoord2D &pos, int x, int y, int pixels_per_cell)
{
    while (pos.y < pixels_per_cell + y) {
        ++pos.y;
        if (m_map->Get_Flip_State(m_x + pos.x, m_y + pos.y)) {
            return true;
        }
    }
    while (pos.x < x + pixels_per_cell - 1) {
        ++pos.x;
        if (m_map->Get_Flip_State(m_x + pos.x, m_y + pos.y)) {
            return true;
        }
    }
    return false;
}

bool W3DTerrainBackground::Advance_Right(ICoord2D &pos, int x, int y, int pixels_per_cell)
{
    while (pos.x < pixels_per_cell + x) {
        ++pos.x;
        if (m_map->Get_Flip_State(m_x + pos.x, m_y + pos.y)) {
            return true;
        }
    }
    while (pos.y < y + pixels_per_cell - 1) {
        ++pos.y;
        if (m_map->Get_Flip_State(m_x + pos.x, m_y + pos.y)) {
            return true;
        }
    }
    return false;
}

void W3DTerrainBackground::Fill_VB_Recursive(
    unsigned short *out_buffer, int x, int y, int pixels_per_cell, unsigned short *in_buffer, int &count)
{
    int val1 = in_buffer[(m_pixelsPerCell + 1) * y + x];
    int val2 = in_buffer[(m_pixelsPerCell + 1) * (pixels_per_cell + y) + pixels_per_cell + x];
    int xe = m_map->Get_X_Extent() - 1;
    int ye = m_map->Get_Y_Extent() - 1;

    bool dont_recurse = true;

    int xx = x + m_x;
    int yy = y + m_y;
    int height = m_map->Get_Height(xx, yy);

    int hx;
    int hy;
    for (int i = 0; i <= pixels_per_cell; ++i) {
        for (int j = 0; j <= pixels_per_cell; ++j) {
            if (i + xx < xe) {
                hx = i + xx;
            } else {
                hx = xe;
            }
            if (j + yy < ye) {
                hy = j + yy;
            } else {
                hy = ye;
            }
            if (height != m_map->Get_Height(hx, hy)) {
                dont_recurse = false;
                break;
            }
        }
    }
    if (pixels_per_cell == 1) {
        dont_recurse = true;
    }

    if (dont_recurse) {
        ICoord2D left_pos;
        ICoord2D right_pos;

        left_pos.x = x;
        left_pos.y = y;
        right_pos.x = x;
        right_pos.y = y;
        Advance_Left(left_pos, x, y, pixels_per_cell);
        Advance_Right(right_pos, x, y, pixels_per_cell);

        if (out_buffer) {
            out_buffer[count] = val1;
        }
        ++count;

        unsigned short val3 = in_buffer[(m_pixelsPerCell + 1) * right_pos.y + right_pos.x];
        if (out_buffer) {
            out_buffer[count] = val3;
        }
        ++count;

        unsigned short val4 = in_buffer[(m_pixelsPerCell + 1) * left_pos.y + left_pos.x];
        if (out_buffer) {
            out_buffer[count] = val4;
        }
        ++count;

        bool left = true;
        bool right = true;
        while (left || right) {

            left = Advance_Left(left_pos, x, y, pixels_per_cell);

            if (left) {

                if (out_buffer) {
                    out_buffer[count] = val4;
                }
                ++count;

                if (out_buffer) {
                    out_buffer[count] = val3;
                }
                ++count;

                val4 = in_buffer[(m_pixelsPerCell + 1) * left_pos.y + left_pos.x];
                if (out_buffer) {
                    out_buffer[count] = val4;
                }
                ++count;
            }

            right = Advance_Right(right_pos, x, y, pixels_per_cell);

            if (right) {
                if (out_buffer) {
                    out_buffer[count] = val4;
                }
                ++count;

                if (out_buffer) {
                    out_buffer[count] = val3;
                }
                ++count;

                val3 = in_buffer[(m_pixelsPerCell + 1) * right_pos.y + right_pos.x];
                if (out_buffer) {
                    out_buffer[count] = val3;
                }
                ++count;
            }
        }
        if (out_buffer) {
            out_buffer[count] = val4;
        }
        ++count;

        if (out_buffer) {
            out_buffer[count] = val3;
        }
        ++count;

        if (out_buffer) {
            out_buffer[count] = val2;
        }
        ++count;

    } else {
        int mid = pixels_per_cell / 2;
        Fill_VB_Recursive(out_buffer, x, y, mid, in_buffer, count);
        Fill_VB_Recursive(out_buffer, x, mid + y, mid, in_buffer, count);
        Fill_VB_Recursive(out_buffer, mid + x, y, mid, in_buffer, count);
        Fill_VB_Recursive(out_buffer, mid + x, mid + y, mid, in_buffer, count);
    }
}

void W3DTerrainBackground::Set_Flip_Recursive(int x, int y, int pixels_per_cell)
{
    int ex = m_map->Get_X_Extent() - 1;
    int ey = m_map->Get_Y_Extent() - 1;
    bool dont_recurse = true;
    int xx = x + m_x;
    int yy = y + m_y;
    int height = m_map->Get_Height(x + m_x, yy);

    int hx;
    int hy;
    for (int i = 0; i <= pixels_per_cell; ++i) {
        for (int j = 0; j <= pixels_per_cell; ++j) {
            if (i + xx < ex) {
                hx = i + xx;
            } else {
                hx = ex;
            }
            if (j + yy < ey) {
                hy = j + yy;
            } else {
                hy = ey;
            }
            if (height != m_map->Get_Height(hx, hy)) {
                dont_recurse = false;
                break;
            }
        }
    }
    if (pixels_per_cell == 1) {
        dont_recurse = true;
    }

    if (dont_recurse) {
        m_map->Set_Flip_State(xx, yy, true);
        m_map->Set_Flip_State(pixels_per_cell + xx, yy, true);
        m_map->Set_Flip_State(pixels_per_cell + xx, pixels_per_cell + yy, true);
        m_map->Set_Flip_State(xx, pixels_per_cell + yy, true);
    } else {
        int mid = pixels_per_cell / 2;
        Set_Flip_Recursive(x, y, mid);
        Set_Flip_Recursive(x, mid + y, mid);
        Set_Flip_Recursive(mid + x, y, mid);
        Set_Flip_Recursive(mid + x, mid + y, mid);
    }
}

void W3DTerrainBackground::Do_Tesselated_Update(IRegion2D const &region, WorldHeightMap *map, bool set_textures)
{
    if (m_map != nullptr) {
        Ref_Ptr_Set(m_map, map);
        if (m_allocated) {
            int hi_x = m_x;
            int hi_y = m_y;
            int lo_x = m_pixelsPerCell + m_x;
            int lo_y = m_pixelsPerCell + m_y;

            int ex = m_map->Get_X_Extent() - 1;
            int ey = m_map->Get_Y_Extent() - 1;

            if (region.lo.x <= lo_x && region.lo.y <= lo_y && region.hi.x >= hi_x && region.hi.y >= hi_y) {
                Set_Flip(map);

                int idx_count = (m_pixelsPerCell + 1) * (m_pixelsPerCell + 1);
                unsigned short *vb_array = new unsigned short[idx_count];
                int vert_count = 0;

                for (int y = hi_y; y <= lo_y; ++y) {
                    for (int x = hi_x; x <= lo_x; ++x) {
                        int ndxNdx = (y - hi_y) * (m_pixelsPerCell + 1) + x - hi_x;
                        captainslog_dbgassert(ndxNdx < idx_count, "Bad ndxNdx");
                        vb_array[ndxNdx] = 0;
                        if (m_map->Get_Flip_State(x, y)) {
                            ++vert_count;
                        }
                    }
                }

                if (m_maxVertex < vert_count || m_vertexBuffer == nullptr) {
                    m_maxVertex = vert_count;
                    Ref_Ptr_Release(m_vertexBuffer);
                    m_vertexBuffer =
                        new DX8VertexBufferClass(DX8_FVF_XYZDUV2, m_maxVertex + 4, DX8VertexBufferClass::USAGE_DEFAULT, 0);
                }

                m_curNumTerrainVertices = 0;
                {
                    int xx;
                    int yy;
                    VertexBufferClass::WriteLockClass vb_lock(m_vertexBuffer, 0);

                    VertexFormatXYZDUV2 *vertices = (VertexFormatXYZDUV2 *)vb_lock.Get_Vertex_Array();

                    for (int y = hi_y; y <= lo_y; ++y) {
                        for (int x = hi_x; x <= lo_x; ++x) {
                            if (m_map->Get_Flip_State(x, y)) {
                                vertices->diffuse = g_theTerrainRenderObject->Get_Static_Diffuse(x, y);

                                if (x < ex) {
                                    xx = x;
                                } else {
                                    xx = ex;
                                }

                                if (y < ey) {
                                    yy = y;
                                } else {
                                    yy = ey;
                                }

                                Vector3 pos;
                                pos.X = (double)x * 10.0 - (double)m_map->Border_Size() * 10.0;
                                pos.Y = (double)y * 10.0 - (double)m_map->Border_Size() * 10.0;
                                pos.Z = (double)m_map->Get_Height(xx, yy) * HEIGHTMAP_SCALE;
                                vertices->u1 = (double)(x - hi_x) / (double)m_pixelsPerCell;
                                vertices->v1 = 1.0 - (double)(y - hi_y) / (double)m_pixelsPerCell;
                                vertices->x = pos.X;
                                vertices->y = pos.Y;
                                vertices->z = pos.Z;
                                ++vertices;

                                int ndxNdx = (y - hi_y) * (m_pixelsPerCell + 1) + x - hi_x;

                                captainslog_dbgassert(ndxNdx < idx_count, "Bad ndxNdx");

                                vb_array[ndxNdx] = m_curNumTerrainVertices;
                                ++m_curNumTerrainVertices;
                            }
                        }
                    }
                    idx_count = 0;
                    Fill_VB_Recursive(nullptr, 0, 0, m_pixelsPerCell, vb_array, idx_count);

                    if (m_maxIndex < idx_count || m_indexBuffer == nullptr) {
                        m_maxIndex = idx_count;
                        Ref_Ptr_Release(m_indexBuffer);
                        m_indexBuffer = new DX8IndexBufferClass(m_maxIndex + 4, DX8IndexBufferClass::USAGE_DEFAULT);
                    }
                    m_curNumTerrainIndices = 0;
                    {
                        IndexBufferClass::WriteLockClass ib_lock(m_indexBuffer, 0);

                        unsigned short *idx_array = ib_lock.Get_Index_Array();
                        Fill_VB_Recursive(idx_array, 0, 0, m_pixelsPerCell, vb_array, m_curNumTerrainIndices);
                        delete[] vb_array;

                        MinMaxAABoxClass minbox;
                        minbox.Init_Empty();

                        for (int y = hi_y; y <= lo_y; ++y) {
                            for (int x = hi_x; x <= lo_x; ++x) {
                                Vector3 pos;
                                if (x < ex) {
                                    xx = x;
                                } else {
                                    xx = ex;
                                }

                                if (y < ey) {
                                    yy = y;
                                } else {
                                    yy = ey;
                                }

                                pos.X = (double)x * 10.0 - (double)m_map->Border_Size() * 10.0;
                                pos.Y = (double)y * 10.0 - (double)m_map->Border_Size() * 10.0;
                                pos.Z = (double)m_map->Get_Height(xx, yy) * HEIGHTMAP_SCALE;
                                minbox.Add_Point(pos);
                            }
                        }

                        m_boundingBox.Init(minbox);
                        if (m_texture8 == nullptr || set_textures) {
                            Ref_Ptr_Release(m_texture8);
                            Ref_Ptr_Release(m_texture16);
                            Ref_Ptr_Release(m_texture32);

                            m_texture8 = m_map->Get_Flat_Texture(m_x, m_y, m_pixelsPerCell, 8);
                            m_texture8->Get_Texture_Filter()->Set_U_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
                            m_texture8->Get_Texture_Filter()->Set_V_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
                        }
                    }
                }
            }
        }
    }
}

W3DTerrainBackground::~W3DTerrainBackground()
{
    Free_Terrain_Buffers();

    Ref_Ptr_Release(m_texture8);
    Ref_Ptr_Release(m_texture16);
    Ref_Ptr_Release(m_texture32);
}

W3DTerrainBackground::W3DTerrainBackground() :
    // BUGFIX init all members
    m_cullResult(CULL_RES_INVALID),
    m_vertexBuffer(nullptr),
    m_maxVertex(0),
    m_indexBuffer(nullptr),
    m_maxIndex(0),
    m_texture8(nullptr),
    m_texture16(nullptr),
    m_texture32(nullptr),
    m_state(1),
    m_curNumTerrainVertices(0),
    m_curNumTerrainIndices(0),
    m_x(0),
    m_y(0),
    m_pixelsPerCell(0),
    m_map(nullptr),
    m_unk1(false),
    m_allocated(false)
{
}

void W3DTerrainBackground::Free_Terrain_Buffers()
{
    Ref_Ptr_Release(m_vertexBuffer);
    Ref_Ptr_Release(m_indexBuffer);
    m_curNumTerrainVertices = 0;
    m_curNumTerrainIndices = 0;
    m_allocated = false;

    Ref_Ptr_Release(m_map);
}

void W3DTerrainBackground::Allocate_Terrain_Buffers(WorldHeightMap *map, int x, int y, int pixels_per_cell)
{
    if (map != nullptr) {
        Free_Terrain_Buffers();
        m_curNumTerrainVertices = 0;
        m_curNumTerrainIndices = 0;
        m_x = x;
        m_y = y;
        m_pixelsPerCell = pixels_per_cell;
        m_allocated = true;
        Ref_Ptr_Set(m_map, map);
    }
}

void W3DTerrainBackground::Update_Center(CameraClass *camera)
{
    if (camera->Cull_Box(m_boundingBox)) {
        m_cullResult = CULL_RES_OUTSIDE;
    } else {
        m_cullResult = CULL_RES_INSIDE;
    }
    if (m_cullResult == CULL_RES_OUTSIDE) {
        Ref_Ptr_Release(m_texture32);
        Ref_Ptr_Release(m_texture16);
        m_state = 1;
    } else {
        Vector3 cam_pos = camera->Get_Position();
        float far_dist = 310.0;
        float close_dist = 40.0;
        float squared1 = GameMath::Square(far_dist + close_dist);
        float squared2 = GameMath::Square(2.0 * far_dist + close_dist);
        float squared3 = GameMath::Square(4.0 * far_dist + close_dist);
        float best_dist = 2.0 * squared2;

        for (int x = -1; x < 2; ++x) {
            for (int y = -1; y < 2; ++y) {
                for (int z = -1; z < 2; ++z) {
                    Vector3 box_center = m_boundingBox.m_center;
                    box_center.X += (double)x * m_boundingBox.m_extent.X;
                    box_center.Y += (double)y * m_boundingBox.m_extent.Y;
                    box_center.Z += (double)z * m_boundingBox.m_extent.Z;
                    float dist = (cam_pos - box_center).Length2();
                    if (dist < (double)best_dist) {
                        best_dist = dist;
                    }
                }
            }
        }
        m_state = 1;

        if (best_dist < (double)squared1) {
            m_state = 4;
        } else if (best_dist < (double)squared2) {
            m_state = 2;
        } else {
            Ref_Ptr_Release(m_texture32);
            Ref_Ptr_Release(m_texture16);
            int lod = 0;
            if (best_dist > (double)squared3) {
                lod = 1;
            }
            m_texture8->Set_LOD(lod);
        }
    }
}

void W3DTerrainBackground::Update_Texture()
{
    if (m_cullResult == CULL_RES_OUTSIDE) {
        Ref_Ptr_Release(m_texture32);
        Ref_Ptr_Release(m_texture16);
    } else if (m_state == 4) {
        Ref_Ptr_Release(m_texture16);

        if (m_texture32 == nullptr) {
            m_texture32 = m_map->Get_Flat_Texture(m_x, m_y, m_pixelsPerCell, 32);
            m_texture32->Get_Texture_Filter()->Set_U_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
            m_texture32->Get_Texture_Filter()->Set_V_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
        }

    } else if (m_state == 2) {
        Ref_Ptr_Release(m_texture32);
        if (m_texture16 == nullptr) {
            m_texture16 = m_map->Get_Flat_Texture(m_x, m_y, m_pixelsPerCell, 16);
            m_texture16->Get_Texture_Filter()->Set_U_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
            m_texture16->Get_Texture_Filter()->Set_V_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
        }
    } else {
        Ref_Ptr_Release(m_texture32);
        Ref_Ptr_Release(m_texture16);
    }
}

void W3DTerrainBackground::Draw_Visible_Polys(RenderInfoClass *rinfo, bool keep_textures)
{
    if (m_curNumTerrainIndices != 0) {
        if (m_cullResult != CULL_RES_OUTSIDE) {
            DX8Wrapper::Set_Index_Buffer(m_indexBuffer, 0);
            DX8Wrapper::Set_Vertex_Buffer(m_vertexBuffer, 0);
            if (!keep_textures) {
                if (m_texture32 != nullptr) {
                    DX8Wrapper::Set_Texture(1, m_texture32);
                } else if (m_texture16 != nullptr) {
                    DX8Wrapper::Set_Texture(1, m_texture16);
                } else {
                    DX8Wrapper::Set_Texture(1, m_texture8);
                }
            }
            DX8Wrapper::Draw_Triangles(0, m_curNumTerrainIndices / 3, 0, m_curNumTerrainVertices);
        }
    }
}
