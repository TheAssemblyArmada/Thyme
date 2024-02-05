# Build and link the DLL.
set(GAMEENGINE_INCLUDES
    editor
    game
    game/client
    game/client/draw
    game/client/drawable/update
    game/client/gui
    game/client/gui/controlbar
    game/client/gui/disconnectmenu
    game/client/gui/gadget
    game/client/gui/guicallbacks
    game/client/gui/guicallbacks/menus
    game/client/gui/shell
    game/client/input
    game/client/messagestream
    game/client/shader
    game/client/system
    game/client/system/particlesystem
    game/client/terrain
    game/client/videoplayer
    game/common
    game/common/audio
    game/common/bezier
    game/common/compression
    game/common/ini
    game/common/rts
    game/common/system
    game/common/system/savegame
    game/common/thing
    game/logic/ai
    game/logic/map
    game/logic/object
    game/logic/object/behavior
    game/logic/object/body
    game/logic/object/create
    game/logic/object/collide
    game/logic/object/contain
    game/logic/object/damage
    game/logic/object/die
    game/logic/object/helper
    game/logic/object/update
    game/logic/object/update/aiupdate
    game/logic/object/update/dockupdate
    game/logic/object/upgrade
    game/logic/scriptengine
    game/logic/system
    game/network
    game/network/gamespy
    game/network/gamespy/thread
    platform
    platform/audio
    platform/common
    platform/video
    platform/w3dengine/client
    platform/w3dengine/client/drawable/draw
    platform/w3dengine/client/gui
    platform/w3dengine/client/gui/gadget
    platform/w3dengine/client/gui/guicallbacks
    platform/w3dengine/client/shadow
    platform/w3dengine/client/water
    platform/w3dengine/common
    platform/w3dengine/common/system
    platform/w3dengine/common/thing
    platform/w3dengine/logic
    w3d/lib
    w3d/math
    w3d/renderer
    w3d/saveload
)

set(HOOKER_SRC
    hooker/dllmain.cpp
    hooker/mapview.cpp
    hooker/setupglobals_zh.cpp
)

set(HOOKER_WB_SRC
    hooker/setuphooks_wb.cpp
)

set(HOOKER_GAME_SRC
    hooker/setuphooks_zh.cpp
)

