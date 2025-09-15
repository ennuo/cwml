#pragma once

#include "refcount.h"
#include "vector.h"
#include "vm/InstanceLayout.h"

class RScript;
class CInstanceLayout;

class CScriptInstance {
public:
    CP<RScript> Script;
    CP<CInstanceLayout> InstanceLayout;
    CBaseVector<unsigned char> MemberVariables;
};