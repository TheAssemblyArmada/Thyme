/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 * @author tomsons26
 *
 * @brief Class for handling rendering surfaces.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "surfaceclass.h"
#include "dx8wrapper.h"
#include "vector2i.h"
#include <algorithm>
#include <cstring>

#ifdef BUILD_WITH_D3D8
#include <d3dx8.h>
#endif

using std::memcpy;
using std::memset;

SurfaceClass::SurfaceClass(unsigned width, unsigned height, WW3DFormat format) :
    m_d3dSurface(DX8Wrapper::Create_Surface(width, height, format)), m_surfaceFormat(format)
{
}

SurfaceClass::SurfaceClass(w3dsurface_t d3d_surface) : m_d3dSurface(W3D_TYPE_INVALID_SURFACE)
{
    Attach(d3d_surface);
    SurfaceDescription desc;
    Get_Description(desc);
    m_surfaceFormat = desc.format;
}

SurfaceClass::SurfaceClass(const char *name)
{
    m_d3dSurface = DX8Wrapper::Create_Surface(name);
    SurfaceDescription desc;
    Get_Description(desc);
    m_surfaceFormat = desc.format;
}

SurfaceClass::~SurfaceClass()
{
    Detach();
}

/**
 * Populates a SurfaceDescription struct with information about the surface.
 */
void SurfaceClass::Get_Description(SurfaceDescription &surface_desc)
{
#ifdef BUILD_WITH_D3D8
    D3DSURFACE_DESC d3d_desc;
    m_d3dSurface->GetDesc(&d3d_desc);
    surface_desc.height = d3d_desc.Height;
    surface_desc.width = d3d_desc.Width;
    surface_desc.format = D3DFormat_To_WW3DFormat(d3d_desc.Format);
#endif
}

/**
 * Locks a surface for editing.
 *
 * @param pitch A pointer to an integer to get the pitch of the locked surface.
 * @return Returns a pointer to the surfaces data.
 */
void *SurfaceClass::Lock(int *pitch)
{
#ifdef BUILD_WITH_D3D8
    D3DLOCKED_RECT lock_rect;
    lock_rect.Pitch = 0;
    lock_rect.pBits = nullptr;
    HRESULT res = m_d3dSurface->LockRect(&lock_rect, nullptr, 0);

    if (FAILED(res)) {
        captainslog_warn("Failed to lock surface with error %l.", res);
    }

    *pitch = lock_rect.Pitch;

    return lock_rect.pBits;
#else
    return nullptr;
#endif
}

/**
 * Locks a surface for editing.
 *
 * @param pitch A pointer to an integer to get the pitch of the locked surface.
 * @param discard Should the discard option be passed to the underlying representation?
 * @return Returns a pointer to the surfaces data.
 */
void *SurfaceClass::Lock(int *pitch, bool discard)
{
#ifdef BUILD_WITH_D3D8
    D3DLOCKED_RECT lock_rect;
    lock_rect.Pitch = 0;
    lock_rect.pBits = nullptr;
    HRESULT res = m_d3dSurface->LockRect(&lock_rect, nullptr, (discard ? D3DLOCK_DISCARD : 0) | D3DLOCK_NOSYSLOCK);

    if (FAILED(res)) {
        captainslog_warn("Failed to lock surface with error %l.", res);
    }

    *pitch = lock_rect.Pitch;

    return lock_rect.pBits;
#else
    return nullptr;
#endif
}

/**
 * Locks a rect of a surface for editing.
 *
 * @param pitch A pointer to an integer to get the pitch of the locked surface.
 * @return Returns a pointer to the surfaces data.
 */
void *SurfaceClass::Lock_Rect(int *pitch, int left, int top, int right, int bottom)
{
#ifdef BUILD_WITH_D3D8
    RECT rect;
    rect.top = top;
    rect.left = left;
    rect.right = right;
    rect.bottom = bottom;
    D3DLOCKED_RECT lock_rect;
    lock_rect.Pitch = 0;
    lock_rect.pBits = nullptr;
    HRESULT res = m_d3dSurface->LockRect(&lock_rect, &rect, 0); // BFME has D3DLOCK_NOSYSLOCK

    if (FAILED(res)) {
        captainslog_warn("Failed to lock surface with error %l.", res);
    }

    *pitch = lock_rect.Pitch;

    return lock_rect.pBits;
#else
    return nullptr;
#endif
}

/**
 * Unlocks a locked surface.
 */
void SurfaceClass::Unlock()
{
#ifdef BUILD_WITH_D3D8
    HRESULT res = m_d3dSurface->UnlockRect();

    if (FAILED(res)) {
        captainslog_warn("Failed to unlock surface with error %l.", res);
    }
#endif
}

/**
 * Locks a surface for reading only.
 *
 * @param pitch A pointer to an integer to get the pitch of the locked surface.
 * @return Returns a pointer to the surfaces data.
 */
