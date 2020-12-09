/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Function to hook all required functions.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "archivefile.h"
#include "archivefilesystem.h"
#include "asciistring.h"
#include "assetmgr.h"
#include "audioeventrts.h"
#include "audiomanager.h"
#include "binkvideoplayer.h"
#include "binkvideostream.h"
#include "bitmaphandler.h"
#include "camera.h"
#include "cavesystem.h"
#include "chunkio.h"
#include "colmath.h"
#include "commandline.h"
#include "commandlist.h"
#include "compressionmanager.h"
#include "copyprotect.h"
#include "datachunk.h"
#include "ddsfile.h"
#include "dict.h"
#include "dinputkeybd.h"
#include "display.h"
#include "dx8caps.h"
#include "dx8fvf.h"
#include "dx8indexbuffer.h"
#include "dx8renderer.h"
#include "dx8texman.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include "filesystem.h"
#include "filetransfer.h"
#include "force_nocd.h"
#include "functionlexicon.h"
#include "gameclient.h"
#include "gamelogic.h"
#include "gamemath.h"
#include "gamememory.h"
#include "gamememoryinit.h"
#include "gamemessage.h"
#include "gamemessageparser.h"
#include "gamestate.h"
#include "gametext.h"
#include "geometry.h"
#include "globaldata.h"
#include "hanimmgr.h"
#include "hooker.h"
#include "image.h"
#include "ini.h"
#include "keyboard.h"
#include "lightenv.h"
#include "main.h"
#include "mapobject.h"
#include "matpass.h"
#include "messagestream.h"
#include "milesaudiofilecache.h"
#include "milesaudiomanager.h"
#include "modulefactory.h"
#include "mouse.h"
#include "multilist.h"
#include "namekeygenerator.h"
#include "particle.h"
#include "particlesys.h"
#include "particlesysinfo.h"
#include "particlesysmanager.h"
#include "playertemplate.h"
#include "randomvalue.h"
#include "rankinfo.h"
#include "rayeffect.h"
#include "render2d.h"
#include "render2dsentence.h"
#include "rinfo.h"
#include "script.h"
#include "scriptaction.h"
#include "scriptcondition.h"
#include "scriptgroup.h"
#include "scriptlist.h"
#include "seglinerenderer.h"
#include "setuphooks.h"
#include "shader.h"
#include "sidesinfo.h"
#include "sideslist.h"
#include "soundmanager.h"
#include "surfaceclass.h"
#include "targa.h"
#include "teamsinfo.h"
#include "terraintex.h"
#include "texture.h"
#include "texturebase.h"
#include "textureloader.h"
#include "textureloadtask.h"
#include "thread.h"
#include "thumbnail.h"
#include "thumbnailmanager.h"
#include "tiledata.h"
#include "vertmaterial.h"
#include "w3ddebugdisplay.h"
#include "w3ddisplay.h"
#include "w3dfilesystem.h"
#include "w3dmouse.h"
#include "w3dpoly.h"
#include "w3droadbuffer.h"
#include "weapon.h"
#include "win32gameengine.h"
#include "win32localfilesystem.h"
#include "win32mouse.h"
#include "wininstancedata.h"
#include "wwstring.h"
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>
#include <winsock2.h>

struct hostent *__stdcall cnconline_hook(const char *name)
{
    if (strcmp(name, "gamestats.gamespy.com") == 0) {
        return gethostbyname("gamestats.server.cnc-online.net");
    }

    if (strcmp(name, "master.gamespy.com") == 0) {
        return gethostbyname("master.server.cnc-online.net");
    }

    if (strcmp(name, "peerchat.gamespy.com") == 0) {
        return gethostbyname("peerchat.server.cnc-online.net");
    }

    if (strcmp(name, "gpcm.gamespy.com") == 0) {
        return gethostbyname("gpcm.server.cnc-online.net");
    }

    if (strcmp(name, "master.gamespy.com") == 0 || strcmp(name, "ccgenerals.ms19.gamespy.com") == 0
        || strcmp(name, "ccgenzh.ms6.gamespy.com") == 0) {
        return gethostbyname("master.server.cnc-online.net");
    }

    if (strcmp(name, "servserv.generals.ea.com") == 0) {
        return gethostbyname("http.server.cnc-online.net");
    }

    if (strcmp(name, "www.gamespy.com") == 0 || strcmp(name, "ingamead.gamespy.com") == 0) {
        return gethostbyname("server.cnc-online.net");
    }

    return gethostbyname(name);
}

void Null_Func(){};

