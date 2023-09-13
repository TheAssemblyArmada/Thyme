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

#include "archivefile.h"
#include "archivefilesystem.h"
#include "asciistring.h"
#include "audioeventrts.h"
#include "audiomanager.h"
#include "binkvideoplayer.h"
#include "binkvideostream.h"
#include "bitmaphandler.h"
#include "cavesystem.h"
#include "chunkio.h"
#include "commandline.h"
#include "commandlist.h"
#include "compressionmanager.h"
#include "copyprotect.h"
#include "datachunk.h"
#include "ddsfile.h"
#include "dict.h"
#include "dx8caps.h"
#include "dx8fvf.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include "experiencetracker.h"
#include "filesystem.h"
#include "filetransfer.h"
#include "force_nocd.h"
#include "functionlexicon.h"
#include "gamemath.h"
#include "gamememory.h"
#include "gamememoryinit.h"
#include "gamemessage.h"
#include "gamestate.h"
#include "gametext.h"
#include "geometry.h"
#include "globaldata.h"
#include "hooker.h"
#include "ini.h"
#include "keyboard.h"
#include "main.h"
#include "memdynalloc.h"
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
#include "render2d.h"
#include "science.h"
#include "script.h"
#include "scriptaction.h"
#include "scriptcondition.h"
#include "scriptgroup.h"
#include "scriptlist.h"
#include "setuphooks.h"
#include "shader.h"
#include "sidesinfo.h"
#include "sideslist.h"
#include "gamesounds.h"
#include "targa.h"
#include "teamsinfo.h"
#include "texturebase.h"
#include "textureloader.h"
#include "textureloadtask.h"
#include "thread.h"
#include "thumbnail.h"
#include "thumbnailmanager.h"
#include "w3dbuffermanager.h"
#include "w3dfilesystem.h"
#include "weapon.h"
#include "win32gameengine.h"
#include "win32localfilesystem.h"
#include "wwstring.h"
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>
#include <winsock2.h>

SimpleCriticalSectionClass critSec1;
SimpleCriticalSectionClass critSec2;
SimpleCriticalSectionClass critSec3;

void Null_Func(){};

