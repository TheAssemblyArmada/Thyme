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

/**
 * @brief Initialises W3D specific modules on top of those handled by the base factory class.
 *
 * 0x007755D0
 */
void W3DModuleFactory::Init()
{
    ModuleFactory::Init();
    Add_Module_Internal((modcreateproc_t)0x00775D70, (moddatacreateproc_t)0x0061D5D0, MODULE_W3D, "W3DDefaultDraw", 0x400);
    Add_Module_Internal((modcreateproc_t)0x00775C90, (moddatacreateproc_t)0x0061D5D0, MODULE_W3D, "W3DDebrisDraw", 0x400);
    Add_Module_Internal((modcreateproc_t)0x00775E50, (moddatacreateproc_t)0x00775F40, MODULE_W3D, "W3DModelDraw", 0x400);
    Add_Module_Internal((modcreateproc_t)0x00776110, (moddatacreateproc_t)0x007761F0, MODULE_W3D, "W3DLaserDraw", 0x400);
    Add_Module_Internal((modcreateproc_t)0x007763C0, (moddatacreateproc_t)0x007764B0, MODULE_W3D, "W3DOverlordTankDraw", 0x400);
    Add_Module_Internal((modcreateproc_t)0x00776680, (moddatacreateproc_t)0x00776770, MODULE_W3D, "W3DOverlordTruckDraw", 0x400);
    Add_Module_Internal((modcreateproc_t)0x007767E0, (moddatacreateproc_t)0x007768D0, MODULE_W3D, "W3DOverlordAircraftDraw", 0x400);
    Add_Module_Internal((modcreateproc_t)0x00776A30, (moddatacreateproc_t)0x00776B10, MODULE_W3D, "W3DProjectileStreamDraw", 0x400);
    Add_Module_Internal((modcreateproc_t)0x00776940, (moddatacreateproc_t)0x00776610, MODULE_W3D, "W3DPoliceCarDraw", 0x400);
    Add_Module_Internal((modcreateproc_t)0x00776B80, (moddatacreateproc_t)0x0061D5D0, MODULE_W3D, "W3DRopeDraw", 0x400);
    Add_Module_Internal((modcreateproc_t)0x00776DC0, (moddatacreateproc_t)0x00776EB0, MODULE_W3D, "W3DScienceModelDraw", 0x400);
    Add_Module_Internal((modcreateproc_t)0x00776C60, (moddatacreateproc_t)0x00776D50, MODULE_W3D, "W3DSupplyDraw", 0x400);
    Add_Module_Internal((modcreateproc_t)0x00775FB0, (moddatacreateproc_t)0x007760A0, MODULE_W3D, "W3DDependencyModelDraw", 0x400);
    Add_Module_Internal((modcreateproc_t)0x00776260, (moddatacreateproc_t)0x00776350, MODULE_W3D, "W3DTankDraw", 0x400);
    Add_Module_Internal((modcreateproc_t)0x00776520, (moddatacreateproc_t)0x00776610, MODULE_W3D, "W3DTruckDraw", 0x400);
    Add_Module_Internal((modcreateproc_t)0x00777080, (moddatacreateproc_t)0x0061D5D0, MODULE_W3D, "W3DTracerDraw", 0x400);
    Add_Module_Internal((modcreateproc_t)0x00776F20, (moddatacreateproc_t)0x00777010, MODULE_W3D, "W3DTankTruckDraw", 0x400);
    Add_Module_Internal((modcreateproc_t)0x00777160, (moddatacreateproc_t)0x00777240, MODULE_W3D, "W3DTreeDraw", 0x400);
    Add_Module_Internal((modcreateproc_t)0x007772B0, (moddatacreateproc_t)0x00777390, MODULE_W3D, "W3DPropDraw", 0x400);
}
