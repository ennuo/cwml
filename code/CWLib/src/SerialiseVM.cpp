#include <Serialise.h>
#include <SharedSerialise.h>
#include <ResourceScript.h>
#include <vm/VMInstruction.h>

using namespace NVirtualMachine;


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

#include <DebugLog.h>

SERIALISE_TYPE(Instruction)
{    
    rv = Reflect(r, d.Bits);
#ifdef WIN32
    if (rv == REFLECT_OK && r.GetLoading())
    {


        EInstructionType op = (EInstructionType)(d.Bits & 0xff);

        // We actually want to keep the bits in big endian format,
        // since we actually just need to swap the bits of the
        // individual instruction components.
        d.Bits = _byteswap_uint64(d.Bits);

        switch (gInstructionClasses[op])
        {
            case IC_LOAD_CONST:
            {
                switch (op)
                {
                    default:
                    {
                        d.LoadConst.ConstantIdx = _byteswap_ulong(d.LoadConst.ConstantIdx);
                        d.LoadConst.DstIdx = _byteswap_ushort(d.LoadConst.DstIdx);
                        break;
                    }
                }

                // MMLog("LC %08x, %08x, %08x\n", d.LoadConst.ConstantIdx, d.LoadConst.DstIdx, d.LoadConst.Op);
                break;
            }

            case IC_UNARY:
            {
                d.Unary.SrcIdx = _byteswap_ushort(d.Unary.SrcIdx);
                d.Unary.DstIdx = _byteswap_ushort(d.Unary.DstIdx);
                break;
            }

            case IC_BINARY:
            {
                d.Binary.SrcBIdx = _byteswap_ushort(d.Binary.SrcBIdx);
                d.Binary.SrcAIdx = _byteswap_ushort(d.Binary.SrcAIdx);
                d.Binary.DstIdx = _byteswap_ushort(d.Binary.DstIdx);
                break;
            }
            
            case IC_SET_MEMBER:
            {
                d.SetMember.FieldRef = _byteswap_ushort(d.SetMember.FieldRef);
                d.SetMember.BaseIdx = _byteswap_ushort(d.SetMember.BaseIdx);
                d.SetMember.SrcIdx = _byteswap_ushort(d.SetMember.SrcIdx);
                // MMLog("SET_MEMBER %08x, %08x, %08x, %08x, %08x\n", d.SetMember.FieldRef, d.SetMember.BaseIdx, d.SetMember.SrcIdx, d.SetMember.Type, d.SetMember.Op);
                break;
            }

            case IC_WRITE:
            {
                d.Write.SrcIdx = _byteswap_ushort(d.Write.SrcIdx);
                break;
            }

            case IC_ARG:
            {
                d.Arg.ArgIdx = _byteswap_ushort(d.Arg.ArgIdx);
                d.Arg.SrcIdx = _byteswap_ushort(d.Arg.SrcIdx);
                // MMLog("ARG %08x, %08x, %08x\n", d.Arg.ArgIdx, d.Arg.SrcIdx, d.Arg.Op);
                break;
            }
            
            case IC_CALL:
            {
                d.Call.CallIdx = _byteswap_ushort(d.Call.CallIdx);
                d.Call.DstIdx = _byteswap_ushort(d.Call.DstIdx);
                // MMLog("CALL %08x, %08x, %08x\n", d.Call.CallIdx, d.Call.DstIdx, d.Call.Op);
                break;
            }

            case IC_RETURN:
            {
                d.Return.SrcIdx = _byteswap_ushort(d.Return.SrcIdx);
                break;
            }

            case IC_BRANCH:
            {
                d.Branch.BranchOffset = _byteswap_ulong(d.Branch.BranchOffset);
                d.Branch.SrcIdx = _byteswap_ushort(d.Branch.SrcIdx);
                break;
            }
        }
    }
#endif


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