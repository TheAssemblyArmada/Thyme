#ifndef GAME_DLL
#error This file must not be compiled into standalone binary
#endif

#include "hooker.h"
#include "view.h"

#pragma warning(push)
#pragma warning(disable : 4073) // warning C4073: initializers put in library initialization area
#pragma init_seg(lib) // Forces objects and variables in this file to initialize before other stuff.
#pragma warning(pop)

// main.cpp
HWND &g_applicationHWnd = Make_Global<HWND>(PICK_ADDRESS(0x00A27B08, 0x00DE80AC));
unsigned &g_theMessageTime = Make_Global<unsigned>(PICK_ADDRESS(0x00A27B14, 0));
bool &g_gameIsWindowed = Make_Global<bool>(PICK_ADDRESS(0x00A27B0C, 0));
bool &g_gameNotFullscreen = Make_Global<bool>(PICK_ADDRESS(0x009C0ACC, 0));
bool &g_creatingWindow = Make_Global<bool>(PICK_ADDRESS(0x00A27B1C, 0));
bool &g_gameActive = Make_Global<bool>(PICK_ADDRESS(0x00A27B1D, 0));
HGDIOBJ &g_splashImage = Make_Global<HGDIOBJ>(PICK_ADDRESS(0x00A27B20, 0));
HINSTANCE &g_applicationHInstance = Make_Global<HINSTANCE>(PICK_ADDRESS(0x00A27B04, 0));

// maputil.cpp
class WaypointMap;
class MapCache;
WaypointMap *&g_waypoints = Make_Global<WaypointMap *>(PICK_ADDRESS(0x00A2B968, 0x04CAA140));
MapCache *&g_theMapCache = Make_Global<MapCache *>(PICK_ADDRESS(0x00A2B974, 0x04CAA154));

// snow.cpp
#include "snow.h"
SnowManager *&g_theSnowManager = Make_Global<SnowManager *>(PICK_ADDRESS(0x00A2BF04, 0x00E25CF4));
Override<WeatherSetting> &g_theWeatherSetting = Make_Global<Override<WeatherSetting>>(PICK_ADDRESS(0x00A2BF00, 0x00E25CF0));

// displaystringmanager.cpp
class DisplayStringManager;
DisplayStringManager *&g_theDisplayStringManager = Make_Global<DisplayStringManager *>(PICK_ADDRESS(0x00A2A6C4, 0x00A2A6C4));

// gameclient.cpp
class GameClient;
GameClient *&g_theGameClient = Make_Global<GameClient *>(PICK_ADDRESS(0x00A2BBF8, 0x00A2BBF8));

// gametext.cpp
class GameTextInterface;
GameTextInterface *&g_theGameText = Make_Global<GameTextInterface *>(PICK_ADDRESS(0x00A2A2AC, 0x00E23840));

// globallanguage.cpp
class GlobalLanguage;
GlobalLanguage *&g_theGlobalLanguage = Make_Global<GlobalLanguage *>(PICK_ADDRESS(0x00A2A6CC, 0x00E25D14));

// terrainroads.cpp
class TerrainRoadCollection;
TerrainRoadCollection *&g_theTerrainRoads = Make_Global<TerrainRoadCollection *>(PICK_ADDRESS(0x00A2BE4C, 0x00E23A34));

// view.cpp
uint32_t &View::s_idNext = Make_Global<uint32_t>(PICK_ADDRESS(0x009D4178, 0x00D0DD7C));

// water.cpp
#include "water.h"
ARRAY_DEF(PICK_ADDRESS(0x00A2F0B8, 0x00A2F0B8), WaterSetting, g_waterSettings, TIME_OF_DAY_COUNT);
Override<WaterTransparencySetting> &g_theWaterTransparency =
    Make_Global<Override<WaterTransparencySetting>>(PICK_ADDRESS(0x00A2F0B0, 0x00A2F0B0));

// keyboard.cpp
class Keyboard;
Keyboard *&g_theKeyboard = Make_Global<Keyboard *>(PICK_ADDRESS(0x00A29B70, 0x00E26910));

// mouse.cpp
class Mouse;
Mouse *&g_theMouse = Make_Global<Mouse *>(PICK_ADDRESS(0x00A29B60, 0x00E26904));

// particlesysmanager.cpp
class ParticleSystemManager;
ParticleSystemManager *&g_theParticleSystemManager =
    Make_Global<ParticleSystemManager *>(PICK_ADDRESS(0x00A2BDAC, 0x00E2414C));

// videoplayer.cpp
class VideoPlayer;
VideoPlayer *&g_theVideoPlayer = Make_Global<VideoPlayer *>(PICK_ADDRESS(0x00A2C0AC, 0x00E239B8));

// commandlist.cpp
class CommandList;
CommandList *&g_theCommandList = Make_Global<CommandList *>(PICK_ADDRESS(0x00A29B78, 0x04CA8760));

// gameengine.cpp
class GameEngine;
GameEngine *&g_theGameEngine = Make_Global<GameEngine *>(PICK_ADDRESS(0x00A29B80, 0x00E268C0));

// gamelod.cpp
class GameLODManager;
GameLODManager *&g_theGameLODManager = Make_Global<GameLODManager *>(PICK_ADDRESS(0x00A2B924, 0x00E26070));

// globaldata.cpp
class GlobalData;
GlobalData *&g_theWriteableGlobalData = Make_Global<GlobalData *>(PICK_ADDRESS(0x00A2A2A4, 0x00E1D550));

// messagestream.cpp
class MessageStream;
MessageStream *&g_theMessageStream = Make_Global<MessageStream *>(PICK_ADDRESS(0x00A29B74, 0x04CA875C));

// multiplayersettings.cpp
class MultiplayerSettings;
MultiplayerSettings *&g_theMultiplayerSettings = Make_Global<MultiplayerSettings *>(PICK_ADDRESS(0x00A2BE58, 0x00E23A2C));

// namekeygenerator.cpp
class NameKeyGenerator;
NameKeyGenerator *&g_theNameKeyGenerator = Make_Global<NameKeyGenerator *>(PICK_ADDRESS(0x00A2B928, 0x00E1D490));

// staticnamekey.cpp
#include "staticnamekey.h"
StaticNameKey &g_teamNameKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A3A830, 0x00E1A450));
StaticNameKey &g_theInitialCameraPositionKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A3A880, 0x00E1A4A0));
StaticNameKey &g_playerNameKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A32580, 0x00E121A0));
StaticNameKey &g_playerIsHumanKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A3A8F0, 0x00E1A510));
StaticNameKey &g_playerDisplayNameKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A32588, 0x00E121A8));
StaticNameKey &g_playerFactionKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A326B8, 0x00E122D8));
StaticNameKey &g_playerAlliesKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A36758, 0x00E16378));
StaticNameKey &g_playerEnemiesKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A32668, 0x00E12288));
StaticNameKey &g_teamOwnerKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A32568, 0x00E12188));
StaticNameKey &g_teamIsSingletonKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A3A948, 0x00E1A568));
StaticNameKey &g_objectInitialHealthKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A326A8, 0x00E122C8));
StaticNameKey &g_objectEnabledKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A3A8E0, 0x00E1A500));
StaticNameKey &g_objectIndestructibleKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A32598, 0x00E121B8));
StaticNameKey &g_objectUnsellableKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A32650, 0x00E12270));
StaticNameKey &g_objectPoweredKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A32628, 0x00E12248));
StaticNameKey &g_objectRecruitableAIKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A36720, 0x00E16340));
StaticNameKey &g_objectTargetableKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A32578, 0x00E12198));
StaticNameKey &g_originalOwnerKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A32660, 0x00E12280));
StaticNameKey &g_uniqueIDKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A3A938, 0x00E1A558));
StaticNameKey &g_waypointIDKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A3A8B8, 0x00E1A4D8));
StaticNameKey &g_waypointNameKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A3A960, 0x00E1A580));
StaticNameKey &g_weatherKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A3A890, 0x00E1A4B0));
StaticNameKey &g_lightHeightAboveTerrainKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A3A850, 0x00E1A470));
StaticNameKey &g_scorchTypeKey = Make_Global<StaticNameKey>(PICK_ADDRESS(0x00A3A808, 0x00E1A428));

