#include <Serialise.h>
#include <SharedSerialise.h>
#include <ResourceScript.h>

class CScriptObjectAudioHandle {};

SERIALISE_TYPE(CScriptObjectAudioHandle) 
{
    return REFLECT_OK;
} 
SERIALISE_END

SERIALISE_TYPE(CP<CInstanceLayout>)
{
    return REFLECT_NOT_IMPLEMENTED;
}
SERIALISE_END

SERIALISE_TYPE(ModifierBits)
{
    return Reflect(r, *(u32*)&d);
}
SERIALISE_END

SERIALISE_TYPE(CTypeReferenceRow)
{
    ADD(SR_INITIAL, MachineType);
    ADD(SR_INITIAL, FishType);
    ADD(SR_INITIAL, DimensionCount);
    ADD(SR_INITIAL, ArrayBaseMachineType);
    ADD(SR_INITIAL, Script);
    ADD(SR_INITIAL, TypeNameStringIdx);

}
SERIALISE_END

SERIALISE_TYPE(CFieldReferenceRow)
{
    ADD(SR_INITIAL, TypeReferenceIdx);
    ADD(SR_INITIAL, NameStringIdx);
}
SERIALISE_END

SERIALISE_TYPE(CFunctionReferenceRow)
{
    ADD(SR_INITIAL, TypeReferenceIdx);
    ADD(SR_INITIAL, NameStringIdx);
}
SERIALISE_END

SERIALISE_TYPE(CFieldDefinitionRow)
{
    ADD(SR_INITIAL, Modifiers);
    ADD(SR_INITIAL, TypeReferenceIdx);
    ADD(SR_INITIAL, NameStringIdx);
}
SERIALISE_END

SERIALISE_TYPE(CPropertyDefinitionRow)
{
    ADD(SR_INITIAL, Modifiers);
    ADD(SR_INITIAL, TypeReferenceIdx);
    ADD(SR_INITIAL, NameStringIdx);
    ADD(SR_INITIAL, GetFunctionIdx);
    ADD(SR_INITIAL, SetFunctionIdx);
}
SERIALISE_END

SERIALISE_TYPE(CFunctionDefinitionRow)
{
    if (r.GetRevision() < SR_SHARED_SCRIPTS) 
        return REFLECT_FORMAT_TOO_OLD;
    
    ADD(SR_INITIAL, Modifiers);
    ADD(SR_INITIAL, TypeReferenceIdx);
    ADD(SR_INITIAL, NameStringIdx);
    ADD(SR_INITIAL, ArgumentsBegin);
    ADD(SR_INITIAL, ArgumentsEnd);
    ADD(SR_INITIAL, BytecodeBegin);
    ADD(SR_INITIAL, BytecodeEnd);
    ADD(SR_INITIAL, LineNosBegin);
    ADD(SR_INITIAL, LineNosEnd);
    ADD(SR_INITIAL, LocalVariablesBegin);
    ADD(SR_INITIAL, LocalVariablesEnd);
    ADD(SR_INITIAL, StackSize);
}
SERIALISE_END

SERIALISE_TYPE(STypeOffset)
{
    ADD(SR_INITIAL, TypeReferenceIdx);
    ADD(SR_INITIAL, Offset);
}
SERIALISE_END

SERIALISE_TYPE(NVirtualMachine::Instruction)
{
    return Reflect(r, d.Bits);
}
SERIALISE_END

SERIALISE_TYPE(SLocalVariableDefinitionRow)
{
    ADD(SR_INITIAL, Modifiers);
    ADD(SR_INITIAL, TypeReferenceIdx);
    ADD(SR_INITIAL, NameStringIdx);
    ADD(SR_INITIAL, Offset);
}
SERIALISE_END

SERIALISE_TYPE(RScript)
{
    if (r.GetRevision() < SR_SHARED_SCRIPTS) 
        return REFLECT_FORMAT_TOO_OLD;

    ADD(SR_INITIAL, UpToDateScript);
    ADD(SR_INITIAL, ClassName);
    ADD(SR_INITIAL, SuperClassScript);
    ADD(SR_INITIAL, Modifiers);
    ADD(SR_INITIAL, TypeReferences);
    ADD(SR_INITIAL, FieldReferences);
    ADD(SR_INITIAL, FunctionReferences);
    ADD(SR_INITIAL, FieldDefinitions);
    ADD(SR_INITIAL, PropertyDefinitions);
    ADD(SR_INITIAL, FunctionDefinitions);
    ADD(SR_INITIAL, SharedArguments);
    ADD(SR_INITIAL, SharedBytecode);
    ADD(SR_INITIAL, SharedLineNos);
    ADD(SR_INITIAL, SharedLocalVariables);
    ADD(SR_INITIAL, StringAIndices);
    ADD(SR_INITIAL, StringATable);
    ADD(SR_INITIAL, StringWIndices);
    ADD(SR_INITIAL, StringWTable);
    ADD(SR_INITIAL, ConstantTable);
    ADD(SR_INITIAL, DependingGUIDs);
}
SERIALISE_END

INSTANTIATE_SERIALISE_TYPE(RScript);