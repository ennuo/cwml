#include <ResourceScript.h>
#include <ResourceSystem.h>
#include <BitUtils.h>
using namespace NVirtualMachine;

void SFunctionLookupTable::Insert(u32 hash, RScript* script, u32 function_idx) // 36
{
    SEntry* it = std::lower_bound(Entries.begin(), Entries.end(), hash, SOrderByFunctionNameHash());
    if (it != Entries.end() && it->FunctionNameHash == hash)
    {
        it->FunctionIdx = function_idx;
        it->Script = script;
    }
    else
    {
        SEntry entry;
        entry.FunctionNameHash = hash;
        entry.Script = script;
        entry.FunctionIdx = function_idx;

        Entries.insert(it, entry);
    }
}

const SFunctionLookupTable::SEntry* SFunctionLookupTable::Lookup(u32 hash) const // 53
{
    SEntry* it = std::lower_bound(Entries.begin(), Entries.end(), hash, SOrderByFunctionNameHash());
    if (it != Entries.end() && it->FunctionNameHash == hash)
        return it;
    return NULL;
}

RScript::RScript(EResourceFlag flags) : CResource(flags),
ClassName(), SuperClassScript(), Modifiers(), TypeReferences(),
FieldReferences(), FunctionReferences(), FieldDefinitions(),
PropertyDefinitions(), FunctionDefinitions(), SharedArguments(),
SharedBytecode(), SharedLineNos(), SharedLocalVariables(), StringAIndices(),
StringATable(), StringWIndices(), StringWTable(), ConstantTable(), DependingGUIDs(),
UpToDateScript(), ExportsFixedUp(), FixedUp(), Finishing(), TotalInstanceSize(),
InstanceLayout(), SuperInstanceLayout(), FunctionLookupTable(), LastFrameLogged(),
TimeLogged(), LastTimeLogged()
{

}

RScript::~RScript()
{
    Unload();
}

void RScript::Unload()
{
    SuperClassScript = NULL;
    TypeReferences.clear();
    FieldReferences.clear();
    FunctionReferences.clear();
    FieldDefinitions.clear();
    PropertyDefinitions.clear();
    FunctionDefinitions.clear();
    SharedArguments.clear();
    SharedBytecode.clear();
    SharedLineNos.clear();
    SharedLocalVariables.clear();
    StringAIndices.clear();
    StringATable.clear();
    StringWIndices.clear();
    StringWTable.clear();
    ConstantTable.clear();
    DependingGUIDs.clear();
    UpToDateScript = NULL;

    ExportsFixedUp = false;
    Finishing = false;
    FixedUp = false;

    TotalInstanceSize = 0;
    InstanceLayout = NULL;
    SuperInstanceLayout = NULL;
    FunctionLookupTable.Reset();
}

ReflectReturn RScript::LoadFinished(const SRevision& revision)
{
    Finishing = false;
    FixedUp = false;

    return REFLECT_OK;
}

CP<RScript> RScript::BlockUntilLoaded(int key)
{
#ifndef WIN32
    // TODO: fix this shit later
    return NULL;
#else
    ScriptSet set;
    CP<RScript> script = LoadResourceByKey<RScript>(key, 0, STREAM_PRIORITY_DEFAULT);
    if (!script->BlockUntilLoaded(set))
        return NULL;
    script->Fixup();
    return script;
#endif
}

bool RScript::BlockUntilLoaded(ScriptSet& set)
{
    if (set.find(this) != set.end())
        return true;
    set.insert(this);
    
    if (FixedUp) return true;

    CResource::BlockUntilLoaded();
    if (!IsLoaded()) return false;

    if (SuperClassScript)
    {
        if (!SuperClassScript->BlockUntilLoaded(set))
            return false;
    }

    for (CTypeReferenceRow* it = TypeReferences.begin(); it != TypeReferences.end(); ++it)
    {
        RScript* script = it->Script;
        if (script == NULL) continue;
        if (!script->BlockUntilLoaded(set))
            return false;
    }

    return true;
}

bool RScript::BlockUntilLoaded()
{
    if (FixedUp) return true;
    ScriptSet set;
    bool loaded = BlockUntilLoaded(set);
    if (loaded) Fixup();
    return loaded;
}

