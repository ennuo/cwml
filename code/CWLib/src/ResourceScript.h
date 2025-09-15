#pragma once

#include <Resource.h>
#include <ResourceDescriptor.h>
#include <MMString.h>
#include <TextStream.h>
#include <refcount.h>
#include <vector.h>
#include <vm/InstanceLayout.h>
#include <vm/VMTypes.h>
#include <vm/ScriptContext.h>
#include <vm/VMInstruction.h>
#include <vm/NativeRegistry.h>
#include <vm/ScriptFunction.h>

#include <set>
#include <Part.h>

// so annoying
#ifdef WIN32
    #undef GetClassName
#endif

class CTypeReferenceRow {
public:
    inline CTypeReferenceRow() : FishType(), MachineType(), DimensionCount(), ArrayBaseMachineType(), Script(), TypeNameStringIdx()
    {

    }
public:
    inline EBuiltInType GetFishType() const { return (EBuiltInType)FishType; }
    inline EMachineType GetMachineType() const { return (EMachineType)MachineType; }
    inline EMachineType GetArrayBaseMachineType() const { return (EMachineType)ArrayBaseMachineType; }
    inline bool IsArray() const { return DimensionCount != 0; }
    inline bool IsObjectType() const { return !!Script; }
public:
    u8 FishType;
    u8 MachineType;
    u8 DimensionCount;
    u8 ArrayBaseMachineType;
    CP<RScript> Script;
    u32 TypeNameStringIdx;
};

const u32 INVALID_FIELD_OFFSET = ~0ul;


class CFieldReferenceRow {
public:
    inline CFieldReferenceRow() : 
    TypeReferenceIdx(~0ul), NameStringIdx(~0ul),
    Modifiers(), InstanceOffset(~0ul), PartOffset(~0ul),
    PartType(), MachineType(), GetFn(), SetFn()
    {

    }
public:
    u32 TypeReferenceIdx;
    u32 NameStringIdx;
    ModifierBits Modifiers;
    u32 InstanceOffset;
    u32 PartOffset;
    EPartType PartType;
    EMachineType MachineType;
    void (*GetFn)(NVirtualMachine::CScriptContext*, const void*, void*);
    void (*SetFn)(NVirtualMachine::CScriptContext*, void*, const void*);
};

class CFunctionReferenceRow {
public:
    inline void Clear()
    {
        BoundFunction.Clear();
    }
public:
    u32 TypeReferenceIdx;
    u32 NameStringIdx;
    NVirtualMachine::CScriptFunctionBinding BoundFunction;
    NVirtualMachine::NativeFunctionWrapper BoundFunctionNative;
};

class CFieldDefinitionRow {
public:
    inline CFieldDefinitionRow() : Modifiers(), TypeReferenceIdx(~0ul),
    NameStringIdx(~0ul), InstanceOffset(~0ul), FieldNameHash()
    {

    }
public:
    ModifierBits Modifiers;
    u32 TypeReferenceIdx;
    u32 NameStringIdx;
    u32 InstanceOffset;
    u32 FieldNameHash;
};

class CPropertyDefinitionRow {
public:
    ModifierBits Modifiers;
    u32 TypeReferenceIdx;
    u32 NameStringIdx;
    u32 GetFunctionIdx;
    u32 SetFunctionIdx;
};

struct STypeOffset {
    u32 TypeReferenceIdx;
    u32 Offset;
};

struct SLocalVariableDefinitionRow {
    ModifierBits Modifiers;
    u32 TypeReferenceIdx;
    u32 NameStringIdx;
    u32 Offset;
};

class CFunctionDefinitionRow {
public:
    inline u32 GetNumArguments() const { return ArgumentsEnd - ArgumentsBegin; }
    inline u32 GetNumLineNos() const { return LineNosEnd - LineNosBegin; }
    inline u32 GetBytecodeSize() const { return BytecodeEnd - BytecodeBegin; }
    inline u32 GetNumLocalVariables() const { return LocalVariablesEnd - LocalVariablesBegin; }
    inline u32 GetSharedArgumentIdx(u32 i) const { return ArgumentsBegin + i; }
    inline u32 GetSharedLineNoIdx(u32 i) const { return LineNosBegin + i; }
    inline u32 GetSharedLocalVariablesIdx(u32 i) const { return LocalVariablesBegin + i; } 
public:
    ModifierBits Modifiers;
    u32 TypeReferenceIdx;
    u32 NameStringIdx;
    u32 ArgumentsBegin;
    u32 ArgumentsEnd;
    u32 BytecodeBegin;
    u32 BytecodeEnd;
    u32 LineNosBegin;
    u32 LineNosEnd;
    u32 LocalVariablesBegin;
    u32 LocalVariablesEnd;
    u32 StackSize;
};

