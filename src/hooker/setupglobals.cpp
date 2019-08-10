#include "hooker.h"
#include "view.h"

// main.cpp
HWND &g_applicationHWnd = Make_Global<HWND>(0x00A27B08);
unsigned &g_theMessageTime = Make_Global<unsigned>(0x00A27B14);
bool &g_gameIsWindowed = Make_Global<bool>(0x00A27B0C);
bool &g_gameNotFullscreen = Make_Global<bool>(0x009C0ACC);
bool &g_creatingWindow = Make_Global<bool>(0x00A27B1C);
HGDIOBJ &g_splashImage = Make_Global<HGDIOBJ>(0x00A27B20);
HINSTANCE &g_applicationHInstance = Make_Global<HINSTANCE>(0x00A27B04);

// maputil.cpp
class WaypointMap;
class MapCache;
WaypointMap *&g_waypoints = Make_Global<WaypointMap *>(0x00945AD4);
MapCache *&g_theMapCache = Make_Global<MapCache *>(0x00A2B974);

// weather.cpp
class WeatherSetting;
WeatherSetting *&g_theWeatherSetting = Make_Global<WeatherSetting *>(0x00A2BF00);

// displaystringmanager.cpp
class DisplayStringManager;
DisplayStringManager *&g_theDisplayStringManger = Make_Global<DisplayStringManager *>(0x00A2A6C4);

// gameclient.cpp
class GameClient;
GameClient *&g_theGameClient = Make_Global<GameClient *>(0x00A2BBF8);

// gametext.cpp
class GameTextInterface;
GameTextInterface *&g_theGameText = Make_Global<GameTextInterface *>(0x00A2A2AC);

// globallanguage.cpp
class GlobalLanguage;
GlobalLanguage *&g_theGlobalLanguage = Make_Global<GlobalLanguage *>(0x00A2A6CC);

// terrainroads.cpp
class TerrainRoadCollection;
TerrainRoadCollection *&g_theTerrainRoads = Make_Global<TerrainRoadCollection *>(0x00A2BE4C);

// view.cpp
uint32_t &View::s_idNext = Make_Global<uint32_t>(0x009D4178);

// water.cpp
class WaterSetting;
class WaterTransparencySetting;
WaterSetting *g_waterSettings = Make_Pointer<WaterSetting>(0x00A2F0B8);
WaterTransparencySetting *&g_theWaterTransparency = Make_Global<WaterTransparencySetting *>(0x00A2F0B0);

// keyboard.cpp
class Keyboard;
Keyboard *&g_theKeyboard = Make_Global<Keyboard *>(0x00A29B70);

// mouse.cpp
class Mouse;
Mouse *&g_theMouse = Make_Global<Mouse *>(0x00A29B60);

// particlesysmanager.cpp
class ParticleSystemManager;
ParticleSystemManager *&g_theParticleSystemManager = Make_Global<ParticleSystemManager *>(0x00A2BDAC);

// videoplayer.cpp
class VideoPlayer;
VideoPlayer *&g_theVideoPlayer = Make_Global<VideoPlayer *>(0x00A2C0AC);

// commandlist.cpp
class CommandList;
CommandList *&g_theCommandList = Make_Global<CommandList *>(0x00A29B78);

// gameengine.cpp
class GameEngine;
GameEngine *&g_theGameEngine = Make_Global<GameEngine *>(0x00A29B80);

// gamelod.cpp
class GameLODManager;
GameLODManager *&g_theGameLODManager = Make_Global<GameLODManager *>(0x00A2B924);

// globaldata.cpp
class GlobalData;
GlobalData *&g_theWriteableGlobalData = Make_Global<GlobalData *>(0x00A2A2A4);

// messagestream.cpp
class MessageStream;
MessageStream *&g_theMessageStream = Make_Global<MessageStream *>(0x00A29B74);

// multiplayersettings.cpp
class MultiplayerSettings;
MultiplayerSettings *&g_theMultiplayerSettings = Make_Global<MultiplayerSettings *>(0x00A2BE58);

// namekeygenerator.cpp
class NameKeyGenerator;
NameKeyGenerator *&g_theNameKeyGenerator = Make_Global<NameKeyGenerator *>(0x00A2B928);

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

// terraintypes.cpp
class TerrainTypeCollection;
TerrainTypeCollection *&g_theTerrainTypes = Make_Global<TerrainTypeCollection *>(0x00A2BE54);

// version.cpp
class Version;
Version *&g_theVersion = Make_Global<Version *>(0x00A29BA0);

// audiopmanager.cpp
class AudioManager;
AudioManager *&g_theAudio = Make_Global<AudioManager *>(0x00A29B6C);

// ini.cpp
class Xfer;
Xfer *&g_sXfer = Make_Global<Xfer *>(0x00A2A6B8);

// modulefactory.cpp
class ModuleFactory;
ModuleFactory *&g_theModuleFactory = Make_Global<ModuleFactory *>(0x00A2BE40);

