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
#include "missingtexture.h"
#include "rtsutilsw3d.h"
#include "shader.h"
#include "synctextureloadtasklist.h"
#include "targa.h"
#include "textureloadtask.h"
#include "thumbnailmanager.h"
#include "vector3.h"

#ifdef GAME_DLL
extern FastCriticalSectionClass &g_backgroundCritSec;
extern FastCriticalSectionClass &g_foregroundCritSec;
#else
unsigned TextureLoader::s_textureInactiveOverrideTime;
LoaderThreadClass TextureLoader::s_textureLoadThread("Thyme texture Loader thread");
bool TextureLoader::s_textureLoadSuspended;
FastCriticalSectionClass g_backgroundCritSec;
FastCriticalSectionClass g_foregroundCritSec;
#endif

/**
 * Loads a dds formatted texture file into a platform texture.
 *
 * 0x0082EB00
 */
w3dtexture_t Load_Compressed_Texture(
    const StringClass &filename, unsigned base_mip_index, MipCountType mip_level_count, WW3DFormat dest_format)
{
#ifdef BUILD_WITH_D3D8
    DDSFileClass dds(filename.Peek_Buffer(), base_mip_index);

    w3dsurface_t surface;
    w3dtexture_t texture;

    if (dds.Have_Level_Sizes() && dds.Load()) {
        texture = DX8Wrapper::Create_Texture(dds.Get_Width(0),
            dds.Get_Height(0),
            dest_format != WW3D_FORMAT_UNKNOWN ? dest_format : Get_Valid_Texture_Format(dds.Get_Format(), true),
            (MipCountType)dds.Get_Mip_Level_Count(),
            D3DPOOL_MANAGED,
            false);

        for (unsigned i = 0; i < dds.Get_Mip_Level_Count(); ++i) {
            texture->GetSurfaceLevel(i, &surface);
            dds.Copy_Level_To_Surface(i, surface, Vector3(0.0f, 0.0f, 0.0f));
            surface->Release();
        };

        return texture;
    };
#endif

    return W3D_TYPE_INVALID_TEXTURE;
};

/**
 * LoaderThread implementation of Thread_Function, thread loads textures pushed to background queue.
 *
 * 0x0082FDE0
 */
void LoaderThreadClass::Thread_Function()
{
    while (m_isRunning) {
        if (!g_backgroundQueue.Empty()) {
            FastCriticalSectionClass::LockClass background_lock(g_backgroundCritSec);
            TextureLoadTaskClass *task = g_backgroundQueue.Pop_Front();
            if (task) {
                task->Load();
                FastCriticalSectionClass::LockClass foreground_lock(g_foregroundCritSec);
                g_foregroundQueue.Push_Back(task);
            }
        }

        Switch_Thread();
    }
}

/**
 * Initialise the texture loader.
 *
 * 0x0082EC90
 */
void TextureLoader::Init()
{
    ThumbnailManagerClass::Init();
    s_textureLoadThread.Execute();
    s_textureLoadThread.Set_Priority(-4);
    s_textureInactiveOverrideTime = 0;
}

/**
 * Deinitialise the texture loader.
 *
 * 0x0082ECC0
 */
void TextureLoader::Deinit()
{
    FastCriticalSectionClass::LockClass lock(g_backgroundCritSec);
    s_textureLoadThread.Stop(3000);
    ThumbnailManagerClass::Deinit();
    TextureLoadTaskClass::Delete_Free_Pool();
}

/**
 * Check if the current thread is the main thread.
 *
 * 0x0082EDF0
 */
bool TextureLoader::Is_DX8_Thread()
{
    return DX8Wrapper::Get_Main_Thread_ID() == ThreadClass::Get_Current_Thread_ID();
}

/**
 * Validates the texture dimensions are a multiple of 2.
 *
 * 0x0082EE10
 */
void TextureLoader::Validate_Texture_Size(unsigned &width, unsigned &height, unsigned &volume)
{
    unsigned w;
    unsigned h;
    unsigned v;
    // clang-format off
    for (w = 1; w < width; w *= 2);
    for (h = 1; h < height; h *= 2);
    for (v = 1; v < volume; v *= 2);
    // clang-format on

    if (DX8Wrapper::Get_Current_Caps()->Get_Max_Tex_Width() < w) {
        w = DX8Wrapper::Get_Current_Caps()->Get_Max_Tex_Width();
    }

    if (h > DX8Wrapper::Get_Current_Caps()->Get_Max_Tex_Height()) {
        h = DX8Wrapper::Get_Current_Caps()->Get_Max_Tex_Height();
    }

    if (v > DX8Wrapper::Get_Current_Caps()->Get_Max_Vol_Extent()) {
        v = DX8Wrapper::Get_Current_Caps()->Get_Max_Vol_Extent();
    }

    if (w > h) {
        while ((w / h) > 8) {
            h *= 2;
        }
    } else {
        while ((h / w) > 8) {
            w *= 2;
        }
    }

    width = w;
    height = h;
    volume = v;
}