struct SFunctionLookupTable {
    struct SEntry 
    {
        u32 FunctionNameHash;
        RScript* Script;
        u32 FunctionIdx;
    };

    struct SOrderByFunctionNameHash 
    {
        inline bool operator()(const SEntry& lhs, const SEntry& rhs) const
        {
            return lhs.FunctionNameHash < rhs.FunctionNameHash;
        }

        inline bool operator()(u32 lhs, const SEntry& rhs) const
        {
            return lhs < rhs.FunctionNameHash;
        }

        inline bool operator()(const SEntry& lhs, u32 rhs) const
        {
            return lhs.FunctionNameHash < rhs;
        }
    };

    SFunctionLookupTable() : Entries(), Built() {}
    inline void Reset()
    {
        Entries.clear();
        Built = false;
    }

    inline void Reserve(u32 count)
    {
        Entries.reserve(Entries.size() + count);
    }
    
    void Insert(u32, RScript*, u32);
    const SEntry* Lookup(u32) const;

    CRawVector<SEntry> Entries;
    bool Built;
};

typedef std::set<RScript*> ScriptSet;

class RScript : public CResource {
template <typename R>
friend ReflectReturn Reflect(R& r, RScript& d);
public:
    RScript(EResourceFlag);
    ~RScript();
public:
    virtual void Unload();
    virtual ReflectReturn LoadFinished(const SRevision& revision);
public:
    inline const char* GetClassName() const { return ClassName.c_str(); }

    bool IsValid() const;

    inline const CP<RScript>& GetSuperClassScript() const
    {
        return SuperClassScript;
    }

    bool DerivesFrom(const RScript*) const;
    bool DerivesFrom(const char*) const;
    
    inline bool IsDivergent() const
    {
        return Modifiers.IsSet(MT_DIVERGENT);
    }

    bool IsValidType(u32) const;
    
    inline u32 GetNumTypes() const
    {
        return TypeReferences.size();
    }

    inline const CTypeReferenceRow* GetType(u32 type_idx) const
    {
        return &TypeReferences[type_idx];
    }

    inline u32 GetNumStringAs() const
    {
        return StringAIndices.size();
    }

    inline u32 GetNumStringWs() const
    {
        return StringWIndices.size();
    }

    bool IsValidStringA(u32) const;
    bool IsValidStringW(u32) const;
    const char* LookupStringA(u32) const;
    const wchar_t* LookupStringW(u32) const;

    inline const CRawVector<float>& GetConstantTable() const
    {
        return ConstantTable;
    }

    v4 GetConstantV4(u32) const;
    
    inline u32 GetNumFields() const
    {
        return FieldDefinitions.size();
    }
    
    bool IsValidFieldIdx(u32) const;
    const CFieldDefinitionRow* LookupField(const char*) const;

    inline const CFieldDefinitionRow* GetField(u32 field_idx) const
    {
        return &FieldDefinitions[field_idx];
    }

    inline u32 GetNumProperties() const
    {
        return PropertyDefinitions.size();    
    }

    bool IsValidPropertyIdx(u32) const;

    inline const CPropertyDefinitionRow* GetProperty(u32 property_idx) const
    {
        return &PropertyDefinitions[property_idx];
    }
    
    inline u32 GetNumFunctions() const
    {
        return FunctionDefinitions.size();
    }

    bool IsValidFunctionIdx(u32) const;

    inline const CFunctionDefinitionRow* GetFunction(u32 function_idx) const
    {
        return &FunctionDefinitions[function_idx];
    }

    bool LookupFunction(const CSignature&, NVirtualMachine::CScriptFunctionBinding*) const;
    bool LookupFunctionHier(const CSignature&, NVirtualMachine::CScriptFunctionBinding*, NVirtualMachine::NativeFunctionWrapper*) const;
    
