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
#include "w3ddefaultdraw.h"
#include "w3ddependencymodeldraw.h"
#include "w3dlaserdraw.h"
#include "w3dmodeldraw.h"
#include "w3doverlordtankdraw.h"
#include "w3dprojectilestreamdraw.h"
#include "w3dtanktruckdraw.h"
#include "w3dtreedraw.h"
/**
 * @brief Initialises W3D specific modules on top of those handled by the base factory class.
 *
 * 0x007755D0
 */
void W3DModuleFactory::Init()
{
    ModuleFactory::Init();
    Add_Module_Internal(W3DDefaultDraw::Friend_New_Module_Instance,
        W3DDefaultDraw::Friend_New_Module_Data,
        DrawModule::Get_Module_Type(),
        "W3DDefaultDraw",
        DrawModule::Get_Interface_Mask());
    Add_Module_Internal((modcreateproc_t)0x00775C90,
        (moddatacreateproc_t)0x0061D5D0,
        DrawModule::Get_Module_Type(),
        "W3DDebrisDraw",
        DrawModule::Get_Interface_Mask());
    Add_Module_Internal(&W3DModelDraw::Friend_New_Module_Instance,
        &W3DModelDraw::Friend_New_Module_Data,
        DrawModule::Get_Module_Type(),
        "W3DModelDraw",
        DrawModule::Get_Interface_Mask());
    Add_Module_Internal(&W3DLaserDraw::Friend_New_Module_Instance,
        &W3DLaserDraw::Friend_New_Module_Data,
        DrawModule::Get_Module_Type(),
        "W3DLaserDraw",
        DrawModule::Get_Interface_Mask());
    Add_Module_Internal(W3DOverlordTankDraw::Friend_New_Module_Instance,
        W3DOverlordTankDraw::Friend_New_Module_Data,
        DrawModule::Get_Module_Type(),
        "W3DOverlordTankDraw",
        DrawModule::Get_Interface_Mask());
    Add_Module_Internal((modcreateproc_t)0x00776680,
        (moddatacreateproc_t)0x00776770,
        DrawModule::Get_Module_Type(),
        "W3DOverlordTruckDraw",
        DrawModule::Get_Interface_Mask());
    Add_Module_Internal((modcreateproc_t)0x007767E0,
        (moddatacreateproc_t)0x007768D0,
        DrawModule::Get_Module_Type(),
        "W3DOverlordAircraftDraw",
        DrawModule::Get_Interface_Mask());
    Add_Module_Internal(W3DProjectileStreamDraw::Friend_New_Module_Instance,
        W3DProjectileStreamDraw::Friend_New_Module_Data,
        DrawModule::Get_Module_Type(),
        "W3DProjectileStreamDraw",
        DrawModule::Get_Interface_Mask());
    Add_Module_Internal((modcreateproc_t)0x00776940,
        (moddatacreateproc_t)0x00776610,
        DrawModule::Get_Module_Type(),
        "W3DPoliceCarDraw",
        DrawModule::Get_Interface_Mask());
    Add_Module_Internal((modcreateproc_t)0x00776B80,
        (moddatacreateproc_t)0x0061D5D0,
        DrawModule::Get_Module_Type(),
        "W3DRopeDraw",
        DrawModule::Get_Interface_Mask());
    Add_Module_Internal((modcreateproc_t)0x00776DC0,
        (moddatacreateproc_t)0x00776EB0,
        DrawModule::Get_Module_Type(),
        "W3DScienceModelDraw",
        DrawModule::Get_Interface_Mask());
    Add_Module_Internal((modcreateproc_t)0x00776C60,
        (moddatacreateproc_t)0x00776D50,
        DrawModule::Get_Module_Type(),
        "W3DSupplyDraw",
        DrawModule::Get_Interface_Mask());
    Add_Module_Internal(W3DDependencyModelDraw::Friend_New_Module_Instance,
        W3DDependencyModelDraw::Friend_New_Module_Data,
        DrawModule::Get_Module_Type(),
        "W3DDependencyModelDraw",
        DrawModule::Get_Interface_Mask());
    Add_Module_Internal(W3DTankDraw::Friend_New_Module_Instance,
        W3DTankDraw::Friend_New_Module_Data,
        DrawModule::Get_Module_Type(),
        "W3DTankDraw",
        DrawModule::Get_Interface_Mask());
    Add_Module_Internal((modcreateproc_t)0x00776520,
        (moddatacreateproc_t)0x00776610,
        DrawModule::Get_Module_Type(),
        "W3DTruckDraw",
        DrawModule::Get_Interface_Mask());
    Add_Module_Internal((modcreateproc_t)0x00777080,
        (moddatacreateproc_t)0x0061D5D0,
        DrawModule::Get_Module_Type(),
        "W3DTracerDraw",
        DrawModule::Get_Interface_Mask());
    Add_Module_Internal(W3DTankTruckDraw::Friend_New_Module_Instance,
        W3DTankTruckDraw::Friend_New_Module_Data,
        DrawModule::Get_Module_Type(),
        "W3DTankTruckDraw",
        DrawModule::Get_Interface_Mask());
    Add_Module_Internal(&W3DTreeDraw::Friend_New_Module_Instance,
        &W3DTreeDraw::Friend_New_Module_Data,
        DrawModule::Get_Module_Type(),
        "W3DTreeDraw",
        DrawModule::Get_Interface_Mask());
    Add_Module_Internal((modcreateproc_t)0x007772B0,
        (moddatacreateproc_t)0x00777390,
        DrawModule::Get_Module_Type(),
        "W3DPropDraw",
        DrawModule::Get_Interface_Mask());
}