// terraintypes.cpp
class TerrainTypeCollection;
TerrainTypeCollection *&g_theTerrainTypes = Make_Global<TerrainTypeCollection *>(PICK_ADDRESS(0x00A2BE54, 0x00E1D4B8));

// version.cpp
class Version;
Version *&g_theVersion = Make_Global<Version *>(PICK_ADDRESS(0x00A29BA0, 0x00E27F84));

// audiopmanager.cpp
class AudioManager;
AudioManager *&g_theAudio = Make_Global<AudioManager *>(PICK_ADDRESS(0x00A29B6C, 0x00E1D62C));

// ini.cpp
class Xfer;
Xfer *&g_sXfer = Make_Global<Xfer *>(PICK_ADDRESS(0x00A2A6B8, 0x00E2557C));

// modulefactory.cpp
class ModuleFactory;
ModuleFactory *&g_theModuleFactory = Make_Global<ModuleFactory *>(PICK_ADDRESS(0x00A2BE40, 0x00E241F4));

// playerlist.cpp
class PlayerList;
PlayerList *&g_thePlayerList = Make_Global<PlayerList *>(PICK_ADDRESS(0x00A2B688, 0x00E25D2C));

// playertemplate.cpp
class PlayerTemplateStore;
PlayerTemplateStore *&g_thePlayerTemplateStore = Make_Global<PlayerTemplateStore *>(PICK_ADDRESS(0x00A2BDF4, 0x00E1D484));

// science.cpp
class ScienceStore;
ScienceStore *&g_theScienceStore = Make_Global<ScienceStore *>(PICK_ADDRESS(0x00A2B978, 0x00E23868));

// sideslist.cpp
class SidesList;
SidesList *&g_theSidesList = Make_Global<SidesList *>(PICK_ADDRESS(0x00A2BE3C, 0x00E1B3D8));

// archivefilesystem.cpp
class ArchiveFileSystem;
ArchiveFileSystem *&g_theArchiveFileSystem = Make_Global<ArchiveFileSystem *>(PICK_ADDRESS(0x00A2BA00, 0x00E255B0));

// filesystem.cpp
class FileSystem;
FileSystem *&g_theFileSystem = Make_Global<FileSystem *>(PICK_ADDRESS(0x00A2B670, 0x00E239C0));

// functionlexicon.cpp
class FunctionLexicon;
FunctionLexicon *&g_theFunctionLexicon = Make_Global<FunctionLexicon *>(PICK_ADDRESS(0x00A2BE44, 0x04CAB9A8));

// gamememory.cpp
bool &g_thePreMainInitFlag = Make_Global<bool>(PICK_ADDRESS(0x00A29B90, 0x00E1B340));
bool &g_theMainInitFlag = Make_Global<bool>(PICK_ADDRESS(0x00A29B91, 0x00E1B341));

// gamestate.cpp
class GameState;
GameState *&g_theGameState = Make_Global<GameState *>(PICK_ADDRESS(0x00A2BA04, 0x00E26058));

// localfilesystem.cpp
class LocalFileSystem;
LocalFileSystem *&g_theLocalFileSystem = Make_Global<LocalFileSystem *>(PICK_ADDRESS(0x00A2BE60, 0x00E255BC));

// memdynalloc.cpp
class SimpleCriticalSectionClass;
class DynamicMemoryAllocator;
SimpleCriticalSectionClass *&g_dmaCriticalSection =
    Make_Global<SimpleCriticalSectionClass *>(PICK_ADDRESS(0x00A2A298, 0x00E27F40));
DynamicMemoryAllocator *&g_dynamicMemoryAllocator =
    Make_Global<DynamicMemoryAllocator *>(PICK_ADDRESS(0x00A29B98, 0x00E1B348));

// mempool.cpp
SimpleCriticalSectionClass *&g_memoryPoolCriticalSection =
    Make_Global<SimpleCriticalSectionClass *>(PICK_ADDRESS(0x00A2A29C, 0x00E27F44));

// mempoolfact.cpp
class MemoryPoolFactory;
MemoryPoolFactory *&g_memoryPoolFactory = Make_Global<MemoryPoolFactory *>(PICK_ADDRESS(0x00A29B94, 0x00E1B344));

// stackdump.cpp
Utf8String &g_exceptionFileBuffer = Make_Global<Utf8String>(PICK_ADDRESS(0x00A29FB8, 0x00E259C8));

// subsysteminterface.cpp
SubsystemInterfaceList *&g_theSubsystemList = Make_Global<SubsystemInterfaceList *>(PICK_ADDRESS(0x00A29B84, 0x00E268C4));
float &SubsystemInterface::s_totalSubsystemTime = Make_Global<float>(PICK_ADDRESS(0, 0x00E1D4B0));

// unicodestring.cpp
SimpleCriticalSectionClass *&g_unicodeStringCriticalSection =
    Make_Global<SimpleCriticalSectionClass *>(PICK_ADDRESS(0x00A2A294, 0x00E27F3C));

// cavesystem.cpp
class CaveSystem;
CaveSystem *&g_theCaveSystem = Make_Global<CaveSystem *>(PICK_ADDRESS(0x00A2BDF8, 0x00E241D8));

// rankinfo.cpp
class RankInfoStore;
RankInfoStore *&g_theRankInfoStore = Make_Global<RankInfoStore *>(PICK_ADDRESS(0x00A2B97C, 0x00E241B4));

// lanapiinterface.cpp
class LANAPIInterface;
LANAPIInterface *&g_theLAN = Make_Global<LANAPIInterface *>(PICK_ADDRESS(0x00A2E0EC, 0x04CAA100));

// ffactory.cpp
class RawFileFactoryClass;
class FileFactoryClass;
RawFileFactoryClass *&g_theWritingFileFactory = Make_Global<RawFileFactoryClass *>(PICK_ADDRESS(0x00A1EEB8, 0x00CD4700));
FileFactoryClass *&g_theFileFactory = Make_Global<FileFactoryClass *>(PICK_ADDRESS(0x00A1EEB0, 0x00CD46F8));

// dx8caps.cpp
#include "dx8caps.h"
StringClass &DX8Caps::s_capsWorkStrings = Make_Global<StringClass>(PICK_ADDRESS(0x00A51ECC, 0x00DF0F90));

// multilist.cpp
#include "multilist.h"
HOOK_AUTOPOOL(MultiListNodeClass, 256, PICK_ADDRESS(0x00A66288, 0x00E0BB30));

// dx8wrapper.cpp
#include "dx8wrapper.h"
uint32_t &g_numberOfDx8Calls = Make_Global<uint32_t>(PICK_ADDRESS(0x00A47F40, 0x00DEE778));
IDirect3D8 *(__stdcall *&DX8Wrapper::s_d3dCreateFunction)(unsigned) = Make_Global<IDirect3D8 *(__stdcall *)(unsigned)>(
    PICK_ADDRESS(0x00A47F6C, 0x00DEE7A4));
