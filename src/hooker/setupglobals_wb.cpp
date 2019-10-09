#include "hooker.h"
#include "view.h"

// editmain.cpp done
HWND &g_applicationHWnd = Make_Global<HWND>(0x00DE80AC);

// maputil.cpp done
class WaypointMap;
class MapCache;
WaypointMap *&g_waypoints = Make_Global<WaypointMap *>(0x04CAA140);
MapCache *&g_theMapCache = Make_Global<MapCache *>(0x04CAA154);

// weather.cpp done
class WeatherSetting;
WeatherSetting *&g_theWeatherSetting = Make_Global<WeatherSetting *>(0x00E25CF0);

// displaystringmanager.cpp
class DisplayStringManager;
DisplayStringManager *&g_theDisplayStringManger = Make_Global<DisplayStringManager *>(0x00A2A6C4);

// gameclient.cpp
class GameClient;
GameClient *&g_theGameClient = Make_Global<GameClient *>(0x00A2BBF8);

// gametext.cpp done
class GameTextInterface;
GameTextInterface *&g_theGameText = Make_Global<GameTextInterface *>(0x00E23840);

// globallanguage.cpp done
class GlobalLanguage;
GlobalLanguage *&g_theGlobalLanguage = Make_Global<GlobalLanguage *>(0x00E25D14);

// terrainroads.cpp done
class TerrainRoadCollection;
TerrainRoadCollection *&g_theTerrainRoads = Make_Global<TerrainRoadCollection *>(0x00E23A34);

// view.cpp
uint32_t &View::s_idNext = Make_Global<uint32_t>(0); // not found yet

// water.cpp
class WaterSetting;
class WaterTransparencySetting;
WaterSetting *g_waterSettings = Make_Pointer<WaterSetting>(0x00A2F0B8);
WaterTransparencySetting *&g_theWaterTransparency = Make_Global<WaterTransparencySetting *>(0x00A2F0B0);

// keyboard.cpp done
class Keyboard;
Keyboard *&g_theKeyboard = Make_Global<Keyboard *>(0); // Doesn't appear to exist

// mouse.cpp done
class Mouse;
Mouse *&g_theMouse = Make_Global<Mouse *>(0); // Doesn't appear to exist

// particlesysmanager.cpp done
class ParticleSystemManager;
ParticleSystemManager *&g_theParticleSystemManager = Make_Global<ParticleSystemManager *>(0x00E2414C);

// videoplayer.cpp done
class VideoPlayer;
VideoPlayer *&g_theVideoPlayer = Make_Global<VideoPlayer *>(0x00E239B8);

// commandlist.cpp done
class CommandList;
CommandList *&g_theCommandList = Make_Global<CommandList *>(0x04CA8760);

// gameengine.cpp done
class GameEngine;
GameEngine *&g_theGameEngine = Make_Global<GameEngine *>(0x00E268C0);

// gamelod.cpp done
class GameLODManager;
GameLODManager *&g_theGameLODManager = Make_Global<GameLODManager *>(0x00E26070);

// globaldata.cpp done
class GlobalData;
GlobalData *&g_theWriteableGlobalData = Make_Global<GlobalData *>(0x00E1D550);

// messagestream.cpp done
class MessageStream;
MessageStream *&g_theMessageStream = Make_Global<MessageStream *>(0x04CA875C);

// multiplayersettings.cpp done
class MultiplayerSettings;
MultiplayerSettings *&g_theMultiplayerSettings = Make_Global<MultiplayerSettings *>(0x00E23A2C);

// namekeygenerator.cpp done
class NameKeyGenerator;
NameKeyGenerator *&g_theNameKeyGenerator = Make_Global<NameKeyGenerator *>(0x00E1D490);