set(GAMEENGINE_SRC
    game/client/crashpreferences.cpp
    game/client/color.cpp
    game/client/credits.cpp
    game/client/display.cpp
    game/client/displaystring.cpp
    game/client/displaystringmanager.cpp
    game/client/draw/tintenvelope.cpp
    game/client/drawable.cpp
    game/client/drawable/update/swayclientupdate.cpp
    game/client/drawgroupinfo.cpp
    game/client/eva.cpp
    game/client/fxlist.cpp
    game/client/gameclient.cpp
    game/client/gametext.cpp
    game/client/globallanguage.cpp
    game/client/gui/animatewindowmanager.cpp
    game/client/gui/challengegenerals.cpp
    game/client/gui/controlbar/controlbar.cpp
    game/client/gui/controlbar/controlbarbeacon.cpp
    game/client/gui/controlbar/controlbarcommand.cpp
    game/client/gui/controlbar/controlbarcommandprocessing.cpp
    game/client/gui/controlbar/controlbarmultiselect.cpp
    game/client/gui/controlbar/controlbarocltimer.cpp
    game/client/gui/controlbar/controlbarobserver.cpp
    game/client/gui/controlbar/controlbarresizer.cpp
    game/client/gui/controlbar/controlbarscheme.cpp
    game/client/gui/controlbar/controlbarstructureinventory.cpp
    game/client/gui/controlbar/controlbarunderconstruction.cpp
    game/client/gui/disconnectmenu/disconnectmenu.cpp
    game/client/gui/gadget/gadgetcheckbox.cpp
    game/client/gui/gadget/gadgetcombobox.cpp
    game/client/gui/gadget/gadgethorizontalslider.cpp
    game/client/gui/gadget/gadgetlistbox.cpp
    game/client/gui/gadget/gadgetprogressbar.cpp
    game/client/gui/gadget/gadgetpushbutton.cpp
    game/client/gui/gadget/gadgetradiobutton.cpp
    game/client/gui/gadget/gadgetstatictext.cpp
    game/client/gui/gadget/gadgettabcontrol.cpp
    game/client/gui/gadget/gadgettextentry.cpp
    game/client/gui/gadget/gadgetverticalslider.cpp
    game/client/gui/gamefont.cpp
    game/client/gui/gamewindow.cpp
    game/client/gui/gamewindowglobal.cpp
    game/client/gui/gamewindowmanager.cpp
    game/client/gui/gamewindowmanagerscript.cpp
    game/client/gui/gamewindowtransitions.cpp
    game/client/gui/gamewindowtransitionsstyles.cpp
    game/client/gui/guicallbacks/controlbarcallback.cpp
    game/client/gui/guicallbacks/controlbarpopupdescription.cpp
    game/client/gui/guicallbacks/diplomacy.cpp
    game/client/gui/guicallbacks/generalsexppoints.cpp
    game/client/gui/guicallbacks/ingamechat.cpp
    game/client/gui/guicallbacks/menus/quitmenu.cpp
    game/client/gui/guicallbacks/skirmishgameoptionsmenu.cpp
    game/client/gui/headertemplate.cpp
    game/client/gui/imemanager.cpp
    game/client/gui/imemanagerinterface.cpp
    game/client/gui/loadscreen.cpp
    game/client/gui/processanimatewindow.cpp
    game/client/gui/shell/shell.cpp
    game/client/gui/shell/shellmenuscheme.cpp
    game/client/gui/windowlayout.cpp
    game/client/gui/windowvideomanager.cpp
    game/client/gui/wininstancedata.cpp
    game/client/ingameui.cpp
    game/client/input/keyboard.cpp
    game/client/input/mouse.cpp
    game/client/languagefilter.cpp
    game/client/line2d.cpp
    game/client/maputil.cpp
    game/client/messagestream/commandxlat.cpp
    game/client/messagestream/guicommandtranslator.cpp
    game/client/messagestream/hintspy.cpp
    game/client/messagestream/hotkey.cpp
    game/client/messagestream/lookatxlat.cpp
    game/client/messagestream/metaevent.cpp
    game/client/messagestream/placeeventtranslator.cpp
    game/client/messagestream/selectionxlat.cpp
    game/client/messagestream/windowxlat.cpp
    game/client/optionpreferences.cpp
    game/client/parabolicease.cpp
    game/client/radiusdecal.cpp
    game/client/selectioninfo.cpp
    game/client/shader/bwfilter.cpp
    game/client/shader/cloudshader.cpp
    game/client/shader/crossfadefilter.cpp
    game/client/shader/defaultfilter.cpp
    game/client/shader/flatshroudshader.cpp
    game/client/shader/flatterrainshader.cpp
    game/client/shader/maskshader.cpp
    game/client/shader/motionblurfilter.cpp
    game/client/shader/roadshader.cpp
    game/client/shader/shroudshader.cpp
    game/client/shader/terrainshader.cpp
    game/client/shadermanager.cpp
    game/client/snow.cpp
    game/client/system/anim2d.cpp
    game/client/system/campaignmanager.cpp
    game/client/system/debugdisplay.cpp
    game/client/system/image.cpp
    game/client/system/particlesystem/particle.cpp
    game/client/system/particlesystem/particleinfo.cpp
    game/client/system/particlesystem/particlesys.cpp
    game/client/system/particlesystem/particlesysinfo.cpp
    game/client/system/particlesystem/particlesysmanager.cpp
    game/client/system/particlesystem/particlesystemplate.cpp
    game/client/system/rayeffect.cpp
    game/client/system/smudge.cpp
    game/client/terrain/terrainroads.cpp
    game/client/terrain/terrainvisual.cpp
    game/client/terraintex.cpp
    game/client/videoplayer/videobuffer.cpp
    game/client/videoplayer/videoplayer.cpp
    game/client/videoplayer/videostream.cpp
    game/client/view.cpp
    game/client/w3dassetmanager.cpp
    game/client/water.cpp
    game/common/audio/audioeventinfo.cpp
    game/common/audio/audioeventrts.cpp
    game/common/audio/audiomanager.cpp
    game/common/audio/audiomisc.cpp
    game/common/audio/audiosettings.cpp
    game/common/audio/gamemusic.cpp
    game/common/audio/gamesounds.cpp
    game/common/bezier/bezfwditerator.cpp
    game/common/bezier/beziersegment.cpp
    game/common/bitflags.cpp
    game/common/commandline.cpp
    game/common/commandlist.cpp
    game/common/compression/compressionmanager.cpp
    game/common/compression/refpack.cpp
    game/common/crc.cpp
    game/common/damagefx.cpp
    game/common/dict.cpp
    game/common/gameengine.cpp
    game/common/gamelod.cpp
    game/common/gamemain.cpp
    game/common/gamemessage.cpp
    game/common/gamemessagelist.cpp
    game/common/globaldata.cpp
    game/common/ini/ini.cpp
    game/common/ini/inidrawgroupinfo.cpp
    game/common/mapobject.cpp
    game/common/messagestream.cpp
    game/common/multiplayersettings.cpp
    game/common/namekeygenerator.cpp
    game/common/randomvalue.cpp
    game/common/recorder.cpp
    game/common/rts/academystats.cpp
    game/common/rts/actionmanager.cpp
    game/common/rts/buildinfo.cpp
    game/common/rts/energy.cpp
    game/common/rts/handicap.cpp
    game/common/rts/missionstats.cpp
    game/common/rts/money.cpp
    game/common/rts/player.cpp
    game/common/rts/playerlist.cpp
    game/common/rts/playertemplate.cpp
    game/common/rts/productionprerequisite.cpp
    game/common/rts/resourcegatheringmanager.cpp
    game/common/rts/science.cpp
    game/common/rts/scorekeeper.cpp
    game/common/rts/sidesinfo.cpp
    game/common/rts/specialpower.cpp
    game/common/rts/team.cpp
    game/common/rts/teamsinfo.cpp
    game/common/skirmishbattlehonors.cpp
    game/common/statemachine.cpp
    game/common/staticnamekey.cpp
    game/common/statscollector.cpp
    game/common/system/archivefile.cpp
    game/common/system/archivefilesystem.cpp
    game/common/system/asciistring.cpp
    game/common/system/buildassistant.cpp
    game/common/system/cachedfileinputstream.cpp
    game/common/system/cdmanager.cpp
    game/common/system/datachunk.cpp
    game/common/system/datachunktoc.cpp
    game/common/system/disabledtypes.cpp
    game/common/system/file.cpp
    game/common/system/filesystem.cpp
    game/common/system/functionlexicon.cpp
    game/common/system/gamememory.cpp
    game/common/system/gamememoryinit.cpp
    game/common/system/gametype.cpp
    game/common/system/geometry.cpp
    game/common/system/kindof.cpp
    game/common/system/localfile.cpp
    game/common/system/localfilesystem.cpp
    game/common/system/memblob.cpp
    game/common/system/memdynalloc.cpp
    game/common/system/mempool.cpp
    game/common/system/mempoolfact.cpp
    game/common/system/quotedprintable.cpp
    game/common/system/radar.cpp
    game/common/system/ramfile.cpp
    game/common/system/registryget.cpp
    game/common/system/savegame/gamestate.cpp
    game/common/system/savegame/gamestatemap.cpp
    game/common/system/snapshot.cpp
    game/common/system/stackdump.cpp
    game/common/system/streamingarchivefile.cpp
    game/common/system/subsysteminterface.cpp
    game/common/system/unicodestring.cpp
    game/common/system/upgrade.cpp
    game/common/system/xfer.cpp
    game/common/system/xfercrc.cpp
    game/common/terraintypes.cpp
    game/common/thing/module.cpp
    game/common/thing/modulefactory.cpp
    game/common/thing/moduleinfo.cpp
    game/common/thing/thing.cpp
    game/common/thing/thingfactory.cpp
    game/common/thing/thingtemplate.cpp
    game/common/userpreferences.cpp
    game/common/version.cpp
    game/copyprotect.cpp
    game/crashwrapper.cpp
    game/force_nocd.cpp
    game/logic/ai/ai.cpp
    game/logic/ai/aigroup.cpp
    game/logic/ai/aiguard.cpp
    game/logic/ai/aipathfind.cpp
    game/logic/ai/aiplayer.cpp
    game/logic/ai/aistates.cpp
    game/logic/ai/squad.cpp
    game/logic/ai/turretai.cpp
    game/logic/map/polygontrigger.cpp
    game/logic/map/sideslist.cpp
    game/logic/map/terrainlogic.cpp
    game/logic/object/armor.cpp
    game/logic/object/armortemplateset.cpp
    game/logic/object/behavior/autohealbehavior.cpp
    game/logic/object/behavior/behaviormodule.cpp
    game/logic/object/behavior/bridgebehavior.cpp
    game/logic/object/behavior/bridgetowerbehavior.cpp
    game/logic/object/behavior/overchargebehavior.cpp
    game/logic/object/behavior/rebuildholebehavior.cpp
    game/logic/object/collide/collidemodule.cpp
    game/logic/object/collide/squishcollide.cpp
    game/logic/object/create/veterancygaincreate.cpp
    game/logic/object/create/createmodule.cpp
    game/logic/object/damage/damagemodule.cpp
    game/logic/object/die/diemodule.cpp
    game/logic/object/die/specialpowercompletiondie.cpp
    game/logic/object/experiencetracker.cpp
    game/logic/object/firingtracker.cpp
    game/logic/object/ghostobject.cpp
    game/logic/object/helper/objectdefectionhelper.cpp
    game/logic/object/helper/objecthelper.cpp
    game/logic/object/helper/objectrepulsorhelper.cpp
    game/logic/object/helper/objectsmchelper.cpp
    game/logic/object/helper/objectweaponstatushelper.cpp
    game/logic/object/helper/statusdamagehelper.cpp
    game/logic/object/helper/subdualdamagehelper.cpp
    game/logic/object/helper/tempweaponbonushelper.cpp
    game/logic/object/locomotor.cpp
    game/logic/object/object.cpp
    game/logic/object/objectcreationlist.cpp
    game/logic/object/objecttypes.cpp
    game/logic/object/partitionmanager.cpp
    game/logic/object/simpleobjectiterator.cpp
    game/logic/object/update/aiupdate.cpp
    game/logic/object/update/aiupdate/dozeraiupdate.cpp
    game/logic/object/update/assistedtargetingupdate.cpp
    game/logic/object/update/autodepositupdate.cpp
    game/logic/object/update/battleplanupdate.cpp
    game/logic/object/update/dockupdate/dockupdate.cpp
    game/logic/object/update/dockupdate/supplywarehousedockupdate.cpp
    game/logic/object/update/laserupdate.cpp
    game/logic/object/update/mobmemberslavedupdate.cpp
    game/logic/object/update/oclupdate.cpp
    game/logic/object/update/physicsupdate.cpp
    game/logic/object/update/projectilestreamupdate.cpp
    game/logic/object/update/specialabilityupdate.cpp
    game/logic/object/update/specialpowerupdatemodule.cpp
    game/logic/object/update/stickybombupdate.cpp
    game/logic/object/update/toppleupdate.cpp
    game/logic/object/update/updatemodule.cpp
    game/logic/object/upgrade/powerplantupgrade.cpp
    game/logic/object/upgrade/radarupgrade.cpp
    game/logic/object/upgrade/upgrademodule.cpp
    game/logic/object/weapon.cpp
    game/logic/object/weaponset.cpp
    game/logic/object/weapontemplateset.cpp
    game/logic/scriptengine/script.cpp
    game/logic/scriptengine/scriptaction.cpp
    game/logic/scriptengine/scriptactions.cpp
    game/logic/scriptengine/scriptcondition.cpp
    game/logic/scriptengine/scriptconditions.cpp
    game/logic/scriptengine/scriptengine.cpp
    game/logic/scriptengine/scriptgroup.cpp
    game/logic/scriptengine/scriptlist.cpp
    game/logic/scriptengine/scriptparam.cpp
    game/logic/scriptengine/scripttemplate.cpp
    game/logic/scriptengine/sequentialscript.cpp
    game/logic/scriptengine/victoryconditions.cpp
    game/logic/system/cavesystem.cpp
    game/logic/system/cratesystem.cpp
    game/logic/system/damage.cpp
    game/logic/system/gamelogic.cpp
    game/logic/system/gamelogicdispatch.cpp
    game/logic/system/rankinfo.cpp
    game/network/filetransfer.cpp
    game/network/framemetrics.cpp
    game/network/gameinfo.cpp
    game/network/gamemessageparser.cpp
    game/network/gamespy/chat.cpp
    game/network/gamespy/peerdefs.cpp
    game/network/gamespy/stagingroomgameinfo.cpp
    game/network/gamespy/thread/buddythread.cpp
    game/network/gamespy/thread/gameresultsthread.cpp
    game/network/gamespy/thread/threadutils.cpp
    game/network/ipenumeration.cpp
    game/network/lanapi.cpp
    game/network/lanapiinterface.cpp
    game/network/network.cpp
    game/network/networkutil.cpp
    game/network/transport.cpp
    game/network/udp.cpp
    platform/fpusetting.cpp
    platform/standardfile.cpp
    platform/w3dengine/client/baseheightmap.cpp
    platform/w3dengine/client/camerashakesystem.cpp
    platform/w3dengine/client/drawable/draw/w3ddebrisdraw.cpp
    platform/w3dengine/client/drawable/draw/w3ddefaultdraw.cpp
    platform/w3dengine/client/drawable/draw/w3ddependencymodeldraw.cpp
    platform/w3dengine/client/drawable/draw/w3dlaserdraw.cpp
    platform/w3dengine/client/drawable/draw/w3dmodeldraw.cpp
    platform/w3dengine/client/drawable/draw/w3doverlordaircraftdraw.cpp
    platform/w3dengine/client/drawable/draw/w3doverlordtankdraw.cpp
    platform/w3dengine/client/drawable/draw/w3doverlordtruckdraw.cpp
    platform/w3dengine/client/drawable/draw/w3dpolicecardraw.cpp
    platform/w3dengine/client/drawable/draw/w3dprojectilestreamdraw.cpp
    platform/w3dengine/client/drawable/draw/w3dpropdraw.cpp
    platform/w3dengine/client/drawable/draw/w3dropedraw.cpp
    platform/w3dengine/client/drawable/draw/w3dsciencemodeldraw.cpp
    platform/w3dengine/client/drawable/draw/w3dsupplydraw.cpp
    platform/w3dengine/client/drawable/draw/w3dtankdraw.cpp
    platform/w3dengine/client/drawable/draw/w3dtanktruckdraw.cpp
    platform/w3dengine/client/drawable/draw/w3dtracerdraw.cpp
    platform/w3dengine/client/drawable/draw/w3dtreedraw.cpp
    platform/w3dengine/client/drawable/draw/w3dtruckdraw.cpp
    platform/w3dengine/client/flatheightmap.cpp
    platform/w3dengine/client/gui/gadget/w3dcheckbox.cpp
    platform/w3dengine/client/gui/gadget/w3dcombobox.cpp
    platform/w3dengine/client/gui/gadget/w3dhorizontalslider.cpp
    platform/w3dengine/client/gui/gadget/w3dlistbox.cpp
    platform/w3dengine/client/gui/gadget/w3dprogressbar.cpp
    platform/w3dengine/client/gui/gadget/w3dpushbutton.cpp
    platform/w3dengine/client/gui/gadget/w3dradiobutton.cpp
    platform/w3dengine/client/gui/gadget/w3dstatictext.cpp
    platform/w3dengine/client/gui/gadget/w3dtabcontrol.cpp
    platform/w3dengine/client/gui/gadget/w3dtextentry.cpp
    platform/w3dengine/client/gui/gadget/w3dverticalslider.cpp
    platform/w3dengine/client/gui/guicallbacks/w3dcontrolbar.cpp
    platform/w3dengine/client/gui/w3dgamefont.cpp
    platform/w3dengine/client/gui/w3dgamewindow.cpp
    platform/w3dengine/client/gui/w3dgamewindowmanager.cpp
    platform/w3dengine/client/heightmap.cpp
    platform/w3dengine/client/shadow/w3dbuffermanager.cpp
    platform/w3dengine/client/shadow/w3dprojectedshadow.cpp
    platform/w3dengine/client/shadow/w3dshadow.cpp
    platform/w3dengine/client/shadow/w3dvolumetricshadow.cpp
    platform/w3dengine/client/tiledata.cpp
    platform/w3dengine/client/w3dbibbuffer.cpp
    platform/w3dengine/client/w3dbridgebuffer.cpp
    platform/w3dengine/client/w3ddebugdisplay.cpp
    platform/w3dengine/client/w3ddebugicons.cpp
    platform/w3dengine/client/w3ddisplay.cpp
    platform/w3dengine/client/w3ddisplaystring.cpp
    platform/w3dengine/client/w3ddisplaystringmanager.cpp
    platform/w3dengine/client/w3ddynamiclight.cpp
    platform/w3dengine/client/w3dfilesystem.cpp
    platform/w3dengine/client/w3dgameclient.cpp
    platform/w3dengine/client/w3dgamelogic.cpp
    platform/w3dengine/client/w3dingameui.cpp
    platform/w3dengine/client/w3dmouse.cpp
    platform/w3dengine/client/w3dparticlesys.cpp
    platform/w3dengine/client/w3dpoly.cpp
    platform/w3dengine/client/w3dpropbuffer.cpp
    platform/w3dengine/client/w3droadbuffer.cpp
    platform/w3dengine/client/w3dscene.cpp
    platform/w3dengine/client/w3dshroud.cpp
    platform/w3dengine/client/w3dsmudge.cpp
    platform/w3dengine/client/w3dsnow.cpp
    platform/w3dengine/client/w3dstatuscircle.cpp
    platform/w3dengine/client/w3dterrainbackground.cpp
    platform/w3dengine/client/w3dterraintracks.cpp
    platform/w3dengine/client/w3dterrainvisual.cpp
    platform/w3dengine/client/w3dtreebuffer.cpp
    platform/w3dengine/client/w3dview.cpp
    platform/w3dengine/client/w3dwaypointbuffer.cpp
    platform/w3dengine/client/water/w3dwater.cpp
    platform/w3dengine/client/water/w3dwatertracks.cpp
    platform/w3dengine/client/win32mouse.cpp
    platform/w3dengine/client/worldheightmap.cpp
    platform/w3dengine/common/system/w3dfunctionlexicon.cpp
    platform/w3dengine/common/thing/w3dmodulefactory.cpp
    platform/w3dengine/common/win32bigfile.cpp
    platform/w3dengine/common/win32bigfilesystem.cpp
    platform/w3dengine/common/win32gameengine.cpp
    platform/w3dengine/common/win32localfile.cpp
    platform/w3dengine/common/win32localfilesystem.cpp
    platform/w3dengine/logic/w3dghostobject.cpp
    platform/w3dengine/logic/w3dterrainlogic.cpp
    w3d/lib/buff.cpp
    w3d/lib/bufffile.cpp
    w3d/lib/chunkio.cpp
    w3d/lib/cpudetect.cpp
    w3d/lib/crcengine.cpp
    w3d/lib/critsection.cpp
    w3d/lib/ffactory.cpp
    w3d/lib/filestraw.cpp
    w3d/lib/gcd_lcm.cpp
    w3d/lib/hash.cpp
    w3d/lib/iniclass.cpp
    w3d/lib/mpu.cpp
    w3d/lib/multilist.cpp
    w3d/lib/mutex.cpp
    w3d/lib/nstrdup.cpp
    w3d/lib/random.cpp
    w3d/lib/rawfile.cpp
    w3d/lib/rcfile.cpp
    w3d/lib/readline.cpp
    w3d/lib/realcrc.cpp
    w3d/lib/refcount.cpp
    w3d/lib/straw.cpp
    w3d/lib/systimer.cpp
    w3d/lib/targa.cpp
    w3d/lib/thread.cpp
    w3d/lib/threadtrack.cpp
    w3d/lib/wwfile.cpp
    w3d/lib/wwstring.cpp
    w3d/lib/xstraw.cpp
    w3d/math/aabox.cpp
    w3d/math/colmath.cpp
    w3d/math/colmathaabox.cpp
    w3d/math/colmathaabtri.cpp
    w3d/math/colmathaaplane.cpp
    w3d/math/colmathfrustum.cpp
    w3d/math/colmathline.cpp
    w3d/math/colmathobbobb.cpp
    w3d/math/colmathobbtri.cpp
    w3d/math/colmathplane.cpp
    w3d/math/cullsys.cpp
    w3d/math/frustum.cpp
    w3d/math/gamemath.cpp
    w3d/math/lineseg.cpp
    w3d/math/matrix3.cpp
    w3d/math/matrix3d.cpp
    w3d/math/matrix4.cpp
    w3d/math/quat.cpp
    w3d/math/tri.cpp
    w3d/math/v3_rnd.cpp
    w3d/math/vector4.cpp
    w3d/math/vp.cpp
    w3d/renderer/aabtree.cpp
    w3d/renderer/aabtreebuilder.cpp
    w3d/renderer/animobj.cpp
    w3d/renderer/assetmgr.cpp
    w3d/renderer/bitmaphandler.cpp
    w3d/renderer/bmp2d.cpp
    w3d/renderer/boxrobj.cpp
    w3d/renderer/bwrender.cpp
    w3d/renderer/camera.cpp
    w3d/renderer/colorspace.cpp
    w3d/renderer/coltest.cpp
    w3d/renderer/composite.cpp
    w3d/renderer/dazzle.cpp
    w3d/renderer/ddsfile.cpp
    w3d/renderer/dx8caps.cpp
    w3d/renderer/dx8fvf.cpp
    w3d/renderer/dx8indexbuffer.cpp
    w3d/renderer/dx8polygonrenderer.cpp
    w3d/renderer/dx8renderer.cpp
    w3d/renderer/dx8texman.cpp
    w3d/renderer/dx8vertexbuffer.cpp
    w3d/renderer/dx8wrapper.cpp
    w3d/renderer/dynamesh.cpp
    w3d/renderer/hanimmgr.cpp
    w3d/renderer/hcanim.cpp
    w3d/renderer/hlod.cpp
    w3d/renderer/hrawanim.cpp
    w3d/renderer/htree.cpp
    w3d/renderer/htreemgr.cpp
    w3d/renderer/intersec.cpp
    w3d/renderer/light.cpp
    w3d/renderer/lightenv.cpp
    w3d/renderer/line3d.cpp
    w3d/renderer/linegrp.cpp
    w3d/renderer/loaders.cpp
    w3d/renderer/mapper.cpp
    w3d/renderer/matinfo.cpp
    w3d/renderer/matpass.cpp
    w3d/renderer/matrixmapper.cpp
    w3d/renderer/mesh.cpp
    w3d/renderer/meshgeometry.cpp
    w3d/renderer/meshmatdesc.cpp
    w3d/renderer/meshmdl.cpp
    w3d/renderer/missingtexture.cpp
    w3d/renderer/motchan.cpp
    w3d/renderer/nullrobj.cpp
    w3d/renderer/part_buf.cpp
    w3d/renderer/part_emt.cpp
    w3d/renderer/part_ldr.cpp
    w3d/renderer/pivot.cpp
    w3d/renderer/pointgr.cpp
    w3d/renderer/pot.cpp
    w3d/renderer/projector.cpp
    w3d/renderer/render2d.cpp
    w3d/renderer/render2dsentence.cpp
    w3d/renderer/rendobj.cpp
    w3d/renderer/rinfo.cpp
    w3d/renderer/scene.cpp
    w3d/renderer/segline.cpp
    w3d/renderer/seglinerenderer.cpp
    w3d/renderer/shader.cpp
    w3d/renderer/sortingrenderer.cpp
    w3d/renderer/streak.cpp
    w3d/renderer/streakrender.cpp
    w3d/renderer/surfaceclass.cpp
    w3d/renderer/synctextureloadtasklist.cpp
    w3d/renderer/texproject.cpp
    w3d/renderer/texture.cpp
    w3d/renderer/texturebase.cpp
    w3d/renderer/textureloader.cpp
    w3d/renderer/textureloadtask.cpp
    w3d/renderer/textureloadtasklist.cpp
    w3d/renderer/thumbnail.cpp
    w3d/renderer/thumbnailmanager.cpp
    w3d/renderer/vertmaterial.cpp
    w3d/renderer/w3d.cpp
    w3d/renderer/w3d_util.cpp
    w3d/renderer/w3dexclusionlist.cpp
    w3d/renderer/w3dformat.cpp
)