HMODULE &DX8Wrapper::s_d3dLib = Make_Global<HMODULE>(PICK_ADDRESS(0x00A47F70, 0x00DEE7A8));
IDirect3D8 *&DX8Wrapper::s_d3dInterface = Make_Global<IDirect3D8 *>(PICK_ADDRESS(0x00A47EEC, 0x00DEE724));
IDirect3DDevice8 *&DX8Wrapper::s_d3dDevice = Make_Global<IDirect3DDevice8 *>(PICK_ADDRESS(0x00A47EF0, 0x00DEE728));
ARRAY_DEF(PICK_ADDRESS(0x00A42784, 0x00DE8FBC), w3dbasetexture_t, DX8Wrapper::s_textures, MAX_TEXTURE_STAGES);
void *&DX8Wrapper::s_shadowMap = Make_Global<void *>(PICK_ADDRESS(0x00A47EBC, 0x00DEE6F4));
HWND &DX8Wrapper::s_hwnd = Make_Global<HWND>(PICK_ADDRESS(0x00A47EC4, 0x00DEE6FC));
ARRAY_DEF(PICK_ADDRESS(0x00A46CC0, 0x00DED4F8), unsigned, DX8Wrapper::s_renderStates, 256);
ARRAY2D_DEF(PICK_ADDRESS(0x00A46880, 0x00DED0B8), unsigned, DX8Wrapper::s_textureStageStates, MAX_TEXTURE_STAGES, 32);
ARRAY_DEF(PICK_ADDRESS(0x00A47778, 0x00DEDFB0), Vector4, DX8Wrapper::s_vertexShaderConstants, 96);
ARRAY_DEF(PICK_ADDRESS(0x00A427C0, 0x00DE8FF8), unsigned, DX8Wrapper::s_pixelShaderConstants, 32);
bool &DX8Wrapper::s_isInitialised = Make_Global<bool>(PICK_ADDRESS(0x00A47EC8, 0x00DEE700));
bool &DX8Wrapper::s_isWindowed = Make_Global<bool>(PICK_ADDRESS(0x00A47EC9, 0x00DEE701));
bool &DX8Wrapper::s_debugIsWindowed = Make_Global<bool>(PICK_ADDRESS(0x00A15CD8, 0x00CC36B8));
RenderStateStruct &DX8Wrapper::s_renderState = Make_Global<RenderStateStruct>(PICK_ADDRESS(0x00A47508, 0x00DEDD40));
unsigned &DX8Wrapper::s_renderStateChanged = Make_Global<unsigned>(PICK_ADDRESS(0x00A42778, 0x00DE8FB0));
float &DX8Wrapper::s_zNear = Make_Global<float>(PICK_ADDRESS(0x00A47E38, 0x00DEE670));
float &DX8Wrapper::s_zFar = Make_Global<float>(PICK_ADDRESS(0x00A47EB8, 0x00DEE6F0));
D3DMATRIX &DX8Wrapper::s_projectionMatrix = Make_Global<D3DMATRIX>(PICK_ADDRESS(0x00A47DF8, 0x00DEE630));
int &DX8Wrapper::s_mainThreadID = Make_Global<int>(PICK_ADDRESS(0x00A47F2C, 0x00DEE764));
int &DX8Wrapper::s_currentRenderDevice = Make_Global<int>(PICK_ADDRESS(0x00A15CDC, 0x00CC36BC));
DX8Caps *&DX8Wrapper::s_currentCaps = Make_Global<DX8Caps *>(PICK_ADDRESS(0x00A47F30, 0x00DEE768));
int &DX8Wrapper::s_resolutionWidth = Make_Global<int>(PICK_ADDRESS(0x00A15CE0, 0x00CC36C0));
int &DX8Wrapper::s_resolutionHeight = Make_Global<int>(PICK_ADDRESS(0x00A15CE4, 0x00CC36C4));
int &DX8Wrapper::s_bitDepth = Make_Global<int>(PICK_ADDRESS(0x00A15CE8, 0x00CC36C8));
int &DX8Wrapper::s_textureBitDepth = Make_Global<int>(PICK_ADDRESS(0x00A15CEC, 0x00CC36CC));
ARRAY_DEF(PICK_ADDRESS(0x00A4277C, 0x00DE8FB4), bool, DX8Wrapper::s_currentLightEnables, 4);
unsigned &DX8Wrapper::s_matrixChanges = Make_Global<unsigned>(PICK_ADDRESS(0x00A47F08, 0x00DEE740));
unsigned &DX8Wrapper::s_materialChanges = Make_Global<unsigned>(PICK_ADDRESS(0x00A47F0C, 0x00DEE744));
unsigned &DX8Wrapper::s_vertexBufferChanges = Make_Global<unsigned>(PICK_ADDRESS(0x00A47F10, 0x00DEE748));
unsigned &DX8Wrapper::s_indexBufferChanges = Make_Global<unsigned>(PICK_ADDRESS(0x00A47F14, 0x00DEE74C));
unsigned &DX8Wrapper::s_lightChanges = Make_Global<unsigned>(PICK_ADDRESS(0x00A47F18, 0x00DEE750));
unsigned &DX8Wrapper::s_textureChanges = Make_Global<unsigned>(PICK_ADDRESS(0x00A47F1C, 0x00DEE754));
unsigned &DX8Wrapper::s_renderStateChanges = Make_Global<unsigned>(PICK_ADDRESS(0x00A47F20, 0x00DEE758));
unsigned &DX8Wrapper::s_textureStageStateChanges = Make_Global<unsigned>(PICK_ADDRESS(0x00A47F24, 0x00DEE75C));
unsigned &DX8Wrapper::s_drawCalls = Make_Global<unsigned>(PICK_ADDRESS(0x00A47F28, 0x00DEE760));
unsigned &DX8Wrapper::s_lastFrameMatrixChanges = Make_Global<unsigned>(PICK_ADDRESS(0x00A47F44, 0x00DEE77C));
unsigned &DX8Wrapper::s_lastFrameMaterialChanges = Make_Global<unsigned>(PICK_ADDRESS(0x00A47F48, 0x00DEE780));
unsigned &DX8Wrapper::s_lastFrameVertexBufferChanges = Make_Global<unsigned>(PICK_ADDRESS(0x00A47F4C, 0x00DEE784));
unsigned &DX8Wrapper::s_lastFrameIndexBufferChanges = Make_Global<unsigned>(PICK_ADDRESS(0x00A47F50, 0x00DEE788));
unsigned &DX8Wrapper::s_lastFrameLightChanges = Make_Global<unsigned>(PICK_ADDRESS(0x00A47F54, 0x00DEE78C));
unsigned &DX8Wrapper::s_lastFrameTextureChanges = Make_Global<unsigned>(PICK_ADDRESS(0x00A47F58, 0x00DEE790));
unsigned &DX8Wrapper::s_lastFrameRenderStateChanges = Make_Global<unsigned>(PICK_ADDRESS(0x00A47F5C, 0x00DEE794));
unsigned &DX8Wrapper::s_lastFrameTextureStageStateChanges = Make_Global<unsigned>(PICK_ADDRESS(0x00A47F60, 0x00DEE798));
unsigned &DX8Wrapper::s_lastFrameNumberDX8Calls = Make_Global<unsigned>(PICK_ADDRESS(0x00A47F64, 0x00DEE79C));
unsigned &DX8Wrapper::s_lastFrameDrawCalls = Make_Global<unsigned>(PICK_ADDRESS(0x00A47F68, 0x00DEE7A0));
D3DFORMAT &DX8Wrapper::s_displayFormat = Make_Global<D3DFORMAT>(PICK_ADDRESS(0x00A47ECC, 0x00DEE704));
DynamicVectorClass<StringClass> &DX8Wrapper::s_renderDeviceNameTable =
    Make_Global<DynamicVectorClass<StringClass>>(PICK_ADDRESS(0x00A47DC8, 0x00DEE600));
DynamicVectorClass<StringClass> &DX8Wrapper::s_renderDeviceShortNameTable =
    Make_Global<DynamicVectorClass<StringClass>>(PICK_ADDRESS(0x00A474F0, 0x00DEDD28));
DynamicVectorClass<RenderDeviceDescClass> &DX8Wrapper::s_renderDeviceDescriptionTable =
    Make_Global<DynamicVectorClass<RenderDeviceDescClass>>(PICK_ADDRESS(0x00A427A8, 0x00DE8FE0));
w3dadapterid_t &DX8Wrapper::s_currentAdapterIdentifier = Make_Global<w3dadapterid_t>(PICK_ADDRESS(0x00A470C0, 0x00DED8F8));
static_assert(D3DTS_WORLD1 == 257, "Expected match");
ARRAY_DEF(PICK_ADDRESS(0x00A42840, 0x00DE9078), D3DMATRIX, DX8Wrapper::s_DX8Transforms, D3DTS_WORLD1);
D3DMATRIX &DX8Wrapper::s_oldPrj = Make_Global<D3DMATRIX>(PICK_ADDRESS(0x00A46C80, 0x00DED4B8));
D3DMATRIX &DX8Wrapper::s_oldView = Make_Global<D3DMATRIX>(PICK_ADDRESS(0x00A47D78, 0x00DEE5B0));
D3DMATRIX &DX8Wrapper::s_oldWorld = Make_Global<D3DMATRIX>(PICK_ADDRESS(0x00A47E78, 0x00DEE6B0));
D3DPRESENT_PARAMETERS &DX8Wrapper::s_presentParameters =
    Make_Global<D3DPRESENT_PARAMETERS>(PICK_ADDRESS(0x00A47E40, 0x00DEE678));