void *SurfaceClass::Lock_ReadOnly(int *pitch)
{
#ifdef BUILD_WITH_D3D8
    D3DLOCKED_RECT lock_rect;
    lock_rect.Pitch = 0;
    lock_rect.pBits = nullptr;
    HRESULT res = m_d3dSurface->LockRect(&lock_rect, nullptr, D3DLOCK_READONLY);

    if (FAILED(res)) {
        captainslog_warn("Failed to lock surface with error %l.", res);
    }

    *pitch = lock_rect.Pitch;
    return lock_rect.pBits;
#else
    return nullptr;
#endif
}

/**
 * Clears a surface by writing 0 to all pixels.
 */
void SurfaceClass::Clear()
{
    SurfaceDescription desc;
    Get_Description(desc);
    int px_size = Pixel_Size(desc);
    int pitch;
    uint8_t *bytes = static_cast<uint8_t *>(Lock(&pitch));

    for (unsigned i = 0; i < desc.height; ++i) {
        memset(bytes, 0, px_size * desc.width);
        bytes += pitch;
    }

    Unlock();
}

/**
 * Copies bytes to a specified rect on the surface.
 */
void SurfaceClass::Copy(Vector2i &min, Vector2i &max, unsigned char *other)
{
    SurfaceDescription sd;
    Get_Description(sd);
    int size = Pixel_Size(sd);
    int pitch;
    uint8_t *bytes = static_cast<uint8_t *>(Lock_Rect(&pitch, min.I, max.I, min.J, max.J));
    int diff = max.I - min.I;

    for (int i = min.J; i < max.J; ++i) {
        memcpy(bytes, &other[size * (min.I + sd.width * i)], diff * size);
        bytes += pitch;
    }

    Unlock();
}

/**
 * Copies one surface to another.
 *
 * @warning Doesn't currently support copying between different dimensions or formats of surfaces.
 */
void SurfaceClass::Copy(
    unsigned dst_x, unsigned dst_y, unsigned src_x, unsigned src_y, unsigned width, unsigned height, SurfaceClass *other)
{
#ifdef BUILD_WITH_D3D8
    captainslog_assert(other != nullptr);
    captainslog_assert(width != 0);
    captainslog_assert(height != 0);
    SurfaceDescription src_desc;
    SurfaceDescription dst_desc;
    RECT src_rect;
    Get_Description(src_desc);
    other->Get_Description(dst_desc);
    src_rect.left = src_x;
    src_rect.top = src_y;
    src_rect.right = std::min(src_x + width, dst_desc.width);
    src_rect.bottom = std::min(src_y + height, dst_desc.height);

    if (src_desc.format == dst_desc.format && src_desc.width == dst_desc.width && src_desc.height == dst_desc.height) {
        POINT p;
        p.x = dst_x;
        p.y = dst_y;
        DX8CALL(CopyRects(other->m_d3dSurface, &src_rect, 1, m_d3dSurface, &p));
    } else {
        RECT dst_rect;
        dst_rect.left = dst_x;
        dst_rect.top = dst_y;
        dst_rect.right = std::min(dst_x + width, dst_desc.width);
        dst_rect.bottom = std::min(dst_y + height, dst_desc.height);
        D3DXLoadSurfaceFromSurface(m_d3dSurface, nullptr, &dst_rect, other->m_d3dSurface, nullptr, &src_rect, 1, 0);
    }
#endif
}

/**
 * Draws a pixel of a specific color to a given location in the surface.
 */
void SurfaceClass::Draw_Pixel(unsigned x, unsigned y, unsigned color)
{
    SurfaceDescription desc;
    Get_Description(desc);
    int px_size = Pixel_Size(desc);
    int pitch;
    void *bytes = Lock_Rect(&pitch, x, y, x + 1, y + 1);

    switch (px_size) {
        case 1:
            *static_cast<uint8_t *>(bytes) = color;
            break;
        case 2:
            *static_cast<uint16_t *>(bytes) = color;
            break;
        case 4:
            *static_cast<uint32_t *>(bytes) = color;
            break;
        default:
            break;
    }

    Unlock();
}

/**
 * Draws a hrozontal line on the surface.
 */
void SurfaceClass::Draw_Horizonal_Line(unsigned y, unsigned x1, unsigned x2, unsigned color)
{
    SurfaceDescription desc;
    Get_Description(desc);
    int px_size = Pixel_Size(desc);
    int pitch;
    uint8_t *bytes = static_cast<uint8_t *>(Lock_Rect(&pitch, x1, y, x2 + 1, y + 1));

    for (unsigned i = x1; i <= x2; ++i) {
        switch (px_size) {
            case 1:
                *bytes = color;
                break;
            case 2:
                *reinterpret_cast<uint16_t *>(bytes) = color;
                break;
            case 4:
                *reinterpret_cast<uint32_t *>(bytes) = color;
                break;
            default:
                captainslog_error("%s Does not support pixel size of %d", __CURRENT_FUNCTION__, px_size);
                break;
        }

        bytes += px_size;
    }

    Unlock();
}

/**
 * Retrieves the underlying surface representation in the platforms graphics API.
 */
w3dsurface_t SurfaceClass::Peek_D3D_Surface()
{
    return m_d3dSurface;
}