void RScript::ForceFixup() // 384
{
    FixedUp = false;
    ExportsFixedUp = false;
    InstanceLayout = NULL;
    SuperInstanceLayout = NULL;
}

bool RScript::FixupFieldDefinitions() const // 733
{
    u32 offset = 0;
    if (SuperClassScript)
    {
        SuperClassScript->FixupExports();
        offset = SuperClassScript->GetInstanceSize();
    }

    for (int i = 0; i < FieldDefinitions.size(); ++i)
    {
        CFieldDefinitionRow& field = FieldDefinitions[i];
        if (field.Modifiers.IsSet(MT_NATIVE))
        {
            field.InstanceOffset = ~0ul;
            field.FieldNameHash = 0;
        }
        else
        {
            const CTypeReferenceRow* type_ref = GetType(field.TypeReferenceIdx);
            u32 type_size = GetTypeSize(type_ref->GetMachineType());

            offset = RoundUpPow2(offset, type_size);

            field.InstanceOffset = offset;
            field.FieldNameHash = CInstanceLayout::MakeFieldNameHash(LookupStringA(field.NameStringIdx));

            offset += type_size;
        }
    }

    TotalInstanceSize = offset;
    return true;
}

bool RScript::FixupExports() const // 772
{
    if (!ExportsFixedUp)
    {
        FixupFieldDefinitions();
        ExportsFixedUp = true;
    }

    return true;
}

const CFieldDefinitionRow* RScript::LookupField(const char* field_name) const
{
    for (CFieldDefinitionRow* it = FieldDefinitions.begin(); it != FieldDefinitions.end(); ++it)
    {
        if (strcmp(LookupStringA(it->NameStringIdx), field_name) == 0)
            return it;
    }

    if (SuperClassScript != NULL)
        return SuperClassScript->LookupField(field_name);
        
    return NULL;
}

bool RScript::FixupFieldReferences()
{
    for (u32 i = 0; i < FieldReferences.size(); ++i)
    {
        CFieldReferenceRow& field_ref = FieldReferences[i];
        const CTypeReferenceRow* type_ref = GetType(field_ref.TypeReferenceIdx);
        RScript* script = type_ref->Script;
        if (script == NULL)
        {
            // TODO
            // part offset shit
        }
        else
        {
            script->FixupExports();
            const CFieldDefinitionRow* field_def = script->LookupField(LookupStringA(field_ref.NameStringIdx));
            field_ref.Modifiers = field_def->Modifiers;
            field_ref.InstanceOffset = field_def->InstanceOffset;

            if (field_def->Modifiers.IsSet(MT_DIVERGENT))
            {
                // TODO
                // part stuff
            }
        }
    }

    return true;
}

bool RScript::FixupFunctionReferences() // 902
{
    bool errors = false;

    for (u32 i = 0; i < FunctionReferences.size(); ++i)
    {
        CFunctionReferenceRow* function_ref = &FunctionReferences[i];
        const CTypeReferenceRow* type_ref = GetType(function_ref->TypeReferenceIdx);

        CSignature signature;
        signature.SetMangledName(LookupStringA(function_ref->NameStringIdx));
        function_ref->Clear();

        if (!type_ref->Script->LookupFunctionHier(signature, &function_ref->BoundFunction, &function_ref->BoundFunctionNative))
            errors = true;
    }

    return !errors;
}

void RScript::Fixup() // 935
{
    if (FixedUp) return;

    if (SuperClassScript)
    {
        SuperClassScript->Fixup();
        SuperInstanceLayout = SuperClassScript->GetInstanceLayout();
    }

    FixupFieldReferences();
    FixupFunctionReferences();

    FixedUp = true;

    for (u32 i = 0; i < TypeReferences.size(); ++i)
    {
        CP<RScript>& script = TypeReferences[i].Script;
        if (script) script->Fixup();
    }

    InstanceLayout = GetInstanceLayout();
}

void RScript::BuildFunctionLookupTable() const // 979
{
    if (FunctionLookupTable.Built) return;

    if (SuperClassScript)
    {
        SuperClassScript->BuildFunctionLookupTable();
        FunctionLookupTable.Entries = SuperClassScript->FunctionLookupTable.Entries;
        FunctionLookupTable.Built = SuperClassScript->FunctionLookupTable.Built;
    }

    FunctionLookupTable.Reserve(FunctionDefinitions.size());
    for (u32 i = 0; i < FunctionDefinitions.size(); ++i)
    {
        CFunctionDefinitionRow& function_def = FunctionDefinitions[i];
        FunctionLookupTable.Insert(
            CSignature::MakeHash(LookupStringA(function_def.NameStringIdx)),
            (RScript*)this,
            i
        );
    }

    FunctionLookupTable.Built = true;
}

