/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Static class for handling texture loading.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "textureloader.h"
#include "bitmaphandler.h"
#include "critsection.h"
#include "ddsfile.h"
#include "dx8wrapper.h"
#include "gamedebug.h"
#include "missing.h"
#include "shader.h"
#include "synctextureloadtasklist.h"
#include "targa.h"
#include "textureloadtask.h"
#include "thumbnailmanager.h"
#include "vector3.h"

#ifndef THYME_STANDALONE
// bool &TextureLoader::s_textureLoadSuspended = Make_Global<bool>(0x00000000);
unsigned &TextureLoader::s_textureInactiveOverrideTime = Make_Global<unsigned>(0x00A4C688);
LoaderThreadClass &TextureLoader::s_textureLoadThread = Make_Global<LoaderThreadClass>(0x00A4C620);
FastCriticalSectionClass &g_backgroundCritSec = Make_Global<FastCriticalSectionClass>(0x00A4C608);
FastCriticalSectionClass &g_foregroundCritSec = Make_Global<FastCriticalSectionClass>(0x00A4C60C);
#else
// bool TextureLoader::s_textureLoadSuspended;
unsigned TextureLoader::s_textureInactiveOverrideTime;
LoaderThreadClass TextureLoader::s_textureLoadThread("Thyme texture Loader thread");
FastCriticalSectionClass g_backgroundCritSec;
FastCriticalSectionClass g_foregroundCritSec;
#endif

w3dtexture_t Load_Compressed_Texture(
    const StringClass &filename, unsigned base_mip_index, MipCountType mip_level_count, WW3DFormat dest_format)
{
    DDSFileClass dds(filename.Peek_Buffer(), base_mip_index);

    w3dsurface_t surface;
    w3dtexture_t texture;

#ifdef BUILD_WITH_D3D8
    if (dds.Have_Level_Sizes() && dds.Load()) {
        texture = DX8Wrapper::Create_Texture(dds.Get_Width(0),
            dds.Get_Height(0),
            dest_format != WW3D_FORMAT_UNKNOWN ? dest_format : Get_Valid_Texture_Format(dds.Get_Format(), true),
            (MipCountType)dds.Get_Mip_Level_Count(),
            D3DPOOL_MANAGED,
            false);

        for (unsigned i = 0; i < dds.Get_Mip_Level_Count(); ++i) {
            texture->GetSurfaceLevel(i, &surface);
            dds.Copy_Level_To_Surface(i, surface, Vector3());
            surface->Release();
        };

        return texture;
    };
#endif

    return W3D_TYPE_INVALID_TEXTURE;
};

void LoaderThreadClass::Thread_Function() {}

void TextureLoader::Init()
{
    ThumbnailManagerClass::Init();
    s_textureLoadThread.Execute();
    s_textureLoadThread.Set_Priority(-4);
    s_textureInactiveOverrideTime = 0;
}

void TextureLoader::Deinit()
{
    FastCriticalSectionClass::LockClass lock(g_backgroundCritSec);
    s_textureLoadThread.Stop(3000);
    ThumbnailManagerClass::Deinit();
    TextureLoadTaskClass::Delete_Free_Pool();
}

bool TextureLoader::Is_DX8_Thread()
{
    return DX8Wrapper::Get_Main_Thread_ID() == ThreadClass::Get_Current_Thread_ID();
}

void TextureLoader::Validate_Texture_Size(unsigned &width, unsigned &height, unsigned &volume)
{
    unsigned w = 1;
    unsigned h = 1;
    unsigned v = 1;

    if (width > 1) {
        do {
            w *= 2;
        } while (width > w);
    }

    if (height > 1) {
        do {
            h *= 2;
        } while (height > h);
    }

    if (volume > 1) {
        do {
            v *= 2;
        } while (v < volume);
    }

    if (DX8Wrapper::Get_Caps()->Get_Max_Tex_Width() < w) {
        w = DX8Wrapper::Get_Caps()->Get_Max_Tex_Width();
    }

    if (h > DX8Wrapper::Get_Caps()->Get_Max_Tex_Height()) {
        h = DX8Wrapper::Get_Caps()->Get_Max_Tex_Height();
    }

    if (v > DX8Wrapper::Get_Caps()->Get_Max_Vol_Extent()) {
        v = DX8Wrapper::Get_Caps()->Get_Max_Vol_Extent();
    }

    if (w > h) {
        while (w / h > 8) {
            h *= 2;
        }

        width = w;
        height = h;
        volume = v;
    } else {
        while (h / w > 8) {
            w *= 2;
        }

        width = w;
        height = h;
        volume = v;
    }
}