// staticnamekey.cpp
#include "staticnamekey.h"
StaticNameKey &g_teamNameKey = Make_Global<StaticNameKey>(0x00A3A830);
StaticNameKey &g_theInitialCameraPositionKey = Make_Global<StaticNameKey>(0x00A3A880);
StaticNameKey &g_playerNameKey = Make_Global<StaticNameKey>(0x00A32580);
StaticNameKey &g_playerIsHumanKey = Make_Global<StaticNameKey>(0x00A3A8F0);
StaticNameKey &g_playerDisplayNameKey = Make_Global<StaticNameKey>(0x00A32588);
StaticNameKey &g_playerFactionKey = Make_Global<StaticNameKey>(0x00A326B8);
StaticNameKey &g_playerAlliesKey = Make_Global<StaticNameKey>(0x00A36758);
StaticNameKey &g_playerEnemiesKey = Make_Global<StaticNameKey>(0x00A32668);
StaticNameKey &g_teamOwnerKey = Make_Global<StaticNameKey>(0x00A32568);
StaticNameKey &g_teamIsSingletonKey = Make_Global<StaticNameKey>(0x00A3A948);

// terraintypes.cpp done
class TerrainTypeCollection;
TerrainTypeCollection *&g_theTerrainTypes = Make_Global<TerrainTypeCollection *>(0x00E1D4B8);

// version.cpp done
class Version;
Version *&g_theVersion = Make_Global<Version *>(0x00E27F84);

// audiomanager.cpp done
class AudioManager;
AudioManager *&g_theAudio = Make_Global<AudioManager *>(0x00E1D62C);

// ini.cpp done
class Xfer;
Xfer *&g_sXfer = Make_Global<Xfer *>(0x00E2557C);

// modulefactory.cpp done
class ModuleFactory;
ModuleFactory *&g_theModuleFactory = Make_Global<ModuleFactory *>(0x00E241F4);

// playerlist.cpp done
class PlayerList;
PlayerList *&g_thePlayerList = Make_Global<PlayerList *>(0x00E25D2C);

// playertemplate.cpp done
class PlayerTemplateStore;
PlayerTemplateStore *&g_thePlayerTemplateStore = Make_Global<PlayerTemplateStore *>(0x00E1D484);

// science.cpp done
class ScienceStore;
ScienceStore *&g_theScienceStore = Make_Global<ScienceStore *>(0x00E23868);

// sideslist.cpp done
class SidesList;
SidesList *&g_theSidesList = Make_Global<SidesList *>(0x00E1B3D8);

// archivefilesystem.cpp done
class ArchiveFileSystem;
ArchiveFileSystem *&g_theArchiveFileSystem = Make_Global<ArchiveFileSystem *>(0x00E255B0);

// filesystem.cpp done
class FileSystem;
FileSystem *&g_theFileSystem = Make_Global<FileSystem *>(0x00E239C0);

// functionlexicon.cpp done
class FunctionLexicon;
FunctionLexicon *&g_theFunctionLexicon = Make_Global<FunctionLexicon *>(0x04CAB9A8);

// gamememory.cpp done
bool &g_thePreMainInitFlag = Make_Global<bool>(0x00E1B340);
bool &g_theMainInitFlag = Make_Global<bool>(0x00E1B341);

// gamestate.cpp done
class GameState;
GameState *&g_theGameState = Make_Global<GameState *>(0x00E26058);

// localfilesystem.cpp done
class LocalFileSystem;
LocalFileSystem *&g_theLocalFileSystem = Make_Global<LocalFileSystem *>(0x00E255BC);

// memdynalloc.cpp done
class SimpleCriticalSectionClass;
class DynamicMemoryAllocator;
SimpleCriticalSectionClass *&g_dmaCriticalSection = Make_Global<SimpleCriticalSectionClass *>(0x00E27F40);
DynamicMemoryAllocator *&g_dynamicMemoryAllocator = Make_Global<DynamicMemoryAllocator *>(0x00E1B348);

// mempool.cpp done
SimpleCriticalSectionClass *&g_memoryPoolCriticalSection = Make_Global<SimpleCriticalSectionClass *>(0x00E27F44);

// mempoolfact.cpp done
class MemoryPoolFactory;
MemoryPoolFactory *&g_memoryPoolFactory = Make_Global<MemoryPoolFactory *>(0x00E1B344);

// stackdump.cpp done
Utf8String &g_exceptionFileBuffer = Make_Global<Utf8String>(0x00E259C8);

// subsysteminterface.cpp done
SubsystemInterfaceList *&g_theSubsystemList = Make_Global<SubsystemInterfaceList *>(0x00E268C4);

