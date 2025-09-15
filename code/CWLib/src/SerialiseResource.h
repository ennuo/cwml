#pragma once

#include <SharedSerialise.h>
#include <ResourceGuidSubst.h>
#include <ResourceDLC.h>
#include <ResourceScript.h>

SERIALISE_TYPE(SDLCGUID)
{
    ADD(SR_INITIAL, GUID.guid);
    ADD(SR_INITIAL, Flags);
}
SERIALISE_END

SERIALISE_TYPE(SDLCFile)
{
    ADD(SR_INITIAL, Directory);
    ADD(SR_INITIAL, File);
    ADD(SR_INITIAL, ContentID);
    ADD(SR_INITIAL, InGameCommerceID);
    ADD(SR_DLC_CATEGORY, CategoryID);
    ADD(SR_INITIAL, GUIDs);
    ADD(SR_INITIAL, Flags);
}
SERIALISE_END

SERIALISE_TYPE(RDLC)
{
    ADD(SR_INITIAL, GUIDS);
    ADD(SR_INITIAL, Files);
}
SERIALISE_END

SERIALISE_TYPE(RGuidSubst::V)
{
    return ReflectVector(r, d);
}
SERIALISE_END

SERIALISE_TYPE(CGuidSubst)
{
    ADD(SR_INITIAL, From.guid);
    ADD(SR_INITIAL, To.guid);
}
SERIALISE_END

SERIALISE_TYPE(RGuidSubst)
{
    ADD(SR_INITIAL, Substitutions);
}
SERIALISE_END

FORWARD_DECLARE_SERIALISE_TYPE(RScript);