/**
 * Attaches an underlying surface representation in the platforms graphics API.
 */
void SurfaceClass::Attach(w3dsurface_t surface)
{
    Detach();
    m_d3dSurface = surface;
#ifdef BUILD_WITH_D3D8
    if (surface != W3D_TYPE_INVALID_SURFACE) {
        surface->AddRef();
    }
#endif
}

/**
 * Detaches the underlying surface representation in the platforms graphics API.
 */
void SurfaceClass::Detach()
{
#ifdef BUILD_WITH_D3D8
    if (m_d3dSurface != W3D_TYPE_INVALID_SURFACE) {
        m_d3dSurface->Release();
    }
#endif
    m_d3dSurface = W3D_TYPE_INVALID_SURFACE;
}

/**
 * Gets the pixel size for a given SurfaceDescription in bytes.
 */
int SurfaceClass::Pixel_Size(const SurfaceDescription &desc)
{
    switch (desc.format) {
        case WW3D_FORMAT_R8G8B8:
            return 3;
        case WW3D_FORMAT_A8R8G8B8:
        case WW3D_FORMAT_X8R8G8B8:
            return 4;
        case WW3D_FORMAT_R5G6B5:
        case WW3D_FORMAT_X1R5G5B5:
        case WW3D_FORMAT_A1R5G5B5:
        case WW3D_FORMAT_A4R4G4B4:
        case WW3D_FORMAT_A8R3G3B2:
        case WW3D_FORMAT_X4R4G4B4:
        case WW3D_FORMAT_A8P8:
        case WW3D_FORMAT_A8L8:
            return 2;
        case WW3D_FORMAT_R3G3B2:
        case WW3D_FORMAT_A8:
        case WW3D_FORMAT_P8:
        case WW3D_FORMAT_L8:
        case WW3D_FORMAT_A4L4:
            return 1;
        default:
            break;
    }

    return 0;
}

/**
 * Finds the extents of the surface.
 */
void SurfaceClass::FindBB(Vector2i *min, Vector2i *max)
{
#ifdef BUILD_WITH_D3D8
    SurfaceDescription desc;
    Get_Description(desc);
    captainslog_assert(Has_Alpha(desc.format));
    uint8_t alpha = Alpha_Bits(desc.format);
    uint8_t mask = 0;

    switch (alpha) {
        case 1:
            mask = 1;
            break;
        case 4:
            mask = 15;
            break;
        case 8:
            mask = 255;
            break;
        default:
            break;
    }

    int pitch;
    uint8_t *bytes = static_cast<uint8_t *>(Lock_Rect(&pitch, min->I, min->J, max->I, max->J));

    int px_size = Pixel_Size(desc);
    Vector2i real_min = *max;
    Vector2i real_max = *min;

    for (int i = min->J; i < max->J; ++i) {
        for (int j = min->I; j < max->I; ++j) {
            if ((bytes[pitch * (i - min->J) + px_size * (j - min->I) + px_size - 1] >> (8 - alpha)) & mask) {
                real_min.I = std::min(real_min.I, j);
                real_max.I = std::max(real_max.I, j);
                real_min.J = std::min(real_min.J, i);
                real_max.J = std::max(real_max.J, i);
            }
        }
    }

    Unlock();

    *max = real_max;
    *min = real_min;
#endif
}

/**
 * Detects if a given column in the surface is transparent.
 */
bool SurfaceClass::Is_Transparent_Column(unsigned column)
{
#ifdef BUILD_WITH_D3D8
    SurfaceDescription desc;
    Get_Description(desc);

    if (desc.height <= 0) {
        return true;
    }

    captainslog_assert(column < desc.width);
    captainslog_assert(Has_Alpha(desc.format));
    int alpha = Alpha_Bits(desc.format);
    int mask = 0;

    switch (alpha) {
        case 1:
            mask = 1;
            break;
        case 4:
            mask = 0xF;
            break;
        case 8:
            mask = 0xFF;
            break;
        default:
            break;
    }

    D3DLOCKED_RECT lock_rect;
    RECT rect;
    lock_rect.Pitch = 0;
    lock_rect.pBits = nullptr;
    rect.bottom = desc.height;
    rect.top = 0;
    rect.left = column;
    rect.right = column + 1;

    HRESULT res = m_d3dSurface->LockRect(&lock_rect, &rect, D3DLOCK_READONLY);
    if (FAILED(res)) {
        captainslog_warn("Failed to lock surface with error %l.", res);
        // #BUGFIX Return early to avoid null pointer access.
        return false;
    }

    captainslog_assert(lock_rect.pBits != nullptr);

    const int px_size = Pixel_Size(desc);
    uint8_t *pos = static_cast<uint8_t *>(lock_rect.pBits) + px_size - 1;
    unsigned row = 0;

    while (!(mask & (*pos >> (8 - alpha)))) {
        ++row;
        pos += lock_rect.Pitch;

        if (row >= desc.height) {
            Unlock();
            return true;
        }
    }

    Unlock();
    return false;
#else
    return false;
#endif
}