/**
 * Loads a thumbnail.
 *
 * 0x0082EED0
 */
w3dtexture_t TextureLoader::Load_Thumbnail(const StringClass &texture, const Vector3 &adjust)
{
    // Checks for Peek_Thumbnail_Instance_From_Any_Manager(texture), but it always returns nullptr.
    return MissingTexture::Get_Missing_Texture();
}

/**
 * Loads a texture file to a platform surface immediately.
 *
 * 0x0082F080
 */
w3dsurface_t TextureLoader::Load_Surface_Immediate(const StringClass &texture, WW3DFormat format, bool allow_compressed)
{
    if (Is_Format_Compressed(format, allow_compressed)) {
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
    targa.Open(texture, TargaImage::TARGA_READ);
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
    uint8_t *src_surface = reinterpret_cast<uint8_t *>(targa.Get_Image());

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
            reinterpret_cast<uint8_t *>(targa.Get_Palette()),
            targa.Get_Header().cmap_depth >> 3,
            false,
            Vector3(0.0f, 0.0f, 0.0f));
        src_surface = dest_surface;
        src_format = WW3D_FORMAT_A8R8G8B8;
        src_bpp = Get_Bytes_Per_Pixel(WW3D_FORMAT_A8R8G8B8);
    }

    w3dsurface_t surface = DX8Wrapper::Create_Surface(width, height, dest_format);
#ifdef BUILD_WITH_D3D8
    D3DLOCKED_RECT rect;
    surface->LockRect(&rect, 0, 0);
    BitmapHandlerClass::Copy_Image(reinterpret_cast<uint8_t *>(rect.pBits),
        width,
        height,
        rect.Pitch,
        dest_format,
        src_surface,
        width,
        height,
        src_bpp * width,
        src_format,
        reinterpret_cast<uint8_t *>(targa.Get_Palette()),
        targa.Get_Header().cmap_depth >> 3,
        false,
        Vector3(0.0f, 0.0f, 0.0f));
    surface->UnlockRect();
#endif
    if (dest_surface) {
        delete[] dest_surface;
    }

    return surface;
}

/**
 * Requests loading of a thumbnail texture object.
 *
 * 0x0082F3A0
 */
void TextureLoader::Request_Thumbnail(TextureBaseClass *texture)
{
    FastCriticalSectionClass::LockClass lock(g_foregroundCritSec);
    if (texture->Peek_Platform_Base_Texture() == W3D_TYPE_INVALID_TEXTURE) {
        if (TextureLoader::Is_DX8_Thread()) {
            Load_Thumbnail(texture);
            // This seems at odds with the Renegade code where the load tasks are the other way around.
            // Possibly this code isn't ever actually used with Thumbnail code in general being minimally
            // called.
            if (texture->Get_Thumbnail_Load_Task() != nullptr) {
                g_foregroundQueue.Remove(texture->Get_Thumbnail_Load_Task());
                texture->Get_Thumbnail_Load_Task()->Destroy();
            }
        } else {
            if (texture->Get_Thumbnail_Load_Task() == nullptr) {
                if (texture->Get_Normal_Load_Task() == nullptr
                    || texture->Get_Normal_Load_Task()->Get_Load_State() == TextureLoadTaskClass::STATE_NONE) {
                    g_foregroundQueue.Push_Back(TextureLoadTaskClass::Create(
                        texture, TextureLoadTaskClass::TASK_THUMBNAIL, TextureLoadTaskClass::PRIORITY_BACKGROUND));
                }
            }
        }
    }
}

/**
 * Requests background loading of a texture object. Handled off the main thread.
 *
 * 0x0082F520
 */
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

/**
 * Requests foreground loading of a texture object. Handled on the main thread.
 *
 * 0x0082F6A0
 */
void TextureLoader::Request_Foreground_Loading(TextureBaseClass *texture)
{
    captainslog_assert(texture != nullptr);
    FastCriticalSectionClass::LockClass lock(g_foregroundCritSec);

    if (texture->Is_Initialized()) {
        return;
    }

    TextureLoadTaskClass *task = texture->Get_Normal_Load_Task();
    TextureLoadTaskClass *thumb_task = texture->Get_Thumbnail_Load_Task();

    if (!TextureLoader::Is_DX8_Thread()) {
        FastCriticalSectionClass::LockClass lock2(g_backgroundCritSec);
        if (thumb_task != nullptr) {
            thumb_task->Set_Load_State(TextureLoadTaskClass::STATE_FOREGROUND_OVERRIDE);
        }

        if (task != nullptr) {
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
            task = TextureLoadTaskClass::Create(
                texture, TextureLoadTaskClass::TASK_LOAD, TextureLoadTaskClass::PRIORITY_FOREGROUND);
        }

        task->Finish_Load();
        task->Destroy();
    }
}

