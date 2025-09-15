#pragma once

#include "Part.h"
#include "vm/InstanceLayout.h"
#include "vm/ScriptInstance.h"

class PScript : public CPart {
public:
    inline const CScriptInstance& GetInstance() const { return ScriptInstance; }
    inline const CP<RScript>& GetScript() const { return ScriptInstance.Script; }
public:
    template <typename T> 
    T GetValue(char const* member, T default_value)
    {
        CP<CInstanceLayout>& layout = ScriptInstance.InstanceLayout;
        if (!layout) return default_value;
    
        SFieldLayoutDetails* field = layout->LookupField(member);
        if (field == NULL) return default_value;
    
        CBaseVector<unsigned char>& data = ScriptInstance.MemberVariables;
        if (data.size() >= field->InstanceOffset + sizeof(T))
            return *(T*)(data.begin() + field->InstanceOffset);
        
        return default_value;
    }
private:
    CScriptInstance ScriptInstance;
};