D3DCOLOR &DX8Wrapper::s_fogColor = Make_Global<D3DCOLOR>(PICK_ADDRESS(0x00A47EE8, 0x00DEE720));
bool &DX8Wrapper::s_EnableTriangleDraw = Make_Global<bool>(PICK_ADDRESS(0x00A15CD9, 0x00CC36B9));
int &DX8Wrapper::s_ZBias = Make_Global<int>(PICK_ADDRESS(0x00A427A4, 0x00DE8FDC));
Vector3 &DX8Wrapper::s_ambientColor = Make_Global<Vector3>(PICK_ADDRESS(0x00A47DB8, 0x00DEE5F0));
bool &DX8Wrapper::s_isDeviceLost = Make_Global<bool>(PICK_ADDRESS(0x00A47E74, 0x00DEE6AC));
int &DX8Wrapper::s_FPUPreserve = Make_Global<int>(PICK_ADDRESS(0x00A47EC0, 0x00DEE6F8));
unsigned long &DX8Wrapper::s_vertexShader = Make_Global<unsigned long>(PICK_ADDRESS(0x00A47ED0, 0x00DEE708));
unsigned long &DX8Wrapper::s_pixelShader = Make_Global<unsigned long>(PICK_ADDRESS(0x00A47ED4, 0x00DEE70C));
LightEnvironmentClass *&DX8Wrapper::s_lightEnvironment =
    Make_Global<LightEnvironmentClass *>(PICK_ADDRESS(0x00A47ED8, 0x00DEE710));
unsigned long &DX8Wrapper::s_vertexProcessingBehavior = Make_Global<unsigned long>(PICK_ADDRESS(0x00A47EE0, 0x00DEE718));
bool &DX8Wrapper::s_fogEnable = Make_Global<bool>(PICK_ADDRESS(0x00A47EE4, 0x00DEE71C));
w3dsurface_t &DX8Wrapper::s_currentRenderTarget = Make_Global<w3dsurface_t>(PICK_ADDRESS(0x00A47EF4, 0x00DEE72C));
w3dsurface_t &DX8Wrapper::s_currentDepthBuffer = Make_Global<w3dsurface_t>(PICK_ADDRESS(0x00A47EF8, 0x00DEE730));
w3dsurface_t &DX8Wrapper::s_defaultRenderTarget = Make_Global<w3dsurface_t>(PICK_ADDRESS(0x00A47EFC, 0x00DEE734));
w3dsurface_t &DX8Wrapper::s_defaultDepthBuffer = Make_Global<w3dsurface_t>(PICK_ADDRESS(0x00A47F00, 0x00DEE738));
bool &DX8Wrapper::s_isRenderToTexture = Make_Global<bool>(PICK_ADDRESS(0x00A47F04, 0x00DEE73C));
unsigned int &DX8Wrapper::s_drawPolygonLowBoundLimit = Make_Global<unsigned int>(PICK_ADDRESS(0x00A47F34, 0x00DEE76C));
unsigned long &DX8Wrapper::s_frameCount = Make_Global<unsigned long>(PICK_ADDRESS(0x00A47F38, 0x00DEE770));
bool &DX8Wrapper::s_DX8SingleThreaded = Make_Global<bool>(PICK_ADDRESS(0x00A47F3C, 0x00DEE774));
DX8_CleanupHook *&DX8Wrapper::s_cleanupHook = Make_Global<DX8_CleanupHook *>(PICK_ADDRESS(0x00A47F74, 0x00DEE7AC));

// missingtexture.cpp
#include "missingtexture.h"
#include "w3dformat.h"
w3dtexture_t &MissingTexture::s_missingTexture = Make_Global<w3dtexture_t>(PICK_ADDRESS(0x00A522F8, 0x00DF69D8));

// render2d.cpp
#include "render2d.h"
RectClass &Render2DClass::s_screenResolution = Make_Global<RectClass>(PICK_ADDRESS(0x00A480B0, 0x00DE8F98));

// shader.cpp
bool &ShaderClass::s_shaderDirty = Make_Global<bool>(PICK_ADDRESS(0x00A16C1C, 0x00CC19DC));
uint32_t &ShaderClass::s_currentShader = Make_Global<uint32_t>(PICK_ADDRESS(0x00A4C1B4, 0x00DE8E34));
uint32_t &ShaderClass::s_polygonCullMode = Make_Global<uint32_t>(PICK_ADDRESS(0x00A16C20, 0x00CC19E0));

// synctextureloadlist.cpp
class SynchronizedTextureLoadTaskListClass;
SynchronizedTextureLoadTaskListClass &g_foregroundQueue =
    Make_Global<SynchronizedTextureLoadTaskListClass>(PICK_ADDRESS(0x00A4C610, 0x00DF0FD0));
SynchronizedTextureLoadTaskListClass &g_backgroundQueue =
    Make_Global<SynchronizedTextureLoadTaskListClass>(PICK_ADDRESS(0x00A4C678, 0x00DF1038));

// texturebase.cpp
#include "texturebase.h"
unsigned &TextureBaseClass::s_unusedTextureID = Make_Global<unsigned>(PICK_ADDRESS(0x00A4C388, 0x00DEE880));

// textureloader.cpp
#include "textureloader.h"
unsigned &TextureLoader::s_textureInactiveOverrideTime = Make_Global<unsigned>(PICK_ADDRESS(0x00A4C688, 0x00DF1048));
LoaderThreadClass &TextureLoader::s_textureLoadThread = Make_Global<LoaderThreadClass>(PICK_ADDRESS(0x00A4C620, 0x00DF0FE0));
bool &TextureLoader::s_textureLoadSuspended = Make_Global<bool>(PICK_ADDRESS(0x00A4C604, 0x00DF0FC4));
FastCriticalSectionClass &g_backgroundCritSec = Make_Global<FastCriticalSectionClass>(PICK_ADDRESS(0x00A4C608, 0x00DF0FC8));
FastCriticalSectionClass &g_foregroundCritSec = Make_Global<FastCriticalSectionClass>(PICK_ADDRESS(0x00A4C60C, 0x00DF0FCC));

// textureloadtasklist.cpp
class TextureLoadTaskListClass;
TextureLoadTaskListClass &g_freeList = Make_Global<TextureLoadTaskListClass>(PICK_ADDRESS(0x00A4C5E8, 0x00DF0FA8));
TextureLoadTaskListClass &g_cubeFreeList = Make_Global<TextureLoadTaskListClass>(PICK_ADDRESS(0x00A4C5D8, 0x00DF0F98));
TextureLoadTaskListClass &g_volFreeList = Make_Global<TextureLoadTaskListClass>(PICK_ADDRESS(0x00A4C5F8, 0x00DF0FB8));

// thumbnailmanager.cpp
#include "thumbnailmanager.h"
DLListClass<ThumbnailManagerClass> &ThumbnailManagerClass::s_thumbnailManagerList =
    Make_Global<DLListClass<ThumbnailManagerClass>>(PICK_ADDRESS(0x00A544B0, 0x00DFAD90));
ThumbnailManagerClass *&g_thumbnailManager = Make_Global<ThumbnailManagerClass *>(PICK_ADDRESS(0x00A544A8, 0x00DFAD88));
// bool &ThumbnailManagerClass::s_createIfNotFound = Make_Global<bool>(PICK_ADDRESS(0x007F66AC, 0));

// w3d.cpp
#include "w3d.h"
unsigned &W3D::s_syncTime = Make_Global<unsigned>(PICK_ADDRESS(0x00A47F9C, 0x00DEE7FC));
unsigned &W3D::s_previousSyncTime = Make_Global<unsigned>(PICK_ADDRESS(0x00A47FA0, 0x00DEE800));
unsigned &W3D::s_textureReduction = Make_Global<unsigned>(PICK_ADDRESS(0x00A47FE4, 0x00DEE844));
unsigned &W3D::s_textureMinDimension = Make_Global<unsigned>(PICK_ADDRESS(0x00A1699C, 0x00CC58BC));
bool &W3D::s_largeTextureExtraReduction = Make_Global<bool>(PICK_ADDRESS(0x00A47FE8, 0x00DEE848));
bool &W3D::s_isScreenUVBiased = Make_Global<bool>(PICK_ADDRESS(0x00A47FAF, 0x00DEE80F));
bool &W3D::s_texturingEnabled = Make_Global<bool>(PICK_ADDRESS(0x00A16998, 0x00CC58B8));
bool &W3D::s_thumbnailEnabled = Make_Global<bool>(PICK_ADDRESS(0x00A1698C, 0x00CC58AC));
TextureFilterClass::TextureFilterMode &W3D::s_textureFilter =
    Make_Global<TextureFilterClass::TextureFilterMode>(PICK_ADDRESS(0x00A47FEC, 0x00DEE84C));