// unicodestring.cpp done
SimpleCriticalSectionClass *&g_unicodeStringCriticalSection = Make_Global<SimpleCriticalSectionClass *>(0x00E27F3C);

// cavesystem.cpp done
class CaveSystem;
CaveSystem *&g_theCaveSystem = Make_Global<CaveSystem *>(0x00E241D8);

// rankinfo.cpp done
class RankInfoStore;
RankInfoStore *&g_theRankInfoStore = Make_Global<RankInfoStore *>(0x00E241B4);

// lanapiinterface.cpp
class LANAPIInterface;
LANAPIInterface *&g_theLAN = Make_Global<LANAPIInterface *>(0); // might not exist, probably not needed in editor

// ffactory.cpp done
class RawFileFactoryClass;
class FileFactoryClass;
RawFileFactoryClass *&g_theWritingFileFactory = Make_Global<RawFileFactoryClass *>(0x00CD4700);
FileFactoryClass *&g_theFileFactory = Make_Global<FileFactoryClass *>(0x00CD46F8);

// dx8caps.cpp done
#include "dx8caps.h"
StringClass &DX8Caps::s_videoCardDetails = Make_Global<StringClass>(0x00DF0F90);

// multilist.cpp done
#include "multilist.h"
HOOK_AUTOPOOL(MultiListNodeClass, 256, 0x00E0BB30);

// dx8wrapper.cpp
#include "dx8wrapper.h"
uint32_t &g_numberOfDx8Calls = Make_Global<uint32_t>(0x00DEE778); // 
IDirect3D8 *(__stdcall *&DX8Wrapper::s_d3dCreateFunction)(unsigned) = Make_Global<IDirect3D8 *(__stdcall *)(unsigned)>(
    0x00DEE7A4); //
