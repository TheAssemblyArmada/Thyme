/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Factory class for modules extended to support w3d specific modules.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dmodulefactory.h"
#include "moduleinfo.h"

/**
 * @brief Initialises W3D specific modules on top of those handled by the base factory class.
 *
 * 0x007755D0
 */
void W3DModuleFactory::Init()
{
    ModuleFactory::Init();
    Add_Module_Internal(
        (modcreateproc_t)0x00775D70, (moddatacreateproc_t)0x0061D5D0, MODULE_DRAW, "W3DDefaultDraw", MODULEINTERFACE_DRAW);
    Add_Module_Internal(
        (modcreateproc_t)0x00775C90, (moddatacreateproc_t)0x0061D5D0, MODULE_DRAW, "W3DDebrisDraw", MODULEINTERFACE_DRAW);
    Add_Module_Internal(
        (modcreateproc_t)0x00775E50, (moddatacreateproc_t)0x00775F40, MODULE_DRAW, "W3DModelDraw", MODULEINTERFACE_DRAW);
    Add_Module_Internal(
        (modcreateproc_t)0x00776110, (moddatacreateproc_t)0x007761F0, MODULE_DRAW, "W3DLaserDraw", MODULEINTERFACE_DRAW);
    Add_Module_Internal((modcreateproc_t)0x007763C0,
        (moddatacreateproc_t)0x007764B0,
        MODULE_DRAW,
        "W3DOverlordTankDraw",
        MODULEINTERFACE_DRAW);
    Add_Module_Internal((modcreateproc_t)0x00776680,
        (moddatacreateproc_t)0x00776770,
        MODULE_DRAW,
        "W3DOverlordTruckDraw",
        MODULEINTERFACE_DRAW);
    Add_Module_Internal((modcreateproc_t)0x007767E0,
        (moddatacreateproc_t)0x007768D0,
        MODULE_DRAW,
        "W3DOverlordAircraftDraw",
        MODULEINTERFACE_DRAW);
    Add_Module_Internal((modcreateproc_t)0x00776A30,
        (moddatacreateproc_t)0x00776B10,
        MODULE_DRAW,
        "W3DProjectileStreamDraw",
        MODULEINTERFACE_DRAW);
    Add_Module_Internal(
        (modcreateproc_t)0x00776940, (moddatacreateproc_t)0x00776610, MODULE_DRAW, "W3DPoliceCarDraw", MODULEINTERFACE_DRAW);
    Add_Module_Internal(
        (modcreateproc_t)0x00776B80, (moddatacreateproc_t)0x0061D5D0, MODULE_DRAW, "W3DRopeDraw", MODULEINTERFACE_DRAW);
    Add_Module_Internal((modcreateproc_t)0x00776DC0,
        (moddatacreateproc_t)0x00776EB0,
        MODULE_DRAW,
        "W3DScienceModelDraw",
        MODULEINTERFACE_DRAW);
    Add_Module_Internal(
        (modcreateproc_t)0x00776C60, (moddatacreateproc_t)0x00776D50, MODULE_DRAW, "W3DSupplyDraw", MODULEINTERFACE_DRAW);
    Add_Module_Internal((modcreateproc_t)0x00775FB0,
        (moddatacreateproc_t)0x007760A0,
        MODULE_DRAW,
        "W3DDependencyModelDraw",
        MODULEINTERFACE_DRAW);
    Add_Module_Internal(
        (modcreateproc_t)0x00776260, (moddatacreateproc_t)0x00776350, MODULE_DRAW, "W3DTankDraw", MODULEINTERFACE_DRAW);
    Add_Module_Internal(
        (modcreateproc_t)0x00776520, (moddatacreateproc_t)0x00776610, MODULE_DRAW, "W3DTruckDraw", MODULEINTERFACE_DRAW);
    Add_Module_Internal(
        (modcreateproc_t)0x00777080, (moddatacreateproc_t)0x0061D5D0, MODULE_DRAW, "W3DTracerDraw", MODULEINTERFACE_DRAW);
    Add_Module_Internal(
        (modcreateproc_t)0x00776F20, (moddatacreateproc_t)0x00777010, MODULE_DRAW, "W3DTankTruckDraw", MODULEINTERFACE_DRAW);
    Add_Module_Internal(
        (modcreateproc_t)0x00777160, (moddatacreateproc_t)0x00777240, MODULE_DRAW, "W3DTreeDraw", MODULEINTERFACE_DRAW);
    Add_Module_Internal(
        (modcreateproc_t)0x007772B0, (moddatacreateproc_t)0x00777390, MODULE_DRAW, "W3DPropDraw", MODULEINTERFACE_DRAW);
}