if(USE_STDFS)
    list(APPEND GAMEENGINE_SRC
        platform/stdlocalfilesystem.cpp
    )

    list(APPEND GAME_COMPILE_OPTIONS -DBUILD_WITH_STDFS)
endif()

if(USE_ZLIB)
    list(APPEND GAMEENGINE_SRC
        game/common/compression/zlibcompr.cpp
    )
    list(APPEND GAME_LINK_LIBRARIES ZLIB::ZLIB)
    list(APPEND GAME_COMPILE_OPTIONS -DBUILD_WITH_ZLIB)
endif()

# Game binary only source
set(GAMEENGINE_GAME_SRC
    game/main.cpp
)

set(GAMEENGINE_WB_SRC
    editor/editmain.cpp
)

# Don't try and build the miles sound system when not building to link to the original game.
if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        list(APPEND GAMEENGINE_SRC 
        platform/audio/milesaudiofilecache.cpp
        platform/audio/milesaudiomanager.cpp
        platform/video/binkvideoplayer.cpp
        platform/video/binkvideostream.cpp)
    endif()
endif()

if(USE_FFMPEG)
    list(APPEND GAMEENGINE_SRC
        platform/common/ffmpegfile.cpp 
        platform/audio/ffmpegaudiofilecache.cpp
        platform/video/ffmpegvideoplayer.cpp
        platform/video/ffmpegvideostream.cpp
        platform/video/swvideobuffer.cpp)
    list(APPEND GAMEENGINE_INCLUDES ${FFMPEG_INCLUDE_DIRS})
    list(APPEND GAME_LINK_LIBRARIES ${FFMPEG_LIBRARIES})
    list(APPEND GAME_COMPILE_OPTIONS -DBUILD_WITH_FFMPEG ${FFMPEG_DEFINITIONS})