    inline u32 GetNumFieldReferences() const
    {
        return FieldReferences.size();
    }

    const CFieldReferenceRow* GetFieldReference(u32 field_idx) const
    {
        return &FieldReferences[field_idx];
    }

    inline const CFunctionReferenceRow* GetFunctionReference(u32 function_idx) const
    {
        return &FunctionReferences[function_idx];
    }

    bool IsValidFieldReference(u32) const;
    bool IsValidFunctionReference(u32) const;
    const char* GetFieldReferenceName(u32) const;
    const char* GetFunctionReferenceName(u32) const;
    const STypeOffset& GetFunctionArgument(const CFunctionDefinitionRow*, u32) const;
    u16 GetFunctionLineNo(const CFunctionDefinitionRow*, u32) const;
    const SLocalVariableDefinitionRow& GetFunctionLocalVariable(const CFunctionDefinitionRow*, u32) const;
    CParasiticVector<const STypeOffset> GetFunctionArguments(const CFunctionDefinitionRow*) const;
    CParasiticVector<const u16> GetFunctionLineNos(const CFunctionDefinitionRow*) const;
    CParasiticVector<const NVirtualMachine::Instruction> GetFunctionBytecode(const CFunctionDefinitionRow*) const;
    void ForceFixup();
    bool IsInstanceLayoutValid() const;
    void MemberDataHToN(CRawVector<unsigned char>&, const CRawVector<unsigned char>&) const;
    void MemberDataNToH(CRawVector<unsigned char>&, const CRawVector<unsigned char>&) const;
    
    inline u32 GetInstanceSize() const
    {
        return TotalInstanceSize;
    }

    void Fixup();
    inline bool IsFixedUp() const { return FixedUp; }
    CP<CInstanceLayout> GetInstanceLayout();
    void Dump(MMOTextStreamA&);
public:
    // static CP<RScript> BlockUntilLoaded(int key);
public:
    // bool BlockUntilLoaded();
    void UpgradeMemberData(CRawVector<unsigned char>&, const CInstanceLayout*, const CRawVector<unsigned char>&) const;
private:
    RScript(const RScript&);
    RScript& operator=(const RScript&);
    RScript();
private:
    // bool BlockUntilLoaded(ScriptSet&);
    bool FixupExports() const;
    bool FixupFieldDefinitions() const;
    bool FixupFieldReferences();
    bool FixupFunctionReferences();
    CP<CInstanceLayout> BuildInstanceLayout();
    void PopulateInstanceLayout(CInstanceLayout*);
    void BuildFunctionLookupTable() const;
    void MemberDataByteswap(CRawVector<unsigned char>&, const CRawVector<unsigned char>&) const;
private:
    MMString<char> ClassName;
    CP<RScript> SuperClassScript;
    ModifierBits Modifiers;
    CVector<CTypeReferenceRow> TypeReferences;
    CRawVector<CFieldReferenceRow> FieldReferences;
    CVector<CFunctionReferenceRow> FunctionReferences;
    CRawVector<CFieldDefinitionRow> FieldDefinitions;
    CRawVector<CPropertyDefinitionRow> PropertyDefinitions;
    CVector<CFunctionDefinitionRow> FunctionDefinitions;
    CRawVector<STypeOffset> SharedArguments;
    CRawVector<NVirtualMachine::Instruction> SharedBytecode;
    CRawVector<u16> SharedLineNos;
    CRawVector<SLocalVariableDefinitionRow> SharedLocalVariables;
    CRawVector<u32> StringAIndices;
    ByteArray StringATable;
    CRawVector<u32> StringWIndices;
    CRawVector<wchar_t> StringWTable;
    CRawVector<float> ConstantTable;
    CRawVector<CGUID> DependingGUIDs;
    CP<RScript> UpToDateScript;
    mutable bool ExportsFixedUp;
    bool FixedUp;
    bool Finishing;
    mutable u32 TotalInstanceSize;
    CP<CInstanceLayout> InstanceLayout;
    CP<CInstanceLayout> SuperInstanceLayout;
    mutable SFunctionLookupTable FunctionLookupTable;
public:
    u32 LastFrameLogged;
    u64 TimeLogged;
    u64 LastTimeLogged;
};