float &W3D::s_defaultNativeScreenSize = Make_Global<float>(PICK_ADDRESS(0x00A16984, 0x00CC58A4));
bool &W3D::s_isSortingEnabled = Make_Global<bool>(PICK_ADDRESS(0x00A1697C, 0x00CC589C));
bool &W3D::s_isColoringEnabled = Make_Global<bool>(PICK_ADDRESS(0x00A47FDC, 0x00DEE83C));
bool &W3D::s_mungeSortOnLoad = Make_Global<bool>(PICK_ADDRESS(0x00A47FB1, 0x00DEE811));
bool &W3D::s_overbrightModifyOnLoad = Make_Global<bool>(PICK_ADDRESS(0x00A47FB2, 0x00DEE812));
W3D::PrelitModeEnum &W3D::s_prelitMode = Make_Global<W3D::PrelitModeEnum>(PICK_ADDRESS(0x00A16988, 0x00CC58A8));
HWND &W3D::s_hwnd = Make_Global<HWND>(PICK_ADDRESS(0x00A47FE0, 0x00DEE840));
bool &W3D::s_areStaticSortListsEnabled = Make_Global<bool>(PICK_ADDRESS(0x00A47FB0, 0x00DEE810));
bool &W3D::s_isInited = Make_Global<bool>(PICK_ADDRESS(0x00A47FAC, 0x00DEE80C));
bool &W3D::s_isRendering = Make_Global<bool>(PICK_ADDRESS(0x00A47FAD, 0x00DEE80D));
bool &W3D::s_lite = Make_Global<bool>(PICK_ADDRESS(0x00A47FF0, 0x00DEE850));
int &W3D::s_frameCount = Make_Global<int>(PICK_ADDRESS(0x00A47FB8, 0x00DEE818));
StaticSortListClass *&W3D::s_defaultStaticSortLists =
    Make_Global<StaticSortListClass *>(PICK_ADDRESS(0x00A47FC8, 0x00DEE828));
StaticSortListClass *&W3D::s_currentStaticSortLists =
    Make_Global<StaticSortListClass *>(PICK_ADDRESS(0x00A47FCC, 0x00DEE82C));

// dx8renderer.cpp
#include "dx8renderer.h"
DX8MeshRendererClass &g_theDX8MeshRenderer = Make_Global<DX8MeshRendererClass>(PICK_ADDRESS(0x00A4C4E0, 0x00DEEBF0));
DynamicVectorClass<Vector3> &g_tempVertexBuffer =
    Make_Global<DynamicVectorClass<Vector3>>(PICK_ADDRESS(0x00A4C4B0, 0x00DEEBC0));
DynamicVectorClass<Vector3> &g_tempNormalBuffer =
    Make_Global<DynamicVectorClass<Vector3>>(PICK_ADDRESS(0x00A4C498, 0x00DEEBA8));
MultiListClass<DX8TextureCategoryClass> &g_textureCategoryDeleteList =
    Make_Global<MultiListClass<DX8TextureCategoryClass>>(PICK_ADDRESS(0x00A4C438, 0x00DEEB48));
MultiListClass<DX8FVFCategoryContainer> &g_fvfCategoryContainerDeleteList =
    Make_Global<MultiListClass<DX8FVFCategoryContainer>>(PICK_ADDRESS(0x00A4C480, 0x00DEEB90));
MultiListClass<MeshModelClass> &g_registeredMeshList =
    Make_Global<MultiListClass<MeshModelClass>>(PICK_ADDRESS(0x00A4C4C8, 0x00DEEBD8));
bool &DX8TextureCategoryClass::s_forceMultiply = Make_Global<bool>(PICK_ADDRESS(0x00A4C500, 0x00DEEC10));
HOOK_AUTOPOOL(PolyRenderTaskClass, 256, PICK_ADDRESS(0x00A4C450, 0x00DEEB60));
HOOK_AUTOPOOL(MatPassTaskClass, 256, PICK_ADDRESS(0x00A4C468, 0x00DEEB78));

// w3dformat.cpp
ARRAY_DEF(PICK_ADDRESS(0x00A5243C, 0x00DF6B20), WW3DFormat, g_D3DFormatToWW3DFormatConversionArray, 63);
ARRAY_DEF(PICK_ADDRESS(0x00A522FC, 0x00DF69E0), WW3DZFormat, g_D3DFormatToWW3DZFormatConversionArray, 80);

// wwstring.cpp
#include "wwstring.h"
FastCriticalSectionClass &StringClass::m_mutex = Make_Global<FastCriticalSectionClass>(PICK_ADDRESS(0x00A65184, 0x00E0BB54));
char &StringClass::m_nullChar = Make_Global<char>(PICK_ADDRESS(0x00A66188, 0x00E0CB58));
char *&StringClass::m_emptyString = Make_Global<char *>(PICK_ADDRESS(0x00A1DB70, 0x00CD3E94));
unsigned &StringClass::m_reserveMask = Make_Global<unsigned>(PICK_ADDRESS(0x00A6618C, 0x00E0CB5C));

// thread.cpp
#include "threadtrack.h"
DynamicVectorClass<ThreadTracker *> &g_threadTracker =
    Make_Global<DynamicVectorClass<ThreadTracker *>>(PICK_ADDRESS(0x00A65020, 0x00E0B9D8));

// drawgroupinfo.cpp
class DrawGroupInfo;
DrawGroupInfo *&g_theDrawGroupInfo = Make_Global<DrawGroupInfo *>(PICK_ADDRESS(0x00A2C880, 0x04CAA52C));

// shadermanager.cpp
#include "shadermanager.h"
ARRAY_DEF(PICK_ADDRESS(0x00A3AB20, 0x00E1A7C8), TextureClass *, W3DShaderManager::s_textures, MAX_TEXTURE_STAGES);
bool &W3DShaderManager::s_renderingToTexture = Make_Global<bool>(PICK_ADDRESS(0x00A3AB98, 0x00E1A840));
W3DShaderManager::ShaderTypes &W3DShaderManager::s_currentShader =
    Make_Global<ShaderTypes>(PICK_ADDRESS(0x00A3AA80, 0x00E1A728));

// assetmgr.cpp
#include "assetmgr.h"
W3DAssetManager *&W3DAssetManager::s_theInstance = Make_Global<W3DAssetManager *>(PICK_ADDRESS(0x00A4C1F4, 0x00DE8F84));

// baseheightmap.cpp
#include "baseheightmap.h"
BaseHeightMapRenderObjClass *&g_theTerrainRenderObject =
    Make_Global<BaseHeightMapRenderObjClass *>(PICK_ADDRESS(0x00A3ABC8, 0x00E1A5F4));

// mapboject.cpp
#include "mapobject.h"
MapObject *&MapObject::s_theMapObjectListPtr = Make_Global<MapObject *>(PICK_ADDRESS(0x00A3A968, 0x00E1A588));
Dict &MapObject::s_theWorldDict = Make_Global<Dict>(PICK_ADDRESS(0x00A3A8A8, 0x00E1A4C8));

// rayeffect.cpp
#include "rayeffect.h"
RayEffectSystem *&g_theRayEffects = Make_Global<RayEffectSystem *>(PICK_ADDRESS(0x00A2F44C, 0x04CAB644));

// win32mouse.cpp
#include "win32mouse.h"
Win32Mouse *&g_theWin32Mouse = Make_Global<Win32Mouse *>(PICK_ADDRESS(0x00A27B10, 0));
ARRAY2D_DEF(PICK_ADDRESS(0x00A31F00, 0), HCURSOR, Win32Mouse::s_loadedCursors, CURSOR_COUNT, 8);

