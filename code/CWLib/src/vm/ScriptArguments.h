#pragma once

#include <vm/ScriptVariant.h>

class CScriptArguments {
public:
    inline CScriptArguments() : Arguments(), NumArguments() {}
public:
    inline u32 GetCount() const { return NumArguments; }
    inline const CScriptVariant& GetArgument(u32 i) const { return Arguments[i]; }
    inline void AppendArg(const CScriptVariant& value) { Arguments[NumArguments++] = value; }
private:
    CScriptVariant Arguments[12];
    u32 NumArguments;
};