w3dtexture_t TextureLoader::Load_Thumbnail(const StringClass &texture, const Vector3 &adjust)
{
    // Checks for Peek_Thumbnail_Instance_From_Any_Manager(texture), but it always returns nullptr.
    return MissingTextureClass::Get_Missing_Texture();
}

w3dsurface_t TextureLoader::Load_Surface_Immediate(const StringClass &texture, WW3DFormat format, bool allow_compressed)
{
    bool dxt = format == WW3D_FORMAT_DXT1 || format == WW3D_FORMAT_DXT2 || format == WW3D_FORMAT_DXT3
        || format == WW3D_FORMAT_DXT4 || format == WW3D_FORMAT_DXT5;
    bool compressed = format == WW3D_FORMAT_UNKNOWN && DX8Wrapper::Supports_DXTC() && allow_compressed;

    if ((dxt || compressed)) {
        w3dtexture_t tex = Load_Compressed_Texture(texture, 0, MIP_LEVELS_1, WW3D_FORMAT_UNKNOWN);
#ifdef BUILD_WITH_D3D8
        if (tex != W3D_TYPE_INVALID_TEXTURE) {
            w3dsurface_t surf;
            tex->GetSurfaceLevel(0, &surf);
            tex->Release();

            return surf;
        }
#endif
    }

    TargaImage targa;
    targa.Open(texture, 0);
    targa.Toggle_Flip_Y();
    WW3DFormat dest_format;
    WW3DFormat src_format;
    unsigned int src_bpp = 0;
    Get_WW3D_Format(dest_format, src_format, src_bpp, targa);

    if (format != WW3D_FORMAT_UNKNOWN) {
        dest_format = format;
    }

    char palette[1024];
    targa.Set_Palette(palette);
    targa.Load(texture.Peek_Buffer(), 1, 0);
    int width = targa.Get_Header().width;
    int height = targa.Get_Header().height;
    uint8_t *dest_surface = 0;
    uint8_t *src_surface = (uint8_t *)targa.Get_Image();

    if (src_format == WW3D_FORMAT_A1R5G5B5 || src_format == WW3D_FORMAT_R5G6B5 || src_format == WW3D_FORMAT_A4R4G4B4
        || src_format == WW3D_FORMAT_P8 || src_format == WW3D_FORMAT_L8) {
        dest_surface = new uint8_t[4 * width * height];
        dest_format = Get_Valid_Texture_Format(WW3D_FORMAT_A8R8G8B8, 0);
        BitmapHandlerClass::Copy_Image(dest_surface,
            width,
            height,
            4 * width,
            WW3D_FORMAT_A8R8G8B8,
            src_surface,
            width,
            height,
            src_bpp * width,
            src_format,
            (uint8_t *)targa.Get_Palette(),
            targa.Get_Header().cmap_depth >> 3,
            false,
            Vector3());
        src_surface = dest_surface;
        src_format = WW3D_FORMAT_A8R8G8B8;
        src_bpp = Get_Bytes_Per_Pixel(WW3D_FORMAT_A8R8G8B8);
    }

    w3dsurface_t surface = DX8Wrapper::Create_Surface(width, height, dest_format);
#ifdef BUILD_WITH_D3D8
    D3DLOCKED_RECT rect;
    surface->LockRect(&rect, 0, 0);
    BitmapHandlerClass::Copy_Image((uint8_t *)rect.pBits,
        width,
        height,
        rect.Pitch,
        dest_format,
        src_surface,
        width,
        height,
        src_bpp * width,
        src_format,
        (uint8_t *)targa.Get_Palette(),
        targa.Get_Header().cmap_depth >> 3,
        false,
        Vector3());
    surface->UnlockRect();
#endif
    if (dest_surface) {
        delete[] dest_surface;
    }

    return surface;
}

