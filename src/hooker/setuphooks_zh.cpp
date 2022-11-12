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
#ifndef GAME_DLL
#error This file must not be compiled into standalone binary
#endif

#include "aabtree.h"
#include "aiupdate.h"
#include "anim2d.h"
#include "archivefile.h"
#include "archivefilesystem.h"
#include "asciistring.h"
#include "assetmgr.h"
#include "audioeventinfo.h"
#include "audioeventrts.h"
#include "audiomanager.h"
#include "bezfwditerator.h"
#include "beziersegment.h"
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
#include "drawable.h"
#include "dx8caps.h"
#include "dx8fvf.h"
#include "dx8indexbuffer.h"
#include "dx8renderer.h"
#include "dx8texman.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include "energy.h"
#include "experiencetracker.h"
#include "filesystem.h"
#include "filetransfer.h"
#include "force_nocd.h"
#include "framemetrics.h"
#include "functionlexicon.h"
#include "fxlist.h"
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
#include "hlod.h"
#include "hooker.h"
#include "htree.h"
#include "image.h"
#include "ini.h"
#include "keyboard.h"
#include "laserupdate.h"
#include "lightenv.h"
#include "line3d.h"
#include "locomotor.h"
#include "main.h"
#include "mapobject.h"
#include "matpass.h"
#include "memdynalloc.h"
#include "mesh.h"
#include "meshgeometry.h"
#include "meshmatdesc.h"
#include "meshmdl.h"
#include "messagestream.h"
#include "milesaudiofilecache.h"
#include "milesaudiomanager.h"
#include "modulefactory.h"
#include "mouse.h"
#include "multilist.h"
#include "namekeygenerator.h"
#include "object.h"
#include "objectcreationlist.h"
#include "objecttypes.h"
#include "parabolicease.h"
#include "particle.h"
#include "particlesys.h"
#include "particlesysinfo.h"
#include "particlesysmanager.h"
#include "partitionmanager.h"
#include "physicsupdate.h"
#include "playerlist.h"
#include "playertemplate.h"
#include "pointgr.h"
#include "polygontrigger.h"
#include "projectilestreamupdate.h"
#include "randomvalue.h"
#include "rankinfo.h"
#include "rayeffect.h"
#include "render2d.h"
#include "render2dsentence.h"
#include "rinfo.h"
#include "scene.h"
#include "science.h"
#include "scorekeeper.h"
#include "script.h"
#include "scriptaction.h"
#include "scriptcondition.h"
#include "scriptgroup.h"
#include "scriptlist.h"
#include "segline.h"
#include "seglinerenderer.h"
#include "setuphooks.h"
#include "shader.h"
#include "shadermanager.h"
#include "sidesinfo.h"
#include "sideslist.h"
#include "smudge.h"
#include "sortingrenderer.h"
#include "soundmanager.h"
#include "statemachine.h"
#include "streak.h"
#include "surfaceclass.h"
#include "swayclientupdate.h"
#include "targa.h"
#include "team.h"
#include "teamsinfo.h"
#include "terraintex.h"
#include "texproject.h"
#include "texture.h"
#include "texturebase.h"
#include "textureloader.h"
#include "textureloadtask.h"
#include "thing.h"
#include "thingfactory.h"
#include "thingtemplate.h"
#include "thread.h"
#include "thumbnail.h"
#include "thumbnailmanager.h"
#include "tiledata.h"
#include "tintenvelope.h"
#include "turretai.h"
#include "updatemodule.h"
#include "vertmaterial.h"
#include "w3d.h"
#include "w3dbibbuffer.h"
#include "w3dbridgebuffer.h"
#include "w3dbuffermanager.h"
#include "w3ddebugdisplay.h"
#include "w3ddisplay.h"
#include "w3dfilesystem.h"
#include "w3dgameclient.h"
#include "w3dmodeldraw.h"
#include "w3dmouse.h"
#include "w3dpoly.h"
#include "w3dprojectedshadow.h"
#include "w3dpropbuffer.h"
#include "w3droadbuffer.h"
#include "w3dscene.h"
#include "w3dshroud.h"
#include "w3dsmudge.h"
#include "w3dterraintracks.h"
#include "w3dview.h"
#include "w3dvolumetricshadow.h"
#include "w3dwater.h"
#include "weapon.h"
#include "win32gameengine.h"
#include "win32localfilesystem.h"
#include "win32mouse.h"
#include "wininstancedata.h"
#include "worldheightmap.h"
#include "wwstring.h"
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>
#include <winsock2.h>

