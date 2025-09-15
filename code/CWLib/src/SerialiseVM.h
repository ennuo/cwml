#pragma once

#include <vm/VMTypes.h>
#include <SharedSerialise.h>

SERIALISE_TYPE(ScriptObjectUID)
{
    return Reflect(r, d.UID);
}
SERIALISE_END

FORWARD_DECLARE_SERIALISE_TYPE(RScript);
