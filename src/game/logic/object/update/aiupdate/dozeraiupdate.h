/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Dozer AI Update
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once
#include "always.h"
#include "object.h"

enum DozerTask
{
    DOZER_TASK_INVALID = -1,
    DOZER_TASK_BUILD = 0,
    DOZER_TASK_REPAIR,
    DOZER_TASK_FORTIFY,
    DOZER_NUM_TASKS,
};

enum DozerDockPoint
{
    DOZER_DOCK_POINT_START = 0,
    DOZER_DOCK_POINT_ACTION,
    DOZER_DOCK_POINT_END,
    DOZER_NUM_DOCK_POINTS,
};

enum DozerBuildSubTask
{
    DOZER_SELECT_BUILD_DOCK_LOCATION = 0,
    DOZER_MOVING_TO_BUILD_DOCK_LOCATION,
    DOZER_DO_BUILD_AT_DOCK,
};

class DozerAIInterface
{
public:
    virtual void On_Delete() = 0;
    virtual float Get_Repair_Health_Per_Second() const = 0;
    virtual float Get_Bored_Time() const = 0;
    virtual float Get_Bored_Range() const = 0;
    virtual Object *Construct(const ThingTemplate *tmplate, const Coord3D *pos, float f, Player *player, bool b) = 0;
    virtual DozerTask Get_Most_Recent_Command() = 0;
    virtual bool Is_Task_Pending(DozerTask task) = 0;
    virtual ObjectID Get_Task_Target(DozerTask task) = 0;
    virtual bool Is_Any_Task_Pending() = 0;
    virtual DozerTask Get_Current_Task() const = 0;
    virtual void Set_Current_Task(DozerTask task) = 0;
    virtual bool Get_Is_Rebuild() = 0;
    virtual void New_Task(DozerTask task, Object *obj) = 0;
    virtual void Cancel_Task(DozerTask task) = 0;
    virtual void Internal_Task_Complete(DozerTask task) = 0;
    virtual void Internal_Cancel_Task(DozerTask task) = 0;
    virtual void Internal_Task_Complete_Or_Cancelled(DozerTask task) = 0;
    virtual const Coord3D *Get_Dock_Point(DozerTask task, DozerDockPoint point) = 0;
    virtual void Set_Build_Sub_Task(DozerBuildSubTask task) = 0;
    virtual DozerBuildSubTask Get_Build_Sub_Task() = 0;
    virtual bool Can_Accept_New_Repair(Object *obj) = 0;
    virtual void Create_Bridge_Scaffolding(Object *obj) = 0;
    virtual void Remove_Bridge_Scaffolding(Object *obj) = 0;
    virtual void Start_Building_Sound(const AudioEventRTS *sound, ObjectID id) = 0;
    virtual void Finish_Building_Sound() = 0;
};