endif()

if(DINPUT8_FOUND)
    list(APPEND GAMEENGINE_GAME_SRC
        platform/w3dengine/client/win32dikeyboard.cpp
    )
endif()

if(CMAKE_CONFIGURATION_TYPES)
    # Glob all the header files together to add to the project for msvc/xcode.
    # Not ideal as CMake won't notice if you add any until something else prompts a CMake run
    # but this is largely a convenience feature for MSVC/XCode users.
    file(GLOB_RECURSE GAMEENGINE_HEADERS "*.h")
    set(_GAMEENGINE_SOURCES ${GAMEENGINE_SRC} ${GAMEENGINE_GAME_SRC} ${GAMEENGINE_WB_SRC})

    if(NOT STANDALONE)
        list(APPEND _GAMEENGINE_SOURCES ${HOOKER_SRC} ${HOOKER_GAME_SRC} ${HOOKER_WB_SRC})
    endif()

    # Filter out headers from included 3rd party libraries like STLPort or gamemath.
    list(FILTER GAMEENGINE_HEADERS EXCLUDE REGEX "libs/.*")
    list(FILTER GAMEENGINE_HEADERS EXCLUDE REGEX "base/.*")

    foreach(_GAMEENGINE_SOURCE IN ITEMS ${_GAMEENGINE_SOURCES})
        get_filename_component(_GAMEENGINE_SOURCE_PATH "${_GAMEENGINE_SOURCE}" PATH)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" _GAMEENGINE_GROUP_PATH "${_GAMEENGINE_SOURCE_PATH}")
        string(REPLACE "/" "\\" _GAMEENGINE_GROUP_PATH "${_GAMEENGINE_GROUP_PATH}")
        source_group("Source Files\\${_GAMEENGINE_GROUP_PATH}" FILES "${_GAMEENGINE_SOURCE}")
    endforeach()

    foreach(_GAMEENGINE_HEADER IN ITEMS ${GAMEENGINE_HEADERS})
        get_filename_component(_GAMEENGINE_HEADER_PATH "${_GAMEENGINE_HEADER}" PATH)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" _GAMEENGINE_HGROUP_PATH "${_GAMEENGINE_HEADER_PATH}")
        string(REPLACE "/" "\\" _GAMEENGINE_HGROUP_PATH "${_GAMEENGINE_HGROUP_PATH}")
        source_group("Header Files\\${_GAMEENGINE_HGROUP_PATH}" FILES "${_GAMEENGINE_HEADER}")
    endforeach()