// playerlist.cpp
class PlayerList;
PlayerList *&g_thePlayerList = Make_Global<PlayerList *>(0x00A2B688);

// playertemplate.cpp
class PlayerTemplateStore;
PlayerTemplateStore *&g_thePlayerTemplateStore = Make_Global<PlayerTemplateStore *>(0x00A2BDF4);

// science.cpp
class ScienceStore;
ScienceStore *&g_theScienceStore = Make_Global<ScienceStore *>(0x00A2B978);

// sideslist.cpp
class SidesList;
SidesList *&g_theSidesList = Make_Global<SidesList *>(0x00A2BE3C);

// archivefilesystem.cpp
class ArchiveFileSystem;
ArchiveFileSystem *&g_theArchiveFileSystem = Make_Global<ArchiveFileSystem *>(0x00A2BA00);

// filesystem.cpp
class FileSystem;
FileSystem *&g_theFileSystem = Make_Global<FileSystem *>(0x00A2B670);

// functionlexicon.cpp
class FunctionLexicon;
FunctionLexicon *&g_theFunctionLexicon = Make_Global<FunctionLexicon *>(0x00A2BE44);

// gamememory.cpp
bool &g_thePreMainInitFlag = Make_Global<bool>(0x00A29B90);
bool &g_theMainInitFlag = Make_Global<bool>(0x00A29B91);

// gamestate.cpp
class GameState;
GameState *&g_theGameState = Make_Global<GameState *>(0x00A2BA04);

// localfilesystem.cpp
class LocalFileSystem;
LocalFileSystem *&g_theLocalFileSystem = Make_Global<LocalFileSystem *>(0x00A2BE60);

// memdynalloc.cpp
class SimpleCriticalSectionClass;
class DynamicMemoryAllocator;
SimpleCriticalSectionClass *&g_dmaCriticalSection = Make_Global<SimpleCriticalSectionClass *>(0x00A2A298);
DynamicMemoryAllocator *&g_dynamicMemoryAllocator = Make_Global<DynamicMemoryAllocator *>(0x00A29B98);

// mempool.cpp
SimpleCriticalSectionClass *&g_memoryPoolCriticalSection = Make_Global<SimpleCriticalSectionClass *>(0x00A2A29C);

// mempoolfact.cpp
class MemoryPoolFactory;
MemoryPoolFactory *&g_memoryPoolFactory = Make_Global<MemoryPoolFactory *>(0x00A29B94);

// stackdump.cpp
Utf8String &g_exceptionFileBuffer = Make_Global<Utf8String>(0x00A29FB8);

// subsysteminterface.cpp
SubsystemInterfaceList *&g_theSubsystemList = Make_Global<SubsystemInterfaceList *>(0x00A29B84);

// unicodestring.cpp
SimpleCriticalSectionClass *&g_unicodeStringCriticalSection = Make_Global<SimpleCriticalSectionClass *>(0x00A2A294);

// cavesystem.cpp
class CaveSystem;
CaveSystem *&g_theCaveSystem = Make_Global<CaveSystem *>(0x00A2BDF8);

// rankinfo.cpp
class RankInfoStore;
RankInfoStore *&g_theRankInfoStore = Make_Global<RankInfoStore *>(0x00A2B97C);

// lanapiinterface.cpp
class LANAPIInterface;
LANAPIInterface *&g_theLAN = Make_Global<LANAPIInterface *>(0x00A2E0EC);

// ffactory.cpp
class RawFileFactoryClass;
class FileFactoryClass;
RawFileFactoryClass *&g_theWritingFileFactory = Make_Global<RawFileFactoryClass *>(0x00A1EEB8);
FileFactoryClass *&g_theFileFactory = Make_Global<FileFactoryClass *>(0x00A1EEB0);

// dx8caps.cpp
#include "dx8caps.h"
StringClass &DX8Caps::s_videoCardDetails = Make_Global<StringClass>(0x00A51ECC);

// multilist.cpp
#include "multilist.h"
HOOK_AUTOPOOL(MultiListNodeClass, 256, 0x00A66288);

// dx8wrapper.cpp
#include "dx8wrapper.h"
uint32_t &g_numberOfDx8Calls = Make_Global<uint32_t>(0x00A47F40);
IDirect3D8 *(__stdcall *&DX8Wrapper::s_d3dCreateFunction)(unsigned) = Make_Global<IDirect3D8 *(__stdcall *)(unsigned)>(
    0x00A47F6C);