// globallanguage.cpp
class FontLibrary;
FontLibrary *&g_theFontLibrary = Make_Global<FontLibrary *>(PICK_ADDRESS(0x00A2A6C8, 0x00E239CC));

// view.cpp
class View;
View *&g_theTacticalView = Make_Global<View *>(PICK_ADDRESS(0x00A2B684, 0x00E23AD8));

// w3dmouse.cpp
#include "w3dmouse.h"
MouseThreadClass &W3DMouse::s_mouseThread = Make_Global<MouseThreadClass>(0x00A3B370);
bool &W3DMouse::s_mouseThreadIsDrawing = Make_Global<bool>(0x00A3B3C8);
ARRAY_DEF(0x00A3B18C, HAnimClass *, W3DMouse::s_W3DMouseAssets1, CURSOR_COUNT);
ARRAY_DEF(0x00A3B22C, RenderObjClass *, W3DMouse::s_W3DMouseAssets2, CURSOR_COUNT);
ARRAY2D_DEF(
    0x00A3B3D8, TextureBaseClass *, W3DMouse::s_D3DMouseAssets, CURSOR_COUNT, W3DMouse::MAX_FRAMES); // TODO unsure on type
ARRAY_DEF(0x00A3B2CC, Image *, W3DMouse::s_PolyMouseAssets, CURSOR_COUNT); // unsure on type
CriticalSectionClass &g_mouseCriticalSection = Make_Global<CriticalSectionClass>(0x00A3B3D0);

// w3ddisplay.cpp
#include "w3ddisplay.h"
GameAssetManager *&W3DDisplay::s_assetManager = Make_Global<GameAssetManager *>(PICK_ADDRESS(0x00A32518, 0x00E1B13C));
RTS3DScene *&W3DDisplay::s_3DScene = Make_Global<RTS3DScene *>(PICK_ADDRESS(0x00A3250C, 0x00E1B130));
RTS2DScene *&W3DDisplay::s_2DScene = Make_Global<RTS2DScene *>(PICK_ADDRESS(0x00A32510, 0x00E1B134));
RTS3DInterfaceScene *&W3DDisplay::s_3DInterfaceScene =
    Make_Global<RTS3DInterfaceScene *>(PICK_ADDRESS(0x00A32514, 0x00E1B138));

// w3dshadow.cpp
#include "w3dshadow.h"
const FrustumClass *&g_shadowCameraFrustum = Make_Global<const FrustumClass *>(PICK_ADDRESS(0x00A3AE3C, 0x00E1A604));
W3DShadowManager *&g_theW3DShadowManager = Make_Global<W3DShadowManager *>(PICK_ADDRESS(0x00A3AE4C, 0x00E1A614));

// thingfactory.cpp
#include "thingfactory.h"
ThingFactory *&g_theThingFactory = Make_Global<ThingFactory *>(PICK_ADDRESS(0x00A2BC04, 0x00E1D4F8));

// gamelogic.cpp
#include "gamelogic.h"
GameLogic *&g_theGameLogic = Make_Global<GameLogic *>(PICK_ADDRESS(0x00A2BBEC, 0x00E25CA0));

// display.cpp
#include "display.h"
Display *&g_theDisplay = Make_Global<Display *>(PICK_ADDRESS(0x00A2A6D0, 0x00E25D1C));

// vertmaterial.cpp
#include "vertmaterial.h"
ARRAY_DEF(PICK_ADDRESS(0x00A4C1FC, 0x00DE8E38),
    VertexMaterialClass *,
    VertexMaterialClass::s_presets,
    VertexMaterialClass::PRESET_COUNT);

// w3dsmudge.cpp
#include "w3dsmudge.h"
W3DSmudgeManager *&g_theSmudgeManager = Make_Global<W3DSmudgeManager *>(PICK_ADDRESS(0x00A3AE5C, 0x00E1ACB4));

// ai.cpp
#include "ai.h"
AI *&g_theAI = Make_Global<AI *>(PICK_ADDRESS(0x00A2BBF4, 0x00E27E24));

// terrainlogic.cpp
#include "terrainlogic.h"
TerrainLogic *&g_theTerrainLogic = Make_Global<TerrainLogic *>(PICK_ADDRESS(0x00A2B680, 0x00E23AE8));
WaterHandle &TerrainLogic::m_gridWaterHandle = Make_Global<WaterHandle>(PICK_ADDRESS(0x00A2B67C, 0x00E23AE4));

// assetmgr.cpp
#include "assetmgr.h"
NullPrototypeClass &s_nullPrototype = Make_Global<NullPrototypeClass>(PICK_ADDRESS(0x00A4C1B8, 0x00DE8F4C));

// meshmatdesc.cpp
#include "meshmatdesc.h"
ShaderClass &MeshMatDescClass::s_NullShader = Make_Global<ShaderClass>(PICK_ADDRESS(0x00A544C0, 0x00DEF7E0));

// meshgeometry.cpp
SimpleVecClass<Vector4> &g_planeEQArray = Make_Global<SimpleVecClass<Vector4>>(PICK_ADDRESS(0x00A51E60, 0x00DEF648));

// objectcreationlist.cpp
#include "objectcreationlist.h"
ObjectCreationListStore *&g_theObjectCreationListStore =
    Make_Global<ObjectCreationListStore *>(PICK_ADDRESS(0x00A2BC44, 0x00E23E3C));
std::vector<Utf8String> &g_debrisModelNamesGlobalHack =
    Make_Global<std::vector<Utf8String>>(PICK_ADDRESS(0x00A2BC38, 0x00E23E30));

// partitionmanager.cpp
#include "partitionmanager.h"
PartitionManager *&g_thePartitionManager = Make_Global<PartitionManager *>(PICK_ADDRESS(0x00A2C150, 0x00E26308));

// w3dwatertracks.cpp
#include "w3dwatertracks.h"
WaterTracksRenderSystem *&g_theWaterTracksRenderSystem =
    Make_Global<WaterTracksRenderSystem *>(PICK_ADDRESS(0x00A3AF08, 0x00E1AC04));

// polygontrigger.cpp
#include "polygontrigger.h"
PolygonTrigger *&PolygonTrigger::s_thePolygonTriggerListPtr =
    Make_Global<PolygonTrigger *>(PICK_ADDRESS(0x00A2C278, 0x00E1D53C));

// w3dterraintracks.cpp
#include "w3dterraintracks.h"
TerrainTracksRenderObjClassSystem *&g_theTerrainTracksRenderObjClassSystem =
    Make_Global<TerrainTracksRenderObjClassSystem *>(PICK_ADDRESS(0x00A3AD5C, 0x00E1ACC0));

// scriptengine.cpp
class ScriptEngine;
ScriptEngine *&g_theScriptEngine = Make_Global<ScriptEngine *>(PICK_ADDRESS(0x00A2B668, 0x00E1E7AC));

// w3dprojectedshadow.cpp
class W3DProjectedShadowManager;
class ProjectedShadowManager;
W3DProjectedShadowManager *&g_theW3DProjectedShadowManager =
    Make_Global<W3DProjectedShadowManager *>(PICK_ADDRESS(0x00A3ACD4, 0x00E1AFC8));
ProjectedShadowManager *&g_theProjectedShadowManager =
    Make_Global<ProjectedShadowManager *>(PICK_ADDRESS(0x00A3AC58, 0x00E1AF44));
class FrustumClass;
#ifdef BUILD_WITH_D3D8
IDirect3DVertexBuffer8 *&g_shadowDecalVertexBufferD3D =
    Make_Global<IDirect3DVertexBuffer8 *>(PICK_ADDRESS(0x00A3ACE8, 0x00E1AFDC));
IDirect3DIndexBuffer8 *&g_shadowDecalIndexBufferD3D =
    Make_Global<IDirect3DIndexBuffer8 *>(PICK_ADDRESS(0x00A3ACEC, 0x00E1AFE0));