endif()

# Gather needed link libraries and compile defintions
list(APPEND GAME_LINK_LIBRARIES base captnlog)

if(USE_SDL2)
    if(TARGET SDL2::SDL2)
        list(APPEND GAME_LINK_LIBRARIES SDL2::SDL2 SDL2::SDL2main)
    else()
        list(APPEND GAME_LINK_LIBRARIES ${SDL2_LIBRARIES})
        list(APPEND GAMEENGINE_INCLUDES ${SDL2_INCLUDE_DIRS})
    endif()
    list(APPEND GAME_COMPILE_OPTIONS -DBUILD_WITH_SDL2)

    list(APPEND GAMEENGINE_SRC
        platform/sdl2/sdl2keybd.cpp
        platform/sdl2/sdl2mouse.cpp
        platform/sdl2/sdl2gameengine.cpp
    )

    list(APPEND GAMEENGINE_INCLUDES
        platform/sdl2/
    )
endif()

if(USE_ALSOFT)
    list(APPEND GAME_LINK_LIBRARIES OpenAL)
    list(APPEND GAME_COMPILE_OPTIONS -DBUILD_WITH_OPENAL)

    list(APPEND GAMEENGINE_SRC 
        platform/audio/alaudiomanager.cpp
        platform/audio/alaudiostream.cpp
    )