/**
 * Checks if there are load tasks queued.
 */
bool TextureLoader::Queues_Not_Empty()
{
    FastCriticalSectionClass::LockClass lock(g_backgroundCritSec);

    return !g_backgroundQueue.Empty() && !g_foregroundQueue.Empty();
}

bool TextureLoader::Is_Format_Compressed(WW3DFormat format, bool allow_compressed)
{
    if (format == WW3D_FORMAT_DXT1 || format == WW3D_FORMAT_DXT2 || format == WW3D_FORMAT_DXT3 || format == WW3D_FORMAT_DXT4
        || format == WW3D_FORMAT_DXT5) {
        return true;
    }

    if (format == WW3D_FORMAT_UNKNOWN && DX8Wrapper::Get_Current_Caps()->Supports_DXTC() && allow_compressed) {
        return true;
    }

    return false;
}

/**
 * Flushes all loading tasks held on the foreground queue to completion.
 *
 * 0x0082F960
 */
void TextureLoader::Flush_Pending_Load_Tasks()
{
    while (Queues_Not_Empty()) {
        FastCriticalSectionClass::LockClass lock(g_backgroundCritSec);
        Update(nullptr);
        ThreadClass::Switch_Thread();
    }
}

/**
 * Processes the next task in the foreground queue.
 */
void TextureLoader::Update(void (*update)(void))
{
    if (!s_textureLoadSuspended) {
        FastCriticalSectionClass::LockClass lock(g_foregroundCritSec);
        int time = rts::Get_Time();
        TextureLoadTaskClass *task;

        while ((task = g_foregroundQueue.Pop_Front()) != nullptr) {
            if (update != nullptr) {
                int time2 = rts::Get_Time();

                if ((time2 - time) > 20) {
                    update();
                    time2 = time;
                }
            }

            if (task->Get_Task_Type() == TextureLoadTaskClass::TASK_THUMBNAIL) {
                TextureLoader::Process_Foreground_Thumbnail(task);
            } else if (task->Get_Task_Type() == TextureLoadTaskClass::TASK_LOAD) {
                TextureLoader::Process_Foreground_Load(task);
            }
        }

        TextureBaseClass::Invalidate_Old_Unused_Textures(0);
    }
}

/**
 * Processes a thumbnail task, initiating loading it it hasn't started, destroying it if it has finished.
 */
void TextureLoader::Process_Foreground_Thumbnail(TextureLoadTaskClass *task)
{
    switch (task->Get_Load_State()) {
        case TextureLoadTaskClass::STATE_NONE:
            Load_Thumbnail(task->Get_Texture());
        case TextureLoadTaskClass::STATE_FOREGROUND_OVERRIDE: // Fallthrough
            task->Destroy();
            break;
        default:
            break;
    }
}

/**
 * Processes a task, initiating loading it it hasn't started, destroying it if it has finished.
 */
void TextureLoader::Process_Foreground_Load(TextureLoadTaskClass *task)
{
    if (task->Get_Priority() == TextureLoadTaskClass::PRIORITY_FOREGROUND) {
        task->Finish_Load();
        task->Destroy();
        return;
    }

    // Finish up background tasks
    switch (task->Get_Load_State()) {
        case TextureLoadTaskClass::STATE_NONE:
            TextureLoader::Begin_Load_And_Queue(task);
            break;
        case TextureLoadTaskClass::STATE_LOADED:
            task->End_Load();
            task->Destroy();
            break;
        default:
            break;
    }
}

/**
 * Starts a task loading and pushes it onto the background queue.
 */
void TextureLoader::Begin_Load_And_Queue(TextureLoadTaskClass *task)
{
    // Start the background loader thread if it isn't already running.
    if (!s_textureLoadThread.Is_Running()) {
        s_textureLoadThread.Execute();
        s_textureLoadThread.Set_Priority(-4);
    }

    // If we can't start the load of either a dds or tga for the filename in the task set it to missing.
    if (task->Begin_Load()) {
        g_backgroundQueue.Push_Front(task);
    } else {
        task->Apply_Missing_Texture();
        task->Destroy();
    }
}

/**
 * Loads a thumbnail to a texture object.
 */
void TextureLoader::Load_Thumbnail(TextureBaseClass *texture)
{
    const StringClass *name = texture->Get_Full_Path().Is_Empty() ? &texture->Get_Name() : &texture->Get_Full_Path();
    w3dtexture_t tex = Load_Thumbnail(*name, texture->Get_HSV_Shift());
    texture->Apply_New_Surface(tex, false, false);
#ifdef BUILD_WITH_D3D8
    tex->Release();
#endif
}