void Setup_Hooks()
{
    // Hook WinMain
    Hook_Function(0x00401700, Main_Func);

    Hook_Function(0x007F5B06, cnconline_hook);

    // Code that checks the launcher is running, launcher does CD check.
    Hook_Function(0x00412420, CopyProtect::checkForMessage);
    Hook_Function(0x00412450, CopyProtect::validate);

    // Returns true for any CD checks
    Hook_Function(0x005F1CB0, IsFirstCDPresent);

    // Replace memory init functions.
    Hook_Function(0x00414510, Init_Memory_Manager);
    Hook_Function(0x004148C0, Init_Memory_Manager_Pre_Main);

    // Replace memory allocation operators
    // TODO, use linker check globals so no need to hook new.
    Hook_Function(0x00414450, New_New); // operator new
    Hook_Function(0x00414490, New_New); // operator new[]
    Hook_Function(0x004144D0, New_Delete); // operator delete
    Hook_Function(0x004144F0, New_Delete); // operator delete[]
    Hook_Function(0x00414B30, Create_Named_Pool);

    // Replace pool functions
    Hook_Method(0x00413C10, &MemoryPool::Allocate_Block);
    Hook_Method(0x00413C40, &MemoryPool::Free_Block);

    // Replace DMA functions
    Hook_Method(0x00413EE0, &DynamicMemoryAllocator::Allocate_Bytes_No_Zero);
    Hook_Method(0x00413FE0, &DynamicMemoryAllocator::Allocate_Bytes);
    Hook_Method(0x00414010, &DynamicMemoryAllocator::Free_Bytes);
    Hook_Method(0x00414140, &DynamicMemoryAllocator::Get_Actual_Allocation_Size);

    // Replace MemoryPoolFactory functions
    Hook_Method(0x00414180,
        static_cast<MemoryPool *(MemoryPoolFactory::*const)(char const *, int, int, int)>(
            &MemoryPoolFactory::Create_Memory_Pool));

    // Replace File functions
    // virtuals for these are hooked by calls to Win32GameEngine which creates the objects with our vtables.
    Hook_Function(0x004469C0, FileSystem::Are_Music_Files_On_CD); // This one is part of the CD Check.
    Hook_Method(0x00446770, &FileSystem::Get_File_List_From_Dir);
    Hook_Method(0x0048F410, &ArchiveFileSystem::Get_File_List_From_Dir);
    Hook_Method(0x0048F250, &ArchiveFileSystem::Get_Archive_Filename_For_File);
    Hook_Method(0x0048F160, &ArchiveFileSystem::Get_File_Info);

    // Replace Utf8String
    Hook_Method(0x00402430, &Utf8String::Hook_Ctor2);
    Hook_Method(0x00401D40, &Utf8String::Hook_Ctor1);
    Hook_Method(0x0040FB40, static_cast<void (Utf8String::*)(char const *)>(&Utf8String::Concat));
    Hook_Method(0x004159E0, &Utf8String::Ends_With);
    Hook_Method(0x00415A40, &Utf8String::Ends_With_No_Case);
    Hook_Method(0x00415290, &Utf8String::Ensure_Unique_Buffer_Of_Size);
    Hook_Method(0x00415750, static_cast<void (Utf8String::*)(Utf8String, ...)>(&Utf8String::Format));
    Hook_Method(0x00415860, static_cast<void (Utf8String::*)(char const *, ...)>(&Utf8String::Format));
    // The below method is implemented, but due to only being called from Format above it is not hooked.
    // Hook_Method(0x00044772, static_cast<void (Utf8String::*)(char const *, va_list args)>(&Utf8String::Format_VA));
    Hook_Method(0x00415280, &Utf8String::Free_Bytes);
    Hook_Method(0x00415430, &Utf8String::Get_Buffer_For_Read);
    Hook_Method(0x00415AA0, &Utf8String::Is_None);
    Hook_Method(0x0040FB20, &Utf8String::Get_Length);
    Hook_Method(0x00415AD0, &Utf8String::Next_Token);
    Hook_Method(0x004F3CD0, &Utf8String::Release_Buffer);
    Hook_Method(0x00415710, &Utf8String::Remove_Last_Char);
    Hook_Method(0x0040D640, static_cast<void (Utf8String::*)(char const *)>(&Utf8String::Set));
    Hook_Method(0x00415920, &Utf8String::Starts_With);
    Hook_Method(0x00415980, &Utf8String::Starts_With_No_Case);
    Hook_Method(0x00404210, &Utf8String::Str);
    Hook_Method(0x00415630, &Utf8String::To_Lower);
    Hook_Method(0x00415450, &Utf8String::Translate);
    Hook_Method(0x00415530, &Utf8String::Trim);

    // Replace Win32GameEngine
    Hook_Any(0x007420F0, Win32GameEngine::Create_Local_File_System);
    Hook_Any(0x00742150, Win32GameEngine::Create_Archive_File_System);
    Hook_Any(0x00741FA0, Win32GameEngine::Create_Module_Factory);
    Hook_Any(0x007424B0, Win32GameEngine::Create_Audio_Manager);

    // Replace INI
    Hook_Method(0x0041D6E0, &INI::Get_Next_Token);
    Hook_Method(0x0041D720, &INI::Get_Next_Token_Or_Null);
    Hook_Method(0x0041D950, &INI::Get_Next_Sub_Token);
    Hook_Method(0x0041A4B0, &INI::Prep_File);
    Hook_Method(0x0041D460, &INI::Init_From_INI_Multi);
    Hook_Method(0x0041A5C0, &INI::Load);
    Hook_Method(0x0041A1C0, &INI::Load_Directory);

    // Field parsing functions
    Hook_Function(0x0041ADA0, &INI::Parse_Bool);
    Hook_Function(0x0041A980, &INI::Parse_Byte);
    Hook_Function(0x0041AAB0, &INI::Parse_Int);
    Hook_Function(0x0041AB20, &INI::Parse_Unsigned);
    Hook_Function(0x0041AB90, &INI::Parse_Real);
    Hook_Function(0x0041AC00, &INI::Parse_Positive_None_Zero_Real);
    Hook_Function(0x0041BA50, &INI::Parse_Percent_To_Real);
    Hook_Function(0x0041ACA0, &INI::Parse_Angle_Real);
    Hook_Function(0x0041AD20, &INI::Parse_Angular_Velocity_Real);
    Hook_Function(0x0041AF20, &INI::Parse_AsciiString);
    Hook_Function(0x0041B1B0, &INI::Parse_AsciiString_Vector_Append);
    Hook_Function(0x0041BDD0, &INI::Parse_RGB_Color);
    Hook_Function(0x0041C100, &INI::Parse_Color_Int);
    Hook_Function(0x0041C530, &INI::Parse_Coord2D);
    Hook_Function(0x0041C2C0, &INI::Parse_Coord3D);
    Hook_Function(0x0041D140, &INI::Parse_Index_List);
    Hook_Function(0x0041DB50, &INI::Parse_Duration_Real);
    Hook_Function(0x0041DBD0, &INI::Parse_Duration_Int);
    Hook_Function(0x0041DD10, &INI::Parse_Velocity_Real);
    Hook_Function(0x0041DD90, &INI::Parse_Acceleration_Real);
    Hook_Function(0x0041BB20, &INI::Parse_Bitstring32);

    // Replace NameKeyGenerator
    Hook_Method(0x0047B2F0, &NameKeyGenerator::Key_To_Name);
    Hook_Method(0x0047B360, &NameKeyGenerator::Name_To_Key);
    Hook_Method(0x0047B500, &NameKeyGenerator::Name_To_Lower_Case_Key);

    // Replace GameTextManager
    Hook_Function(0x00418320, &GameTextManager::Create_Game_Text_Interface);
    Hook_Function(0x0041A020, &GameTextManager::Compare_LUT);

    // Replace GameMessage
    Hook_Method(0x0040A8F0, &GameMessage::Append_Pixel_Arg);
    Hook_Method(0x0040A800, &GameMessage::Append_Int_Arg);

    // Replace GameMessageList
    Hook_Any(0x0040D760, GameMessageList::Append_Message);
    Hook_Any(0x0040D7A0, GameMessageList::Insert_Message);
    Hook_Any(0x0040D7D0, GameMessageList::Remove_Message);
    Hook_Any(0x0040D820, GameMessageList::Contains_Message_Of_Type);

    // messagestream.h MessageStream
    Hook_Any(0x0040D960, MessageStream::Append_Message);
    Hook_Any(0x0040DA00, MessageStream::Insert_Message);
    Hook_Method(0x0040DAA0, &MessageStream::Attach_Translator);
    Hook_Method(0x0040DB60, &MessageStream::Find_Translator);
    Hook_Method(0x0040DB90, &MessageStream::Remove_Translator);
    Hook_Method(0x0040DBF0, &MessageStream::Propagate_Messages);

    // Replaces CommandList
    // This one actually replaces Reset as it is just inlined Destroy_All_Messages
    Hook_Method(0x0040DD70, &CommandList::Destroy_All_Messages);

    // Replaces randomvalue.h functions
    Hook_Function(0x0048DC00, static_cast<void (*)()>(Init_Random));
    Hook_Function(0x0048DCF0, static_cast<void (*)(uint32_t)>(Init_Random));
    Hook_Function(0x0048DDB0, Init_Game_Logic_Random);
    Hook_Function(0x0048DBB0, Get_Logic_Random_Seed_CRC);
    Hook_Function(0x0048DBA0, Get_Logic_Random_Seed);
    Hook_Function(0x0048DEB0, Get_Client_Random_Value);
    Hook_Function(0x0048DE00, Get_Logic_Random_Value);
    Hook_Function(0x0048DEE0, Get_Audio_Random_Value);
    Hook_Function(0x0048DF70, Get_Client_Random_Value_Real);
    Hook_Function(0x0048DF10, Get_Logic_Random_Value_Real);
    Hook_Function(0x0048DFD0, Get_Audio_Random_Value_Real);
    Hook_Method(0x0048E030, &GameClientRandomVariable::Get_Value);
    Hook_Method(0x0048E0D0, &GameLogicRandomVariable::Get_Value);

    // Replaces Command line parser.
    Hook_Function(0x004F8150, &Parse_Command_Line);

    // Replaces W3DFileSystem
    Hook_Any(0x00763880, W3DFileSystem::Get_File);
    Hook_Any(0x00815370, W3DFileSystem::Return_File);

    // Replace ChunkSaveClass
    Hook_Method(0x008A0F90, &ChunkSaveClass::Begin_Chunk);
    Hook_Method(0x008A1020, &ChunkSaveClass::End_Chunk);
    Hook_Method(0x008A10D0, &ChunkSaveClass::Begin_Micro_Chunk);
    Hook_Method(0x008A1160, &ChunkSaveClass::End_Micro_Chunk);
    Hook_Method(0x008A11C0, static_cast<unsigned (ChunkSaveClass::*)(void const *, unsigned)>(&ChunkSaveClass::Write));

    // Replace ChunkLoadClass
    Hook_Method(0x008A1290, &ChunkLoadClass::Open_Chunk);
    Hook_Method(0x008A12E0, &ChunkLoadClass::Close_Chunk);
    Hook_Method(0x008A1330, &ChunkLoadClass::Cur_Chunk_ID);
    Hook_Method(0x008A1340, &ChunkLoadClass::Cur_Chunk_Length);
    Hook_Method(0x008A1350, &ChunkLoadClass::Open_Micro_Chunk);
    Hook_Method(0x008A1380, &ChunkLoadClass::Close_Micro_Chunk);
    Hook_Method(0x008A13D0, &ChunkLoadClass::Cur_Micro_Chunk_ID);
    Hook_Method(0x008A13E0, &ChunkLoadClass::Cur_Micro_Chunk_Length);
    Hook_Method(0x008A13F0, &ChunkLoadClass::Seek);
    Hook_Method(0x008A1480, static_cast<unsigned (ChunkLoadClass::*)(void *, unsigned)>(&ChunkLoadClass::Read));

    // targa.h
    Hook_Method(0x008A43F0, static_cast<int (TargaImage::*)(char const *, int, bool)>(&TargaImage::Load));
    Hook_Method(0x008A3E60, &TargaImage::Open);
    Hook_Method(0x008A45F0, &TargaImage::Set_Palette);
    Hook_Function(0x008A4780, &TargaImage::Error_Handler);

    // texturebase.h
    Hook_Method(0x0081A760, &TextureBaseClass::Invalidate);
    Hook_Method(0x0081A830, &TextureBaseClass::Get_Reduction);
    Hook_Method(0x0081A7A0, &TextureBaseClass::Peek_Platform_Base_Texture);
    Hook_Method(0x0081A7B0, &TextureBaseClass::Set_Platform_Base_Texture);

    // dx8caps.h
    Hook_Method(0x00844BB0, &DX8Caps::Compute_Caps);
    Hook_Method(0x00846FC0, &DX8Caps::Is_Valid_Display_Format);
    Hook_Method(0x008462D0, &DX8Caps::Check_Texture_Format_Support);
    Hook_Method(0x008464B0, &DX8Caps::Check_Render_To_Texture_Support);
    Hook_Method(0x00846690, &DX8Caps::Check_Depth_Stencil_Support);
    Hook_Method(0x008461E0, &DX8Caps::Check_Texture_Compression_Support);
    Hook_Method(0x00846870, &DX8Caps::Check_Shader_Support);
    Hook_Method(0x00844A90, &DX8Caps::Hook_Ctor1);
    Hook_Method(0x00844950, &DX8Caps::Hook_Ctor2);

    // dx8fvf.h
    Hook_Method(0x0086B6E0, &FVFInfoClass::Hook_Ctor);

    // dx8vertexbuffer.h
    Hook_Method(0x0081EF10, &VertexBufferClass::Release_Engine_Ref);
    Hook_Method(0x0081EF20, &VertexBufferClass::WriteLockClass::Hook_Ctor);
    Hook_Method(0x0081EFF0, &VertexBufferClass::WriteLockClass::Hook_Dtor);
    Hook_Method(0x0081EF80, &VertexBufferClass::AppendLockClass::Hook_Ctor);
    Hook_Method(0x0081EFF0, &VertexBufferClass::AppendLockClass::Hook_Dtor);
    Hook_Method(0x0081F030, &SortingVertexBufferClass::Hook_Ctor);
    Hook_Method(0x0081F1D0, &DX8VertexBufferClass::Hook_Ctor);
    Hook_Method(0x0081F480, &DynamicVBAccessClass::Hook_Ctor);
    Hook_Method(0x0081F4C0, &DynamicVBAccessClass::Hook_Dtor);
    Hook_Method(0x0081F510, &DynamicVBAccessClass::Deinit);
    Hook_Method(0x0081F930, &DynamicVBAccessClass::WriteLockClass::Hook_Ctor);
    Hook_Method(0x0081F9F0, &DynamicVBAccessClass::WriteLockClass::Hook_Dtor);
    Hook_Method(0x0081FA20, &DynamicVBAccessClass::Reset);
    Hook_Method(0x0081FA40, &DynamicVBAccessClass::Get_Default_Vertex_Count);

    // dx8indexbuffer.h
    Hook_Method(0x0081E0D0, &IndexBufferClass::Add_Engine_Ref);
    Hook_Method(0x0081E0E0, &IndexBufferClass::Release_Engine_Ref);
    Hook_Method(0x0081E0F0, &IndexBufferClass::WriteLockClass::Hook_Ctor);
    Hook_Method(0x0081E160, &IndexBufferClass::WriteLockClass::Hook_Dtor);
    Hook_Method(0x0081E1A0, &IndexBufferClass::AppendLockClass::Hook_Ctor);
    Hook_Method(0x0081E160, &IndexBufferClass::AppendLockClass::Hook_Dtor);
    Hook_Method(0x0081E210, &DX8IndexBufferClass::Hook_Ctor);
    Hook_Method(0x0081E450, &SortingIndexBufferClass::Hook_Ctor);
    Hook_Method(0x0081E610, &DynamicIBAccessClass::Hook_Ctor);
    Hook_Method(0x0081E6E0, &DynamicIBAccessClass::Hook_Dtor);
    Hook_Method(0x0081E770, &DynamicIBAccessClass::Deinit);
    Hook_Method(0x0081E7E0, &DynamicIBAccessClass::WriteLockClass::Hook_Ctor);
    Hook_Method(0x0081E880, &DynamicIBAccessClass::WriteLockClass::Hook_Dtor);
    Hook_Method(0x0081EC50, &DynamicIBAccessClass::Reset);
    Hook_Method(0x0081EC70, &DynamicIBAccessClass::Get_Default_Index_Count);
    Hook_Method(0x0081EC80, &DynamicIBAccessClass::Get_Next_Index);

    // bitmaphandler.h
    Hook_Function(0x0087E7A0, BitmapHandlerClass::Copy_Image);
    Hook_Function(0x008202B0, BitmapHandlerClass::Copy_Image_Generate_Mipmap);

    // thumbnailmanager.h
    Hook_Method(0x0086AE20, &ThumbnailManagerClass::Remove_From_Hash);
    Hook_Function(0x0086ABC0, &ThumbnailManagerClass::Peek_Thumbnail_Instance_From_Any_Manager);
    Hook_Function(0x0086AFE0, &ThumbnailManagerClass::Init);
    Hook_Function(0x0086B070, &ThumbnailManagerClass::Deinit);

    // thumnail.h
    Hook_Method(0x0086A040, &ThumbnailClass::Hook_Ctor);

    // textureloadtask.h
    Hook_Method(0x0082FF20, &TextureLoadTaskClass::Hook_Ctor);
    Hook_Method(0x0082FF80, &TextureLoadTaskClass::Hook_Dtor);
    Hook_Any(0x008301D0, TextureLoadTaskClass::Destroy);
    Hook_Any(0x00830210, TextureLoadTaskClass::Init);
    Hook_Any(0x00831160, TextureLoadTaskClass::Deinit);
    Hook_Any(0x008303B0, TextureLoadTaskClass::Begin_Compressed_Load);
    Hook_Any(0x00830950, TextureLoadTaskClass::Begin_Uncompressed_Load);
    Hook_Any(0x00830B30, TextureLoadTaskClass::Load_Compressed_Mipmap);
    Hook_Any(0x00830C40, TextureLoadTaskClass::Load_Uncompressed_Mipmap);
    Hook_Any(0x00830A80, TextureLoadTaskClass::Lock_Surfaces);
    Hook_Any(0x00830AE0, TextureLoadTaskClass::Unlock_Surfaces);
    Hook_Method(0x00830380, &TextureLoadTaskClass::Apply);
    Hook_Method(0x008302D0, &TextureLoadTaskClass::Begin_Load);
    Hook_Method(0x00830310, &TextureLoadTaskClass::Load);
    Hook_Method(0x00830340, &TextureLoadTaskClass::End_Load);
    Hook_Function(0x0082FFD0, &TextureLoadTaskClass::Create);
    Hook_Function(0x008305F0, &TextureLoadTaskClass::Get_Texture_Information);

    // textureloader.h
    Hook_Function(0x0082EC90, &TextureLoader::Init);
    Hook_Function(0x0082ECC0, &TextureLoader::Deinit);
    Hook_Function(0x0082EE10, &TextureLoader::Validate_Texture_Size);
    // Hook_Function(0x0082EED0, &TextureLoader::Load_Thumbnail);
    Hook_Function(0x0082F080, &TextureLoader::Load_Surface_Immediate);
    Hook_Function(0x0082F520, &TextureLoader::Request_Background_Loading);
    Hook_Function(0x0082F6A0, &TextureLoader::Request_Foreground_Loading);
    Hook_Function(0x0082F960, &TextureLoader::Flush_Pending_Load_Tasks);
    Hook_Function(0x0082FBC0, &TextureLoader::Update);

    // ddsfile.h
    Hook_Method(0x0087A170,
        static_cast<void (DDSFileClass::*)(unsigned, WW3DFormat, unsigned, unsigned, uint8_t *, unsigned, const Vector3 &)>(
            &DDSFileClass::Copy_Level_To_Surface));
    Hook_Method(0x0087A0E0,
        static_cast<void (DDSFileClass::*)(unsigned, w3dsurface_t, const Vector3 &)>(&DDSFileClass::Copy_Level_To_Surface));
    Hook_Method(0x0087AEE0, &DDSFileClass::Copy_CubeMap_Level_To_Surface);
    Hook_Method(0x0087BBF0, &DDSFileClass::Get_4x4_Block);
    Hook_Method(0x00879F80, &DDSFileClass::Load);
    Hook_Method(0x00879BF0, &DDSFileClass::Hook_Ctor);

    // shader.h
    Hook_Method(0x00813420, &ShaderClass::Init_From_Material3);
    Hook_Method(0x00813F80, &ShaderClass::Get_Static_Sort_Category);
    Hook_Method(0x00814010, &ShaderClass::Guess_Sort_Level);
    Hook_Method(0x00813F60, &ShaderClass::Invert_Backface_Culling);

    // multilist.h
    Hook_Method(0x008A03F0, &GenericMultiListClass::Contains);
    Hook_Method(0x008A0420, &GenericMultiListClass::Internal_Add);
    Hook_Method(0x008A0800, &GenericMultiListClass::Internal_Remove);
    Hook_Method(0x008A0880, &GenericMultiListClass::Internal_Remove_List_Head);
    Hook_Method(0x00824720, &GenericMultiListClass::Internal_Get_List_Head);

    // Replace filetransfer.h functions.
    Hook_Function(0x006F6200, &Get_Base_Path_From_Path);
    Hook_Function(0x006F6390, &Get_File_From_Path);
    Hook_Function(0x006F6490, &Get_Base_File_From_File);
    Hook_Function(0x006F6620, &Get_Preview_From_Map);

    // Replace GameState
    Hook_Method(0x004939A0, &GameState::Real_To_Portable_Map_Path);
    Hook_Method(0x00493C90, &GameState::Portable_To_Real_Map_Path);

    // weapon.h WeaponBonusSet
    Hook_Function(0x004C9860, WeaponBonusSet::Parse_Weapon_Bonus_Set_Ptr);

    // globaldata.h GlobalData
    Hook_Function(0x00418090, GlobalData::Parse_Game_Data_Definitions);

    // thread.h ThreadClass
    Hook_Method(0x0089CDC0, &ThreadClass::Execute);
    Hook_Method(0x0089CE10, &ThreadClass::Stop);
    Hook_Method(0x0089CDF0, &ThreadClass::Set_Priority);
    Hook_Function(0x0089CEF0, &ThreadClass::Switch_Thread);
    Hook_Function(0x0089CD10, &ThreadClass::Internal_Thread_Function);
    Hook_Function(0x0089CF00, &ThreadClass::Get_Current_Thread_ID);

    // dict.h Dict
    Hook_Method(0x00573660, &Dict::Release_Data);
    Hook_Method(0x00573750, &Dict::operator=);
    Hook_Method(0x00573820, &Dict::Get_Type);
    Hook_Method(0x00573880, &Dict::Get_Bool);
    Hook_Method(0x005738F0, &Dict::Get_Int);
    Hook_Method(0x00573960, &Dict::Get_Real);
    Hook_Method(0x005739E0, &Dict::Get_AsciiString);
    Hook_Method(0x00573A80, &Dict::Get_UnicodeString);
    Hook_Method(0x00573B00, &Dict::Sort_Pairs);
    Hook_Method(0x00573320, &Dict::Ensure_Unique);
    Hook_Method(0x00573BB0, &Dict::Set_Bool);
    Hook_Method(0x00573C60, &Dict::Set_Int);
    Hook_Method(0x00573D20, &Dict::Set_Real);
    Hook_Method(0x00573DE0, &Dict::Set_AsciiString);
    Hook_Method(0x00573EC0, &Dict::Set_UnicodeString);

    // compressionmanager.h CompressionManager
    Hook_Function(0x006F5FE0, &CompressionManager::Get_Compression_Type);
    Hook_Function(0x006F6110, &CompressionManager::Decompress_Data);

    // datachunk.h DataChunkInput
    Hook_Method(0x00572450, &DataChunkInput::Register_Parser);
    Hook_Method(0x00572680, &DataChunkInput::Parse);
    Hook_Method(0x005729D0, &DataChunkInput::Open_Data_Chunk);
    // Hook_Method(0x005729C0, &DataChunkInput::Is_Valid_File);
    // Hook_Method(0x00572BE0, &DataChunkInput::At_End_Of_Chunk);
    Hook_Method(0x00572C20, &DataChunkInput::Read_Real32);
    Hook_Method(0x00572C50, &DataChunkInput::Read_Int32);
    Hook_Method(0x00572C80, &DataChunkInput::Read_Byte);
    Hook_Method(0x00572CB0, &DataChunkInput::Read_Byte_Array);
    Hook_Method(0x00572CE0, &DataChunkInput::Read_Name_Key);
    Hook_Method(0x00572DA0, &DataChunkInput::Read_Dict);
    Hook_Method(0x00573010, &DataChunkInput::Read_AsciiString);
    Hook_Method(0x00573120, &DataChunkInput::Read_UnicodeString);

    // cave.h CaveSystem
    Hook_Method(0x004D5730, &CaveSystem::Can_Switch_Index_to_Index);
    Hook_Method(0x004D5790, &CaveSystem::Register_New_Cave);
    Hook_Method(0x004D5880, &CaveSystem::Get_Tunnel_Tracker_For_Cave_Index);

    // teamsinfo.h TeamsInfoRec
    Hook_Method(0x004D8F80, &TeamsInfoRec::Clear);
    Hook_Method(0x004D9050, &TeamsInfoRec::Find_Team);
    Hook_Method(0x004D91C0, &TeamsInfoRec::Add_Team);

    // scriptparam.h
    Hook_Function(0x0051F760, Parameter::Read_Parameter);
    Hook_Method(0x0051EB90, &Parameter::Get_UI_Text);
    Hook_Method(0x0051E990, &Parameter::Qualify);

    // scriptcondition.h Condition OrCondition
    Hook_Method(0x0051DD90, &Condition::Duplicate);
    Hook_Method(0x0051E0B0, &Condition::Duplicate_And_Qualify);
    Hook_Method(0x0051D7B0, &OrCondition::Duplicate);
    Hook_Method(0x0051D8A0, &OrCondition::Duplicate_And_Qualify);
    Hook_Function(0x0051D9B0, OrCondition::Parse_OrCondition_Chunk);

    // scriptaction.h ScriptAction
    Hook_Method(0x0051FF80, &ScriptAction::Duplicate);
    Hook_Method(0x00520240, &ScriptAction::Duplicate_And_Qualify);
    Hook_Function(0x00521240, &ScriptAction::Parse_Action_Chunk);
    Hook_Function(0x00521280, &ScriptAction::Parse_False_Action_Chunk);

    // script.h Script
    Hook_Any(0x0051CD70, Script::Xfer_Snapshot);
    Hook_Method(0x0051CDB0, &Script::Duplicate);
    Hook_Method(0x0051CFC0, &Script::Duplicate_And_Qualify);
    Hook_Function(0x0051D610, Script::Parse_Script_From_Group_Chunk);
    Hook_Function(0x0051D5B0, Script::Parse_Script_From_List_Chunk);

    // scriptgroup.h ScriptGroup
    Hook_Any(0x0051C3B0, ScriptGroup::Xfer_Snapshot);
    Hook_Method(0x0051C510, &ScriptGroup::Duplicate);
    Hook_Method(0x0051C670, &ScriptGroup::Duplicate_And_Qualify);
    Hook_Function(0x0051C860, ScriptGroup::Parse_Group_Chunk);

    // scriptlist.h ScriptList
    Hook_Any(0x0051B920, ScriptList::Xfer_Snapshot);
    Hook_Method(0x0051BC70, &ScriptList::Duplicate);
    Hook_Method(0x0051BD80, &ScriptList::Duplicate_And_Qualify);
    Hook_Function(0x0051C040, ScriptList::Get_Read_Scripts); // Must hooke with Parse_Scripts_Chunk
    Hook_Function(0x0051C080, ScriptList::Parse_Script_List_Chunk);
    Hook_Function(0x0051BF00, ScriptList::Parse_Scripts_Chunk); // Must hook with Get_Read_Scripts

    // sidesinfo.h SidesInfo
    Hook_Method(0x004D5C00, &SidesInfo::Init);
    Hook_Method(0x004D5C80, &SidesInfo::operator=);

    // sideslist.h SidesList
    Hook_Method(0x004D77A0, &SidesList::Clear);
    Hook_Method(0x004D7F70, &SidesList::Validate_Sides);
    Hook_Method(0x004D79A0, &SidesList::Validate_Ally_Enemy_List);
    Hook_Method(0x004D7BC0, &SidesList::Add_Player_By_Template);
    Hook_Method(0x004D6A60, &SidesList::Find_Side_Info);
    Hook_Method(0x004D6BD0, &SidesList::Find_Skirmish_Side_Info);
    Hook_Function(0x004D62A0, &SidesList::Parse_Sides_Chunk);

    // keyboard.h Keyboard
    Hook_Any(0x0040A120, Keyboard::Init);
    Hook_Any(0x0040A140, Keyboard::Update);
    Hook_Any(0x00407F50, Keyboard::Create_Stream_Messages);
    Hook_Method(0x0040A4A0, &Keyboard::Get_Printable_Key);
    Hook_Method(0x0040A150, &Keyboard::Reset_Keys);
    Hook_Method(0x0040A460, &Keyboard::Is_Shift);
    Hook_Method(0x0040A480, &Keyboard::Is_Ctrl);
    Hook_Method(0x0040A490, &Keyboard::Is_Alt);

    // mouse.h Mouse
    Hook_Any(0x004031F0, Mouse::Create_Stream_Messages);
    Hook_Method(0x004024E0, &Mouse::Process_Mouse_Event);
    Hook_Method(0x00403FC0, static_cast<void (Mouse::*)(const MouseCursor)>(&Mouse::Set_Mouse_Text));
    Hook_Method(0x00403A80,
        static_cast<void (Mouse::*)(const Utf16String, const RGBAColorInt *, const RGBAColorInt *)>(&Mouse::Set_Mouse_Text));

    // geometry.h
    Hook_Any(0x005CFAF0, GeometryInfo::Xfer_Snapshot);
    Hook_Method(0x005CF220, &GeometryInfo::Set);
    Hook_Method(0x005CFA60, &GeometryInfo::Calc_Bounding_Stuff);
    Hook_Method(0x005CF350, &GeometryInfo::Set_Max_Height_Above_Position);
    Hook_Method(0x005CF380, &GeometryInfo::Get_Max_Height_Above_Position);
    Hook_Method(0x005CF3A0, &GeometryInfo::Get_Max_Height_Below_Position);
    Hook_Method(0x005CF3C0, &GeometryInfo::Get_ZDelta_To_Center_Position);
    Hook_Method(0x005CF3E0, &GeometryInfo::Get_Center_Position);
    Hook_Method(0x005CFA20, &GeometryInfo::Get_Footprint_Area);
    Hook_Method(0x005CF430, &GeometryInfo::Expand_Footprint);
    Hook_Method(0x005CF290, &GeometryInfo::Calc_Pitches);
    Hook_Method(0x005CF450, &GeometryInfo::Get_2D_Bounds);
    Hook_Method(0x005CF620, &GeometryInfo::Clip_Point_To_Footprint);
    Hook_Method(0x005CF720, &GeometryInfo::Is_Point_In_Footprint);
    Hook_Method(0x005CF7E0, &GeometryInfo::Make_Random_Offset_In_Footprint);
    Hook_Method(0x005CF8E0, &GeometryInfo::Make_Random_Offset_In_Perimeter);
    Hook_Function(0x005CF200, GeometryInfo::Parse_Geometry_MinorRadius);
    Hook_Function(0x005CF1E0, GeometryInfo::Parse_Geometry_MajorRadius);
    Hook_Function(0x005CF1C0, GeometryInfo::Parse_Geometry_Height);
    Hook_Function(0x005CF1A0, GeometryInfo::Parse_Geometry_IsSmall);
    Hook_Function(0x005CF170, GeometryInfo::Parse_Geometry_Type);

    // modulefactory.h
    // Virtual functions not hooked here, hook to create factory in Win32GameEngine handles vtable.
    Hook_Method(0x004F2D60, &ModuleFactory::Make_Decorated_Name_Key);
    Hook_Method(0x004F2E80, &ModuleFactory::Add_Module_Internal);
    Hook_Method(0x004F2B80, &ModuleFactory::Find_Interface_Mask);
    Hook_Method(0x004F2DD0, &ModuleFactory::New_Module);
    Hook_Method(0x004F2C20, &ModuleFactory::New_Module_Data_From_INI);

    // rankinfo.h
    Hook_Function(0x00489520, &RankInfoStore::Parse_Rank_Definition);
    // Hook_Any(0x00489410, RankInfoStore::Init); // Works, but shares function in binary with ScienceStore as
    // compiles to same code.
    Hook_Any(0x00489440, RankInfoStore::Reset);
    Hook_Method(0x004894E0, &RankInfoStore::Get_Rank_Info);
    Hook_Method(0x004894D0, &RankInfoStore::Get_Rank_Level_Count);

    // playertemplate.h
    Hook_Method(0x004D3170, &PlayerTemplateStore::Get_Template_Number_By_Name);
    Hook_Method(0x004D32D0, &PlayerTemplateStore::Find_Player_Template);
    Hook_Method(0x004D35E0, &PlayerTemplateStore::Get_Nth_Player_Template);
    Hook_Method(0x004D3630, &PlayerTemplateStore::Get_All_Side_Strings);
    Hook_Function(0x004D3860, &PlayerTemplateStore::Parse_Player_Template_Definitions);

    // partsysinfo.h
    Hook_Method(0x004CD460, &ParticleSystemInfo::Hook_Ctor);
    Hook_Any(0x004CD5D0, ParticleSystemInfo::Xfer_Snapshot);
    Hook_Method(0x004CD540, &ParticleSystemInfo::Tint_All_Colors);

    // particle.h
    Hook_Method(0x004CCC30, &Particle::Hook_Ctor);
    Hook_Method(0x004CD040, &Particle::Hook_Dtor);
    Hook_Any(0x004CD2E0, Particle::Xfer_Snapshot);
    Hook_Any(0x004CD3F0, Particle::Load_Post_Process);
    Hook_Method(0x004CCB50, &Particle::Compute_Alpha_Rate);
    Hook_Method(0x004CCBA0, &Particle::Compute_Color_Rate);

    // particlesys.h
    Hook_Method(0x004CDA10, &ParticleSystem::Hook_Ctor);
    Hook_Method(0x004CE310, &ParticleSystem::Hook_Dtor);
    Hook_Any(0x004D11E0, ParticleSystem::Xfer_Snapshot);
    Hook_Any(0x004D1430, ParticleSystem::Load_Post_Process);
    Hook_Method(0x004CE500, &ParticleSystem::Destroy);
    Hook_Method(0x004CE530, &ParticleSystem::Get_Position);
    Hook_Method(0x004CE570, &ParticleSystem::Set_Position);
    Hook_Method(0x004CE5A0, &ParticleSystem::Set_Local_Transform);
    Hook_Method(0x004CE620, &ParticleSystem::Rotate_Local_Transform_X);
    Hook_Method(0x004CE6C0, &ParticleSystem::Rotate_Local_Transform_Y);
    Hook_Method(0x004CE760, &ParticleSystem::Rotate_Local_Transform_Z);
    Hook_Method(0x004CE860, &ParticleSystem::Compute_Particle_Velocity);
    Hook_Method(0x004CEF60, &ParticleSystem::Compute_Particle_Position);
    Hook_Any(0x004CF530, ParticleSystem::Create_Particle);
    Hook_Method(0x004CF750, &ParticleSystem::Generate_Particle_Info);
    Hook_Method(0x004D0920, &ParticleSystem::Update_Wind_Motion);
    Hook_Function(0x004D0B30, &ParticleSystem::Merge_Related_Systems);

    // particlesysmanager.h
    Hook_Method(0x004D1790, &ParticleSystemManager::Hook_Ctor);
    Hook_Method(0x004D18E0, &ParticleSystemManager::Hook_Dtor);
    Hook_Any(0x004D1BA0, ParticleSystemManager::Init);
    Hook_Any(0x004D1C40, ParticleSystemManager::Reset);
    Hook_Any(0x004D2460, ParticleSystemManager::Xfer_Snapshot);
    Hook_Method(0x004D1EB0, &ParticleSystemManager::Find_Template);
    Hook_Method(0x004D1EE0, &ParticleSystemManager::New_Template);
    Hook_Method(0x004D2130, &ParticleSystemManager::Find_Parent_Template);
    Hook_Method(0x004D1D40, &ParticleSystemManager::Create_Particle_System);
    Hook_Method(0x004D1E30, &ParticleSystemManager::Find_Particle_System);
    Hook_Method(0x004D1E60, &ParticleSystemManager::Destroy_Particle_System_By_ID);
    Hook_Method(0x004D22D0, &ParticleSystemManager::Remove_Particle);

#ifdef BUILD_WITH_BINK
    // binkvideostream.h
    Hook_Any(0x007AAA20, BinkVideoStream::Update);
    Hook_Any(0x007AAA30, BinkVideoStream::Is_Frame_Ready);
    Hook_Any(0x007AAA40, BinkVideoStream::Decompress_Frame);
    Hook_Any(0x007AAA50, BinkVideoStream::Render_Frame);
    Hook_Any(0x007AAAD0, BinkVideoStream::Next_Frame);
    Hook_Any(0x007AAB00, BinkVideoStream::Goto_Frame);

    // binkvideoplayer.h
    Hook_Any(0x007AA550, BinkVideoPlayer::Init);
    Hook_Any(0x007AA570, BinkVideoPlayer::Deinit);
    Hook_Any(0x007AA6A0, BinkVideoPlayer::Open);
    Hook_Any(0x007AA8E0, BinkVideoPlayer::Load);
    Hook_Any(0x007AA970, BinkVideoPlayer::Notify_Player_Of_New_Provider);
    Hook_Any(0x007AA9A0, BinkVideoPlayer::Initialise_Bink_With_Miles);
    Hook_Method(0x007AA5B0, &BinkVideoPlayer::Create_Stream);
#endif

    // audioeventrts.h AudioEventRTS
    Hook_Method(0x00445080, &AudioEventRTS::Generate_Filename);
    Hook_Method(0x00445380, &AudioEventRTS::Generate_Play_Info);
    Hook_Method(0x00445A20, &AudioEventRTS::Generate_Filename_Prefix);
    Hook_Method(0x00445C30, &AudioEventRTS::Generate_Filename_Extension);
    Hook_Method(0x00445D80, &AudioEventRTS::Adjust_For_Localization);
    Hook_Method(0x00444F90, &AudioEventRTS::Set_Event_Name);
    Hook_Method(0x00445900, &AudioEventRTS::Is_Positional_Audio);
    Hook_Method(0x00445950, &AudioEventRTS::Get_Volume);

    // soundmanager.h SoundManager
    Hook_Any(0x00445FF0, SoundManager::Reset);
    Hook_Any(0x00446010, SoundManager::Add_Audio_Event);
    // Hook_Any(0x00446120, SoundManager::Can_Play_Now); // Not implemented.
    Hook_Any(0x004462F0, SoundManager::Violates_Voice);
    Hook_Any(0x00446350, SoundManager::Is_Interrupting);

    /* These are all hooked via Win32Engine::Create_Audio_Manager.
    // audiomanager.h AudioManager
    Hook_Any(0x00404C60, AudioManager::Init);
    Hook_Any(0x00404F30, AudioManager::Reset);
    // Hook_Any(0x00404F30, AudioManager::Update); // Not Implemented.
    Hook_Any(0x00406050, AudioManager::Set_Listener_Position);
    Hook_Any(0x00406090, AudioManager::Allocate_Audio_Request);
    Hook_Any(0x004061A0, AudioManager::Release_Audio_Request);
    Hook_Any(0x004061D0, AudioManager::Append_Audio_Request);
    Hook_Any(0x00406250, AudioManager::New_Audio_Event_Info);
    Hook_Any(0x004065D0, AudioManager::Add_Audio_Event_Info);
    Hook_Any(0x004067B0, AudioManager::Find_Audio_Event_Info);
    Hook_Any(0x00406920, AudioManager::Refresh_Cached_Variables);
    Hook_Any(0x00406970, AudioManager::Get_Audio_Length_MS);
    Hook_Any(0x00406A90, AudioManager::Is_Music_Already_Loaded);
    Hook_Any(0x00406C10, AudioManager::Find_All_Audio_Events_Of_Type);
    Hook_Any(0x005B9460, AudioManager::Get_All_Audio_Events);
    Hook_Any(0x00406D00, AudioManager::Is_Current_Provider_Hardware_Accelerated);
    Hook_Any(0x00406DE0, AudioManager::Is_Current_Speaker_Type_Surround);
    // Hook_Any(0x00406E00, AudioManager::Should_Play_Locally); // Not Implemented.
    Hook_Any(0x00406F00, AudioManager::Allocate_New_Handle);
    Hook_Any(0x00406860, AudioManager::Remove_Level_Specific_Audio_Event_Infos);
    Hook_Method(0x00406200, &AudioManager::Remove_All_Audio_Requests);
    Hook_Method(0x00405700, &AudioManager::Next_Track_Name);
    Hook_Method(0x004057D0, &AudioManager::Prev_Track_Name);
    Hook_Any(0x00406F10, AudioManager::Lose_Focus);
    Hook_Any(0x00406F70, AudioManager::Regain_Focus);
    Hook_Any(0x00405390, AudioManager::Add_Audio_Event);
    Hook_Any(0x004058A0, AudioManager::Remove_Audio_Event);
    Hook_Any(0x00405C30, AudioManager::Remove_Audio_Event_String);
    Hook_Any(0x00405680, AudioManager::Is_Valid_Const_Audio_Event);
    Hook_Any(0x004058F0, AudioManager::Set_Audio_Event_Enabled);
    Hook_Any(0x00405990, AudioManager::Set_Audio_Event_Volume_Override);
    Hook_Any(0x00405CC0, AudioManager::Remove_Disabled_Events);
    Hook_Any(0x00405340, AudioManager::Get_Info_For_Audio_Event);
    Hook_Any(0x00405CD0, AudioManager::Translate_From_Speaker_Type);
    Hook_Any(0x00405DD0, AudioManager::Translate_To_Speaker_Type);
    Hook_Any(0x00405E50, AudioManager::Is_On);
    Hook_Any(0x00405E90, AudioManager::Set_On);
    Hook_Any(0x00405F20, AudioManager::Set_Volume);
    Hook_Any(0x00405FC0, AudioManager::Get_Volume);
    Hook_Any(0x00405FF0, AudioManager::Set_3D_Volume_Adjustment);
#ifdef BUILD_WITH_MILES
    // milesaudiofilecache.h MilesAudioFileCache
    Hook_Method(0x00780F80, &MilesAudioFileCache::Open_File);
    Hook_Method(0x007813D0, &MilesAudioFileCache::Close_File);
    Hook_Method(0x007814D0, &MilesAudioFileCache::Free_Space_For_Sample);

    // milesaudiomanager.h MilesAudioManager
    Hook_Function(0x00780C90, &MilesAudioManager::Streaming_File_Open);
    Hook_Function(0x00780CC0, &MilesAudioManager::Streaming_File_Close);
    Hook_Function(0x00780CD0, &MilesAudioManager::Streaming_File_Seek);
    Hook_Function(0x00780CF0, &MilesAudioManager::Streaming_File_Read);
    Hook_Function(0x00780C70, &MilesAudioManager::Set_Stream_Complete);
    Hook_Function(0x00780C30, &MilesAudioManager::Set_Sample_Complete);
    Hook_Function(0x00780C50, &MilesAudioManager::Set_3DSample_Complete);
    Hook_Method(0x0077D9B0, &MilesAudioManager::Release_Playing_Audio);
    Hook_Method(0x0077D8E0, &MilesAudioManager::Release_Miles_Handles);
    Hook_Method(0x0077DB80, &MilesAudioManager::Free_All_Miles_Handles);
    Hook_Method(0x0077DA30, &MilesAudioManager::Stop_All_Audio_Immediately);
    Hook_Method(0x00780520, &MilesAudioManager::Play_Sample3D);
    Hook_Method(0x00780400, &MilesAudioManager::Start_Next_Loop);
    Hook_Method(0x0077DD80, &MilesAudioManager::Init_Filters);
    Hook_Method(0x007806F0, &MilesAudioManager::Init_Sample_Pools);
    Hook_Method(0x0077D080, &MilesAudioManager::Play_Audio_Event);
    Hook_Method(0x0077D630, &MilesAudioManager::Stop_Audio_Event);
    Hook_Method(0x0077DD20, &MilesAudioManager::Stop_All_Speech);
    Hook_Method(0x0077DC30, &MilesAudioManager::Adjust_Playing_Volume);
    Hook_Method(0x00780280, &MilesAudioManager::Get_Effective_Volume);
    Hook_Method(0x0077F330, &MilesAudioManager::Kill_Lowest_Priority_Sound_Immediately);
    Hook_Method(0x0077F1C0, &MilesAudioManager::Find_Lowest_Priority_Sound);
    // Hook virtuals
    Hook_Any(0x0077CCE0, MilesAudioManager::Init);
    Hook_Any(0x0077CD30, MilesAudioManager::Reset);
    Hook_Any(0x0077CD60, MilesAudioManager::Update);
    Hook_Any(0x0077CDA0, MilesAudioManager::Stop_Audio);
    Hook_Any(0x0077CEB0, MilesAudioManager::Pause_Audio);
    Hook_Any(0x0077CFC0, MilesAudioManager::Resume_Audio);
    Hook_Any(0x0077D780, MilesAudioManager::Kill_Event_Immediately);
    Hook_Any(0x0077DE80, MilesAudioManager::Next_Music_Track);
    Hook_Any(0x0077E020, MilesAudioManager::Prev_Music_Track);
    Hook_Any(0x0077E1C0, MilesAudioManager::Is_Music_Playing);
    Hook_Any(0x0077E200, MilesAudioManager::Has_Music_Track_Completed);
    Hook_Any(0x0077E2D0, MilesAudioManager::Music_Track_Name);
    Hook_Any(0x0077E5F0, MilesAudioManager::Is_Currently_Playing);
    Hook_Any(0x0077E3B0, MilesAudioManager::Open_Device);
    Hook_Any(0x0077E5D0, MilesAudioManager::Close_Device);
    Hook_Any(0x0077E6C0, MilesAudioManager::Notify_Of_Audio_Completion);
    Hook_Any(0x0077E900, MilesAudioManager::Get_Provider_Name);
    Hook_Any(0x0077E970, MilesAudioManager::Get_Provider_Index);
    Hook_Any(0x0077EA60, MilesAudioManager::Select_Provider);
    Hook_Any(0x0077ECB0, MilesAudioManager::Unselect_Provider);
    Hook_Any(0x0077ED40, MilesAudioManager::Set_Speaker_Type);
    Hook_Any(0x0077ED80, MilesAudioManager::Get_Speaker_Type);
    Hook_Any(0x0077EDD0, MilesAudioManager::Does_Violate_Limit);
    Hook_Any(0x0077F2A0, MilesAudioManager::Is_Playing_Lower_Priority);
    Hook_Any(0x0077F020, MilesAudioManager::Is_Playing_Already);
    Hook_Any(0x0077F130, MilesAudioManager::Is_Object_Playing_Voice);
    Hook_Any(0x0077F400, MilesAudioManager::Adjust_Volume_Of_Playing_Audio);
    Hook_Any(0x0077F6C0, MilesAudioManager::Remove_Playing_Audio);
    Hook_Any(0x0077F8F0, MilesAudioManager::Remove_All_Disabled_Audio);
    Hook_Any(0x0077FD80, MilesAudioManager::Has_3D_Sensitive_Streams_Playing);
    Hook_Any(0x00780820, MilesAudioManager::Get_Bink_Handle);
    Hook_Any(0x007809C0, MilesAudioManager::Release_Bink_Handle);
    Hook_Any(0x007809E0, MilesAudioManager::Force_Play_Audio_Event);
    Hook_Any(0x0077FA00, MilesAudioManager::Process_Request_List);
    Hook_Any(0x0077FF30, MilesAudioManager::Set_Hardware_Accelerated);
    Hook_Any(0x00780020, MilesAudioManager::Set_Speaker_Surround);
    Hook_Any(0x0077C970, MilesAudioManager::Set_Preferred_3D_Provider);
    Hook_Any(0x0077CA10, MilesAudioManager::Set_Preferred_Speaker);
    Hook_Any(0x00780090, MilesAudioManager::Get_Audio_Length_MS);
    Hook_Any(0x00780190, MilesAudioManager::Close_Any_Sample_Using_File);
    Hook_Any(0x00780230, MilesAudioManager::Set_Device_Listener_Position);
    Hook_Any(0x0077E860, MilesAudioManager::Find_Playing_Audio_From);
    Hook_Any(0x0077FB10, MilesAudioManager::Process_Playing_List);
    Hook_Any(0x0077FDE0, MilesAudioManager::Process_Fading_List);
    Hook_Any(0x0077FEE0, MilesAudioManager::Process_Stopped_List);
    Hook_Method(0x0077C700, &MilesAudioManager::Hook_Ctor);
    Hook_Method(0x0077CAD0, &MilesAudioManager::Hook_Dtor);
#endif
    */

    // gamemath.h
    Hook_Function(0x00537580, &GameMath::Sin);
    Hook_Function(0x00537590, &GameMath::Cos);
    Hook_Function(0x005375B0, &GameMath::Asin);
    Hook_Function(0x005375A0, &GameMath::Acos);

    // functionlexicon.h
    Hook_Method(0x004F3D00, &FunctionLexicon::Load_Table);

    // render2d.h
    Hook_Method(
        0x0080A790, static_cast<void (Render2DClass::*)(const RectClass &, float, uint32_t)>(&Render2DClass::Add_Outline));
    Hook_Method(0x0080A580, &Render2DClass::Add_Rect);
    Hook_Method(0x0080A350,
        static_cast<void (Render2DClass::*)(const Vector2 &, const Vector2 &, float, uint32_t, uint32_t)>(
            &Render2DClass::Add_Line));
    Hook_Method(0x00809EC0,
        static_cast<void (Render2DClass::*)(const Vector2 &, const Vector2 &, float, uint32_t)>(&Render2DClass::Add_Line));
    Hook_Method(0x00809CA0, &Render2DClass::Add_Tri);
    Hook_Method(0x00809AF0,
        static_cast<void (Render2DClass::*)(const RectClass &, const RectClass &, uint32_t)>(&Render2DClass::Add_Quad));
    Hook_Method(0x008096C0, &Render2DClass::Set_Coordinate_Range);
    Hook_Method(0x00809690, &Render2DClass::Enable_Texturing);
    Hook_Method(0x00809640, &Render2DClass::Enable_Additive);
    Hook_Method(0x008095F0, &Render2DClass::Enable_Alpha);
    Hook_Any(0x008094E0, Render2DClass::Reset);
    Hook_Method(0x008090C0, &Render2DClass::Hook_Ctor);

    // wwstring.h
    Hook_Method(0x0089D4E0, &StringClass::Format);

    // cpudetect.h
    // This nulls out CPUDetect::Init_Compact_Log in the original binary.
    // This is a fix for an odd crash in the windows CRT related to vsnprintf.
    Hook_Function(0x0089FA20, Null_Func);

    // dx8wrapper.h
    Hook_Method(0x00800670, &DX8Wrapper::Init);
    Hook_Method(0x00800860, &DX8Wrapper::Shutdown);
    Hook_Method(0x008009E0, &DX8Wrapper::Invalidate_Cached_Render_States);
    Hook_Method(0x00800F50, &DX8Wrapper::Reset_Device);
    Hook_Method(0x00801DA0, &DX8Wrapper::Set_Render_Device);
    Hook_Method(0x008021E0, &DX8Wrapper::Has_Stencil);
    Hook_Method(0x00802200, &DX8Wrapper::Get_Render_Device);
    Hook_Method(0x00802210, &DX8Wrapper::Get_Render_Device_Desc);
    Hook_Method(0x00802240, &DX8Wrapper::Set_Device_Resolution);
    Hook_Method(0x00802370, &DX8Wrapper::Get_Device_Resolution);
    Hook_Method(0x008023A0, &DX8Wrapper::Get_Render_Target_Resolution);
    Hook_Method(0x00802820, &DX8Wrapper::Begin_Scene);
    Hook_Method(0x00802840, &DX8Wrapper::End_Scene);
    Hook_Method(0x00802A00, &DX8Wrapper::Clear);
    Hook_Method(0x00802B50, &DX8Wrapper::Set_Viewport);
    Hook_Method(0x00802B70, static_cast<void (*)(const VertexBufferClass *, int)>(&DX8Wrapper::Set_Vertex_Buffer));
    Hook_Method(0x00802C10, static_cast<void (*)(const IndexBufferClass *, unsigned short)>(&DX8Wrapper::Set_Index_Buffer));
    Hook_Method(0x00802CB0, static_cast<void (*)(const DynamicVBAccessClass &)>(&DX8Wrapper::Set_Vertex_Buffer));
    Hook_Method(
        0x00802D40, static_cast<void (*)(const DynamicIBAccessClass &, unsigned short)>(&DX8Wrapper::Set_Index_Buffer));
    Hook_Method(0x008031B0,
        static_cast<void (*)(unsigned int, unsigned short, unsigned short, unsigned short, unsigned short)>(
            &DX8Wrapper::Draw_Triangles));
    Hook_Method(0x00803200,
        static_cast<void (*)(unsigned short, unsigned short, unsigned short, unsigned short)>(&DX8Wrapper::Draw_Triangles));
    Hook_Method(0x00803220, &DX8Wrapper::Draw_Strip);
    Hook_Method(0x00803240, &DX8Wrapper::Apply_Render_State_Changes);
    Hook_Method(0x008036F0,
        static_cast<w3dtexture_t (*)(unsigned int, unsigned int, WW3DFormat, MipCountType, w3dpool_t, bool)>(
            &DX8Wrapper::Create_Texture));
    Hook_Method(0x00803990, static_cast<w3dtexture_t (*)(w3dsurface_t, MipCountType)>(&DX8Wrapper::Create_Texture));
    Hook_Method(
        0x00803E90, static_cast<w3dsurface_t (*)(unsigned int, unsigned int, WW3DFormat)>(&DX8Wrapper::Create_Surface));
    Hook_Method(0x00803EE0, static_cast<w3dsurface_t (*)(const char *)>(&DX8Wrapper::Create_Surface));
    Hook_Method(0x008041B0, &DX8Wrapper::Set_Light);
    Hook_Method(0x008044B0, &DX8Wrapper::Get_DX8_Front_Buffer);
    Hook_Method(0x00804520, &DX8Wrapper::Get_DX8_Back_Buffer);
    Hook_Method(0x008045E0, &DX8Wrapper::Create_Render_Target);
    Hook_Method(0x00804780, &DX8Wrapper::Set_Render_Target_With_Z);
    Hook_Method(0x008047E0, &DX8Wrapper::Set_Render_Target);
    Hook_Method(0x00804AC0, &DX8Wrapper::Set_Gamma);
    Hook_Method(0x00804D00, &DX8Wrapper::Get_DX8_Render_State_Value_Name);
    Hook_Method(0x00805520, &DX8Wrapper::Get_DX8_Texture_Stage_State_Value_Name);
    Hook_Method(0x00805B60, &DX8Wrapper::Get_Back_Buffer_Format);

    // dinputkeybd.h
    // Hooks all the virtual functions for DirectInputKeyboard.
    Hook_Method(0x007ACC70, &DirectInputKeyboard::Hook_Ctor);

    // dx8texman.h
    Hook_Function(0x0084A9C0, &DX8TextureManagerClass::Add);
    Hook_Function(0x0084AA60, &DX8TextureManagerClass::Recreate_Textures);
    Hook_Function(0x0084AA30, &DX8TextureManagerClass::Release_Textures);
    Hook_Function(0x0084A9E0, &DX8TextureManagerClass::Remove);
    Hook_Function(0x0084A990, &DX8TextureManagerClass::Shutdown);

    // texture.h
    Hook_Method(0x0081A150, &TextureFilterClass::Hook_Ctor);
    Hook_Method(0x0081A180, &TextureFilterClass::Apply);
    Hook_Function(0x0081A220, &TextureFilterClass::Init_Filters);
    Hook_Method(0x0081A900, &TextureClass::Hook_Ctor1);
    Hook_Method(0x0081AA30, &TextureClass::Hook_Ctor2);
    Hook_Method(0x0081AC50, &TextureClass::Hook_Ctor3);

    // surfaceclass.h
    Hook_Method(0x0081D2B0, &SurfaceClass::Hook_Ctor1);
    Hook_Method(0x0081D430, &SurfaceClass::Hook_Ctor2);
    Hook_Method(0x0081D380, &SurfaceClass::Hook_Ctor3);
    Hook_Method(0x0081D4E0, &SurfaceClass::Hook_Dtor);
    Hook_Method(0x0081D8D0, &SurfaceClass::FindBB);
    Hook_Method(0x0081DBA0, &SurfaceClass::Is_Transparent_Column);
    Hook_Method(0x0081D550, &SurfaceClass::Get_Description);
    Hook_Method(0x0081D5B0, static_cast<void *(SurfaceClass::*)(int *)>(&SurfaceClass::Lock));
    Hook_Method(0x0081D610, &SurfaceClass::Unlock);
    Hook_Method(0x0081D630, &SurfaceClass::Clear);
    Hook_Method(0x0081D740,
        static_cast<void (SurfaceClass::*)(unsigned, unsigned, unsigned, unsigned, unsigned, unsigned, SurfaceClass *)>(
            &SurfaceClass::Copy));
    Hook_Method(0x0081DD90, &SurfaceClass::Draw_Pixel);

    // image.h
    Hook_Method(0x00519070, &Image::Hook_Ctor);
    Hook_Method(0x005191C0, &Image::Hook_Dtor);
    // Hook_Method(0x00000000, &Image::Clear_Status);
    Hook_Method(0x00519260, &Image::Set_Status);
    Hook_Method(0x00518F10, &Image::Parse_Image_Coords);
    Hook_Method(0x00519030, &Image::Parse_Image_Status);

    // w3droadbuffer.h
    Hook_Method(0x00795F90, &W3DRoadBuffer::Hook_Dtor);
    Hook_Method(0x00790890, &W3DRoadBuffer::Add_Map_Object);
    Hook_Method(0x00790F50, &W3DRoadBuffer::Add_Map_Objects);
    Hook_Method(0x00794360, &W3DRoadBuffer::Adjust_Stacking);
    Hook_Method(0x007960A0, &W3DRoadBuffer::Allocate_Road_Buffers);
    Hook_Method(0x00790400, &W3DRoadBuffer::Check_Link_After);
    Hook_Method(0x00790660, &W3DRoadBuffer::Check_Link_Before);
    Hook_Method(0x00796320, &W3DRoadBuffer::Clear_All_Roads);
    Hook_Method(0x00796530, &W3DRoadBuffer::Draw_Roads);
    Hook_Method(0x00794360, &W3DRoadBuffer::Find_Cross_Type_Join_Vector);
    Hook_Method(0x00796040, &W3DRoadBuffer::Free_Road_Buffers);
    Hook_Method(0x007938D0, &W3DRoadBuffer::Insert_4Way);
    Hook_Method(0x00794720, &W3DRoadBuffer::Insert_Cross_Type_Joins);
    Hook_Method(0x00795290, &W3DRoadBuffer::Insert_Curve_Segment_At);
    Hook_Method(0x00794250, &W3DRoadBuffer::Insert_Curve_Segments);
    Hook_Method(0x00791940, &W3DRoadBuffer::Insert_Tee);
    Hook_Method(0x00794180, &W3DRoadBuffer::Insert_Tee_Intersections);
    Hook_Method(0x00792320, &W3DRoadBuffer::Insert_Y);
    Hook_Method(0x0078DD20, &W3DRoadBuffer::Load_Alpha_Join);
    Hook_Method(0x0078F2C0, &W3DRoadBuffer::Load_Curve);
    Hook_Method(0x0078E790, &W3DRoadBuffer::Load_Float_4Pt_Section);
    Hook_Method(0x0078E550, &W3DRoadBuffer::Load_Float_Section);
    Hook_Method(0x0078E210, &W3DRoadBuffer::Load_H);
    Hook_Method(0x0078FE10, &W3DRoadBuffer::Load_Road_Segment);
    Hook_Method(0x007963F0, &W3DRoadBuffer::Load_Roads);
    Hook_Method(0x0078FB90, &W3DRoadBuffer::Load_Roads_In_Vertex_And_Index_Buffers);
    Hook_Method(0x0078DBC0, &W3DRoadBuffer::Load_Tee);
    Hook_Method(0x0078DF90, &W3DRoadBuffer::Load_Y);
    Hook_Method(0x00794EB0, &W3DRoadBuffer::Miter);
    Hook_Method(0x0078FEF0, &W3DRoadBuffer::Move_Road_Seg_To);
    Hook_Method(0x00792CB0, &W3DRoadBuffer::Offset_3Way);
    Hook_Method(0x00793580, &W3DRoadBuffer::Offset_4Way);
    Hook_Method(0x00792F60, &W3DRoadBuffer::Offset_H);
    Hook_Method(0x007932E0, &W3DRoadBuffer::Offset_Y);
    Hook_Method(0x0078F780, &W3DRoadBuffer::Preload_Road_Segment);
    Hook_Method(0x0078F890, &W3DRoadBuffer::Preload_Roads_In_Vertex_And_Index_Buffers);
    Hook_Method(0x00795F20, &W3DRoadBuffer::Rotate_About);
    Hook_Method(0x007963C0, &W3DRoadBuffer::Set_Map);
    Hook_Method(0x00796520, &W3DRoadBuffer::Update_Center);
    Hook_Method(0x00791730, &W3DRoadBuffer::Update_Counts_And_Flags);
    Hook_Method(0x00791590, &W3DRoadBuffer::Update_Counts);
    Hook_Method(0x00796430, &W3DRoadBuffer::Update_Lighting);
    Hook_Method(0x0078FCF0, &W3DRoadBuffer::Visibility_Changed);
    Hook_Method(0x00796000, &W3DRoadBuffer::Hook_Ctor);

    // rayeffect.h
    Hook_Method(0x00613AB0, &RayEffectSystem::Add_Ray_Effect);
    Hook_Method(0x00613B30, &RayEffectSystem::Delete_Ray_Effect);
    Hook_Method(0x00613B70, &RayEffectSystem::Get_Ray_Effect_Data);

    // gamefont.h
    Hook_Method(0x0041FE60, &FontLibrary::Unlink_Font);
    Hook_Method(0x00420000, &FontLibrary::Get_Font);

    // wininstancedata.h
    Hook_Method(0x006DF300, &WinInstanceData::Hook_Ctor);
    Hook_Method(0x006DF4C0, &WinInstanceData::Init);
    Hook_Method(0x006DF680, &WinInstanceData::Set_Text);
    Hook_Method(0x004F8F30, &WinInstanceData::Get_Text);
    Hook_Method(0x006DF600, &WinInstanceData::Set_Tooltip_Text);
    Hook_Method(0x004F93E0, &WinInstanceData::Get_Tooltip_Text);
    Hook_Method(0x006DF700, &WinInstanceData::Set_VideoBuffer);

    // w3ddebugdisplay.h
    Hook_Any(0x007F5CD0, W3DDebugDisplay::Printf);
    Hook_Any(0x007F5C70, W3DDebugDisplay::Reset);
    Hook_Method(0x00763A40, &W3DDebugDisplay::Hook_Ctor);
    Hook_Any(0x00763A40, W3DDebugDisplay::Draw_Text);
    Hook_Method(0x00763A20, &W3DDebugDisplay::Init);
    Hook_Method(0x00763B50, &W3DDebugDisplay::Set_Font);

    // w3dpoly.h
    Hook_Method(0x007A2BA0, &ClipPolyClass::Add_Vertex);
    Hook_Method(0x007A2C00, &ClipPolyClass::Clip);
    Hook_Method(0x007A2F50, &ClipPolyClass::Reset);

    // render2dsentence.h
    Hook_Method(0x00842CD0, &FontCharsClass::Hook_Ctor);
    Hook_Method(0x00842E60, &FontCharsClass::Hook_Dtor);
    Hook_Method(0x00843680, &FontCharsClass::Initialize_GDI_Font);
    Hook_Method(0x00843710, &FontCharsClass::Is_Font);
    Hook_Method(0x00843000, &FontCharsClass::Get_Char_Spacing);
    Hook_Method(0x00843090, &FontCharsClass::Blit_Char);
    Hook_Method(0x00843470, &FontCharsClass::Create_GDI_Font);
    Hook_Method(0x00843190, &FontCharsClass::Store_GDI_Char);
    Hook_Method(0x00842FA0, &FontCharsClass::Get_Char_Data);
    Hook_Method(0x00843750, &FontCharsClass::Grow_Unicode_Array);
    Hook_Method(0x00843860, &FontCharsClass::Free_Character_Arrays);

    // gameclient.h
    Hook_Any(0x00613D10, GameClientMessageDispatcher::Translate_Game_Message);

    // gamemessageparser.h
    Hook_Method(0x005F9C60, &GameMessageParser::Hook_Ctor);
    Hook_Method(0x005F9F80, &GameMessageParser::Add_Arg_Type);

    // display.h
    Hook_Any(0x00421A00, Display::Reset);
    Hook_Any(0x00421870, Display::Update);
    Hook_Any(0x00421380, Display::Draw);
    Hook_Any(0x00421390, Display::Set_Display_Mode);
    Hook_Any(0x00421300, Display::Attach_View);
    Hook_Any(0x004210A0, Display::Get_Next_View);
    Hook_Any(0x00421320, Display::Draw_Views);
    Hook_Any(0x00421350, Display::Update_Views);
    Hook_Any(0x00421500, Display::Play_Logo_Movie);
    Hook_Any(0x00421670, Display::Play_Movie);
    Hook_Any(0x004217D0, Display::Stop_Movie);
    Hook_Any(0x004212C0, Display::Delete_Views);

    // win32Mouse.h
    Hook_Any(0x0073C130, Win32Mouse::Init);
    Hook_Any(0x0073C260, Win32Mouse::Init_Cursor_Resources);
    Hook_Any(0x0073C320, Win32Mouse::Set_Cursor);
    Hook_Any(0x0073C200, Win32Mouse::Set_Visibility);
    Hook_Any(0x0073BE20, Win32Mouse::Get_Mouse_Event);
    Hook_Method(0x0073C170, &Win32Mouse::Add_Win32_Event);

    // rinfo.h
    Hook_Any(0x00823230, RenderInfoClass::Push_Material_Pass);
    Hook_Any(0x00823260, RenderInfoClass::Pop_Material_Pass);
    Hook_Any(0x008232A0, RenderInfoClass::Peek_Additional_Pass);
    Hook_Any(0x008232B0, RenderInfoClass::Push_Override_Flags);
    Hook_Any(0x008232D0, RenderInfoClass::Pop_Override_Flags);
    Hook_Any(0x008232E0, RenderInfoClass::Current_Override_Flags);

    // dx8renderer.h
    Hook_Any(0x008237C0, DX8TextureCategoryClass::Add_Render_Task);
    Hook_Any(0x008238F0, DX8TextureCategoryClass::Remove_Polygon_Renderer);
    Hook_Any(0x00823990, DX8FVFCategoryContainer::Add_Visible_Material_Pass);
    Hook_Any(0x00823AD0, DX8FVFCategoryContainer::Render_Procedural_Material_Passes);
    Hook_Any(0x00823B70, DX8RigidFVFCategoryContainer::Add_Delayed_Visible_Material_Pass);
    Hook_Any(0x00823CB0, DX8RigidFVFCategoryContainer::Render_Delayed_Procedural_Material_Passes);
    Hook_Any(0x008240F0, DX8FVFCategoryContainer::Change_Polygon_Renderer_Texture);
    Hook_Any(0x00824490, DX8FVFCategoryContainer::Define_FVF);
    Hook_Any(0x00824620, DX8RigidFVFCategoryContainer::Render);
    Hook_Any(0x008246A0, DX8RigidFVFCategoryContainer::Check_If_Mesh_Fits);
    Hook_Any(0x00824740, DX8RigidFVFCategoryContainer::Add_Mesh);
    Hook_Any(0x00824D50, DX8FVFCategoryContainer::Insert_To_Texture_Category);
    Hook_Any(0x00824F00, DX8FVFCategoryContainer::Generate_Texture_Categories);
    Hook_Any(0x00825460, DX8SkinFVFCategoryContainer::Render);
    Hook_Any(0x00825830, DX8SkinFVFCategoryContainer::Check_If_Mesh_Fits);
    Hook_Any(0x00825880, DX8SkinFVFCategoryContainer::Add_Visible_Skin);
    Hook_Any(0x008258D0, DX8SkinFVFCategoryContainer::Add_Mesh);
    Hook_Any(0x00825A10, DX8TextureCategoryClass::Add_Mesh);
    Hook_Any(0x00825FB0, DX8TextureCategoryClass::Render);
    Hook_Any(0x00826C60, DX8MeshRendererClass::Init);
    Hook_Any(0x00826CB0, DX8MeshRendererClass::Shutdown);
    Hook_Any(0x00826D70, DX8MeshRendererClass::Clear_Pending_Delete_Lists);
    Hook_Any(0x00826DC0, DX8MeshRendererClass::Unregister_Mesh_Type);
    Hook_Any(0x00826E20, DX8MeshRendererClass::Register_Mesh_Type);
    Hook_Any(0x008271B0, DX8MeshRendererClass::Flush);
    Hook_Any(0x00827350, DX8MeshRendererClass::Add_To_Render_List);
    Hook_Any(0x00827390, DX8MeshRendererClass::Invalidate);

    // w3dmouse.h
    Hook_Any(0x007AD600, W3DMouse::Init);
    Hook_Any(0x007AD650, W3DMouse::Set_Cursor);
    Hook_Any(0x007ADF60, W3DMouse::Set_Redraw_Mode);
    Hook_Any(0x007AD2D0, W3DMouse::Free_D3D_Assets);
    Hook_Any(0x007AD060, W3DMouse::Release_D3D_Cursor_Texture);
    Hook_Function(0x00775540, &Create_Mouse); // This is actually a W3DGameClient virtual method but this not used

    // matpass.h
    Hook_Any(0x00833320, MaterialPassClass::Install_Materials);
    Hook_Any(0x008333E0, MaterialPassClass::Set_Texture);
    Hook_Any(0x00833410, MaterialPassClass::Set_Shader);
    Hook_Any(0x00833430, MaterialPassClass::Set_Material);
    Hook_Any(0x00833460, MaterialPassClass::Get_Texture);
    Hook_Any(0x00833480, MaterialPassClass::Get_Material);

    // camera.h
    Hook_Any(0x00820C70, CameraClass::Clone);
    Hook_Any(0x00820CD0, CameraClass::Get_Obj_Space_Bounding_Sphere);
    Hook_Any(0x00820CF0, CameraClass::Get_Obj_Space_Bounding_Box);
    Hook_Any(0x00820D20, CameraClass::Set_Transform);
    Hook_Any(0x00820D40, CameraClass::Set_Position);
    Hook_Method(
        0x00820D60, static_cast<void (CameraClass::*)(const Vector2 &, const Vector2 &)>(&CameraClass::Set_View_Plane));
    Hook_Method(0x00820DB0, static_cast<void (CameraClass::*)(float, float)>(&CameraClass::Set_View_Plane));
    Hook_Any(0x00820E30, CameraClass::Set_Aspect_Ratio);
    Hook_Any(0x00820EA0, CameraClass::Project);
    Hook_Any(0x008210D0, CameraClass::Project_Camera_Space_Point);
    Hook_Any(0x00821250, CameraClass::Un_Project);
    Hook_Any(0x00821330, CameraClass::Transform_To_View_Space);
    Hook_Any(0x008213E0, CameraClass::Cull_Box);
    Hook_Any(0x00821410, CameraClass::Update_Frustum);
    Hook_Any(0x00821830, CameraClass::Device_To_View_Space);
    Hook_Any(0x008218F0, CameraClass::Device_To_World_Space);
    Hook_Any(0x00821990, CameraClass::Apply);
    Hook_Any(0x00821E90, CameraClass::Set_Clip_Planes);
    Hook_Any(0x00821F50, CameraClass::Get_D3D_Projection_Matrix);

    // lightenv.h
    Hook_Any(0x00822130, LightEnvironmentClass::InputLightStruct::Init_From_Point_Or_Spot_Light);
    Hook_Any(0x008224E0, LightEnvironmentClass::InputLightStruct::Init_From_Directional_Light);
    Hook_Any(0x008225B0, LightEnvironmentClass::Reset);
    Hook_Any(0x008225F0, LightEnvironmentClass::Add_Light);
    Hook_Any(0x008229B0, LightEnvironmentClass::Pre_Render_Update);
    Hook_Any(0x00822B60, LightEnvironmentClass::Add_Fill_Light);
    Hook_Any(0x00822CF0, LightEnvironmentClass::Calculate_Fill_Light);

    // colmath.h
    Hook_Function(
        0x008E9F80, static_cast<bool (*)(AABoxClass const &, AABoxClass const &)>(&CollisionMath::Intersection_Test));
    Hook_Function(0x008EA030,
        static_cast<CollisionMath::OverlapType (*)(AABoxClass const &, LineSegClass const &)>(&CollisionMath::Overlap_Test));
    Hook_Function(0x008EA410,
        static_cast<bool (*)(AABoxClass const &, Vector3 const &, AABoxClass const &, CastResultStruct *)>(
            &CollisionMath::Collide));
    Hook_Function(0x008E7E20,
        static_cast<bool (*)(AABoxClass const &, Vector3 const &, TriClass const &, CastResultStruct *)>(
            &CollisionMath::Collide));
    Hook_Function(0x007F9D90,
        static_cast<CollisionMath::OverlapType (*)(FrustumClass const &, Vector3 const &)>(&CollisionMath::Overlap_Test));
    Hook_Function(0x007F9E10,
        static_cast<CollisionMath::OverlapType (*)(FrustumClass const &, SphereClass const &)>(
            &CollisionMath::Overlap_Test));
    Hook_Function(0x007F9E60,
        static_cast<CollisionMath::OverlapType (*)(FrustumClass const &, AABoxClass const &)>(&CollisionMath::Overlap_Test));
    Hook_Function(0x007F9540,
        static_cast<bool (*)(LineSegClass const &, TriClass const &, CastResultStruct *)>(&CollisionMath::Collide));
    Hook_Function(0x007F96D0,
        static_cast<bool (*)(LineSegClass const &, AABoxClass const &, CastResultStruct *)>(&CollisionMath::Collide));
    Hook_Function(0x007F9940,
        static_cast<bool (*)(LineSegClass const &, OBBoxClass const &, CastResultStruct *)>(&CollisionMath::Collide));
    Hook_Function(
        0x008EB480, static_cast<bool (*)(OBBoxClass const &, OBBoxClass const &)>(&CollisionMath::Intersection_Test));
    Hook_Function(
        0x008EB550, static_cast<bool (*)(OBBoxClass const &, AABoxClass const &)>(&CollisionMath::Intersection_Test));
    Hook_Function(
        0x008EB6B0, static_cast<bool (*)(AABoxClass const &, OBBoxClass const &)>(&CollisionMath::Intersection_Test));
    Hook_Function(0x008EEC20,
        static_cast<bool (*)(OBBoxClass const &, Vector3 const &, OBBoxClass const &, Vector3 const &, CastResultStruct *)>(
            &CollisionMath::Collide));
    Hook_Function(0x008EED50,
        static_cast<bool (*)(OBBoxClass const &, Vector3 const &, AABoxClass const &, Vector3 const &, CastResultStruct *)>(
            &CollisionMath::Collide));
    Hook_Function(0x008EEF30,
        static_cast<bool (*)(AABoxClass const &, Vector3 const &, OBBoxClass const &, Vector3 const &, CastResultStruct *)>(
            &CollisionMath::Collide));
    Hook_Function(0x008E2DD0,
        static_cast<bool (*)(OBBoxClass const &, Vector3 const &, TriClass const &, Vector3 const &, CastResultStruct *)>(
            &CollisionMath::Collide));
    Hook_Function(
        0x008E6930, static_cast<bool (*)(OBBoxClass const &, TriClass const &)>(&CollisionMath::Intersection_Test));
    Hook_Function(0x008003A0,
        static_cast<CollisionMath::OverlapType (*)(PlaneClass const &, SphereClass const &)>(&CollisionMath::Overlap_Test));

    // w3ddisplay.h
    Hook_Any(0x0073D030, W3DDisplay::Reset);
    Hook_Any(0x0073C980, W3DDisplay::Set_Width);
    Hook_Any(0x0073CA00, W3DDisplay::Set_Height);
    Hook_Any(0x0073C840, W3DDisplay::Set_Display_Mode);
    Hook_Any(0x0073C5D0, W3DDisplay::Get_Display_Mode_Count);
    Hook_Any(0x0073C650, W3DDisplay::Get_Display_Mode_Description);
    Hook_Any(0x00741360, W3DDisplay::Do_Smart_Asset_Purge_And_Preload);
    Hook_Any(0x00740C80, W3DDisplay::Set_Clip_Region);
    Hook_Any(0x0073EE10, W3DDisplay::Set_Time_Of_Day);
    Hook_Any(0x0073F2A0, W3DDisplay::Draw_Fill_Rect);
    Hook_Any(0x0073F320, W3DDisplay::Draw_Rect_Clock);
    Hook_Any(0x0073FD40, W3DDisplay::Draw_Remaining_Rect_Clock);
    Hook_Any(0x00740640, W3DDisplay::Draw_Image);
    Hook_Any(0x007411F0, W3DDisplay::Preload_Model_Assets);
    Hook_Any(0x007412D0, W3DDisplay::Preload_Texture_Assets);
    Hook_Any(0x0073ED70, W3DDisplay::Toggle_LetterBox);
    Hook_Any(0x0073EDB0, W3DDisplay::Enable_LetterBox);
    Hook_Any(0x0073EC10, W3DDisplay::Is_LetterBox_Fading);
    Hook_Function(0x007751A0, &Create_Game_Display); // This is actually a W3DGameClient virtual method but this not used

    // vertmaterial.h
    Hook_Any(0x00817150, VertexMaterialClass::Hook_Ctor);
    Hook_Any(0x00817320, VertexMaterialClass::Hook_Ctor2);
    Hook_Any(0x00817520, VertexMaterialClass::Make_Unique);
    Hook_Any(0x008175E0, VertexMaterialClass::operator=);
    Hook_Any(0x00817750, VertexMaterialClass::Compute_CRC);
    Hook_Any(0x00817AD0, VertexMaterialClass::Init_From_Material3);
    Hook_Any(0x00817C20, VertexMaterialClass::Load_W3D);
    Hook_Any(0x00817DB0, VertexMaterialClass::Parse_W3dVertexMaterialStruct);
    Hook_Any(0x00817ED0, VertexMaterialClass::Parse_Mapping_Args);
    Hook_Any(0x008195E0, VertexMaterialClass::Get_Preset);

    // mapobject.h
    Hook_Any(0x00743B50, MapObject::Hook_Ctor);
    Hook_Any(0x00743F60, MapObject::Set_Thing_Template);
    Hook_Any(0x00743FB0, MapObject::Set_Name);
    Hook_Any(0x00744050, MapObject::Get_Waypoint_ID);
    Hook_Any(0x00744070, MapObject::Get_Waypoint_Name);
    Hook_Any(0x007440A0, MapObject::Get_Thing_Template);

    // terraintex.h
    Hook_Any(0x00782630, TerrainTextureClass::Hook_Ctor1);
    Hook_Any(0x007826C0, TerrainTextureClass::Hook_Ctor2);
    Hook_Any(0x007826F0, TerrainTextureClass::Update);
    Hook_Any(0x00782A20, TerrainTextureClass::Update_Flat);
    Hook_Any(0x007829F0, TerrainTextureClass::Set_LOD);
    Hook_Any(0x00782C20, AlphaTerrainTextureClass::Hook_Ctor);
    Hook_Any(0x00783250, LightMapTerrainTextureClass::Hook_Ctor);
    Hook_Any(0x00783360, AlphaEdgeTextureClass::Hook_Ctor);
    Hook_Any(0x007833E0, AlphaEdgeTextureClass::Update);
    Hook_Any(0x007835F0, CloudMapTerrainTextureClass::Hook_Ctor);
    Hook_Any(0x007836C0, CloudMapTerrainTextureClass::Restore);
    Hook_Any(0x00783880, ScorchTextureClass::Hook_Ctor);

    // tiledata.h
    Hook_Any(0x007823D0, TileData::Has_RGB_Data_For_Width);
    Hook_Any(0x00782420, TileData::Get_RGB_Data_For_Width);
    Hook_Any(0x00782480, TileData::Update_Mips);

    // hanimmgr.h
    Hook_Method(0x00831CE0, &HAnimManagerClass::Load_Anim);

    // seglinerenderer.h
    Hook_Any(0x008858F0, SegLineRendererClass::operator=);
    Hook_Any(0x008859B0, SegLineRendererClass::Init);
    Hook_Any(0x00885AB0, SegLineRendererClass::Get_Texture);
    Hook_Any(0x00885AC0, SegLineRendererClass::Set_Texture_Tile_Factor);
    Hook_Any(0x00885B10, SegLineRendererClass::Reset_Line);
    Hook_Any(0x00885B30, SegLineRendererClass::Render);
    Hook_Any(0x008898C0, SegLineRendererClass::Scale);

    // gamelogic.h
    Hook_Any(0x004A67B0, GameLogic::Is_Intro_Movie_Playing);
    Hook_Any(0x004A6940, GameLogic::Rebalance_Parent_Sleepy_Update);
    Hook_Any(0x004A6F30, GameLogic::Get_First_Object);
    Hook_Any(0x004A6F40, GameLogic::Allocate_Object_ID);
    Hook_Any(0x004A7900, GameLogic::Is_Game_Paused);
    Hook_Any(0x004A7A30, GameLogic::Process_Progress_Complete);
    Hook_Any(0x004A7A70, GameLogic::Time_Out_Game_Start);
    Hook_Any(0x004A7BB0, GameLogic::Set_Buildable_Status_Override);
    Hook_Any(0x004A7BD0, GameLogic::Find_Buildable_Status_Override);
    Hook_Any(0x004A7C00, GameLogic::Set_Control_Bar_Override);
    Hook_Any(0x004A7DA0, GameLogic::Find_Control_Bar_Override);
    Hook_Any(0x004A7FA0, GameLogic::Add_TOC_Entry);

    // assetmgr.h
    Hook_Any(0x008147C0, W3DAssetManager::Free_Assets);
    Hook_Any(0x00814850, W3DAssetManager::Release_Unused_Assets);
    // Hook_Any(0x00814870, W3DAssetManager::Free_Assets_With_Exclusion_List);
    Hook_Any(0x00814A60, W3DAssetManager::Create_Asset_List);
    Hook_Any(0x00814FC0, W3DAssetManager::Create_Render_Obj);
    Hook_Any(0x008152C0, W3DAssetManager::Render_Obj_Exists);
    Hook_Any(0x00815340, W3DAssetManager::Create_Render_Obj_Iterator);
    // Can't hook due to reuse of address Hook_Any(0x00815370, W3DAssetManager::Release_Render_Obj_Iterator);
    Hook_Any(0x00815390, W3DAssetManager::Create_HAnim_Iterator);
    Hook_Any(0x008154F0, W3DAssetManager::Get_HAnim);
    Hook_Any(0x00765FF0, W3DAssetManager::Add_Anim);
    Hook_Any(0x00815920, W3DAssetManager::Get_Texture);
    Hook_Any(0x00815C90, W3DAssetManager::Release_All_Textures);
    Hook_Any(0x00815D90, W3DAssetManager::Release_Unused_Textures);
    Hook_Any(0x00816090, W3DAssetManager::Release_Texture);
    Hook_Any(0x008145A0, W3DAssetManager::Load_Procedural_Textures);
    Hook_Any(0x00766010, W3DAssetManager::Peek_Metal_Map_Manager);
    Hook_Any(0x008161A0, W3DAssetManager::Get_Font3DInstance);
    Hook_Any(0x00816370, W3DAssetManager::Get_FontChars);
    Hook_Any(0x00815440, W3DAssetManager::Create_HTree_Iterator);
    Hook_Any(0x00815720, W3DAssetManager::Get_HTree);
    Hook_Any(0x00816550, W3DAssetManager::Register_Prototype_Loader);
    Hook_Any(0x00815460, W3DAssetManager::Create_Font3DData_Iterator);
    Hook_Any(0x008162B0, W3DAssetManager::Add_Font3DData);
    Hook_Any(0x008162D0, W3DAssetManager::Remove_Font3DData);
    Hook_Any(0x00816210, W3DAssetManager::Get_Font3DData);
    Hook_Any(0x00816300, W3DAssetManager::Release_All_Font3DDatas);
    Hook_Any(0x00816330, W3DAssetManager::Release_Unused_Font3DDatas);
    Hook_Any(0x008164C0, W3DAssetManager::Release_All_FontChars);

    Hook_Method(0x00763D70, &GameAssetManager::Hook_Ctor);
}