HMODULE &DX8Wrapper::s_d3dLib = Make_Global<HMODULE>(0x00DEE7A8); //
IDirect3D8 *&DX8Wrapper::s_d3dInterface = Make_Global<IDirect3D8 *>(0x00DEE724); //
IDirect3DDevice8 *&DX8Wrapper::s_d3dDevice = Make_Global<IDirect3DDevice8 *>(0x00DEE728); //
ARRAY_DEF(0x00DE8FBC, w3dbasetexture_t, DX8Wrapper::s_textures, MAX_TEXTURE_STAGES); //
void *&DX8Wrapper::s_shadowMap = Make_Global<void *>(0x00DEE6F4); //
void *&DX8Wrapper::s_hwnd = Make_Global<void *>(0x00DEE6FC); //
ARRAY_DEF(0x00DED4F8, unsigned, DX8Wrapper::s_renderStates, 256); //
ARRAY2D_DEF(0x00DED0B8, unsigned, DX8Wrapper::s_textureStageStates, MAX_TEXTURE_STAGES, 32); //
ARRAY_DEF(0x00DEDFB0, Vector4, DX8Wrapper::s_vertexShaderConstants, 96); //
ARRAY_DEF(0x00DE8FF8, unsigned, DX8Wrapper::s_pixelShaderConstants, 32); //
bool &DX8Wrapper::s_isInitialised = Make_Global<bool>(0x00DEE700); //
bool &DX8Wrapper::s_isWindowed = Make_Global<bool>(0x00DEE701); //
bool &DX8Wrapper::s_debugIsWindowed = Make_Global<bool>(0x00CC36B8);
RenderStateStruct &DX8Wrapper::s_renderState = Make_Global<RenderStateStruct>(0x00DEDD40); //
unsigned &DX8Wrapper::s_renderStateChanged = Make_Global<unsigned>(0x00DE8FB0); //
float &DX8Wrapper::s_zNear = Make_Global<float>(0x00A47E38);
float &DX8Wrapper::s_zFar = Make_Global<float>(0x00A47EB8);
Matrix4 &DX8Wrapper::s_projectionMatrix = Make_Global<Matrix4>(0x00A47DF8);
int &DX8Wrapper::s_mainThreadID = Make_Global<int>(0x00DEE764); //
int &DX8Wrapper::s_currentRenderDevice = Make_Global<int>(0x00CC36BC); //
DX8Caps *&DX8Wrapper::s_currentCaps = Make_Global<DX8Caps *>(0x00A47F30);
int &DX8Wrapper::s_resolutionWidth = Make_Global<int>(0x00CC36C0); //
int &DX8Wrapper::s_resolutionHeight = Make_Global<int>(0x00CC36C4); //
int &DX8Wrapper::s_bitDepth = Make_Global<int>(0x00CC36C8); //
int &DX8Wrapper::s_textureBitDepth = Make_Global<int>(0x00CC36CC); //
ARRAY_DEF(0x00DE8FB4, bool, DX8Wrapper::s_currentLightEnables, 4); //
unsigned &DX8Wrapper::s_matrixChanges = Make_Global<unsigned>(0x00DEE740); ////
unsigned &DX8Wrapper::s_materialChanges = Make_Global<unsigned>(0x00DEE744);
unsigned &DX8Wrapper::s_vertexBufferChanges = Make_Global<unsigned>(0x00DEE748);
unsigned &DX8Wrapper::s_indexBufferChanges = Make_Global<unsigned>(0x00DEE74C);
unsigned &DX8Wrapper::s_lightChanges = Make_Global<unsigned>(0x00DEE750);
unsigned &DX8Wrapper::s_textureChanges = Make_Global<unsigned>(0x00DEE754);
unsigned &DX8Wrapper::s_renderStateChanges = Make_Global<unsigned>(0x00DEE758);
unsigned &DX8Wrapper::s_textureStageStateChanges = Make_Global<unsigned>(0x00DEE75C);
unsigned &DX8Wrapper::s_drawCalls = Make_Global<unsigned>(0x00DEE760);
unsigned &DX8Wrapper::s_lastFrameMatrixChanges = Make_Global<unsigned>(0x00DEE77C);
unsigned &DX8Wrapper::s_lastFrameMaterialChanges = Make_Global<unsigned>(0x00DEE780);
unsigned &DX8Wrapper::s_lastFrameVertexBufferChanges = Make_Global<unsigned>(0x00DEE784);
unsigned &DX8Wrapper::s_lastFrameIndexBufferChanges = Make_Global<unsigned>(0x00DEE788);
unsigned &DX8Wrapper::s_lastFrameLightChanges = Make_Global<unsigned>(0x00DEE78C);
unsigned &DX8Wrapper::s_lastFrameTextureChanges = Make_Global<unsigned>(0x00DEE790);
unsigned &DX8Wrapper::s_lastFrameRenderStateChanges = Make_Global<unsigned>(0x00DEE794);
unsigned &DX8Wrapper::s_lastFrameTextureStageStateChanges = Make_Global<unsigned>(0x00DEE798);
unsigned &DX8Wrapper::s_lastFrameNumberDX8Calls = Make_Global<unsigned>(0x00DEE79C);
unsigned &DX8Wrapper::s_lastFrameDrawCalls = Make_Global<unsigned>(0x00DEE7A0); ////
DynamicVectorClass<StringClass> &DX8Wrapper::s_renderDeviceNameTable =
    Make_Global<DynamicVectorClass<StringClass>>(0x00DEE600); //
DynamicVectorClass<StringClass> &DX8Wrapper::s_renderDeviceShortNameTable =
    Make_Global<DynamicVectorClass<StringClass>>(0x00DEDD28); //
DynamicVectorClass<RenderDeviceDescClass> &DX8Wrapper::s_renderDeviceDescriptionTable =
    Make_Global<DynamicVectorClass<RenderDeviceDescClass>>(0x00DE8FE0); //

// missing.cpp done
#include "missing.h"
#include "w3dformat.h"
w3dtexture_t &MissingTexture::s_missingTexture = Make_Global<w3dtexture_t>(0x00DF69D8);

// render2d.cpp done
#include "render2d.h"
RectClass &Render2DClass::s_screenResolution = Make_Global<RectClass>(0x00DE8F98);

// shader.cpp done
bool &ShaderClass::s_shaderDirty = Make_Global<bool>(0x00CC19DC);
uint32_t &ShaderClass::s_currentShader = Make_Global<uint32_t>(0x00DE8E34);
uint32_t &ShaderClass::s_polygonCullMode = Make_Global<uint32_t>(0x00CC19E0);

