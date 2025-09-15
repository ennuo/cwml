#pragma once

#include <refcount.h>
#include <vector.h>
#include <ReflectionVisitable.h>
#include <vm/VMTypes.h>

struct SFieldLayoutDetails {

    SFieldLayoutDetails() : FieldName(), Modifiers(), FishType(), MachineType(), DimensionCount(), ArrayBaseMachineType(), InstanceOffset() {}
    SFieldLayoutDetails(const char* name, ModifierBits modifiers, EBuiltInType fish_type, EMachineType machine_type, u8 dimension_count, EMachineType array_type, u32 offset);
    
    inline bool operator==(const SFieldLayoutDetails& rhs) const
    {
        return
            StringCompare(FieldName, rhs.FieldName) == 0 &&
            Modifiers == rhs.Modifiers &&
            FishType == rhs.FishType &&
            MachineType == rhs.MachineType &&
            DimensionCount == rhs.DimensionCount &&
            ArrayBaseMachineType == rhs.ArrayBaseMachineType &&
            InstanceOffset == rhs.InstanceOffset;
    }

    inline bool operator!=(const SFieldLayoutDetails& rhs) const
    {
        return !(*this == rhs);
    }

    char FieldName[53];
    s32 Modifiers;
    s32 FishType;
    s32 MachineType;
    u8 DimensionCount;
    s32 ArrayBaseMachineType;
    u32 InstanceOffset;
};

class CInstanceLayout : public CBaseCounted, CReflectionVisitable {
private:
    struct SOrderedFieldName {
        u32 FieldNameHash;
        u32 LayoutIdx;
    };

    struct SSortByFieldNameHash
    {
        inline bool operator()(const SOrderedFieldName& lhs, const SOrderedFieldName& rhs) const
        {
            return lhs.FieldNameHash < rhs.FieldNameHash;
        }

        inline bool operator()(u32 lhs, const SOrderedFieldName& rhs) const
        {
            return lhs < rhs.FieldNameHash;
        }

        inline bool operator()(const SOrderedFieldName& lhs, u32 rhs) const
        {
            return lhs.FieldNameHash < rhs;
        }
    };
public:
    CInstanceLayout(u32 instance_size, u32 num_fields_hint);
public:
    void AddField(const char*, ModifierBits, EBuiltInType, EMachineType, u8, EMachineType, u32, u32);
    u32 GetInstanceSize() const;
    u32 GetNumFields() const;
    const SFieldLayoutDetails* GetFieldByIndex(u32 idx) const;
    const SFieldLayoutDetails* LookupFieldByHash(u32 hash) const;
    const SFieldLayoutDetails* LookupField(const char* name) const;
    bool NeedsGCScan() const;
    inline const CRawVector<u32>& GetObjectHandleOffsets() const { return ObjectHandleOffsets; }
    bool IsEquivalent(const CInstanceLayout& rhs) const;
    static u32 MakeFieldNameHash(const char*);
public:
    ~CInstanceLayout();
public:
    void UpdateInternals();
public:
    CInstanceLayout(const CInstanceLayout& rhs);
    CInstanceLayout& operator=(const CInstanceLayout& rhs);
public:
    CRawVector<SFieldLayoutDetails> FieldLayoutDetailsVec;
    CRawVector<SOrderedFieldName> FieldNameIndex;
    u32 InstanceSize;
    CRawVector<u32> ObjectHandleOffsets;
};