#endif
int &g_nShadowDecalVertsInBuf = Make_Global<int>(PICK_ADDRESS(0x00A3ACF0, 0x00E1AFE4));
int &g_nShadowDecalStartBatchVertex = Make_Global<int>(PICK_ADDRESS(0x00A3ACF4, 0x00E1AFE8));
int &g_nShadowDecalIndicesInBuf = Make_Global<int>(PICK_ADDRESS(0x00A3ACF8, 0x00E1AFEC));
int &g_nShadowDecalStartBatchIndex = Make_Global<int>(PICK_ADDRESS(0x00A3ACFC, 0x00E1AFF0));
int &g_nShadowDecalPolysInBatch = Make_Global<int>(PICK_ADDRESS(0x00A3AD00, 0x00E1AFF4));
int &g_nShadowDecalVertsInBatch = Make_Global<int>(PICK_ADDRESS(0x00A3AD04, 0x00E1AFF8));
int &g_drawStartX = Make_Global<int>(PICK_ADDRESS(0x00A3ACE0, 0x00E1AFD4));
int &g_drawEdgeX = Make_Global<int>(PICK_ADDRESS(0x00A3ACD8, 0x00E1AFCC));
int &g_drawStartY = Make_Global<int>(PICK_ADDRESS(0x00A3ACE4, 0x00E1AFD8));
int &g_drawEdgeY = Make_Global<int>(PICK_ADDRESS(0x00A3ACDC, 0x00E1AFD0));

// worldheightmap.cpp
#include "worldheightmap.h"
ARRAY_DEF(0x00A3A96C, TileData *, WorldHeightMap::s_alphaTiles, 12);

// w3dfilesystem.cpp
class W3DFileSystem;
W3DFileSystem *&g_theW3DFileSystem = Make_Global<W3DFileSystem *>(PICK_ADDRESS(0x00A3AD0C, 0x00E1A964));

// network.cpp
#include "network.h"
Network *&g_theNetwork = Make_Global<Network *>(PICK_ADDRESS(0x00A2B92C, 0x00E268E8));

// ingameui.cpp
#include "ingameui.h"
InGameUI *&g_theInGameUI = Make_Global<InGameUI *>(PICK_ADDRESS(0x00A2C088, 0x00E26268));
GameWindow *&g_replayWindow = Make_Global<GameWindow *>(PICK_ADDRESS(0x00A2C08C, 0x00E2626C));

// waterrenderobj.cpp
class WaterRenderObjClass;
WaterRenderObjClass *&g_theWaterRenderObj = Make_Global<WaterRenderObjClass *>(PICK_ADDRESS(0x00A3ABD8, 0x00E1A5D8));

// fxlist.cpp
#include "fxlist.h"
FXListStore *&g_theFXListStore = Make_Global<FXListStore *>(PICK_ADDRESS(0x00A2BC64, 0x00E23F0C));

// image.cpp
class ImageCollection;
ImageCollection *&g_theMappedImageCollection = Make_Global<ImageCollection *>(PICK_ADDRESS(0x00A2C0A4, 0x00E27F50));

// anim2d.cpp
class Anim2DCollection;
Anim2DCollection *&g_theAnim2DCollection = Make_Global<Anim2DCollection *>(PICK_ADDRESS(0x00A2C0A8, 0x00E23974));

// languagefilter.cpp
class LanguageFilter;
LanguageFilter *&g_theLanguageFilter = Make_Global<LanguageFilter *>(PICK_ADDRESS(0x00A2BEF8, 0x04CA87CC));

// thingtemplate.cpp
#include "thingtemplate.h"
AudioEventRTS &ThingTemplate::s_audioEventNoSound = Make_Global<AudioEventRTS>(PICK_ADDRESS(0x00A2C318, 0x00E1D690));

// drawable.cpp
#include "drawable.h"
int &Drawable::s_modelLockCount = Make_Global<int>(PICK_ADDRESS(0x00A2B920, 0x00E25D70));

// imemanagerinterface.cpp
class IMEManagerInterface;
IMEManagerInterface *&g_theIMEManager = Make_Global<IMEManagerInterface *>(PICK_ADDRESS(0x00A29B8C, 0x04CAB63C));

// locomotor.cpp
class LocomotorStore;
LocomotorStore *&g_theLocomotorStore = Make_Global<LocomotorStore *>(PICK_ADDRESS(0x00A2BC28, 0x00E23DE0));

// weapon.cpp
class WeaponStore;
WeaponStore *&g_theWeaponStore = Make_Global<WeaponStore *>(PICK_ADDRESS(0x00A2BC60, 0x00E1D59C));

// team.cpp
class TeamFactory;
TeamFactory *&g_theTeamFactory = Make_Global<TeamFactory *>(PICK_ADDRESS(0x00A2BBE8, 0x04CA864C));

// challengegenerals.cpp
class ChallengeGenerals;
ChallengeGenerals *&g_theChallengeGenerals = Make_Global<ChallengeGenerals *>(PICK_ADDRESS(0x00A2C09C, 0x04CA9C78));

// cdmanager.cpp
class CDManagerInterface;
CDManagerInterface *&g_theCDManager = Make_Global<CDManagerInterface *>(PICK_ADDRESS(0x00A2BE48, 0x00E255AC));

// specialpower.cpp
class SpecialPowerStore;
SpecialPowerStore *&g_theSpecialPowerStore = Make_Global<SpecialPowerStore *>(PICK_ADDRESS(0x00A2BC20, 0x00E2384C));

// damagefx.cpp
class DamageFXStore;
DamageFXStore *&g_theDamageFXStore = Make_Global<DamageFXStore *>(PICK_ADDRESS(0x00A2BC1C, 0x00E23DD8));

// armor.cpp
class ArmorStore;
ArmorStore *&g_theArmorStore = Make_Global<ArmorStore *>(PICK_ADDRESS(0x00A2BC14, 0x00E23DD0));

// buildassistant.cpp
class BuildAssistant;
BuildAssistant *&g_theBuildAssistant = Make_Global<BuildAssistant *>(PICK_ADDRESS(0x00A2BC10, 0x04CAA500));

// upgrade.cpp
class UpgradeCenter;
UpgradeCenter *&g_theUpgradeCenter = Make_Global<UpgradeCenter *>(PICK_ADDRESS(0x00A2BC00, 0x00E23888));

// aipathfind.cpp
#include "aipathfind.h"
PathfindCellInfo *&PathfindCellInfo::m_infoArray = Make_Global<PathfindCellInfo *>(PICK_ADDRESS(0x00A2C270, 0x00E28154));
PathfindCellInfo *&PathfindCellInfo::m_firstFree = Make_Global<PathfindCellInfo *>(PICK_ADDRESS(0x00A2C274, 0x00E28158));

// camerashakesystem.cpp
#include "camerashakesystem.h"
CameraShakeSystemClass &g_theCameraShakerSystem = Make_Global<CameraShakeSystemClass>(PICK_ADDRESS(0x00A3B138, 0x00E1B2F0));
HOOK_AUTOPOOL(CameraShakeSystemClass::CameraShakerClass, 256, PICK_ADDRESS(0x00A3B150, 0x00E1B308));

// cratesystem.cpp
class CrateSystem;
CrateSystem *&g_theCrateSystem = Make_Global<CrateSystem *>(PICK_ADDRESS(0x00A2BBE4, 0x00E23DAC));

// recorder.cpp
class RecorderClass;
RecorderClass *&g_theRecorder = Make_Global<RecorderClass *>(PICK_ADDRESS(0x00A2B9F4, 0x04CA8B24));
int &g_replayCRCInterval = Make_Global<int>(PICK_ADDRESS(0x009D7638, 0x00D4AA04));

// radar.cpp
class Radar;
Radar *&g_theRadar = Make_Global<Radar *>(PICK_ADDRESS(0x00A2BBE0, 0x04CA8CB0));

// victoryconditions.cpp
class VictoryConditionsInterface;
VictoryConditionsInterface *&g_theVictoryConditions =
    Make_Global<VictoryConditionsInterface *>(PICK_ADDRESS(0x00A2BAA0, 0x04CA9DF8));

// metaevent.cpp
class MetaMap;
MetaMap *&g_theMetaMap = Make_Global<MetaMap *>(PICK_ADDRESS(0x00A2BA2C, 0x04CA9C54));

// actionmanager.cpp
class ActionManager;
ActionManager *&g_theActionManager = Make_Global<ActionManager *>(PICK_ADDRESS(0x00A2BA28, 0x04CA87FC));

// gamestatemap.cpp
class GameStateMap;
GameStateMap *&g_theGameStateMap = Make_Global<GameStateMap *>(PICK_ADDRESS(0x00A2BA08, 0x04CAA5A4));

