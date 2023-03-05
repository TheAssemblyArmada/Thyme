/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Squad
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
#include "mempoolobj.h"
#include "object.h"

class Squad : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(Squad)

public:
    Squad() {}
    virtual ~Squad() override {}
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    bool Is_On_Squad(const Object *obj) const;
    void Clear_Squad();
    void Add_Object(Object *obj);
    void Add_Object_ID(ObjectID id);
    void Remove_Object(Object *obj);
    std::vector<Object *> &Get_All_Objects();
    std::vector<Object *> &Get_Live_Objects();
    int Get_Size_Of_Group() const;
    void Squad_From_Team(const Team *from_team, bool clear_squad_first);
    void Squad_From_AI_Group(const AIGroup *from_ai_group, bool clear_squad_first);
    void AI_Group_From_Squad(AIGroup *ai_group_to_fill);

private:
    std::vector<ObjectID> m_objectIDs;
    std::vector<Object *> m_objectsCached;
};