endif()

if(USE_FREETYPE)
    list(APPEND GAME_LINK_LIBRARIES Freetype::Freetype)
    list(APPEND GAME_COMPILE_OPTIONS -DBUILD_WITH_FREETYPE)
endif()

if(USE_FONTCONFIG)
    list(APPEND GAME_LINK_LIBRARIES Fontconfig::Fontconfig)
    list(APPEND GAME_COMPILE_OPTIONS -DBUILD_WITH_FONTCONFIG)
endif()

if(USE_GAMEMATH)
    list(APPEND GAME_LINK_LIBRARIES gamemath::gamemath)
    list(APPEND GAME_COMPILE_OPTIONS -DBUILD_WITH_GAMEMATH)
endif()

if(BUILD_DLL)
    # Duplicate that will never link to crashpad for dll injection purposes.
    add_library(crash_handler STATIC game/crashhandler.cpp)
    target_include_directories(crash_handler PRIVATE hooker ${GAMEENGINE_INCLUDES})
    target_link_libraries(crash_handler PRIVATE base captnlog)

    if(ICU_FOUND)
        target_link_libraries(crash_handler PRIVATE ICU::data ICU::i18n ICU::io ICU::tu ICU::uc)
        target_compile_options(crash_handler PRIVATE -DBUILD_WITH_ICU)
    endif()