// gameresultsthread.cpp
class GameResultsInterface;
GameResultsInterface *&g_theGameResultsQueue = Make_Global<GameResultsInterface *>(PICK_ADDRESS(0x00A2B930, 0x04CAB9A0));

// kindof.cpp
BitFlags<KINDOF_COUNT> &KINDOFMASK_FS = Make_Global<BitFlags<KINDOF_COUNT>>(PICK_ADDRESS(0x00A2B9D0, 0x00E239F8));

// disabledtypes.cpp
DisabledBitFlags &DISABLEDMASK_ALL = Make_Global<DisabledBitFlags>(PICK_ADDRESS(0x00A2B9C8, 0x04CAA554));

// damage.cpp
BitFlags<DAMAGE_NUM_TYPES> &DAMAGE_TYPE_FLAGS_ALL =
    Make_Global<BitFlags<DAMAGE_NUM_TYPES>>(PICK_ADDRESS(0x00A2B9B8, 0x00E280F8));

// gamewindowmanager.cpp
class GameWindowManager;
GameWindowManager *&g_theWindowManager = Make_Global<GameWindowManager *>(PICK_ADDRESS(0x00A2BEF0, 0x00E27F14));

// terrainvisual.cpp
class TerrainVisual;
TerrainVisual *&g_theTerrainVisual = Make_Global<TerrainVisual *>(PICK_ADDRESS(0x00A2C27C, 0x00E25D10));

// flatheightmap.cpp
class FlatHeightMapRenderObjClass;
FlatHeightMapRenderObjClass *&g_theFlatHeightMap =
    Make_Global<FlatHeightMapRenderObjClass *>(PICK_ADDRESS(0x00A3AEA0, 0x00E1AC98));

// heightmap.cpp
class HeightMapRenderObjClass;
HeightMapRenderObjClass *&g_theHeightMap = Make_Global<HeightMapRenderObjClass *>(PICK_ADDRESS(0x00A3AEC8, 0x00E1A634));

// controlbar.cpp
class ControlBar;
ControlBar *&g_theControlBar = Make_Global<ControlBar *>(PICK_ADDRESS(0x00A2B6E0, 0x00E2391C));

// gameinfo.cpp
class GameInfo;
class SkirmishGameInfo;
GameInfo *&g_theGameInfo = Make_Global<GameInfo *>(PICK_ADDRESS(0x00A2C2B8, 0x04CA8B04));
SkirmishGameInfo *&g_theSkirmishGameInfo = Make_Global<SkirmishGameInfo *>(PICK_ADDRESS(0x00A2DF10, 0x04CA9F00));
SkirmishGameInfo *&g_theChallengeGameInfo = Make_Global<SkirmishGameInfo *>(PICK_ADDRESS(0x00A2F374, 0x04CA9E20));

// skirmishbattlehonors.cpp
int &g_rowsToSkip = Make_Global<int>(PICK_ADDRESS(0x00A2FC68, 0x0098D078));

// eva.cpp
class Eva;
Eva *&g_theEva = Make_Global<Eva *>(PICK_ADDRESS(0x00A2C090, 0x04CA9C28));

// ghostobject.cpp
class GhostObjectManager;
GhostObjectManager *&g_theGhostObjectManager = Make_Global<GhostObjectManager *>(PICK_ADDRESS(0x00A2C280, 0x04CA8CD8));

// scriptactions.cpp
#include "scriptactions.h"
ScriptActionsInterface *&g_theScriptActions = Make_Global<ScriptActionsInterface *>(PICK_ADDRESS(0x00A2C140, 0x04CA8740));

// scriptconditions.cpp
#include "scriptconditions.h"
ScriptConditionsInterface *&g_theScriptConditions =
    Make_Global<ScriptConditionsInterface *>(PICK_ADDRESS(0x00A2C10C, 0x04CA86E4));
TransportStatus *&ScriptConditions::s_transportStatuses =
    Make_Global<TransportStatus *>(PICK_ADDRESS(0x00A2C108, 0x04CA86E0));

// campaignmanager.cpp
class CampaignManager;
CampaignManager *&g_theCampaignManager = Make_Global<CampaignManager *>(PICK_ADDRESS(0x00A2C0A0, 0x04CA8784));

// statscollector.cpp
class StatsCollector;
StatsCollector *&g_theStatsCollector = Make_Global<StatsCollector *>(PICK_ADDRESS(0x00A2F324, 0x04CA9C88));

// peerdefs.cpp
class GameSpyInfoInterface;
class GameSpyStagingRoom;
GameSpyInfoInterface *&g_theGameSpyInfo = Make_Global<GameSpyInfoInterface *>(PICK_ADDRESS(0x00A2C67C, 0x04CA9D10));
GameSpyStagingRoom *&g_theGameSpyGame = Make_Global<GameSpyStagingRoom *>(PICK_ADDRESS(0x00A2C680, 0x04CA9D14));

// buddythread.cpp
class GameSpyBuddyMessageQueueInterface;
GameSpyBuddyMessageQueueInterface *&g_theGameSpyBuddyMessageQueue =
    Make_Global<GameSpyBuddyMessageQueueInterface *>(PICK_ADDRESS(0x00A2F350, 0x04CA9D30));

// gamewindowtransitions.cpp
class GameWindowTransitionsHandler;
GameWindowTransitionsHandler *&g_theTransitionHandler =
    Make_Global<GameWindowTransitionsHandler *>(PICK_ADDRESS(0x00A2C094, 0x04CA8A78));

// shell.cpp
class Shell;
Shell *&g_theShell = Make_Global<Shell *>(PICK_ADDRESS(0x00A2C68C, 0x04CA9D74));

// chat.cpp
#include "chat.h"
ARRAY_DEF(PICK_ADDRESS(0x00A2BF78, 0x04CA9BA8), int, g_gameSpyColor, 27);

// selectionxlat.cpp
class SelectionTranslator;
SelectionTranslator *&g_theSelectionTranslator = Make_Global<SelectionTranslator *>(PICK_ADDRESS(0x00A2F458, 0x04CAB624));

// headertemplate.cpp
class HeaderTemplateManager;
HeaderTemplateManager *&g_theHeaderTemplateManager =
    Make_Global<HeaderTemplateManager *>(PICK_ADDRESS(0x00A2BEFC, 0x04CA9B04));

// hotkey.cpp
class HotKeyManager;
HotKeyManager *&g_theHotKeyManager = Make_Global<HotKeyManager *>(PICK_ADDRESS(0x00A2C678, 0x04CA8B00));

// disconnectmenu.cpp
class DisconnectMenu;
DisconnectMenu *&g_theDisconnectMenu = Make_Global<DisconnectMenu *>(PICK_ADDRESS(0x00A31DA0, 0x04CAD314));

// w3dbuffermanager.cpp
class W3DBufferManager;
W3DBufferManager *&g_theW3DBufferManager = Make_Global<W3DBufferManager *>(PICK_ADDRESS(0x00A3C2B8, 0x00E1B298));

// w3dvolumetricshadow.cpp
class W3DVolumetricShadowManager;
W3DVolumetricShadowManager *&g_theW3DVolumetricShadowManager =
    Make_Global<W3DVolumetricShadowManager *>(PICK_ADDRESS(0x00A3B0F0, 0x00E1AE90));
#ifdef BUILD_WITH_D3D8
IDirect3DVertexBuffer8 *&g_shadowVertexBufferD3D =
    Make_Global<IDirect3DVertexBuffer8 *>(PICK_ADDRESS(0x00A3B0F4, 0x00E1AE94));
IDirect3DIndexBuffer8 *&g_shadowIndexBufferD3D = Make_Global<IDirect3DIndexBuffer8 *>(PICK_ADDRESS(0x00A3B0F8, 0x00E1AE98));
#endif

// lookatxlat.cpp
class LookAtTranslator;
LookAtTranslator *&g_theLookAtTranslator = Make_Global<LookAtTranslator *>(PICK_ADDRESS(0x00A2F450, 0x04CAB628));

// eva.cpp
class CreditsManager;
CreditsManager *&g_theCredits = Make_Global<CreditsManager *>(PICK_ADDRESS(0x00A2C098, 0x04CA9C44));
