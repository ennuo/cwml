#include "ResourcePlan.h"

Ib_DefinePort(CopyOver, void, CThing* thing_to, RPlan* plan, bool enforce_world_pos, CVector<CThingPtr>& things_out, bool remap_uids, void*);
Ib_DefinePort(RPlan_MakeClone, CThing*, RPlan* plan, PWorld* world, const NetworkPlayerID& default_creator, bool remap_uids);
Ib_DefinePort(RPlan_CopyPlan, CP<RPlan>, RPlan* plan, CThing* thing);

CThing* RPlan::MakeClone(RPlan* plan, PWorld* world, const NetworkPlayerID& default_creator, bool remap_uids)
{
    return RPlan_MakeClone(plan, world, default_creator, remap_uids);
}

CP<RPlan> RPlan::CopyPlan(RPlan* plan, CThing* thing)
{
    return RPlan_CopyPlan(plan, thing);
}