void TextureLoader::Request_Thumbnail(TextureBaseClass *texture)
{
    FastCriticalSectionClass::LockClass lock(g_foregroundCritSec);
    if (texture->Peek_Platform_Base_Texture() == W3D_TYPE_INVALID_TEXTURE) {
        if (TextureLoader::Is_DX8_Thread()) {
            w3dtexture_t tex = TextureLoader::Load_Thumbnail(
                texture->Get_Full_Path().Is_Empty() ? texture->Get_Name() : texture->Get_Full_Path(),
                texture->Get_Recolor());

            if (texture->Get_Asset_Type() == 0) {
                texture->Apply_New_Surface(tex, false, false);
            }

#ifdef BUILD_WITH_D3D8
            tex->Release();
#endif
            // This seems at odds with the Renegade code where the load tasks are the other way around.
            // Possibly this code isn't ever actually used with Thumbnail code in general being minimally
            // called.
            if (texture->Get_Thumbnail_Load_Task() != nullptr) {
                g_foregroundQueue.Remove(texture->Get_Thumbnail_Load_Task());
                texture->Get_Thumbnail_Load_Task()->Destroy();
            }
        } else {
            if (texture->Get_Thumbnail_Load_Task() == nullptr) {
                if (texture->Get_Normal_Load_Task() == nullptr || texture->Get_Normal_Load_Task()->Get_Load_State() <= 1) {
                    g_foregroundQueue.Push_Back(TextureLoadTaskClass::Create(
                        texture, TextureLoadTaskClass::TASK_THUMBNAIL, TextureLoadTaskClass::PRIORITY_BACKGROUND));
                }
            }
        }
    }
}

void TextureLoader::Request_Background_Loading(TextureBaseClass *texture)
{
    FastCriticalSectionClass::LockClass lock(g_foregroundCritSec);

    if (!texture->Is_Initialized() && texture->Get_Normal_Load_Task() == nullptr) {
        TextureLoadTaskClass *task = TextureLoadTaskClass::Create(
            texture, TextureLoadTaskClass::TASK_LOAD, TextureLoadTaskClass::PRIORITY_BACKGROUND);

        if (TextureLoader::Is_DX8_Thread()) {
            TextureLoader::Begin_Load_And_Queue(task);
        } else {
            g_foregroundQueue.Push_Back(task);
        }
    }
}

void TextureLoader::Request_Foreground_Loading(TextureBaseClass *texture)
{
    DEBUG_ASSERT(texture != nullptr);
    FastCriticalSectionClass::LockClass lock(g_foregroundCritSec);

    if (texture->Is_Initialized()) {
        return;
    }

    TextureLoadTaskClass *task = texture->Get_Normal_Load_Task();
    TextureLoadTaskClass *thumb_task = texture->Get_Thumbnail_Load_Task();

    if (!TextureLoader::Is_DX8_Thread()) {
        FastCriticalSectionClass::LockClass lock2(g_backgroundCritSec);
        if (thumb_task != nullptr) {
            thumb_task->Set_Load_State(TextureLoadTaskClass::STATE_4);
        }

        if (task !=nullptr) {
            if (task->Get_Parent() == &g_backgroundQueue) {
                g_backgroundQueue.Remove(task);
                g_foregroundQueue.Push_Back(task);
            }

            task->Set_Priority(TextureLoadTaskClass::PRIORITY_FOREGROUND);
        } else {
            g_foregroundQueue.Push_Back(TextureLoadTaskClass::Create(
                texture, TextureLoadTaskClass::TASK_LOAD, TextureLoadTaskClass::PRIORITY_FOREGROUND));
        }
    } else {
        if (thumb_task != nullptr) {
            g_foregroundQueue.Remove(thumb_task);
            thumb_task->Destroy();
        }

        if (task != nullptr) {
            FastCriticalSectionClass::LockClass lock2(g_backgroundCritSec);
            g_foregroundQueue.Remove(task);
            g_backgroundQueue.Remove(task);
        } else {
            task = TextureLoadTaskClass::Create(texture, TextureLoadTaskClass::TASK_LOAD, TextureLoadTaskClass::PRIORITY_FOREGROUND);
        }

        task->Finish_Load();
        task->Destroy();
    }
}

void TextureLoader::Flush_Pending_Load_Tasks() {}

void TextureLoader::Update(void (*update)(void)) {}

void TextureLoader::Suspend_Texture_Load() {}

void TextureLoader::Continue_Texture_Load() {}

void TextureLoader::Process_Foreground_Thumbnail(TextureLoadTaskClass *texture) {}

void TextureLoader::Process_Foreground_Load(TextureLoadTaskClass *texture) {}

void TextureLoader::Begin_Load_And_Queue(TextureLoadTaskClass *texture) {}

void TextureLoader::Load_Thumbnail(TextureBaseClass *texture) {}