static_assert(sizeof(AcademyStats) == 0xB4);
static_assert(sizeof(Energy) == 0x14);
static_assert(sizeof(Handicap) == 0x10);
static_assert(sizeof(MissionStats) == 0x8C);
static_assert(sizeof(Money) == 0xC);
static_assert(sizeof(Object) == 0x27C);
static_assert(sizeof(Player) == 0x44C);
static_assert(sizeof(PolyNeighbor) == 0x16);
static_assert(sizeof(ScoreKeeper) == 0x190);
static_assert(sizeof(Utf8String::AsciiStringData) == 4);
static_assert(sizeof(Utf16String::UnicodeStringData) == 4);

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
    Hook_Function(0x005F1CC0, CheckForCDAtGameStart);

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
    Hook_Method(0x0048F410, &ArchiveFileSystem::Get_File_List_In_Directory);
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
    Hook_Method(0x00415450, static_cast<void (Utf8String::*)(Utf16String const &)>(&Utf8String::Translate));
    Hook_Method(0x00415530, &Utf8String::Trim);

    // Replace Win32GameEngine
    Hook_Any(0x007420F0, Win32GameEngine::Create_Local_File_System);
    Hook_Any(0x00742150, Win32GameEngine::Create_Archive_File_System);
    Hook_Any(0x00741FA0, Win32GameEngine::Create_Module_Factory);
    Hook_Any(0x007424B0, Win32GameEngine::Create_Audio_Manager);
    Hook_Any(0x0040FF60, GameEngine::Create_File_System);

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
    Hook_Function(0x0041A980, &INI::Parse_Unsigned_Byte);
    Hook_Function(0x0041AAB0, &INI::Parse_Int);
    Hook_Function(0x0041AB20, &INI::Parse_Unsigned_Int);
    Hook_Function(0x0041AB90, &INI::Parse_Real);
    Hook_Function(0x0041AC00, &INI::Parse_Positive_Non_Zero_Real);
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
    Hook_Function(0x0041DBD0, &INI::Parse_Duration_Unsigned_Int);
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
    Hook_Function(0x00418090, GlobalData::Parse_Game_Data_Definition);

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
    Hook_Method(PICK_ADDRESS(0x004D55D0, 0x0076E506), &CaveSystem::Hook_Ctor);

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
    Hook_Function(0x0051B760, ScriptList::Reset);

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
    Hook_Method(0x004F2B80, &ModuleFactory::Find_Module_Interface_Mask);
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
    Hook_Function(0x004D3860, &PlayerTemplateStore::Parse_Player_Template_Definition);

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
    Hook_Method(0x0080AAC0, &Render2DClass::Render);

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
    Hook_Method(0x008041B0, static_cast<void (*)(unsigned, const D3DLIGHT8 *)>(&DX8Wrapper::Set_Light));
    Hook_Method(0x008044B0, &DX8Wrapper::Get_DX8_Front_Buffer);
    Hook_Method(0x00804520, &DX8Wrapper::Get_DX8_Back_Buffer);
    Hook_Method(0x008045E0, &DX8Wrapper::Create_Render_Target);
    Hook_Method(0x00804780, &DX8Wrapper::Set_Render_Target_With_Z);
    Hook_Method(0x008047E0, &DX8Wrapper::Set_Render_Target);
    Hook_Method(0x00804AC0, &DX8Wrapper::Set_Gamma);
    Hook_Method(0x00804D00, &DX8Wrapper::Get_DX8_Render_State_Value_Name);
    Hook_Method(0x00805520, &DX8Wrapper::Get_DX8_Texture_Stage_State_Value_Name);
    Hook_Method(0x00805B60, &DX8Wrapper::Get_Back_Buffer_Format);
    Hook_Method(0x00804200, &DX8Wrapper::Set_Light_Environment);

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
    Hook_Method(0x00519280, &ImageCollection::Hook_Ctor);
    Hook_Method(0x005194F0, &ImageCollection::Add_Image);
    Hook_Method(0x00519610, &ImageCollection::Find_Image_By_Name);
    Hook_Method(0x0041B980, &ImageCollection::Parse_Mapped_Image);
    Hook_Method(0x00519680, &ImageCollection::Load);

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

    Hook_Any(0x00841580, Render2DSentenceClass::Reset);
    Hook_Method(0x008416B0, &Render2DSentenceClass::Render);
    Hook_Method(0x008416E0, &Render2DSentenceClass::Set_Location);
    Hook_Method(0x00841700, &Render2DSentenceClass::Get_Formatted_Text_Extents);
    Hook_Method(0x00841720, &Render2DSentenceClass::Build_Textures);
    Hook_Method(0x00841910, &Render2DSentenceClass::Draw_Sentence);
    Hook_Method(0x00841E30, &Render2DSentenceClass::Record_Sentence_Chunk);
    Hook_Method(0x00841F20, &Render2DSentenceClass::Allocate_New_Surface);
    Hook_Method(0x00842190, &Render2DSentenceClass::Build_Sentence_Centered);
    Hook_Method(0x00842740, &Render2DSentenceClass::Build_Sentence_Not_Centered);
    Hook_Method(0x00842C50, &Render2DSentenceClass::Build_Sentence);

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

    Hook_Function(0x0073C740, Reset_D3D_Device);

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
    Hook_Any(0x004A6980, GameLogic::Rebalance_Child_Sleepy_Update);
    Hook_Method(0x004A6A10, &GameLogic::Friend_Awaken_Update_Module);
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

    // shadermanager.h
    Hook_Any(0x0074DF20, W3DShaderManager::Init);
    Hook_Any(0x0074E0A0, W3DShaderManager::Shutdown);
    Hook_Any(0x0074E140, W3DShaderManager::Get_Shader_Passes);
    Hook_Any(0x0074E150, W3DShaderManager::Set_Shader);
    Hook_Any(0x0074E190, W3DShaderManager::Reset_Shader);
    Hook_Any(0x0074E1C0, W3DShaderManager::Filter_Pre_Render);
    Hook_Any(0x0074E1F0, W3DShaderManager::Filter_Post_Render);
    Hook_Any(0x0074E230, W3DShaderManager::Filter_Setup);
    Hook_Any(0x0074E700, W3DShaderManager::Get_Render_Texture);
    Hook_Any(0x0074E710, W3DShaderManager::Get_Chipset);
    Hook_Any(0x0074E990, W3DShaderManager::Load_And_Create_D3D_Shader);
    Hook_Any(0x0074EC10, W3DShaderManager::Set_Shroud_Tex);

    // w3dview.h
    Hook_Any(0x0076E2F0, W3DView::Set_Fade_Parameters);
    Hook_Any(0x0076E330, W3DView::Set_View_Filter_Pos);

    // scene.h
    Hook_Any(0x00832550, SceneClass::Render);
    Hook_Any(0x00832B70, SimpleSceneClass::Add_Render_Object);
    Hook_Any(0x00832BB0, SimpleSceneClass::Remove_Render_Object);
    Hook_Any(0x00833160, SimpleSceneClass::Create_Iterator);
    Hook_Any(0x00832BF0, SimpleSceneClass::Register);
    Hook_Any(0x00832C80, SimpleSceneClass::Unregister);
    Hook_Any(0x00832DF0, SimpleSceneClass::Compute_Point_Visibility);
    Hook_Any(0x00832F40, SimpleSceneClass::Customized_Render);
    Hook_Any(0x008330E0, SimpleSceneClass::Post_Render_Processing);
    Hook_Any(0x00832D20, SimpleSceneClass::Visibility_Check);

    // sortingrenderer.h
    Hook_Any(0x0080C570, SortingRendererClass::Set_Min_Vertex_Buffer_Size);
    Hook_Function(0x0080C610,
        static_cast<void (*)(const SphereClass &, unsigned short, unsigned short, unsigned short, unsigned short)>(
            &SortingRendererClass::Insert_Triangles));
    Hook_Function(0x0080CC40,
        static_cast<void (*)(unsigned short, unsigned short, unsigned short, unsigned short)>(
            &SortingRendererClass::Insert_Triangles));
    Hook_Any(0x0080DBB0, SortingRendererClass::Flush);
    Hook_Any(0x0080E1E0, SortingRendererClass::Deinit);

    // htree.h
    Hook_Any(0x0083E460, HTreeClass::Hook_Ctor);
    Hook_Any(0x0083E690, HTreeClass::Hook_Ctor2);
    Hook_Any(0x0083E4D0, HTreeClass::Init_Default);
    Hook_Any(0x0083E7C0, HTreeClass::Load_W3D);
    Hook_Any(0x0083E9A0, HTreeClass::Read_Pivots);
    Hook_Method(0x0083ECF0,
        static_cast<bool (HTreeClass::*)(HAnimClass *, int, float, Matrix3D const &, Matrix3D *)>(
            &HTreeClass::Simple_Evaluate_Pivot));
    Hook_Method(0x0083EF40,
        static_cast<bool (HTreeClass::*)(int, Matrix3D const &, Matrix3D *)>(&HTreeClass::Simple_Evaluate_Pivot));
    Hook_Any(0x0083F060, HTreeClass::Base_Update);
    Hook_Method(
        0x0083F120, static_cast<void (HTreeClass::*)(Matrix3D const &, HAnimClass *, float)>(&HTreeClass::Anim_Update));
    Hook_Method(
        0x0083F3E0, static_cast<void (HTreeClass::*)(Matrix3D const &, HRawAnimClass *, float)>(&HTreeClass::Anim_Update));
    Hook_Any(0x0083F7C0, HTreeClass::Blend_Update);
    Hook_Any(0x00840070, HTreeClass::Get_Bone_Index);
    Hook_Any(0x008400C0, HTreeClass::Get_Bone_Name);
    Hook_Any(0x008400E0, HTreeClass::Get_Parent_Index);
    Hook_Any(0x00840110, HTreeClass::Scale);
    Hook_Any(0x008401A0, HTreeClass::Capture_Bone);
    Hook_Any(0x008401D0, HTreeClass::Release_Bone);
    Hook_Any(0x00840200, HTreeClass::Is_Bone_Captured);
    Hook_Any(0x00840220, HTreeClass::Control_Bone);

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

    // hlod.h
    Hook_Any(0x0085A260, DistLODLoaderClass::Load_W3D);
    Hook_Any(0x00861B80, CollectionLoaderClass::Load_W3D);
    Hook_Any(0x00862CB0, HModelLoaderClass::Load_W3D);
    Hook_Any(0x0085B580, HLodLoaderClass::Load_W3D);

    // animobj.h
    Hook_Any(0x008600D0, Animatable3DObjClass::Get_HTree);
    Hook_Any(0x00898860, Animatable3DObjClass::Render);
    Hook_Any(0x00898910, Animatable3DObjClass::Special_Render);
    Hook_Any(0x008989A0, Animatable3DObjClass::Set_Transform);
    Hook_Any(0x008989C0, Animatable3DObjClass::Set_Position);
    Hook_Any(0x008989E0, Animatable3DObjClass::Get_Num_Bones);
    Hook_Any(0x00898A00, Animatable3DObjClass::Get_Bone_Name);
    Hook_Any(0x00898A20, Animatable3DObjClass::Get_Bone_Index);
    // Hook_Any(0x00898A40, Animatable3DObjClass::Set_Animation);
    // Hook_Any(0x00898AD0, Animatable3DObjClass::Set_Animation);
    // Hook_Any(0x00898BF0, Animatable3DObjClass::Set_Animation);
    // Hook_Any(0x00898D20, Animatable3DObjClass::Set_Animation);
    Hook_Any(0x00898DF0, Animatable3DObjClass::Peek_Animation);
    // Hook_Any(0x00898E10, Animatable3DObjClass::Get_Bone_Transform);
    // Hook_Any(0x00898E50, Animatable3DObjClass::Get_Bone_Transform);
    Hook_Any(0x00898EB0, Animatable3DObjClass::Capture_Bone);
    Hook_Any(0x00898ED0, Animatable3DObjClass::Release_Bone);
    Hook_Any(0x00898EF0, Animatable3DObjClass::Is_Bone_Captured);
    Hook_Any(0x00898F10, Animatable3DObjClass::Control_Bone);
    Hook_Any(0x00898F40, Animatable3DObjClass::Update_Sub_Object_Transforms);
    // Hook_Any(0x008992B0, Animatable3DObjClass::Simple_Evaluate_Bone);
    // Hook_Any(0x00899380, Animatable3DObjClass::Simple_Evaluate_Bone);
    Hook_Any(0x00899450, Animatable3DObjClass::Compute_Current_Frame);
    Hook_Any(0x00899730, Animatable3DObjClass::Is_Animation_Complete);
    Hook_Any(0x00899790, Animatable3DObjClass::Peek_Animation_And_Info);
    Hook_Any(0x008997F0, Animatable3DObjClass::Set_Animation_Frame_Rate_Multiplier);
    Hook_Any(0x00899800, Animatable3DObjClass::Set_HTree);

    // meshmatdesc.h
    Hook_Method(0x0086B9C0, &MeshMatDescClass::Hook_Ctor);
    Hook_Method(0x0086BAA0, &MeshMatDescClass::Hook_Ctor2);
    Hook_Method(0x0086C0A0, &MeshMatDescClass::Reset);
    Hook_Method(0x0086C1B0, &MeshMatDescClass::Init_Alternate);
    Hook_Method(0x0086C630, &MeshMatDescClass::Is_Empty);
    Hook_Method(0x0086CD00, &MeshMatDescClass::Install_UV_Array);
    Hook_Method(0x0086CEC0, &MeshMatDescClass::Post_Load_Process);
    Hook_Method(0x0086DFA0, &MeshMatDescClass::Do_Mappers_Need_Normals);

    // aabtree.h
    Hook_Method(0x0088D170,
        static_cast<void (AABTreeClass::*)(OBBoxClass const &, SimpleDynVecClass<uint32_t> &)>(&AABTreeClass::Generate_APT));
    Hook_Method(0x0088D4B0,
        static_cast<void (AABTreeClass::*)(OBBoxClass const &, Vector3 const &, SimpleDynVecClass<uint32_t> &)>(
            &AABTreeClass::Generate_APT));
    Hook_Any(0x0088D830, AABTreeClass::Cast_Ray_Recursive);
    Hook_Any(0x0088D960, AABTreeClass::Cast_AABox_Recursive);
    Hook_Any(0x0088DA10, AABTreeClass::Cast_OBBox_Recursive);
    Hook_Any(0x0088DAC0, AABTreeClass::Intersect_OBBox_Recursive);

    // meshgeometry.cpp
    Hook_Any(0x0083C2D0, MeshGeometryClass::Scale);
    Hook_Any(0x0083B8F0, MeshGeometryClass::Generate_Culling_Tree);
    Hook_Any(0x0083C210, MeshGeometryClass::Read_AABTree);
    Hook_Any(0x00839B90, MeshGeometryClass::operator=);
    Hook_Any(0x00839E90, MeshGeometryClass::Reset_Geometry);
    Hook_Any(0x0083A290, MeshGeometryClass::Get_Name);
    Hook_Any(0x0083A2A0, MeshGeometryClass::Set_Name);
    Hook_Any(0x0083A3A0, MeshGeometryClass::Get_Bounding_Box);
    Hook_Any(0x0083A440, MeshGeometryClass::Get_Bounding_Sphere);
    Hook_Any(0x0083A780, MeshGeometryClass::Generate_Skin_APT);
    Hook_Any(0x0083A8B0, MeshGeometryClass::Cast_Ray);
    Hook_Any(0x0083A8E0, MeshGeometryClass::Cast_OBBox);
    Hook_Any(0x0083A910, MeshGeometryClass::Intersect_OBBox);
    Hook_Any(0x0083A940, MeshGeometryClass::Cast_World_Space_AABox);
    Hook_Any(0x0083B1C0, MeshGeometryClass::Compute_Plane_Equations);
    Hook_Any(0x0083B340, MeshGeometryClass::Compute_Vertex_Normals);
    Hook_Any(0x0083B670, MeshGeometryClass::Compute_Bounds);
    Hook_Any(0x0083B760, MeshGeometryClass::Get_Vertex_Normal_Array);
    Hook_Any(0x0083B790, MeshGeometryClass::Compute_Plane);
    Hook_Any(0x0083B9D0, MeshGeometryClass::Load_W3D);
    Hook_Any(0x0083BEA0, MeshGeometryClass::Read_Vertices);
    Hook_Any(0x0083BF10, MeshGeometryClass::Read_Vertex_Normals);
    Hook_Any(0x0083BF80, MeshGeometryClass::Read_Triangles);
    Hook_Any(0x0083C050, MeshGeometryClass::Read_User_Text);
    Hook_Any(0x0083C140, MeshGeometryClass::Read_Vertex_Influences);
    Hook_Any(0x0083C1B0, MeshGeometryClass::Read_Vertex_Shade_Indices);
    Hook_Any(0x0083C7A0, MeshGeometryClass::Get_Deformed_Screenspace_Vertices);
    Hook_Method(0x0083A460,
        static_cast<void (MeshGeometryClass::*)(Vector3 const &, SimpleDynVecClass<uint32_t> &)>(
            &MeshGeometryClass::Generate_Rigid_APT));
    Hook_Method(0x0083A510,
        static_cast<void (MeshGeometryClass::*)(OBBoxClass const &, SimpleDynVecClass<uint32_t> &)>(
            &MeshGeometryClass::Generate_Rigid_APT));
    Hook_Method(0x0083A630,
        static_cast<void (MeshGeometryClass::*)(OBBoxClass const &, Vector3 const &, SimpleDynVecClass<uint32_t> &)>(
            &MeshGeometryClass::Generate_Rigid_APT));
    Hook_Method(0x0083C590,
        static_cast<void (MeshGeometryClass::*)(Vector3 *, HTreeClass const *)>(&MeshGeometryClass::Get_Deformed_Vertices));
    Hook_Method(0x0083C670,
        static_cast<void (MeshGeometryClass::*)(Vector3 *, Vector3 *, HTreeClass const *)>(
            &MeshGeometryClass::Get_Deformed_Vertices));

    // meshmdl.h
    Hook_Any(0x00829100, MeshModelClass::Hook_Ctor);
    Hook_Any(0x008292D0, MeshModelClass::Hook_Ctor2);
    Hook_Any(0x008295E0, MeshModelClass::Register_For_Rendering);
    Hook_Any(0x00829660, MeshModelClass::Replace_Texture);
    Hook_Any(0x00829760, MeshModelClass::Shadow_Render);
    Hook_Any(0x00829830, MeshModelClass::Make_Geometry_Unique);
    Hook_Any(0x00829A90, MeshModelClass::Needs_Vertex_Normals);

    // mesh.h
    Hook_Any(0x00862B40, MeshLoaderClass::Load_W3D);
    Hook_Any(0x0082C050, MeshClass::Get_Model);
    Hook_Any(0x0082D630, MeshClass::Make_Unique);

    // segline.h
    Hook_Any(0x0083D6B0, SegmentedLineClass::Hook_Ctor);
    Hook_Any(0x0083D870, SegmentedLineClass::Set_Points);
    Hook_Any(0x0083D940, SegmentedLineClass::Set_Texture);
    Hook_Any(0x0083D960, SegmentedLineClass::Set_Shader);
    Hook_Any(0x0083D980, SegmentedLineClass::Set_Width);
    Hook_Any(0x0083D9D0, SegmentedLineClass::Set_Color);
    Hook_Any(0x0083DA00, SegmentedLineClass::Set_Texture_Mapping_Mode);
    Hook_Any(0x0083DA20, SegmentedLineClass::Set_Texture_Tile_Factor);
    Hook_Any(0x0083DA40, SegmentedLineClass::Set_UV_Offset_Rate);

    // w3d.h
    Hook_Any(0x008075E0, W3D::Init);
    Hook_Any(0x00807700, W3D::Shutdown);
    Hook_Any(0x00807760, W3D::Set_Render_Device);
    Hook_Any(0x008077A0, W3D::Get_Window);
    Hook_Any(0x008077B0, W3D::Get_Render_Device);
    Hook_Any(0x008077C0, W3D::Get_Render_Device_Desc);
    Hook_Any(0x008077D0, W3D::Set_Device_Resolution);
    Hook_Any(0x00807800, W3D::Get_Render_Target_Resolution);
    Hook_Any(0x00807850, W3D::Invalidate_Textures);
    Hook_Any(0x008078F0, W3D::Begin_Render);
    Hook_Method(
        0x00807A40, static_cast<W3DErrorType (*)(SceneClass *, CameraClass *, bool, bool, Vector3 const &)>(&W3D::Render));
    Hook_Method(0x00807DC0, static_cast<W3DErrorType (*)(RenderObjClass &, RenderInfoClass &)>(&W3D::Render));
    Hook_Any(0x00807E50, W3D::Flush);
    Hook_Any(0x00807E90, W3D::End_Render);
    Hook_Any(0x00807EE0, W3D::Sync);
    Hook_Any(0x00807F00, W3D::Set_Collision_Box_Display_Mask);
    Hook_Any(0x00808170, W3D::Set_Texture_Reduction);
    Hook_Any(0x00808240, W3D::Enable_Texturing);
    Hook_Any(0x00808260, W3D::Enable_Coloring);
    Hook_Any(0x00808270, W3D::Get_Texture_Reduction);
    Hook_Any(0x008082A0, W3D::Set_Texture_Bit_Depth);
    Hook_Any(0x008082C0, W3D::Add_To_Static_Sort_List);
    Hook_Any(0x008082E0, W3D::Render_And_Clear_Static_Sort_Lists);

    // texproject.h
    Hook_Any(0x00827F70, TexProjectClass::Hook_Ctor);
    Hook_Any(0x00828260, TexProjectClass::Set_Intensity);
    Hook_Any(0x008284F0, TexProjectClass::Set_Texture);
    Hook_Any(0x00828520, TexProjectClass::Peek_Material_Pass);
    Hook_Method(0x008285F0,
        static_cast<bool (TexProjectClass::*)(RenderObjClass *, const Vector3 &, float, float)>(
            &TexProjectClass::Compute_Perspective_Projection));
    Hook_Any(0x008289E0, TexProjectClass::Compute_Texture);
    Hook_Any(0x00828AE0, TexProjectClass::Set_Render_Target);

    // line3d.h
    Hook_Any(0x008403C0, Line3DClass::Hook_Ctor);
    Hook_Method(0x00840ED0, static_cast<void (Line3DClass::*)(Vector3 const &, Vector3 const &)>(&Line3DClass::Reset));
    Hook_Method(
        0x00840FF0, static_cast<void (Line3DClass::*)(Vector3 const &, Vector3 const &, float)>(&Line3DClass::Reset));
    Hook_Any(0x00841130, Line3DClass::Re_Color);
    Hook_Any(0x00841160, Line3DClass::Set_Opacity);

    // objectcreationlist.h
    Hook_Any(PICK_ADDRESS(0x004C1280, 0x00756BBD), ObjectCreationListStore::Hook_Ctor);
    Hook_Any(PICK_ADDRESS(0x004C1500, 0x00756CFB), ObjectCreationListStore::Find_Object_Creation_List);

    // partitionmanager.h
    Hook_Any(PICK_ADDRESS(0x0053B550, 0x0081DE80), PartitionManager::Hook_Ctor);
    Hook_Any(0x00541A20, SightingInfo::Is_Invalid);

    // thingfactory.h
    Hook_Any(0x00777420, W3DThingFactory::Hook_Ctor);
    Hook_Any(0x004AFB50, ThingFactory::New_Template);
    Hook_Any(0x004B0850, ThingFactory::New_Override);
    Hook_Any(0x004B0B20, ThingFactory::Find_Template_By_ID);
    Hook_Any(0x004B0B50, ThingFactory::Find_Template_Internal);
    Hook_Any(0x004B0EE0, ThingFactory::New_Drawable);
    Hook_Any(0x004B0E10, ThingFactory::New_Object);

    // thing.h
    Hook_Any(0x00543470, Thing::Get_Template);
    Hook_Any(0x00543540, Thing::Get_Unit_Dir_Vector3D);
    Hook_Any(0x005435C0, Thing::Set_Position_Z);
    Hook_Any(0x00543800, Thing::Set_Position);
    Hook_Any(0x00543A10, Thing::Set_Orientation);
    Hook_Any(0x00543BC0, Thing::Set_Transform_Matrix);
    Hook_Any(0x00543CD0, Thing::Is_KindOf);
    Hook_Any(0x00543D30, Thing::Is_KindOf_Multi);
    Hook_Any(0x00543E30, Thing::Is_Any_KindOf);
    Hook_Any(0x00543EB0, Thing::Calculate_Height_Above_Terrain);
    Hook_Any(0x00543ED0, Thing::Get_Height_Above_Terrain);
    Hook_Any(0x00543EF0, Thing::Get_Height_Above_Terrain_Or_Water);
    Hook_Any(0x00543F60, Thing::Is_Significantly_Above_Terrain);
    Hook_Any(0x00543FA0, Thing::Convert_Bone_Pos_To_World_Pos);
    Hook_Any(0x00544180, Thing::Transform_Point);

    // thingtemplate.h
    Hook_Any(0x0058B3F0, ThingTemplate::Copy_From);
    Hook_Any(0x0058B4D0, ThingTemplate::Set_Copied_From_Default);
    Hook_Any(0x0058C210, ThingTemplate::Get_Skill_Point_Value);
    Hook_Any(0x0058C230, ThingTemplate::Get_Buildable);
    Hook_Any(0x0058C440, ThingTemplate::Get_Max_Simultaneous_Of_Type);
    Hook_Any(0x0058B210, ThingTemplate::Friend_Get_AI_Module_Info);
    Hook_Any(0x0058C1D0, ThingTemplate::Can_Possibly_Have_Any_Weapon);
    Hook_Any(0x0058C230, ThingTemplate::Get_Buildable);
    Hook_Any(0x0058C470, ThingTemplate::Is_Equivalent_To);
    Hook_Any(0x0058BEF0, ThingTemplate::Find_Armor_Template_Set);
    Hook_Any(0x0058C060, ThingTemplate::Find_Weapon_Template_Set);
    Hook_Any(0x0058C260, ThingTemplate::Get_Per_Unit_FX);
    Hook_Any(0x0058C5B0, ThingTemplate::Calc_Cost_To_Build);
    Hook_Any(0x0058C650, ThingTemplate::Calc_Time_To_Build);

    // w3dwater.h
    void Do_Sky_Box_Set(bool b);
    Hook_Any(0x00756F60, Do_Sky_Box_Set);
    Hook_Any(0x00757730, WaterRenderObjClass::Hook_Ctor);
    Hook_Any(0x00757BF0, WaterRenderObjClass::Release_Resources);
    Hook_Any(0x00757D10, WaterRenderObjClass::Re_Acquire_Resources);
    Hook_Any(0x007580B0, WaterRenderObjClass::Load);
    Hook_Any(0x007580C0, WaterRenderObjClass::Init);
    Hook_Any(0x00758590, WaterRenderObjClass::Update_Map_Overrides);
    Hook_Any(0x00758640, WaterRenderObjClass::Reset);
    Hook_Any(0x007586A0, WaterRenderObjClass::Enable_Water_Grid);
    Hook_Any(0x007587F0, WaterRenderObjClass::Update);
    Hook_Any(0x00758A20, WaterRenderObjClass::Replace_Skybox_Texture);
    Hook_Any(0x00758AC0, WaterRenderObjClass::Set_Time_Of_Day);
    Hook_Any(0x00758C90, WaterRenderObjClass::Update_Render_Target_Textures);
    Hook_Any(0x0075BDC0, WaterRenderObjClass::Set_Grid_Height_Clamps);
    Hook_Any(0x0075BDE0, WaterRenderObjClass::Add_Velocity);
    Hook_Any(0x0075BDE0, WaterRenderObjClass::Change_Grid_Height);
    Hook_Any(0x0075C300, WaterRenderObjClass::Set_Grid_Change_Attenuation_Factors);
    Hook_Method(0x0075C340,
        static_cast<void (WaterRenderObjClass::*)(float, float, float, float)>(&WaterRenderObjClass::Set_Grid_Transform));
    Hook_Method(
        0x0075C430, static_cast<void (WaterRenderObjClass::*)(const Matrix3D *)>(&WaterRenderObjClass::Set_Grid_Transform));
    Hook_Any(0x0075A980, WaterRenderObjClass::Render_Water);
    Hook_Any(0x0075B580, WaterRenderObjClass::Render_Water_Mesh);
    Hook_Any(0x007578F0, WaterRenderObjClass::Get_Obj_Space_Bounding_Sphere);
    Hook_Any(0x00757920, WaterRenderObjClass::Get_Obj_Space_Bounding_Box);
    Hook_Any(0x00757950, WaterRenderObjClass::Class_ID);
    Hook_Any(0x0075D1E0, WaterRenderObjClass::Setup_Flat_Water_Shader);
    Hook_Any(0x0075D6C0, WaterRenderObjClass::Draw_Trapezoid_Water);
    Hook_Any(0x00758AF0, WaterRenderObjClass::Load_Setting);
    Hook_Any(0x00759230, WaterRenderObjClass::Render);

    // w3dbridgebuffer.h
    Hook_Any(0x0078BB60, W3DBridge::Get_Model_Vertices);
    Hook_Any(0x0078C050, W3DBridge::Get_Model_Vertices_Fixed);
    Hook_Any(0x0078C210, W3DBridge::Get_Indices_And_Vertices);
    Hook_Any(0x0078C660, W3DBridge::Get_Model_Indices);
    Hook_Any(0x0078C7E0, W3DBridgeBuffer::Hook_Dtor);
    Hook_Any(0x0078C890, W3DBridgeBuffer::Hook_Ctor);
    Hook_Any(0x0078CA60, W3DBridgeBuffer::Free_Bridge_Buffers);
    Hook_Any(0x0078CAB0, W3DBridgeBuffer::Allocate_Bridge_Buffers);
    Hook_Any(0x0078CBD0, W3DBridgeBuffer::Clear_All_Bridges);
    Hook_Any(0x0078CC80, W3DBridgeBuffer::Load_Bridges);
    Hook_Any(0x0078D010, W3DBridgeBuffer::Update_Center);
    Hook_Any(0x0078D080, W3DBridgeBuffer::Draw_Bridges);

    // w3dterraintracks.h
    Hook_Any(0x0076B2A0, TerrainTracksRenderObjClassSystem::Hook_Dtor);
    Hook_Any(0x0076B250, TerrainTracksRenderObjClassSystem::Hook_Ctor);
    Hook_Any(0x0076A8D0, TerrainTracksRenderObjClass::Add_Cap_Edge_To_Track);
    Hook_Any(0x0076AC10, TerrainTracksRenderObjClass::Add_Edge_To_Track);
    Hook_Any(0x0076B010, TerrainTracksRenderObjClassSystem::Bind_Track);
    Hook_Any(0x0076B230, TerrainTracksRenderObjClassSystem::Unbind_Track);
    Hook_Any(0x0076B2C0, TerrainTracksRenderObjClassSystem::Re_Acquire_Resources);
    Hook_Any(0x0076B470, TerrainTracksRenderObjClassSystem::Init);
    Hook_Any(0x0076B6D0, TerrainTracksRenderObjClassSystem::Update);
    Hook_Any(0x0076B8B0, TerrainTracksRenderObjClassSystem::Flush);
    Hook_Any(0x0076BF30, TerrainTracksRenderObjClassSystem::Reset);
    Hook_Any(0x0076C010, TerrainTracksRenderObjClassSystem::Set_Detail);
    Hook_Any(0x0076B580, TerrainTracksRenderObjClassSystem::Shutdown);

    // w3dshroud.h
    Hook_Any(0x00769AD0, W3DShroud::Hook_Ctor);
    Hook_Any(0x00769B20, W3DShroud::Hook_Dtor);
    Hook_Any(0x00769B70, W3DShroud::Init);
    Hook_Any(0x00769DD0, W3DShroud::Reset);
    Hook_Any(0x00769E20, W3DShroud::Release_Resources);
    Hook_Any(0x00769E40, W3DShroud::Re_Acquire_Resources);
    Hook_Any(0x00769F30, W3DShroud::Set_Shroud_Level);
    Hook_Any(0x0076A320, W3DShroud::Set_Border_Shroud_Level);
    Hook_Any(0x0076A330, W3DShroud::Render);
    Hook_Any(0x0076A550, W3DShroudMaterialPassClass::Install_Materials);
    Hook_Any(0x0076A580, W3DShroudMaterialPassClass::UnInstall_Materials);
    Hook_Any(0x0076A590, W3DMaskMaterialPassClass::Install_Materials);
    Hook_Any(0x0076A5A0, W3DMaskMaterialPassClass::UnInstall_Materials);

    // w3dscene.cpp
    Hook_Any(0x00766600, RTS3DScene::Hook_Ctor);
    Hook_Any(0x00766C30, RTS3DScene::Set_Global_Light);
    Hook_Any(0x00766C80, RTS3DScene::Cast_Ray);
    Hook_Any(0x00766F10, RTS3DScene::Visibility_Check);
    Hook_Any(0x00767360, RTS3DScene::Render_Specific_Drawables);
    Hook_Any(0x00767430, RTS3DScene::Render_One_Object);
    Hook_Any(0x00767B40, RTS3DScene::Flush);
    Hook_Any(0x00767C00, RTS3DScene::Update_Fixed_Light_Environments);
    Hook_Any(0x00768140, RTS3DScene::Render);
    Hook_Any(0x00768530, RTS3DScene::Customized_Render);
    Hook_Any(0x00768A50, RTS3DScene::Flush_Occluded_Objects_Into_Stencil);
    Hook_Any(0x00769370, RTS3DScene::flush_Translucent_Objects);
    Hook_Any(0x007694C0, RTS3DScene::Create_Lights_Iterator);
    Hook_Any(0x007694F0, RTS3DScene::Destroy_Lights_Iterator);
    Hook_Any(0x00769500, RTS3DScene::Get_A_Dynamic_Light);
    Hook_Any(0x00769630, RTS3DScene::Do_Render);
    Hook_Any(0x00769660, RTS3DScene::Draw);
    Hook_Any(0x007696B0, RTS2DScene::Hook_Ctor);
    Hook_Any(0x00769860, RTS2DScene::Do_Render);
    Hook_Any(0x007698D0, RTS3DInterfaceScene::Hook_Ctor);

    // w3dgameclient.cpp
    Hook_Any(0x00775540, W3DGameClient::Create_Mouse);
    Hook_Any(0x007751A0, W3DGameClient::Create_GameDisplay);
    Hook_Any(0x00775050, W3DGameClient::Set_Team_Color);
    Hook_Any(0x00774EA0, W3DGameClient::Create_Drawable);

    // w3dprojectedshadow.cpp
    Hook_Any(0x0075ECC0, W3DProjectedShadowManager::Hook_Ctor);
    Hook_Any(0x0075EF60, W3DProjectedShadowManager::Init);
    Hook_Any(0x0075F090, W3DProjectedShadowManager::Re_Acquire_Resources);
    Hook_Any(0x0075F130, W3DProjectedShadowManager::Release_Resources);
    Hook_Any(0x0075F190, W3DProjectedShadowManager::Update_Render_Target_Textures);
    Hook_Any(0x0075F8C0, W3DProjectedShadowManager::Flush_Decals);
    Hook_Any(0x0075FB50, W3DProjectedShadowManager::Queue_Decal);
    Hook_Any(0x00760880, W3DProjectedShadowManager::Render_Shadows);
    Hook_Any(0x00762250, W3DProjectedShadow::Hook_Dtor);
    Hook_Any(0x00762780, W3DProjectedShadow::Update);
    Hook_Any(0x00762290, W3DProjectedShadow::Update_Texture);
    Hook_Any(0x00762950, W3DShadowTexture::Update_Bounds);
    Hook_Any(0x00762F90, W3DShadowTextureManager::Create_Texture);
    Hook_Any(0x00762130, W3DProjectedShadowManager::Remove_Shadow);
    Hook_Any(0x00761290, W3DProjectedShadow::Release);

    // worldheightmap.cpp
    Hook_Any(0x007440C0, WorldHeightMap::Hook_Dtor);
    Hook_Any(0x00744390, WorldHeightMap::Hook_Ctor2);
    Hook_Any(0x00744340, WorldHeightMap::Free_List_Of_Map_Objects);
    Hook_Any(0x007449F0, WorldHeightMap::Get_Flip_State);
    Hook_Any(0x00744A40, WorldHeightMap::Set_Flip_State);
    Hook_Any(0x00744AB0, WorldHeightMap::Clear_Flip_States);
    Hook_Any(0x00744AE0, WorldHeightMap::Get_Seismic_Z_Velocity);
    Hook_Any(0x00744B20, WorldHeightMap::Set_Seismic_Z_Velocity);
    Hook_Any(0x00744B60, WorldHeightMap::Get_Bilinear_Sample_Seismic_Z_Velocity);
    Hook_Any(0x00744C70, WorldHeightMap::Get_Cliff_State);
    Hook_Any(0x00744CC0, WorldHeightMap::Parse_World_Dict_Data_Chunk);
    Hook_Any(0x00744D50, WorldHeightMap::Parse_Lighting_Data_Chunk);
    Hook_Any(0x00745150, WorldHeightMap::Parse_Objects_Data_Chunk);
    Hook_Any(0x00745210, WorldHeightMap::Parse_Height_Map_Data_Chunk);
    Hook_Any(0x00745230, WorldHeightMap::Parse_Height_Map_Data);
    Hook_Any(0x007454A0, WorldHeightMap::Parse_Size_Only_In_Chunk);
    Hook_Any(0x007454C0, WorldHeightMap::Parse_Size_Only);
    Hook_Any(0x007456D0, WorldHeightMap::Parse_Blend_Tile_Data_Chunk);
    Hook_Any(0x007456F0, WorldHeightMap::Read_Tex_Class);
    Hook_Any(0x00745840, WorldHeightMap::Parse_Blend_Tile_Data);
    Hook_Any(0x00745ED0, WorldHeightMap::Parse_Object_Data_Chunk);
    Hook_Any(0x00745F00, WorldHeightMap::Parse_Object_Data);
    Hook_Any(0x00746170, WorldHeightMap::Count_Tiles);
    Hook_Any(0x00746310, WorldHeightMap::Read_Tiles);
    Hook_Any(0x007465E0, WorldHeightMap::Update_Tile_Texture_Positions);
    Hook_Any(0x00746A70, WorldHeightMap::Get_UV_For_Ndx);
    Hook_Any(0x00746B70, WorldHeightMap::Is_Cliff_Mapped_Texture);
    Hook_Any(0x00746BB0, WorldHeightMap::Get_UV_Data);
    Hook_Any(0x00746C00, WorldHeightMap::Get_UV_For_Tile_Index);
    Hook_Any(0x00747480, WorldHeightMap::Get_Extra_Alpha_UV_Data);
    Hook_Any(0x00747610, WorldHeightMap::Get_Alpha_UV_Data);
    Hook_Any(0x007478F0, WorldHeightMap::Set_Texture_LOD);
    Hook_Any(0x00747910, WorldHeightMap::Get_Terrain_Texture);
    Hook_Any(0x00747C50, WorldHeightMap::Get_Alpha_Terrain_Texture);
    Hook_Any(0x00747C70, WorldHeightMap::Get_Flat_Texture);
    Hook_Any(0x00747D60, WorldHeightMap::Set_Draw_Origin);
    Hook_Any(0x00747E30, WorldHeightMap::Set_Cell_Cliff_Flag_From_Heights);
    Hook_Any(0x00748030, WorldHeightMap::Get_Terrain_Color_At);
    Hook_Any(0x00748170, WorldHeightMap::Get_Terrain_Name_At);
    Hook_Any(0x007482F0, WorldHeightMap::Get_Pointer_To_Tile_Data);
    Hook_Any(0x00748550, WorldHeightMap::Setup_Alpha_Tiles);

    // w3dvolumetricshadow.cpp
    Hook_Any(0x007A2FC0, W3DShadowGeometry::Init_From_HLOD);
    Hook_Any(0x007A3460, W3DShadowGeometry::Init_From_Mesh);
    Hook_Any(0x007A3630, W3DShadowGeometryMesh::Hook_Ctor);
    Hook_Any(0x007A3650, W3DShadowGeometryMesh::~W3DShadowGeometryMesh);
    Hook_Any(0x007A36A0, W3DShadowGeometryMesh::Build_Polygon_Neighbors);
    Hook_Any(0x007A39F0, W3DShadowGeometryMesh::Build_Polygon_Normals);
    Hook_Any(0x007A7AF0, W3DShadowGeometryManager::Free_All_Geoms);
    Hook_Any(0x007A7B70, W3DShadowGeometryManager::Load_Geom);

    // w3dbuffermanager.h
    Hook_Any(0x007D5420, W3DBufferManager::Hook_Ctor);
    Hook_Any(0x007D5470, W3DBufferManager::Hook_Dtor);
    Hook_Any(0x007D5490, W3DBufferManager::Free_All_Slots);
    Hook_Any(0x007D5550, W3DBufferManager::Free_All_Buffers);
    Hook_Any(0x007D55E0, W3DBufferManager::Release_Resources);
    Hook_Any(0x007D5650, W3DBufferManager::ReAcquire_Resources);
    Hook_Any(0x007D57B0, W3DBufferManager::Hook_VB_Get_Slot);
    Hook_Any(0x007D5800, W3DBufferManager::Hook_VB_Release_Slot);
    Hook_Any(0x007D5840, W3DBufferManager::Hook_VB_Allocate_Slot_Storage);
    Hook_Any(0x007D59F0, W3DBufferManager::Hook_IB_Get_Slot);
    Hook_Any(0x007D5A40, W3DBufferManager::Hook_IB_Release_Slot);
    Hook_Any(0x007D5A70, W3DBufferManager::Hook_IB_Allocate_Slot_Storage);

    // for messing with buffer sizes
    int *w3dbm_size = (int *)(0x007A762D + 1);
    *w3dbm_size = sizeof(W3DBufferManager);

    // static_assert(sizeof(W3DBufferManager) == 0x3AC5C, "Size of W3DBufferManager is wrong");

    // polygontrigger.h
    Hook_Any(0x00570D40, PolygonTrigger::Get_Polygon_Trigger_By_ID);
    Hook_Any(0x005714C0, PolygonTrigger::Delete_Triggers);
    Hook_Any(0x00571500, PolygonTrigger::Set_Point);
    Hook_Any(0x005715D0, PolygonTrigger::Get_Center_Point);
    Hook_Any(0x00571640, PolygonTrigger::Get_Radius);
    Hook_Any(0x00571660, PolygonTrigger::Point_In_Trigger);
    Hook_Any(0x005717B0, PolygonTrigger::Get_Water_Handle);

    // framemetrics.cpp
    Hook_Any(0x00716720, FrameMetrics::Hook_Ctor);
    Hook_Any(0x00716820, FrameMetrics::Hook_Dtor);
    Hook_Any(0x00716870, FrameMetrics::Init);
    Hook_Any(0x007168F0, FrameMetrics::Do_Per_Frame_Metrics);
    Hook_Any(0x007169B0, FrameMetrics::Process_Latency_Response);
    Hook_Any(0x00716A40, FrameMetrics::Add_Cushion);

    // energy.cpp
    Hook_Any(0x00576EE0, Energy::Hook_Ctor);
    Hook_Any(0x00576F00, Energy::Get_Production);
    Hook_Any(0x00576F20, Energy::Get_Energy_Supply_Ratio);
    Hook_Any(0x00576F50, Energy::Has_Sufficient_Power);
    Hook_Any(0x00576F80, Energy::Adjust_Power);
    Hook_Any(0x005770D0, Energy::Object_Entering_Influence);
    Hook_Any(0x00577180, Energy::Object_Leaving_Influence);
    Hook_Any(0x00577230, Energy::Add_Power_Bonus);
    Hook_Any(0x00577290, Energy::Remove_Power_Bonus);
    Hook_Any(0x005772F0, Energy::Xfer_Snapshot);

    // laserupdate.cpp
    Hook_Any(0x004E6320, LaserUpdate::Friend_New_Module_Instance);
    Hook_Any(0x004E6400, LaserUpdate::Friend_New_Module_Data);
    Hook_Any(0x0061CD90, LaserUpdate::Set_Decay_Frames);
    Hook_Any(0x0061CDC0, LaserUpdate::Init_Laser);
    Hook_Any(0x0061D110, LaserUpdate::Get_Current_Laser_Radius);

    // experiencetracker.cpp
    Hook_Any(0x0061B790, ExperienceTracker::Get_Experience_Value);
    Hook_Any(0x0061B7D0, ExperienceTracker::Is_Trainable);
    Hook_Any(0x0061B7E0, ExperienceTracker::Is_Accepting_Experience_Points);
    Hook_Any(0x0061B810, ExperienceTracker::Set_Experience_Sink);
    Hook_Any(0x0061B820, ExperienceTracker::Set_Min_Veterency_Level);
    Hook_Any(0x0061B860, ExperienceTracker::Set_Veterency_Level);
    Hook_Any(0x0061B8A0, ExperienceTracker::Gain_Exp_For_Level);
    Hook_Any(0x0061B8F0, ExperienceTracker::Can_Gain_Exp_For_Level);
    Hook_Any(0x0061B910, ExperienceTracker::Add_Experience_Points);
    Hook_Any(0x0061B9D0, ExperienceTracker::Set_Experience_And_Level);

    // smudge.cpp
    Hook_Any(0x007F6250, SmudgeManager::Add_Smudge_Set);
    Hook_Any(0x007F6640, SmudgeSet::Add_Smudge_To_Set);

    // w3dsmudge.cpp
    Hook_Any(0x00783A20, W3DSmudgeManager::Hook_Ctor);
    Hook_Any(0x00783AD0, W3DSmudgeManager::Init);
    Hook_Any(0x00783AF0, W3DSmudgeManager::Reset);
    Hook_Any(0x00783B00, W3DSmudgeManager::Release_Resources);
    Hook_Any(0x00783B20, W3DSmudgeManager::Re_Acquire_Resources);
    Hook_Any(0x00784130, W3DSmudgeManager::Render);

    // fxlist.h
    Hook_Method(PICK_ADDRESS(0x004CC200, 0x0075F326), &FXListStore::Find_FXList);

    // parabolicease.cpp
    Hook_Method(0x007F5D70, &ParabolicEase::Set_Ease_Times);
    Hook_Method(0x007F5E50, &ParabolicEase::operator());

    // playerlist.h
    Hook_Any(0x0045A7C0, PlayerList::Hook_Ctor);
    Hook_Any(0x0045A9E0, PlayerList::Find_Player_With_NameKey);
    Hook_Any(0x0045B0A0, PlayerList::Team_About_To_Be_Deleted);
    Hook_Any(0x0045B0D0, PlayerList::Update_Team_States);
    Hook_Any(0x0045B170, PlayerList::Set_Local_Player);
    Hook_Any(0x0045B1B0, PlayerList::Get_Player_From_Mask);
    Hook_Any(0x0045B1F0, PlayerList::Get_Each_Player_From_Mask);
    Hook_Any(0x0045B250, PlayerList::Get_Players_With_Relationship);
    Hook_Any(0x0045A9C0, PlayerList::Get_Nth_Player);

    // w3dmodeldraw.cpp
    Hook_Any(0x007AF130, ModelConditionInfo::Add_Public_Bone);
    Hook_Any(0x007AF240, ModelConditionInfo::Validate_Cached_Bones);
    Hook_Any(0x007AFCA0, ModelConditionInfo::Validate_Weapon_Barrel_Info);
    Hook_Any(0x007B0260, ModelConditionInfo::Validate_Turret_Info);
    Hook_Any(0x007B0320, ModelConditionInfo::Find_Pristine_Bone);
    Hook_Any(0x007B0580, W3DModelDrawModuleData::Hook_Ctor);
    Hook_Any(0x007B0AE0, W3DModelDrawModuleData::Hook_Dtor);
    Hook_Any(0x007B07E0, W3DModelDrawModuleData::Validate_Stuff_For_Time_And_Weather);
    Hook_Any(0x007B0C90, W3DModelDrawModuleData::Get_Best_Model_Name_For_WB);
    Hook_Any(0x007B0E70, W3DModelDrawModuleData::Build_Field_Parse);
    Hook_Any(0x007B74C0, W3DModelDrawModuleData::CRC_Snapshot);
    Hook_Any(0x007B74D0, W3DModelDrawModuleData::Xfer_Snapshot);
    Hook_Any(0x007BEB10, W3DModelDrawModuleData::Get_Minimum_Required_Game_LOD);
    Hook_Any(0x007B12E0, W3DModelDraw::Show_Sub_Object);
    Hook_Any(0x007B2E50, W3DModelDraw::Hook_Ctor);
    Hook_Any(0x007B30D0, W3DModelDraw::Hook_Dtor);
    Hook_Any(0x007B30A0, W3DModelDraw::On_Drawable_Bound_To_Object);
    Hook_Any(0x007B31D0, W3DModelDraw::Set_Hidden);
    Hook_Any(0x007B3290, W3DModelDraw::Release_Shadows);
    Hook_Any(0x007B32B0, W3DModelDraw::Allocate_Shadows);
    Hook_Any(0x007B33B0, W3DModelDraw::Set_Shadows_Enabled);
    Hook_Any(0x007B33D0, W3DModelDraw::Set_Fully_Obscured_By_Shroud);
    Hook_Any(0x007B3460, W3DModelDraw::Adjust_Transform_Mtx);
    Hook_Any(0x007B35A0, W3DModelDraw::Do_Draw_Module);
    Hook_Any(0x007B3BE0, W3DModelDraw::Do_Hide_Show_Sub_Objs);
    Hook_Any(0x007B45F0, W3DModelDraw::Update_Bones_For_Client_Particle_Systems);
    Hook_Any(0x007B4730, W3DModelDraw::Set_Terrain_Decal);
    Hook_Any(0x007B4840, W3DModelDraw::Set_Terrain_Decal_Size);
    Hook_Any(0x007B48B0, W3DModelDraw::Set_Terrain_Decal_Opacity);
    Hook_Any(0x007B52F0, W3DModelDraw::Replace_Model_Condition_State);
    Hook_Any(0x007B5470, W3DModelDraw::Set_Selectable);
    Hook_Any(0x007B54B0, W3DModelDraw::Replace_Indicator_Color);
    Hook_Any(0x007B5500, W3DModelDraw::Client_Only_Get_Render_Obj_Info);
    Hook_Any(0x007B55B0, W3DModelDraw::Get_Projectile_Launch_Offset);
    Hook_Any(0x007B5A50, W3DModelDraw::Get_Pristine_Bone_Positions_For_Condition_State);
    Hook_Any(0x007B5DB0, W3DModelDraw::Client_Only_Get_Render_Obj_Bound_Box);
    Hook_Any(0x007B5F90, W3DModelDraw::Client_Only_Get_Render_Obj_Bone_Transform);
    Hook_Any(0x007B6060, W3DModelDraw::Get_Current_Worldspace_Client_Bone_Positions);
    Hook_Any(0x007B60F0, W3DModelDraw::Get_Current_Bone_Positions);
    Hook_Any(0x007B63E0, W3DModelDraw::React_To_Transform_Change);
    Hook_Any(0x007B64D0, W3DModelDraw::Get_Barrel_Count);
    Hook_Any(0x007B6520, W3DModelDraw::Handle_Weapon_Fire_FX);
    Hook_Any(0x007B6720, W3DModelDraw::Set_Animation_Loop_Duration);
    Hook_Any(0x007B6800, W3DModelDraw::Set_Animation_Completion_Time);
    Hook_Any(0x007B68F0, W3DModelDraw::Set_Animation_Frame);
    Hook_Any(0x007B69A0, W3DModelDraw::Set_Pause_Animation);
    Hook_Any(0x007B6A30, W3DModelDraw::Rebuild_Weapon_RecoilInfo);
    Hook_Any(0x007B6B70, W3DModelDraw::Preload_Assets);
    Hook_Any(0x007B6BC0, W3DModelDraw::Is_Visible);
    Hook_Any(0x007B6BE0, W3DModelDraw::Update_Projectile_Clip_Status);
    Hook_Any(0x007B6C10, W3DModelDraw::Update_Draw_Module_Supply_Status);
    Hook_Any(0x007B6C40, W3DModelDraw::Do_Hide_Show_Projectile_Objects);
    Hook_Any(0x007B6E50, W3DModelDraw::Update_Sub_Objects);
    Hook_Any(0x007B6F90, W3DModelDraw::Xfer_Snapshot);
    Hook_Any(0x007BEB60, W3DModelDraw::Get_Module_Name_Key);
    Hook_Any(0x007B4B00, W3DModelDraw::Set_Model_State);
    Hook_Any(0x007B3920, W3DModelDraw::Adjust_Animation);
    Hook_Any(0x007B3D40, W3DModelDraw::Handle_Client_Turret_Positioning);
    Hook_Any(0x007B4240, W3DModelDraw::Recalc_Bones_For_Client_Particle_Systems);
    Hook_Any(0x007B3F90, W3DModelDraw::Handle_Client_Recoil);

    // bezfwditerator.cpp
    Hook_Method(0x00736100, &BezFwdIterator::Start);
    Hook_Method(0x00736280, &BezFwdIterator::Done);
    Hook_Method(0x007362A0, &BezFwdIterator::Get_Current);
    Hook_Method(0x007362B0, &BezFwdIterator::Next);

    // beziersegment.cpp
    Hook_Method(0x0072BFD0, &BezierSegment::Hook_Ctor1);
    Hook_Method(0x0072BFF0, &BezierSegment::Hook_Ctor5);
    Hook_Method(0x0072C060, &BezierSegment::Evaluate_Bez_Segment_At_T);
    Hook_Method(0x0072C180, &BezierSegment::Get_Segment_Points);
    Hook_Method(0x0072C2A0, &BezierSegment::Get_Approximate_Length);
    Hook_Method(0x0072C440, &BezierSegment::Split_Segment_At_T);

    // swayclientupdate.h
    Hook_Method(0x006885A0, &SwayClientUpdate::Hook_Ctor);
    Hook_Any(0x006889C0, SwayClientUpdate::Xfer_Snapshot);
    Hook_Any(0x00688A40, SwayClientUpdate::Load_Post_Process);
    Hook_Any(0x00688630, SwayClientUpdate::Get_Module_Name_Key);
    Hook_Any(0x006886C0, SwayClientUpdate::Client_Update);

    // statemachine.cpp
    Hook_Any(0x006F7760, StateMachine::Hook_Ctor);
    Hook_Any(0x006F78F0, StateMachine::Hook_Dtor);
    Hook_Any(0x006F7AB0, StateMachine::Clear);
    Hook_Any(0x006F7AE0, StateMachine::Reset_To_Default_State);
    Hook_Any(0x006F7B30, StateMachine::Update_State_Machine);
    Hook_Any(0x006F7BB0, StateMachine::Define_State);
    Hook_Any(0x006F7C70, StateMachine::Internal_Get_State);
    Hook_Any(0x006F7CF0, StateMachine::Set_State);
    Hook_Any(0x006F7D10, StateMachine::Internal_Set_State);
    Hook_Any(0x006F7DF0, StateMachine::Init_Default_State);
    Hook_Any(0x006F7E10, StateMachine::Set_Goal_Object);
    Hook_Any(0x006F7E50, StateMachine::Is_Goal_Object_Destroyed);
    Hook_Any(0x006F7E90, StateMachine::Halt);
    Hook_Any(0x006F7ED0, StateMachine::Set_Goal_Position);
    Hook_Any(0x006F7F00, StateMachine::Xfer_Snapshot);
    Hook_Any(0x006F7650, State::Friend_Check_For_Sleep_Transitions);
    Hook_Any(0x006F74A0, State::Friend_Check_For_Transitions);

    // tintenvelope.cpp
    Hook_Any(0x004796B0, TintEnvelope::Play);
    Hook_Any(0x004797E0, TintEnvelope::Set_Attack_Frames);
    Hook_Any(0x00479850, TintEnvelope::Set_Decay_Frames);
    Hook_Any(0x004798B0, TintEnvelope::Update);
    Hook_Any(0x00479AA0, TintEnvelope::Xfer_Snapshot);

    // audioeventinfo.cpp
    Hook_Any(0x005DA460, DynamicAudioEventInfo::Hook_Ctor);
    Hook_Any(0x005DA5D0, DynamicAudioEventInfo::Hook_Ctor2);
    Hook_Any(0x005DA7B0, DynamicAudioEventInfo::Override_Audio_Name);
    Hook_Any(0x005DA840, DynamicAudioEventInfo::Override_Loop_Flag);
    Hook_Any(0x005DA870, DynamicAudioEventInfo::Override_Loop_Count);
    Hook_Any(0x005DA890, DynamicAudioEventInfo::Override_Volume);
    Hook_Any(0x005DA8B0, DynamicAudioEventInfo::Override_Min_Volume);
    Hook_Any(0x005DA8D0, DynamicAudioEventInfo::Override_Min_Range);
    Hook_Any(0x005DA8F0, DynamicAudioEventInfo::Override_Max_Range);
    Hook_Any(0x005DA910, DynamicAudioEventInfo::Override_Priority);
    Hook_Any(0x005DA930, DynamicAudioEventInfo::Get_Original_Name);
    Hook_Any(0x005DA950, DynamicAudioEventInfo::Xfer_No_Name);

    // anim2d.cpp
    Hook_Any(0x0051A180, Anim2D::Hook_Ctor);
    Hook_Any(0x0051A3E0, Anim2D::Reset);
    Hook_Any(0x0051A590, Anim2D::Get_Current_Frame_Width);
    Hook_Any(0x0051A5C0, Anim2D::Get_Current_Frame_Height);
    Hook_Any(0x0051A5F0, Anim2D::Draw);
    Hook_Any(0x0051A6F0, Anim2DCollection::Hook_Ctor);
    Hook_Any(0x0051A860, Anim2DCollection::Find_Template);

    // physicsupdate.h
    Hook_Method(0x005CDB70, &PhysicsBehavior::Get_Forward_Speed_2D);
    Hook_Method(0x005CDBC0, &PhysicsBehavior::Get_Forward_Speed_3D);
    Hook_Any(0x005CC070, PhysicsBehavior::Apply_Force);
    Hook_Any(0x005CC2E0, PhysicsBehavior::Is_Motive);
    Hook_Any(0x005CDB20, PhysicsBehavior::Get_Velocity_Magnitude);
    Hook_Any(0x005CDCC0, PhysicsBehavior::Scrub_Velocity_2D);
    Hook_Any(0x005CDDB0, PhysicsBehavior::Set_Angles);
    Hook_Any(0x005CDFD0, PhysicsBehavior::Get_Mass);
    Hook_Any(0x004E7CF0, PhysicsBehavior::Friend_New_Module_Instance);
    Hook_Any(0x004E7DE0, PhysicsBehavior::Friend_New_Module_Data);
    Hook_Any(0x005CC300, PhysicsBehavior::Apply_Motive_Force);
    Hook_Any(0x005CDD80, PhysicsBehavior::Add_Velocity_To);
    Hook_Any(0x005CDC30, PhysicsBehavior::Scrub_Velocity_Z);
    Hook_Any(0x005CC330, PhysicsBehavior::Reset_Dynamic_Physics);
    Hook_Any(0x005CDD30, PhysicsBehavior::Transfer_Velocity_To);
    Hook_Any(0x005CC230, PhysicsBehavior::Apply_Random_Rotation);
    Hook_Any(0x005CC190, PhysicsBehavior::Apply_Shock);
    Hook_Any(0x005CC050, PhysicsBehavior::Set_Ignore_Collisions_With);
    Hook_Any(0x005CCBC0, PhysicsBehavior::Set_Pitch_Rate);
    Hook_Any(0x005CCC10, PhysicsBehavior::Set_Roll_Rate);
    Hook_Any(0x005CCC60, PhysicsBehavior::Set_Yaw_Rate);
    Hook_Any(0x005CCCB0, PhysicsBehavior::Set_Bounce_Sound);

    // objecttypes.h
    Hook_Any(0x005382E0, ObjectTypes::Xfer_Snapshot);

    // drawable.cpp
    Hook_Any(0x0046DAB0, Drawable::Init_Static_Images);
    Hook_Any(0x0046E100, Drawable::Kill_Static_Images);
    Hook_Any(0x00473320, Drawable::Draw_Icon_UI);
    Hook_Any(0x004735D0, Drawable::Set_Emoticon);
    Hook_Any(0x00473DB0, Drawable::Draw_UI_Text);
    Hook_Method(
        0x00477050, static_cast<void (Drawable::*)(BodyDamageType, TimeOfDayType, bool)>(&Drawable::Start_Ambient_Sound));
    Hook_Any(0x00477570, Drawable::Stop_Ambient_Sound);
    Hook_Any(0x00476A50, Drawable::Get_Transform_Matrix);
    Hook_Any(0x0046EB10, Drawable::Is_Visible);
    Hook_Any(0x00476970, Drawable::Set_Instance_Matrix);
    Hook_Any(0x004766C0, Drawable::Set_Position);
    Hook_Any(0x00475DC0, Drawable::Clear_And_Set_Model_Condition_State);
    Hook_Any(0x00475FA0, Drawable::Clear_And_Set_Model_Condition_Flags);
    Hook_Any(0x00476F90, Drawable::Set_Custom_Sound_Ambient_Info);
    Hook_Any(0x00476E40, Drawable::Set_Custom_Sound_Ambient_Off);
    Hook_Any(0x004775A0, Drawable::Enable_Ambient_Sound);
    Hook_Any(0x00477650, Drawable::Enable_Ambient_Sound_From_Script);
    Hook_Any(0x00476D20, Drawable::Get_Base_Sound_Ambient_Info);
    Hook_Any(0x00476D50, Drawable::Mangle_Custom_Audio_Name);
    Hook_Any(0x00476090, Drawable::Replace_Model_Condition_Flags);
    Hook_Any(0x00476220, Drawable::Set_ID);
    Hook_Any(0x0046F220, Drawable::Set_Terrain_Decal);
    Hook_Any(0x0046F190, Drawable::Get_Current_Worldspace_Client_Bone_Positions);
    Hook_Any(0x0046EFD0, Drawable::Get_Pristine_Bone_Positions);
    Hook_Any(0x0046EB90, Drawable::Get_Should_Animate);
    Hook_Any(0x0046F430, Drawable::Allocate_Shadows);
    Hook_Any(0x0046EC00, Drawable::Client_Only_Get_First_Render_Obj_Info);
    Hook_Any(0x0046ED40, Drawable::Set_Animation_Loop_Duration);
    Hook_Any(0x0046EDC0, Drawable::Set_Animation_Completion_Time);
    Hook_Any(0x0046EEC0, Drawable::Update_Sub_Objects);
    Hook_Any(0x0046EF40, Drawable::Show_Sub_Object);
    Hook_Any(0x0046F290, Drawable::Set_Terrain_Decal_Size);
    Hook_Any(0x0046F330, Drawable::Set_Shadows_Enabled);
    Hook_Any(0x0046F3C0, Drawable::Release_Shadows);
    Hook_Any(0x0046F0C0, Drawable::Get_Current_Client_Bone_Positions);
    Hook_Any(0x0046F300, Drawable::Set_Terrain_Decal_Fade_Target);
    Hook_Any(0x0046FCE0, Drawable::Imitate_Stealth_Look);
    Hook_Any(0x00472E10, Drawable::Set_Stealth_Look);
    Hook_Any(0x00471BE0, Drawable::Calc_Physics_Xform_Wheels);
    Hook_Any(0x00471370, Drawable::Calc_Physics_Xform_Treads);
    Hook_Any(0x00472550, Drawable::Calc_Physics_Xform_Motorcycle);
    Hook_Any(0x004708E0, Drawable::Apply_Physics_Xform);
    Hook_Any(0x00477A80, Drawable::Find_Client_Update_Module);
    Hook_Any(0x0046F4A0, Drawable::Set_Fully_Obscured_By_Shroud);
    Hook_Any(0x0046FD60, Drawable::Update_Drawable);
    Hook_Any(0x0046F580, Drawable::Color_Flash);
    Hook_Any(0x00472F90, Drawable::Draw);
    Hook_Any(0x00476180, Drawable::Set_Indicator_Color);

    Hook_Any(0x0046F850, Drawable::Color_Tint);
    Hook_Any(0x004794C0, Drawable::Friend_Lock_Dirty_Stuff_For_Iteration);
    Hook_Any(0x00479550, Drawable::Friend_Unlock_Dirty_Stuff_For_Iteration);
    Hook_Any(0x00476490, Drawable::Changed_Team);
    Hook_Any(0x00476BF0, Drawable::Clear_Caption_Text);
    Hook_Any(0x0046FAA0, Drawable::Fade_In);
    Hook_Any(0x0046FAD0, Drawable::Fade_Out);
    Hook_Any(0x004705D0, Drawable::Flash_As_Selected);
    Hook_Any(0x00476290, Drawable::Friend_Bind_To_Object);
    Hook_Any(0x0046F560, Drawable::Friend_Clear_Selected);
    Hook_Any(0x0046F520, Drawable::Friend_Set_Selected);
    Hook_Any(0x0046FA30, Drawable::On_Selected);
    Hook_Any(0x004768E0, Drawable::Get_Barrel_Count);
    Hook_Any(0x00476C20, Drawable::Get_Caption_Text);
    Hook_Any(0x0046EC90, Drawable::Get_Projectile_Launch_Offset);
    Hook_Any(0x004767C0, Drawable::Handle_Weapon_Fire_FX);
    Hook_Any(0x00477A20, Drawable::Is_Mass_Selectable);
    Hook_Any(0x00477A00, Drawable::Is_Selectable);
    Hook_Any(0x004778F0, Drawable::Notify_Drawable_Dependency_Cleared);
    Hook_Any(0x00470520, Drawable::On_Level_Start);
    Hook_Any(0x00477A40, Drawable::Preload_Assets);
    Hook_Any(0x004776F0, Drawable::Prepend_To_List);
    Hook_Any(0x00477720, Drawable::Remove_From_List);
    Hook_Any(0x0046FB00, Drawable::React_To_Body_Damage_State_Change);
    Hook_Any(0x00476750, Drawable::React_To_Geometry_Change);
    Hook_Any(0x00476A70, Drawable::Set_Caption_Text);
    Hook_Any(0x00477760, Drawable::Set_Drawable_Hidden);
    Hook_Any(0x0046FC40, Drawable::Set_Effective_Opacity);
    Hook_Any(0x00477970, Drawable::Set_Selectable);
    Hook_Any(0x00476C60, Drawable::Set_Time_Of_Day);
    Hook_Any(0x00477820, Drawable::Update_Drawable_Clip_Status);
    Hook_Any(0x00477860, Drawable::Update_Drawable_Supply_Status);

    // updatemodule.h
    Hook_Method(0x006F5760, &UpdateModule::Get_Wake_Frame);
    Hook_Method(0x006F5780, &UpdateModule::Set_Wake_Frame);
    Hook_Any(0x006F57A0, UpdateModule::Xfer_Snapshot);

    // projectilestreamupdate.cpp
    Hook_Any(0x004E7FB0, ProjectileStreamUpdate::Friend_New_Module_Instance);
    Hook_Any(0x0061C410, ProjectileStreamUpdate::Add_Projectile);
    Hook_Any(0x0061C650, ProjectileStreamUpdate::Set_Position);

    // science.h
    Hook_Any(0x004887F0, ScienceStore::Get_Science_From_Internal_Name);
    Hook_Any(0x00488820, ScienceStore::Get_Internal_Name_From_Science);
    Hook_Any(0x00488C40, ScienceStore::Get_Science_Purchase_Cost);
    Hook_Any(0x00488C90, ScienceStore::Is_Science_Grantable);
    Hook_Any(0x00488CE0, ScienceStore::Get_Name_And_Description);

    // scorekeeper.cpp
    Hook_Any(0x00574480, ScoreKeeper::Reset);
    Hook_Any(0x00574630, ScoreKeeper::Add_Object_Built);
    Hook_Method(0x00574760,
        static_cast<int (ScoreKeeper::*)(BitFlags<KINDOF_COUNT>, BitFlags<KINDOF_COUNT>)>(
            &ScoreKeeper::Get_Total_Units_Built));
    Hook_Any(0x00574880, ScoreKeeper::Get_Total_Objects_Built);
    Hook_Any(0x00574900, ScoreKeeper::Remove_Object_Built);
    Hook_Any(0x00574A30, ScoreKeeper::Add_Object_Captured);
    Hook_Any(0x00574B00, ScoreKeeper::Add_Object_Destroyed);
    Hook_Any(0x00574C60, ScoreKeeper::Add_Object_Lost);
    Hook_Any(0x00574DD0, ScoreKeeper::Calculate_Score);
    Hook_Any(0x00574E20, ScoreKeeper::Get_Total_Buildings_Destroyed);
    Hook_Any(0x00574E40, ScoreKeeper::Get_Total_Units_Destroyed);
    Hook_Any(0x00574E60, ScoreKeeper::Xfer_Object_Count_Map);
    Hook_Any(0x00575060, ScoreKeeper::Xfer_Snapshot);

    // pointgr.cpp
    Hook_Any(0x00834CE0, PointGroupClass::Hook_Ctor);
    Hook_Any(0x00834E10, PointGroupClass::Set_Arrays);
    Hook_Any(0x00834F20, PointGroupClass::Set_Point_Color);
    Hook_Any(0x00834F40, PointGroupClass::Set_Point_Orientation);
    Hook_Any(0x00834F50, PointGroupClass::Set_Point_Frame);
    Hook_Any(0x00834F60, PointGroupClass::Set_Point_Mode);
    Hook_Any(0x00834F70, PointGroupClass::Set_Flag);
    Hook_Any(0x00834FA0, PointGroupClass::Set_Billboard);
    Hook_Any(0x00834FB0, PointGroupClass::Get_Frame_Row_Column_Count_Log2);
    Hook_Any(0x00834FC0, PointGroupClass::Set_Frame_Row_Column_Count_Log2);
    Hook_Any(0x00835050, PointGroupClass::Render);
    Hook_Any(0x00838180, PointGroupClass::Render_Volume_Particle);

    // streak.cpp
    Hook_Any(0x00834010, StreakLineClass::Prepare_LOD);

    // w3dbibbuffer.cpp
    Hook_Any(0x00796890, W3DBibBuffer::Load_Bibs_In_Vertex_And_Index_Buffers);
    Hook_Any(0x00796C20, W3DBibBuffer::Hook_Dtor);
    Hook_Any(0x00796C80, W3DBibBuffer::Hook_Ctor);
    Hook_Any(0x00796EC0, W3DBibBuffer::Free_Bib_Buffers);
    Hook_Any(0x00796F00, W3DBibBuffer::Allocate_Bib_Buffers);
    Hook_Any(0x00797010, W3DBibBuffer::Clear_All_Bibs);
    Hook_Any(0x00797060, W3DBibBuffer::Add_Bib_To_Object);
    Hook_Any(0x00797140, W3DBibBuffer::Add_Bib_To_Drawable);
    Hook_Any(0x00797220, W3DBibBuffer::Remove_Bib_From_Object);
    Hook_Any(0x00797270, W3DBibBuffer::Remove_Bib_From_Drawable);
    Hook_Any(0x007972C0, W3DBibBuffer::Render_Bibs);

    // locomotor.h
    Hook_Any(0x004B81B0, LocomotorStore::Hook_Ctor);
    Hook_Method(0x004B8460,
        static_cast<LocomotorTemplate *(LocomotorStore::*)(NameKeyType)>(&LocomotorStore::Find_Locomotor_Template));
    Hook_Any(0x004B8A80, Locomotor::Hook_Ctor);
    Hook_Any(0x004B8E70, Locomotor::Get_Max_Speed_For_Condition);
    Hook_Any(0x004B8ED0, Locomotor::Get_Max_Turn_Rate);
    Hook_Any(0x004B8F40, Locomotor::Get_Max_Lift);
    Hook_Any(0x004B9220, Locomotor::Loco_Update_Move_Towards_Position);
    Hook_Any(0x004B8FA0, Locomotor::Loco_Update_Move_Towards_Angle);
    Hook_Any(0x004BAF80, Locomotor::Move_Towards_Position_Thrust);
    Hook_Any(0x004BC500, Locomotor::Move_Towards_Position_Other);
    Hook_Any(0x004BC0E0, Locomotor::Handle_Behavior_Z);
    Hook_Any(0x004B9170, Locomotor::Set_Physics_Options);
    Hook_Any(0x004B8E30, Locomotor::Start_Move);
    Hook_Any(0x004BC880, Locomotor::Loco_Update_Maintain_Current_Position);
    Hook_Any(0x004BCD10, LocomotorSet::Hook_Ctor);
    Hook_Any(0x004BCD60, LocomotorSet::Hook_Dtor);
    Hook_Any(0x004BD3A0, LocomotorSet::Clear);
    Hook_Any(0x004BD420, LocomotorSet::Add_Locomotor);
    Hook_Any(0x004BD550, LocomotorSet::Find_Locomotor);

    // turretai.h
    Hook_Any(0x00715670, TurretAI::Is_Owners_Cur_Weapon_On_Turret);
    Hook_Any(0x007156B0, TurretAI::Is_Weapon_Slot_On_Turret);
    Hook_Any(0x007156D0, TurretAI::Friend_Get_Turret_Target);
    Hook_Any(0x00715780, TurretAI::Set_Turret_Target_Object);
    Hook_Any(0x007159B0, TurretAI::Recenter_Turret);
    Hook_Any(0x00714EB0, TurretAI::Hook_Ctor);
    Hook_Any(0x00715620, TurretAI::Is_Trying_To_Aim_At_Target);
    Hook_Any(0x007158A0, TurretAI::Set_Turret_Target_Position);
    Hook_Any(0x007159C0, TurretAI::Is_Turret_In_Neutral_Position);
    Hook_Any(0x00715A00, TurretAI::Update_Turret_AI);
    Hook_Any(0x00715B40, TurretAI::Set_Turret_Enabled);

    // aiupdate.h
    Hook_Any(0x005CFDF0, AIUpdateModuleData::Parse_Turret);

    // team.h
    Hook_Any(0x0049C4F0, TeamRelationMap::Hook_Ctor);
    Hook_Any(0x0049CA30, TeamFactory::Hook_Ctor);
    Hook_Any(0x0049CE60, TeamFactory::Init_From_Sides);
    Hook_Any(0x0049D120, TeamFactory::Find_Team_Prototype);
    Hook_Any(0x0049D190, TeamFactory::Find_Team_Prototype_By_ID);
    Hook_Any(0x0049D1F0, TeamFactory::Find_Team_By_ID);
    Hook_Any(0x0049D280, TeamFactory::Create_Inactive_Team);
    Hook_Any(0x0049D450, TeamFactory::Create_Team);
    Hook_Any(0x0049D540, TeamFactory::Find_Team);
    Hook_Any(0x0049FEF0, Team::Remove_Override_Team_Relationship);
    Hook_Any(0x0049EE50, TeamPrototype::Count_Objects_By_Thing_Template);
    Hook_Any(0x0049EEA0, TeamPrototype::Increase_AI_Priority_For_Success);
    Hook_Any(0x0049EEC0, TeamPrototype::Decrease_AI_Priority_For_Failure);
    Hook_Any(0x0049EEE0, TeamPrototype::Count_Buildings);
    Hook_Any(0x0049EF40, TeamPrototype::Count_Objects);
    Hook_Any(0x0049F0A0, TeamPrototype::Heal_All_Objects);
    Hook_Any(0x0049F0E0, TeamPrototype::Iterate_Objects);
    Hook_Any(0x0049F130, TeamPrototype::Count_Team_Instances);
    Hook_Any(0x0049F160, TeamPrototype::Has_Any_Buildings);
    Hook_Any(0x0049F210, TeamPrototype::Has_Any_Units);
    Hook_Any(0x0049F240, TeamPrototype::Has_Any_Objects);
    Hook_Any(0x0049F270, TeamPrototype::Update_State);
    Hook_Any(0x0049F390, TeamPrototype::Has_Any_Build_Facility);
    Hook_Any(0x0049F3E0, TeamPrototype::Evaluate_Production_Condition);
    Hook_Any(0x004A0180, Team::Count_Objects_By_Thing_Template);
    Hook_Any(0x004A0220, Team::Count_Buildings);
    Hook_Any(0x004A0250, Team::Has_Any_Units);
    Hook_Any(0x004A0300, Team::Has_Any_Objects);
    Hook_Any(0x004A0360, Team::Update_State);
    Hook_Any(0x0049FBB0, Team::Get_Controlling_Player);
    Hook_Any(0x004A10B0, Team::Damage_Team_Members);
    Hook_Any(0x0049FBC0, Team::Set_Controlling_Player);
    Hook_Any(0x0049FC10, Team::Set_Attack_Priority_Name);
    Hook_Any(0x0049FCB0, Team::Get_Team_As_AI_Group);
    Hook_Any(0x0049FCE0, Team::Get_Relationship);
    Hook_Any(0x0049FD80, Team::Set_Team_Target_Object);
    Hook_Any(0x0049FDC0, Team::Get_Team_Target_Object);
    Hook_Any(0x0049FE40, Team::Set_Override_Team_Relationship);
    Hook_Any(0x0049FFE0, Team::Set_Override_Player_Relationship);
    Hook_Any(0x004A0090, Team::Remove_Override_Player_Relationship);
    Hook_Any(0x004A02B0, Team::Is_Idle);
    Hook_Any(0x004A05F0, Team::Notify_Team_Of_Object_Death);
    Hook_Any(0x004A0620, Team::Did_All_Enter);
    Hook_Any(0x004A06F0, Team::Did_Partial_Enter);
    Hook_Any(0x004A0780, Team::Did_Partial_Exit);
    Hook_Any(0x004A0810, Team::Did_All_Exit);
    Hook_Any(0x004A08F0, Team::All_Inside);
    Hook_Any(0x004A09B0, Team::Some_Inside_Some_Outside);
    Hook_Any(0x004A0A70, Team::Get_Estimate_Team_Position);
    Hook_Any(0x004A0A80, Team::Delete_Team);
    Hook_Any(0x004A0BC0, Team::Transfer_Units_To);
    Hook_Any(0x004A0BF0, Team::Try_To_Recruit);
    Hook_Any(0x004A0DC0, Team::Evacuate_Team);
    Hook_Any(0x004A0F00, Team::Kill_Team);
    Hook_Any(0x004A1230, Team::Update_Generic_Scripts);

    // filesystem.h
    Hook_Any(0x004465D0, FileSystem::Open_File);
    Hook_Any(0x00446610, FileSystem::Does_File_Exist);
    Hook_Any(0x00446770, FileSystem::Get_File_List_In_Directory);
    Hook_Any(0x004468A0, FileSystem::Get_File_Info);
    Hook_Any(0x004468F0, FileSystem::Create_Directory);
    Hook_Any(0x004469C0, FileSystem::Are_Music_Files_On_CD); // This one is part of the CD Check.
    Hook_Any(0x00446BF0, FileSystem::Load_Music_Files_From_CD);
    Hook_Any(0x00446D50, FileSystem::Unload_Music_Files_From_CD);

    // w3dpropbuffer.h
    Hook_Any(0x00797400, W3DPropBuffer::Hook_Dtor);
    Hook_Any(0x007974D0, W3DPropBuffer::Hook_Ctor);
    Hook_Any(0x00797600, W3DPropBuffer::Clear_All_Props);
    Hook_Any(0x00797660, W3DPropBuffer::Add_Prop_Type);
    Hook_Any(0x00797780, W3DPropBuffer::Add_Prop);
    Hook_Any(0x00797A20, W3DPropBuffer::Remove_Props_For_Construction);
    Hook_Any(0x00797B60, W3DPropBuffer::Notify_Shroud_Changed);
    Hook_Any(0x00797BA0, W3DPropBuffer::Draw_Props);
}