const char* RScript::LookupStringA(u32 string_idx) const
{
    return &StringATable[StringAIndices[string_idx]];
}

const wchar_t* RScript::LookupStringW(u32 i) const // 1012
{
    return StringWTable.begin() + StringWIndices[i];
}

bool RScript::LookupFunction(const CSignature& signature, NVirtualMachine::CScriptFunctionBinding* binding) const
{
    if (!FunctionLookupTable.Built)
        BuildFunctionLookupTable();

    const SFunctionLookupTable::SEntry* entry = FunctionLookupTable.Lookup(signature.GetMangledNameHash());
    if (entry != NULL)
    {
        if (binding != NULL) binding->Set(entry->Script, entry->FunctionIdx);
        return true;
    }

    if (binding != NULL) binding->Clear();
    return false;
}

// temp for now
const CFunctionDefinitionRow* NVirtualMachine::CScriptFunctionBinding::GetFunction() const
{
    if (!Script || FunctionIdx == ~0ul) return NULL;
    return Script->GetFunction(FunctionIdx);
}

bool RScript::LookupFunctionHier(const CSignature& signature, NVirtualMachine::CScriptFunctionBinding* o_binding, NVirtualMachine::NativeFunctionWrapper* o_native_fn) const // 1101
{
    CScriptFunctionBinding binding;
    if (!LookupFunction(signature, &binding)) return false;
    const CFunctionDefinitionRow* function = binding.GetFunction();
    if (function->Modifiers.IsSet(MT_NATIVE))
    {
        bool expect_static = false;
        if (!NVirtualMachine::LookupNativeCall(binding.GetScript()->GetClassName(), signature, &expect_static, o_native_fn))
            return false;
        
        if (expect_static != function->Modifiers.IsSet(MT_STATIC))
            return false;
    }

    o_binding->Set(binding.GetScript(), binding.GetFunctionIndex());
    return true;
}

bool RScript::DerivesFrom(const char* class_name) const
{
    const RScript* script = this;
    while (script)
    {
        if (!script->IsLoaded()) return false;
        if (script->ClassName == class_name)
            return true;
        script = script->SuperClassScript;
    }

    return false;
}

bool RScript::DerivesFrom(const RScript* other) const
{
    if (other == NULL) return false;
    if (other == this) return true;

    const RScript* script = this;
    while (script)
    {
        if (script->ClassName == other->ClassName)
            return true;
        script = script->SuperClassScript;
    }

    return false;
}

CP<CInstanceLayout> RScript::BuildInstanceLayout() // 1199
{
    FixupExports();
    CInstanceLayout* layout = new CInstanceLayout(GetInstanceSize(), GetNumFields());
    PopulateInstanceLayout(layout);
    return layout;
}

void RScript::PopulateInstanceLayout(CInstanceLayout* layout) // 1221
{
    Fixup();
    if (SuperClassScript)
        SuperClassScript->PopulateInstanceLayout(layout);
    for (u32 i = 0; i < FieldDefinitions.size(); ++i)
    {
        const CFieldDefinitionRow* field = GetField(i);
        if (field->Modifiers.IsSet(MT_NATIVE) || field->Modifiers.IsSet(MT_STATIC))
            continue;
        
        const CTypeReferenceRow* type_ref = GetType(field->TypeReferenceIdx);
        layout->AddField(
            LookupStringA(field->NameStringIdx),
            field->Modifiers,
            type_ref->GetFishType(),
            type_ref->GetMachineType(),
            type_ref->DimensionCount,
            type_ref->GetArrayBaseMachineType(),
            field->InstanceOffset,
            field->FieldNameHash
        );
    }
}

CP<CInstanceLayout> RScript::GetInstanceLayout() // 1261
{
    if (InstanceLayout) return InstanceLayout;
    InstanceLayout = BuildInstanceLayout();
    return InstanceLayout;
}