endif()

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    list(APPEND GAME_LINK_LIBRARIES winmm imm32)

    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        list(APPEND GAME_LINK_LIBRARIES binkstub milesstub)
        list(APPEND GAME_COMPILE_OPTIONS -DBUILD_WITH_BINK -DBUILD_WITH_MILES)
    endif()
endif()

if(ICU_FOUND)
    list(APPEND GAME_LINK_LIBRARIES ICU::data ICU::i18n ICU::io ICU::tu ICU::uc)
    list(APPEND GAME_COMPILE_OPTIONS -DBUILD_WITH_ICU)
endif()

if(D3D8_FOUND)
    list(APPEND GAME_LINK_LIBRARIES d3d8)
endif()

if(DINPUT8_FOUND)
    list(APPEND GAME_LINK_LIBRARIES dxguid dinput8)
    list(APPEND GAME_COMPILE_OPTIONS -DBUILD_WITH_DINPUT)
endif()

# Static library for standalone builds and linking tools.
if(STANDALONE OR BUILD_TOOLS)
    add_library(thyme_lib STATIC ${GAMEENGINE_SRC} ${GAMEENGINE_HEADERS} game/crashhandler.cpp)
    target_include_directories(thyme_lib PUBLIC ${GAMEENGINE_INCLUDES})
    target_link_libraries(thyme_lib PUBLIC ${GAME_LINK_LIBRARIES})
    target_compile_definitions(thyme_lib PUBLIC ${GAME_COMPILE_OPTIONS} "ALLOW_HOOKING=")
    target_compile_definitions(thyme_lib PUBLIC $<$<CONFIG:DEBUG>:GAME_DEBUG> $<$<CONFIG:DEBUG>:GAME_DEBUG_STRUCTS>)

    if(USE_CRASHPAD)
        # Rename the crash handler so it won't conflict with any other installs of crashpad.
        set_target_properties(crashpad_handler PROPERTIES OUTPUT_NAME thymecrashhandler)
        target_compile_options(thyme_lib PUBLIC -DBUILD_WITH_CRASHPAD)
        target_link_libraries(thyme_lib PUBLIC crashpad_client)

        # Force the crashpad handler to be built if we are using crashpad.
        add_dependencies(thyme_lib crashpad_handler)
    endif()