void Setup_Hooks()
{
    // memory functions crash without this
    g_unicodeStringCriticalSection = &critSec1;
    g_dmaCriticalSection = &critSec2;
    g_memoryPoolCriticalSection = &critSec3;

    // TODO, update with world builder addresses.
#if 0
    // Code that checks the launcher is running, launcher does CD check.
    Hook_Function(0x00412420, CopyProtect::checkForMessage);
    Hook_Function(0x00412450, CopyProtect::validate);

    // Returns true for any CD checks
    Hook_Function(0x005F1CB0, IsFirstCDPresent);
#endif
    // Replace memory init functions.
    Hook_Function(0x006B837C, Init_Memory_Manager);
    Hook_Function(0x006B8570, Init_Memory_Manager_Pre_Main);

    // Replace memory allocation operators
    // TODO, use linker check globals so no need to hook new.
    Hook_Function(0x006B8044, New_New); // operator new
    Hook_Function(0x006B80AB, New_New); // operator new[]
    Hook_Function(0x006B81E0, New_New_Dbg); // debug operator new
    Hook_Function(0x006B82AE, New_New_Dbg); // debug operator new[]
    Hook_Function(0x006B8112, New_Delete); // operator delete
    Hook_Function(0x006B8179, New_Delete); // operator delete[]
    Hook_Function(0x006B8247, New_Delete_Dbg); // debug operator delete
    Hook_Function(0x006B8315, New_Delete_Dbg); //  debug operator delete[]
    Hook_Function(0x006B8716, Create_Named_Pool); // createW3DMemoryPool in wb db.

    // Replace pool functions
    Hook_Method(0x006B70FC, &MemoryPool::Allocate_Block);
    Hook_Method(0x006B712E, &MemoryPool::Free_Block);

    // Replace DMA functions
    Hook_Method(0x006B7819, &DynamicMemoryAllocator::Allocate_Bytes_No_Zero);
    Hook_Method(0x006B7933, &DynamicMemoryAllocator::Allocate_Bytes);
    Hook_Method(0x006B7966, &DynamicMemoryAllocator::Free_Bytes);
    Hook_Method(0x006B7A65, &DynamicMemoryAllocator::Get_Actual_Allocation_Size);

    // Replace MemoryPoolFactory functions
    Hook_Method(0x006B7BA9,
        static_cast<MemoryPool *(MemoryPoolFactory::*const)(char const *, int, int, int)>(
            &MemoryPoolFactory::Create_Memory_Pool));
#if 0
    // Replace File functions
    // virtuals for these are hooked by calls to Win32GameEngine which creates the objects with our vtables.
    Hook_Function(0x004469C0, FileSystem::Are_Music_Files_On_CD); // This one is part of the CD Check.
    Hook_Method(0x00446770, &FileSystem::Get_File_List_From_Dir);
    Hook_Method(0x0048F410, &ArchiveFileSystem::Get_File_List_From_Dir);
    Hook_Method(0x0048F250, &ArchiveFileSystem::Get_Archive_Filename_For_File);
    Hook_Method(0x0048F160, &ArchiveFileSystem::Get_File_Info);

    // Replace Utf8String
    Hook_Method(0x0040D640, static_cast<void (Utf8String::*)(char const *)>(&Utf8String::Set));
    Hook_Method(0x00415290, &Utf8String::Ensure_Unique_Buffer_Of_Size);
    Hook_Method(0x0040FB40, static_cast<void (Utf8String::*)(char const *)>(&Utf8String::Concat));

    // Replace Win32GameEngine
    Hook_Method(0x007420F0, &Win32GameEngine::Hook_Create_Local_File_System);
    Hook_Method(0x00742150, &Win32GameEngine::Hook_Create_Archive_File_System);
    Hook_Method(0x00741FA0, &Win32GameEngine::Hook_Create_Module_Factory);
    Hook_Method(0x007424B0, &Win32GameEngine::Hook_Create_Audio_Manager);

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
    Hook_Method(0x0040D760, &GameMessageList::Hook_Append_Message);
    Hook_Method(0x0040D7A0, &GameMessageList::Hook_Insert_Message);
    Hook_Method(0x0040D7D0, &GameMessageList::Hook_Remove_Message);
    Hook_Method(0x0040D820, &GameMessageList::Hook_Contains_Message_Of_Type);

    // messagestream.h MessageStream
    Hook_Method(0x0040D960, &MessageStream::Hook_Append_Message);
    Hook_Method(0x0040DA00, &MessageStream::Hook_Insert_Message);
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
    Hook_Method(0x00763880, &W3DFileSystem::Hook_Get_File);
    Hook_Method(0x00815370, &W3DFileSystem::Hook_Return_File);

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
    Hook_Method(0x008301D0, &TextureLoadTaskClass::Hook_Destroy);
    Hook_Method(0x00830210, &TextureLoadTaskClass::Hook_Init);
    Hook_Method(0x00831160, &TextureLoadTaskClass::Hook_Deinit);
    Hook_Method(0x008303B0, &TextureLoadTaskClass::Hook_Begin_Compressed_Load);
    Hook_Method(0x00830950, &TextureLoadTaskClass::Hook_Begin_Uncompressed_Load);
    Hook_Method(0x00830B30, &TextureLoadTaskClass::Hook_Load_Compressed_Mipmap);
    Hook_Method(0x00830C40, &TextureLoadTaskClass::Hook_Load_Uncompressed_Mipmap);
    Hook_Method(0x00830A80, &TextureLoadTaskClass::Hook_Lock_Surfaces);
    Hook_Method(0x00830AE0, &TextureLoadTaskClass::Hook_Unlock_Surfaces);
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
    Hook_Method(0x0051CD70, &Script::Hook_Xfer_Snapshot);
    Hook_Method(0x0051CDB0, &Script::Duplicate);
    Hook_Method(0x0051CFC0, &Script::Duplicate_And_Qualify);
    Hook_Function(0x0051D610, Script::Parse_Script_From_Group_Chunk);
    Hook_Function(0x0051D5B0, Script::Parse_Script_From_List_Chunk);

    // scriptgroup.h ScriptGroup
    Hook_Method(0x0051C3B0, &ScriptGroup::Hook_Xfer_Snapshot);
    Hook_Method(0x0051C510, &ScriptGroup::Duplicate);
    Hook_Method(0x0051C670, &ScriptGroup::Duplicate_And_Qualify);
    Hook_Function(0x0051C860, ScriptGroup::Parse_Group_Chunk);
    
    // scriptlist.h ScriptList
    Hook_Method(0x0051B920, &ScriptList::Hook_Xfer_Snapshot);
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
    Hook_Method(0x0040A120, &Keyboard::Hook_Init);
    Hook_Method(0x0040A140, &Keyboard::Hook_Update);
    Hook_Method(0x00407F50, &Keyboard::Hook_Create_Message_Stream);
    Hook_Method(0x0040A4A0, &Keyboard::Get_Printable_Key);
    Hook_Method(0x0040A150, &Keyboard::Reset_Keys);
    Hook_Method(0x0040A460, &Keyboard::Is_Shift);
    Hook_Method(0x0040A480, &Keyboard::Is_Ctrl);
    Hook_Method(0x0040A490, &Keyboard::Is_Alt);

    // mouse.h Mouse
    Hook_Method(0x004031F0, &Mouse::Hook_Create_Stream_Messages);
    Hook_Method(0x004024E0, &Mouse::Process_Mouse_Event);

    // geometry.h
    Hook_Method(0x005CFAF0, &GeometryInfo::Hook_Xfer);
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
    // Hook_Method(0x00489410, &RankInfoStore::Hook_Init); // Works, but shares function in binary with ScienceStore as
    // compiles to same code.
    Hook_Method(0x00489440, &RankInfoStore::Hook_Reset);
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
    Hook_Method(0x004CD5D0, &ParticleSystemInfo::Hook_Xfer);
    Hook_Method(0x004CD540, &ParticleSystemInfo::Tint_All_Colors);

    // particle.h
    Hook_Method(0x004CCC30, &Particle::Hook_Ctor);
    Hook_Method(0x004CD040, &Particle::Hook_Dtor);
    Hook_Method(0x004CD2E0, &Particle::Hook_Xfer);
    Hook_Method(0x004CD3F0, &Particle::Hook_LoadPP);
    Hook_Method(0x004CCB50, &Particle::Compute_Alpha_Rate);
    Hook_Method(0x004CCBA0, &Particle::Compute_Color_Rate);

    // particlesys.h
    Hook_Method(0x004CDA10, &ParticleSystem::Hook_Ctor);
    Hook_Method(0x004CE310, &ParticleSystem::Hook_Dtor);
    Hook_Method(0x004D11E0, &ParticleSystem::Hook_Xfer);
    Hook_Method(0x004D1430, &ParticleSystem::Hook_LoadPP);
    Hook_Method(0x004CE500, &ParticleSystem::Destroy);
    Hook_Method(0x004CE530, &ParticleSystem::Get_Position);
    Hook_Method(0x004CE570, &ParticleSystem::Set_Position);
    Hook_Method(0x004CE5A0, &ParticleSystem::Set_Local_Transform);
    Hook_Method(0x004CE620, &ParticleSystem::Rotate_Local_Transform_X);
    Hook_Method(0x004CE6C0, &ParticleSystem::Rotate_Local_Transform_Y);
    Hook_Method(0x004CE760, &ParticleSystem::Rotate_Local_Transform_Z);
    Hook_Method(0x004CE860, &ParticleSystem::Compute_Particle_Velocity);
    Hook_Method(0x004CEF60, &ParticleSystem::Compute_Particle_Position);
    Hook_Method(0x004CF530, &ParticleSystem::Hook_Create_Particle);
    Hook_Method(0x004CF750, &ParticleSystem::Generate_Particle_Info);
    Hook_Method(0x004D0920, &ParticleSystem::Update_Wind_Motion);
    Hook_Function(0x004D0B30, &ParticleSystem::Merge_Related_Systems);

    // particlesysmanager.h
    Hook_Method(0x004D1790, &ParticleSystemManager::Hook_Ctor);
    Hook_Method(0x004D18E0, &ParticleSystemManager::Hook_Dtor);
    Hook_Method(0x004D1BA0, &ParticleSystemManager::Hook_Init);
    Hook_Method(0x004D1C40, &ParticleSystemManager::Hook_Reset);
    Hook_Method(0x004D2460, &ParticleSystemManager::Hook_Xfer);
    Hook_Method(0x004D1EB0, &ParticleSystemManager::Find_Template);
    Hook_Method(0x004D1EE0, &ParticleSystemManager::New_Template);
    Hook_Method(0x004D2130, &ParticleSystemManager::Find_Parent_Template);
    Hook_Method(0x004D1D40, &ParticleSystemManager::Create_Particle_System);
    Hook_Method(0x004D1E30, &ParticleSystemManager::Find_Particle_System);
    Hook_Method(0x004D1E60, &ParticleSystemManager::Destroy_Particle_System_By_ID);
    Hook_Method(0x004D22D0, &ParticleSystemManager::Remove_Particle);

#ifdef BUILD_WITH_BINK
    // binkvideostream.h
    Hook_Method(0x007AAA20, &BinkVideoStream::Hook_Update);
    Hook_Method(0x007AAA30, &BinkVideoStream::Hook_Is_Frame_Ready);
    Hook_Method(0x007AAA40, &BinkVideoStream::Hook_Decompress_Frame);
    Hook_Method(0x007AAA50, &BinkVideoStream::Hook_Render_Frame);
    Hook_Method(0x007AAAD0, &BinkVideoStream::Hook_Next_Frame);
    Hook_Method(0x007AAB00, &BinkVideoStream::Hook_Goto_Frame);

    // binkvideoplayer.h
    Hook_Method(0x007AA550, &BinkVideoPlayer::Hook_Init);
    Hook_Method(0x007AA570, &BinkVideoPlayer::Hook_Deinit);
    Hook_Method(0x007AA6A0, &BinkVideoPlayer::Hook_Open);
    Hook_Method(0x007AA8E0, &BinkVideoPlayer::Hook_Load);
    Hook_Method(0x007AA970, &BinkVideoPlayer::Hook_Notify_Player_Of_New_Provider);
    Hook_Method(0x007AA9A0, &BinkVideoPlayer::Hook_Initialise_Bink_With_Miles);
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
    Hook_Method(0x00445FF0, &SoundManager::Hook_Reset);
    Hook_Method(0x00446010, &SoundManager::Hook_Add_Event);
    // Hook_Method(0x00446120, &SoundManager::Hook_Can_Play_Now); // Not implemented.
    Hook_Method(0x004462F0, &SoundManager::Hook_Violates_Voice);
    Hook_Method(0x00446350, &SoundManager::Hook_Is_Interrupting);

    /* These are all hooked via Win32Engine::Create_Audio_Manager.
    // audiomanager.h AudioManager
    Hook_Method(0x00404C60, &AudioManager::Hook_Init);
    Hook_Method(0x00404F30, &AudioManager::Hook_Reset);
    // Hook_Method(0x00404F30, &AudioManager::Hook_Update); // Not Implemented.
    Hook_Method(0x00406050, &AudioManager::Hook_Set_Listener_Position);
    Hook_Method(0x00406090, &AudioManager::Hook_Allocate_Audio_Request);
    Hook_Method(0x004061A0, &AudioManager::Hook_Release_Audio_Request);
    Hook_Method(0x004061D0, &AudioManager::Hook_Append_Audio_Request);
    Hook_Method(0x00406250, &AudioManager::Hook_New_Audio_Event_Info);
    Hook_Method(0x004065D0, &AudioManager::Hook_Add_Audio_Event_Info);
    Hook_Method(0x004067B0, &AudioManager::Hook_Find_Audio_Event_Info);
    Hook_Method(0x00406920, &AudioManager::Hook_Refresh_Cached_Variables);
    Hook_Method(0x00406970, &AudioManager::Hook_Get_Audio_Length_MS);
    Hook_Method(0x00406A90, &AudioManager::Hook_Is_Music_Already_Loaded);
    Hook_Method(0x00406C10, &AudioManager::Hook_Find_All_Audio_Events_Of_Type);
    Hook_Method(0x005B9460, &AudioManager::Hook_Get_All_Audio_Events);
    Hook_Method(0x00406D00, &AudioManager::Hook_Is_Current_Provider_Hardware_Accelerated);
    Hook_Method(0x00406DE0, &AudioManager::Hook_Is_Current_Speaker_Type_Surround);
    // Hook_Method(0x00406E00, &AudioManager::Hook_Should_Play_Locally); // Not Implemented.
    Hook_Method(0x00406F00, &AudioManager::Hook_Allocate_New_Handle);
    Hook_Method(0x00406860, &AudioManager::Hook_Remove_Level_Specific_Audio_Event_Infos);
    Hook_Method(0x00406200, &AudioManager::Remove_All_Audio_Requests);
    Hook_Method(0x00405700, &AudioManager::Next_Track_Name);
    Hook_Method(0x004057D0, &AudioManager::Prev_Track_Name);
    Hook_Method(0x00406F10, &AudioManager::Hook_Lose_Focus);
    Hook_Method(0x00406F70, &AudioManager::Hook_Regain_Focus);
    Hook_Method(0x00405390, &AudioManager::Hook_Add_Audio_Event);
    Hook_Method(0x004058A0, &AudioManager::Hook_Remove_Audio_Event);
    Hook_Method(0x00405C30, &AudioManager::Hook_Remove_Audio_Event_String);
    Hook_Method(0x00405680, &AudioManager::Hook_Is_Valid_Const_Audio_Event);
    Hook_Method(0x004058F0, &AudioManager::Hook_Set_Audio_Event_Enabled);
    Hook_Method(0x00405990, &AudioManager::Hook_Set_Audio_Event_Volume_Override);
    Hook_Method(0x00405CC0, &AudioManager::Hook_Remove_Disabled_Events);
    Hook_Method(0x00405340, &AudioManager::Hook_Get_Info_For_Audio_Event);
    Hook_Method(0x00405CD0, &AudioManager::Hook_Translate_From_Speaker_Type);
    Hook_Method(0x00405DD0, &AudioManager::Hook_Translate_To_Speaker_Type);
    Hook_Method(0x00405E50, &AudioManager::Hook_Is_On);
    Hook_Method(0x00405E90, &AudioManager::Hook_Set_On);
    Hook_Method(0x00405F20, &AudioManager::Hook_Set_Volume);
    Hook_Method(0x00405FC0, &AudioManager::Hook_Get_Volume);
    Hook_Method(0x00405FF0, &AudioManager::Hook_Set_3D_Volume_Adjustment);
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
    Hook_Method(0x0077CCE0, &MilesAudioManager::Hook_Init);
    Hook_Method(0x0077CD30, &MilesAudioManager::Hook_Reset);
    Hook_Method(0x0077CD60, &MilesAudioManager::Hook_Update);
    Hook_Method(0x0077CDA0, &MilesAudioManager::Hook_Stop_Audio);
    Hook_Method(0x0077CEB0, &MilesAudioManager::Hook_Pause_Audio);
    Hook_Method(0x0077CFC0, &MilesAudioManager::Hook_Resume_Audio);
    Hook_Method(0x0077D780, &MilesAudioManager::Hook_Kill_Event_Immediately);
    Hook_Method(0x0077DE80, &MilesAudioManager::Hook_Next_Music_Track);
    Hook_Method(0x0077E020, &MilesAudioManager::Hook_Prev_Music_Track);
    Hook_Method(0x0077E1C0, &MilesAudioManager::Hook_Is_Music_Playing);
    Hook_Method(0x0077E200, &MilesAudioManager::Hook_Has_Music_Track_Completed);
    Hook_Method(0x0077E2D0, &MilesAudioManager::Hook_Music_Track_Name);
    Hook_Method(0x0077E5F0, &MilesAudioManager::Hook_Is_Currently_Playing);
    Hook_Method(0x0077E3B0, &MilesAudioManager::Hook_Open_Device);
    Hook_Method(0x0077E5D0, &MilesAudioManager::Hook_Close_Device);
    Hook_Method(0x0077E6C0, &MilesAudioManager::Hook_Notify_Of_Audio_Completion);
    Hook_Method(0x0077E900, &MilesAudioManager::Hook_Get_Provider_Name);
    Hook_Method(0x0077E970, &MilesAudioManager::Hook_Get_Provider_Index);
    Hook_Method(0x0077EA60, &MilesAudioManager::Hook_Select_Provider);
    Hook_Method(0x0077ECB0, &MilesAudioManager::Hook_Unselect_Provider);
    Hook_Method(0x0077ED40, &MilesAudioManager::Hook_Set_Speaker_Type);
    Hook_Method(0x0077ED80, &MilesAudioManager::Hook_Get_Speaker_Type);
    Hook_Method(0x0077EDD0, &MilesAudioManager::Hook_Does_Violate_Limit);
    Hook_Method(0x0077F2A0, &MilesAudioManager::Hook_Is_Playing_Lower_Priority);
    Hook_Method(0x0077F020, &MilesAudioManager::Hook_Is_Playing_Already);
    Hook_Method(0x0077F130, &MilesAudioManager::Hook_Is_Object_Playing_Voice);
    Hook_Method(0x0077F400, &MilesAudioManager::Hook_Adjust_Volume_Of_Playing_Audio);
    Hook_Method(0x0077F6C0, &MilesAudioManager::Hook_Remove_Playing_Audio);
    Hook_Method(0x0077F8F0, &MilesAudioManager::Hook_Remove_All_Disabled_Audio);
    Hook_Method(0x0077FD80, &MilesAudioManager::Hook_Has_3D_Sensitive_Streams_Playing);
    Hook_Method(0x00780820, &MilesAudioManager::Hook_Get_Bink_Handle);
    Hook_Method(0x007809C0, &MilesAudioManager::Hook_Release_Bink_Handle);
    Hook_Method(0x007809E0, &MilesAudioManager::Hook_Force_Play_Audio_Event);
    Hook_Method(0x0077FA00, &MilesAudioManager::Hook_Process_Request_List);
    Hook_Method(0x0077FF30, &MilesAudioManager::Hook_Set_Hardware_Accelerated);
    Hook_Method(0x00780020, &MilesAudioManager::Hook_Set_Speaker_Surround);
    Hook_Method(0x0077C970, &MilesAudioManager::Hook_Set_Preferred_3D_Provider);
    Hook_Method(0x0077CA10, &MilesAudioManager::Hook_Set_Preferred_Speaker);
    Hook_Method(0x00780090, &MilesAudioManager::Hook_Get_Audio_Length_MS);
    Hook_Method(0x00780190, &MilesAudioManager::Hook_Close_Any_Sample_Using_File);
    Hook_Method(0x00780230, &MilesAudioManager::Hook_Set_Device_Listener_Position);
    Hook_Method(0x0077E860, &MilesAudioManager::Hook_Find_Playing_Audio_From);
    Hook_Method(0x0077FB10, &MilesAudioManager::Hook_Process_Playing_List);
    Hook_Method(0x0077FDE0, &MilesAudioManager::Hook_Process_Fading_List);
    Hook_Method(0x0077FEE0, &MilesAudioManager::Hook_Process_Stopped_List);
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
    Hook_Method(0x008094E0, &Render2DClass::Reset);
    Hook_Method(0x008090C0, &Render2DClass::Hook_Ctor);

    // wwstring.h
    Hook_Method(0x0089D4E0, &StringClass::Format);

    // cpudetect.h
    // This nulls out CPUDetect::Init_Compact_Log in the original binary.
    // This is a fix for an odd crash in the windows CRT related to vsnprintf.
    Hook_Function(0x0089FA20, Null_Func);

#endif
    // dx8fvf.h
    Hook_Method(0x00520160, &FVFInfoClass::Hook_Ctor);

    // dx8vertexbuffer.h
    Hook_Method(0x004E7960, &VertexBufferClass::Release_Engine_Ref);
    Hook_Method(0x004E79A0, &VertexBufferClass::WriteLockClass::Hook_Ctor);
    Hook_Method(0x004E7AB0, &VertexBufferClass::WriteLockClass::Hook_Dtor);
    Hook_Method(0x004E7B30, &VertexBufferClass::AppendLockClass::Hook_Ctor);
    Hook_Method(0x004E7CB0, &VertexBufferClass::AppendLockClass::Hook_Dtor);
    Hook_Method(0x004E7D30, &SortingVertexBufferClass::Hook_Ctor);
    Hook_Method(0x004E7E60, &DX8VertexBufferClass::Hook_Ctor);
    Hook_Method(0x004E95D0, &DynamicVBAccessClass::Hook_Ctor);
    Hook_Method(0x004E96A0, &DynamicVBAccessClass::Hook_Dtor);
    Hook_Method(0x004E96F0, &DynamicVBAccessClass::Deinit);
    Hook_Method(0x004E9C60, &DynamicVBAccessClass::WriteLockClass::Hook_Ctor);
    Hook_Method(0x004E9DA0, &DynamicVBAccessClass::WriteLockClass::Hook_Dtor);
    Hook_Method(0x004E9E10, &DynamicVBAccessClass::Reset);
    Hook_Method(0x004E9E30, &DynamicVBAccessClass::Get_Default_Vertex_Count);

    // dx8indexbuffer.h
    Hook_Method(0x004E0C10, &IndexBufferClass::Add_Engine_Ref);
    Hook_Method(0x004E0C20, &IndexBufferClass::Release_Engine_Ref);
    Hook_Method(0x004E0DE0, &IndexBufferClass::WriteLockClass::Hook_Ctor);
    Hook_Method(0x004E0F10, &IndexBufferClass::WriteLockClass::Hook_Dtor);
    Hook_Method(0x004E0F90, &IndexBufferClass::AppendLockClass::Hook_Ctor);
    Hook_Method(0x004E1100, &IndexBufferClass::AppendLockClass::Hook_Dtor);
    Hook_Method(0x004E1180, &DX8IndexBufferClass::Hook_Ctor);
    Hook_Method(0x004E13B0, &SortingIndexBufferClass::Hook_Ctor);
    Hook_Method(0x004E1510, &DynamicIBAccessClass::Hook_Ctor);
    Hook_Method(0x004E1640, &DynamicIBAccessClass::Hook_Dtor);
    Hook_Method(0x004E16D0, &DynamicIBAccessClass::Deinit);
    Hook_Method(0x004E17C0, &DynamicIBAccessClass::WriteLockClass::Hook_Ctor);
    Hook_Method(0x004E18E0, &DynamicIBAccessClass::WriteLockClass::Hook_Dtor);
    Hook_Method(0x004E1D70, &DynamicIBAccessClass::Reset);
    Hook_Method(0x004E1D90, &DynamicIBAccessClass::Get_Default_Index_Count);
    Hook_Method(0x004E1DA0, &DynamicIBAccessClass::Get_Next_Index);

    // dx8wrapper.h
    Hook_Method(0x004F96D0, &DX8Wrapper::Init);
    Hook_Method(0x004F98D0, &DX8Wrapper::Shutdown);
    Hook_Method(0x004F9BF0, &DX8Wrapper::Invalidate_Cached_Render_States);
    Hook_Method(0x004FA080, &DX8Wrapper::Reset_Device);
    Hook_Method(0x004FAF40, &DX8Wrapper::Set_Render_Device);
    Hook_Method(0x004FB5C0, &DX8Wrapper::Has_Stencil);
    Hook_Method(0x004FB5F0, &DX8Wrapper::Get_Render_Device);
    Hook_Method(0x004FB600, &DX8Wrapper::Get_Render_Device_Desc);
    Hook_Method(0x004FB7B0, &DX8Wrapper::Set_Device_Resolution);
    Hook_Method(0x004FB8E0, &DX8Wrapper::Get_Device_Resolution);
    Hook_Method(0x004FB960, &DX8Wrapper::Get_Render_Target_Resolution);
    Hook_Method(0x004FC530, &DX8Wrapper::Begin_Scene);
    Hook_Method(0x004FC550, &DX8Wrapper::End_Scene);
    Hook_Method(0x004FC800, &DX8Wrapper::Clear);
    Hook_Method(0x004FC950, &DX8Wrapper::Set_Viewport);
    Hook_Method(0x004FC970, static_cast<void (*)(const VertexBufferClass *, int)>(&DX8Wrapper::Set_Vertex_Buffer));
    Hook_Method(0x004FCA10, static_cast<void (*)(const IndexBufferClass *, unsigned short)>(&DX8Wrapper::Set_Index_Buffer));
    Hook_Method(0x004FCAB0, static_cast<void (*)(const DynamicVBAccessClass &)>(&DX8Wrapper::Set_Vertex_Buffer));
    Hook_Method(
        0x004FCB40, static_cast<void (*)(const DynamicIBAccessClass &, unsigned short)>(&DX8Wrapper::Set_Index_Buffer));
    Hook_Method(0x004FD3D0,
        static_cast<void (*)(unsigned int, unsigned short, unsigned short, unsigned short, unsigned short)>(
            &DX8Wrapper::Draw_Triangles));
    Hook_Method(0x004FD420,
        static_cast<void (*)(unsigned short, unsigned short, unsigned short, unsigned short)>(&DX8Wrapper::Draw_Triangles));
    Hook_Method(0x004FD440, &DX8Wrapper::Draw_Strip);
    Hook_Method(0x004FD460, &DX8Wrapper::Apply_Render_State_Changes);
    Hook_Method(0x004FDF00,
        static_cast<w3dtexture_t (*)(unsigned int, unsigned int, WW3DFormat, MipCountType, w3dpool_t, bool)>(
            &DX8Wrapper::Create_Texture));
    Hook_Method(0x004FE200, static_cast<w3dtexture_t (*)(w3dsurface_t, MipCountType)>(&DX8Wrapper::Create_Texture));
    Hook_Method(
        0x004FE820, static_cast<w3dsurface_t (*)(unsigned int, unsigned int, WW3DFormat)>(&DX8Wrapper::Create_Surface));
    Hook_Method(0x004FE8B0, static_cast<w3dsurface_t (*)(const char *)>(&DX8Wrapper::Create_Surface));
    Hook_Method(0x004FECD0, static_cast<void (*)(unsigned int, const D3DLIGHT8 *)>(&DX8Wrapper::Set_Light));
    Hook_Method(0x004FF1E0, &DX8Wrapper::Get_DX8_Front_Buffer);
    Hook_Method(0x004FF250, &DX8Wrapper::Get_DX8_Back_Buffer);
    Hook_Method(0x004FF310, &DX8Wrapper::Create_Render_Target);
    Hook_Method(0x004FF810, &DX8Wrapper::Set_Render_Target_With_Z);
    Hook_Method(0x004FF9A0, &DX8Wrapper::Set_Render_Target);
    Hook_Method(0x004FFE50, &DX8Wrapper::Set_Gamma);
    Hook_Method(0x005004A0, &DX8Wrapper::Get_DX8_Render_State_Value_Name);
    Hook_Method(0x00500860, &DX8Wrapper::Get_DX8_Texture_Stage_State_Value_Name);
    Hook_Method(0x00501B10, &DX8Wrapper::Get_Back_Buffer_Format);

    // w3dbuffermanager.h
    Hook_Any(0x0069F09F, W3DBufferManager::Hook_Ctor);
    Hook_Any(0x0069F195, W3DBufferManager::Hook_Dtor);
    Hook_Any(0x0069F1B0, W3DBufferManager::Free_All_Slots);
    Hook_Any(0x0069F3C1, W3DBufferManager::Free_All_Buffers);
    Hook_Any(0x0069F5D6, W3DBufferManager::Release_Resources);
    Hook_Any(0x0069F67D, W3DBufferManager::ReAcquire_Resources);
    Hook_Any(0x0069F8F6, W3DBufferManager::Hook_VB_Get_Slot);
    Hook_Any(0x0069F9C7, W3DBufferManager::Hook_VB_Release_Slot);
    Hook_Any(0x0069FA4E, W3DBufferManager::Hook_VB_Allocate_Slot_Storage);
    Hook_Any(0x0069FD96, W3DBufferManager::Hook_IB_Get_Slot);
    Hook_Any(0x0069FE5B, W3DBufferManager::Hook_IB_Release_Slot);
    Hook_Any(0x0069FEBE, W3DBufferManager::Hook_IB_Allocate_Slot_Storage);

    // for messing with buffer sizes
    int *w3dbm_size = (int *)(0x0067B205 + 1);
    *w3dbm_size = sizeof(W3DBufferManager);

    // experiencetracker.cpp
    Hook_Any(0x008A35F9, ExperienceTracker::Get_Experience_Value);
    Hook_Any(0x008A3637, ExperienceTracker::Is_Trainable);
    Hook_Any(0x008A3654, ExperienceTracker::Is_Accepting_Experience_Points);
    Hook_Any(0x008A368E, ExperienceTracker::Set_Experience_Sink);
    Hook_Any(0x008A36A4, ExperienceTracker::Set_Min_Veterency_Level);
    Hook_Any(0x008A370D, ExperienceTracker::Set_Veterency_Level);
    Hook_Any(0x008A377A, ExperienceTracker::Gain_Exp_For_Level);
    Hook_Any(0x008A37E4, ExperienceTracker::Can_Gain_Exp_For_Level);
    Hook_Any(0x008A381C, ExperienceTracker::Add_Experience_Points);
    Hook_Any(0x008A3934, ExperienceTracker::Set_Experience_And_Level);

    // science.h
    Hook_Any(0x0072727E, ScienceStore::Get_Science_From_Internal_Name);
    Hook_Any(0x007272BE, ScienceStore::Get_Internal_Name_From_Science);
    Hook_Any(0x00727753, ScienceStore::Get_Science_Purchase_Cost);
    Hook_Any(0x00727781, ScienceStore::Is_Science_Grantable);
    Hook_Any(0x007277AF, ScienceStore::Get_Name_And_Description);
}