HMODULE &DX8Wrapper::s_d3dLib = Make_Global<HMODULE>(0x00A47F70);
IDirect3D8 *&DX8Wrapper::s_d3dInterface = Make_Global<IDirect3D8 *>(0x00A47EEC);
IDirect3DDevice8 *&DX8Wrapper::s_d3dDevice = Make_Global<IDirect3DDevice8 *>(0x00A47EF0);
IDirect3DBaseTexture8 **DX8Wrapper::s_textures = Make_Pointer<w3dbasetexture_t>(0x00A42784);
void *&DX8Wrapper::s_shadowMap = Make_Global<void *>(0x00A47EBC);
void *&DX8Wrapper::s_hwnd = Make_Global<void *>(0x00A47EBC);
unsigned *DX8Wrapper::s_renderStates = Make_Pointer<unsigned>(0x00A46CC0);
unsigned *DX8Wrapper::s_textureStageStates = Make_Pointer<unsigned>(0x00A46CC0);
Vector4 *DX8Wrapper::s_vertexShaderConstants = Make_Pointer<Vector4>(0x00A47778);
unsigned *DX8Wrapper::s_pixelShaderConstants = Make_Pointer<unsigned>(0x00A427C0);
bool &DX8Wrapper::s_isInitialised = Make_Global<bool>(0x00A47EC8);
bool &DX8Wrapper::s_isWindowed = Make_Global<bool>(0x00A47EC9);
RenderStateStruct &DX8Wrapper::s_renderState = Make_Global<RenderStateStruct>(0x00A47508);
unsigned &DX8Wrapper::s_renderStateChanged = Make_Global<unsigned>(0x00A42778);
float &DX8Wrapper::s_zNear = Make_Global<float>(0x00A47E38);
float &DX8Wrapper::s_zFar = Make_Global<float>(0x00A47EB8);
Matrix4 &DX8Wrapper::s_projectionMatrix = Make_Global<Matrix4>(0x00A47DF8);
int &DX8Wrapper::s_mainThreadID = Make_Global<int>(0x00A47F2C);
int &DX8Wrapper::s_currentRenderDevice = Make_Global<int>(0x00898BC4);
DX8Caps *&DX8Wrapper::s_currentCaps = Make_Global<DX8Caps *>(0x00A47F30);

// missing.cpp
#include "missing.h"
#include "w3dformat.h"
w3dtexture_t &MissingTexture::s_missingTexture = Make_Global<w3dtexture_t>(0x00A522F8);

// shader.cpp
bool &ShaderClass::s_shaderDirty = Make_Global<bool>(0x00A16C1C);
uint32_t &ShaderClass::s_currentShader = Make_Global<uint32_t>(0x00A4C1B4);
uint32_t &ShaderClass::s_polygonCullMode = Make_Global<uint32_t>(0x00A16C20);

// synctextureloadlist.cpp
class SynchronizedTextureLoadTaskListClass;
SynchronizedTextureLoadTaskListClass &g_foregroundQueue = Make_Global<SynchronizedTextureLoadTaskListClass>(0x00A4C610);
SynchronizedTextureLoadTaskListClass &g_backgroundQueue = Make_Global<SynchronizedTextureLoadTaskListClass>(0x00A4C678);

// texturebase.cpp
#include "texturebase.h"
unsigned &TextureBaseClass::s_unusedTextureID = Make_Global<unsigned>(0x00A4C388);

// textureloader.cpp
#include "textureloader.h"
unsigned &TextureLoader::s_textureInactiveOverrideTime = Make_Global<unsigned>(0x00A4C688);
LoaderThreadClass &TextureLoader::s_textureLoadThread = Make_Global<LoaderThreadClass>(0x00A4C620);
bool &TextureLoader::s_textureLoadSuspended = Make_Global<bool>(0x00A4C604);
FastCriticalSectionClass &g_backgroundCritSec = Make_Global<FastCriticalSectionClass>(0x00A4C608);
FastCriticalSectionClass &g_foregroundCritSec = Make_Global<FastCriticalSectionClass>(0x00A4C60C);

// textureloadtasklist.cpp
class TextureLoadTaskListClass;
TextureLoadTaskListClass &g_freeList = Make_Global<TextureLoadTaskListClass>(0x00A4C5E8);
TextureLoadTaskListClass &g_cubeFreeList = Make_Global<TextureLoadTaskListClass>(0x00A4C5D8);
TextureLoadTaskListClass &g_volFreeList = Make_Global<TextureLoadTaskListClass>(0x00A4C5F8);

// thumbnailmanager.cpp
#include "thumbnailmanager.h"
DLListClass<ThumbnailManagerClass> &ThumbnailManagerClass::ThumbnailManagerList =
    Make_Global<DLListClass<ThumbnailManagerClass>>(0x00A544B0);
ThumbnailManagerClass *&g_thumbnailManager = Make_Global<ThumbnailManagerClass *>(0x00A544A8);
// bool &ThumbnailManagerClass::s_createIfNotFound = Make_Global<bool>(0x007F66AC);

// w3d.cpp
#include "w3d.h"
unsigned &W3D::s_syncTime = Make_Global<unsigned>(0x00A47F9C);
unsigned &W3D::s_textureReduction = Make_Global<unsigned>(0x00A47FE4);
unsigned &W3D::s_textureMinDimension = Make_Global<unsigned>(0x00A1699C);
bool &W3D::s_largeTextureExtraReduction = Make_Global<bool>(0x00A47FE8);