// synctextureloadlist.cpp done
class SynchronizedTextureLoadTaskListClass;
SynchronizedTextureLoadTaskListClass &g_foregroundQueue = Make_Global<SynchronizedTextureLoadTaskListClass>(0x00DF0FD0);
SynchronizedTextureLoadTaskListClass &g_backgroundQueue = Make_Global<SynchronizedTextureLoadTaskListClass>(0x00DF1038);

// texturebase.cpp done
#include "texturebase.h"
unsigned &TextureBaseClass::s_unusedTextureID = Make_Global<unsigned>(0x00DEE880);

// textureloader.cpp done
#include "textureloader.h"
unsigned &TextureLoader::s_textureInactiveOverrideTime = Make_Global<unsigned>(0x00DF1048);
LoaderThreadClass &TextureLoader::s_textureLoadThread = Make_Global<LoaderThreadClass>(0x00DF0FE0);
bool &TextureLoader::s_textureLoadSuspended = Make_Global<bool>(0x00DF0FC4);
FastCriticalSectionClass &g_backgroundCritSec = Make_Global<FastCriticalSectionClass>(0x00DF0FC8);
FastCriticalSectionClass &g_foregroundCritSec = Make_Global<FastCriticalSectionClass>(0x00DF0FCC);

// textureloadtasklist.cpp done
class TextureLoadTaskListClass;
TextureLoadTaskListClass &g_freeList = Make_Global<TextureLoadTaskListClass>(0x00DF0FA8);
TextureLoadTaskListClass &g_cubeFreeList = Make_Global<TextureLoadTaskListClass>(0x00DF0F98);
TextureLoadTaskListClass &g_volFreeList = Make_Global<TextureLoadTaskListClass>(0x00DF0FB8);

// thumbnailmanager.cpp done
#include "thumbnailmanager.h"
DLListClass<ThumbnailManagerClass> &ThumbnailManagerClass::ThumbnailManagerList =
    Make_Global<DLListClass<ThumbnailManagerClass>>(0x00DFAD90);
ThumbnailManagerClass *&g_thumbnailManager = Make_Global<ThumbnailManagerClass *>(0x00DFAD88);
// bool &ThumbnailManagerClass::s_createIfNotFound = Make_Global<bool>(0x007F66AC);

// w3d.cpp
#include "w3d.h"
unsigned &W3D::s_syncTime = Make_Global<unsigned>(0x00DEE7FC); //
unsigned &W3D::s_textureReduction = Make_Global<unsigned>(0x00DEE844); //
unsigned &W3D::s_textureMinDimension = Make_Global<unsigned>(0x00CC58BC); //
bool &W3D::s_largeTextureExtraReduction = Make_Global<bool>(0x00DEE848); //
bool &W3D::s_isScreenUVBiased = Make_Global<bool>(0x00A47FAF);

// w3dformat.cpp done
ARRAY_DEF(0x00DF6B20, WW3DFormat, g_D3DFormatToWW3DFormatConversionArray, 63);
ARRAY_DEF(0x00DF69E0, WW3DZFormat, g_D3DFormatToWW3DZFormatConversionArray, 80);

// wwstring.cpp done
#include "wwstring.h"
FastCriticalSectionClass &StringClass::m_mutex = Make_Global<FastCriticalSectionClass>(0x00E0BB54);
char &StringClass::m_nullChar = Make_Global<char>(0x00E0CB58);
char *&StringClass::m_emptyString = Make_Global<char *>(0x00CD3E94);
unsigned &StringClass::m_reserveMask = Make_Global<unsigned>(0x00E0CB5C);

// thread.cpp done
#include "threadtrack.h"
DynamicVectorClass<ThreadTracker *> &g_threadTracker = Make_Global<DynamicVectorClass<ThreadTracker *>>(0x00E0B9D8);

// drawgroupinfo.cpp
class DrawGroupInfo;
DrawGroupInfo *&g_theDrawGroupInfo = Make_Global<DrawGroupInfo *>(0); //dunno