endif()

if(STANDALONE)
    add_executable(thyme ${GAMEENGINE_GAME_SRC})
    target_link_libraries(thyme PRIVATE thyme_lib)
endif()

if(BUILD_DLL)
    include(TargetExports)
    add_library(thyme_dll SHARED ${HOOKER_SRC} ${HOOKER_GAME_SRC} ${GAMEENGINE_GAME_SRC} ${GAMEENGINE_SRC} ${GAMEENGINE_HEADERS})
    target_include_directories(thyme_dll BEFORE PRIVATE ${CMAKE_SOURCE_DIR}/deps/stlport)
    target_include_directories(thyme_dll PRIVATE hooker ${GAMEENGINE_INCLUDES})
    target_compile_definitions(thyme_dll PRIVATE -DGAME_DLL -DTHYME_USE_STLPORT -D_USE_32BIT_TIME_T)
    target_link_libraries(thyme_dll ${GAME_LINK_LIBRARIES} crash_handler)
    target_compile_definitions(thyme_dll PRIVATE ${GAME_COMPILE_OPTIONS} "ALLOW_HOOKING=friend void Setup_Hooks()\;")
    target_compile_definitions(thyme_dll PRIVATE $<$<CONFIG:DEBUG>:GAME_DEBUG>)
    set_target_properties(thyme_dll PROPERTIES OUTPUT_NAME thyme PDB_NAME thymedll)
    target_exports(thyme_dll SYMBOLS 
        Setup_Hooks
    )
endif()

# Note that we don't care about which math lib we link against for worldbuilder.
# Since worldbuilder isn't multiplayer we don't need to worry about sources of indeterminancy
if(BUILD_EDITOR)
    set(CMAKE_MFC_FLAG 2) #Enable MFC
    if(STANDALONE)
        add_executable(thymeedit ${GAMEENGINE_WB_SRC})
        target_link_libraries(thymeedit PRIVATE thyme_lib)
    endif()

    if(BUILD_DLL)
        add_library(thymeedit_dll SHARED ${HOOKER_SRC} ${HOOKER_WB_SRC} ${GAMEENGINE_WB_SRC} ${GAMEENGINE_SRC} ${GAMEENGINE_HEADERS})
        target_include_directories(thymeedit_dll BEFORE PRIVATE ${CMAKE_SOURCE_DIR}/deps/stlport)
        target_include_directories(thymeedit_dll PRIVATE hooker ${GAMEENGINE_INCLUDES})
        target_compile_definitions(thymeedit_dll PRIVATE -DGAME_DLL -DTHYME_USE_STLPORT -DGAME_DEBUG_STRUCTS -DBUILD_EDITOR -D_USE_32BIT_TIME_T)
        target_link_libraries(thymeedit_dll ${GAME_LINK_LIBRARIES} crash_handler)
        target_compile_definitions(thymeedit_dll PRIVATE ${GAME_COMPILE_OPTIONS} "ALLOW_HOOKING=friend void Setup_Hooks()\;")
        target_compile_definitions(thymeedit_dll PRIVATE $<$<CONFIG:DEBUG>:GAME_DEBUG>)
        set_target_properties(thymeedit_dll PROPERTIES OUTPUT_NAME thymeedit PDB_NAME thymeeditdll)
        target_exports(thymeedit_dll SYMBOLS 
            Setup_Hooks
        )
    endif()
endif()

if(BUILD_TOOLS)
    add_subdirectory(tools)
endif()
