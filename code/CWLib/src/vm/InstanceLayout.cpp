#include <vm/InstanceLayout.h>
#include <Hash.h>
#include <algorithm>

SFieldLayoutDetails::SFieldLayoutDetails(const char* name, ModifierBits modifiers, EBuiltInType fish_type, EMachineType machine_type, u8 dimension_count, EMachineType array_type, u32 offset) : // 21
FieldName(), Modifiers(modifiers.GetBits()), FishType(fish_type), MachineType(machine_type), DimensionCount(dimension_count), ArrayBaseMachineType(array_type), InstanceOffset(offset)
{
    // technically StringCopy<char, 53>
    // but whatever
    strcpy(FieldName, name);
}

CInstanceLayout::CInstanceLayout(u32 instance_size, u32 num_field_hint) : CBaseCounted(), CReflectionVisitable(), // 36
FieldLayoutDetailsVec(), FieldNameIndex(), InstanceSize(instance_size), ObjectHandleOffsets()
{
    FieldLayoutDetailsVec.reserve(num_field_hint);
    FieldNameIndex.reserve(num_field_hint);
}

CInstanceLayout::~CInstanceLayout() // 46
{

}

void CInstanceLayout::AddField(const char* field_name, ModifierBits modifiers, EBuiltInType fish_type, EMachineType machine_type, u8 dimension_count, EMachineType array_type, u32 offset, u32 field_name_hash) // 61
{
    SFieldLayoutDetails field(
        field_name,
        modifiers,
        fish_type,
        machine_type,
        dimension_count,
        array_type,
        offset
    );

    FieldLayoutDetailsVec.push_back(field);
    
    SOrderedFieldName entry;
    entry.FieldNameHash = field_name_hash;
    entry.LayoutIdx = FieldLayoutDetailsVec.size() - 1;

    FieldNameIndex.insert(
        std::lower_bound(FieldNameIndex.begin(), FieldNameIndex.end(), entry, SSortByFieldNameHash()),
        entry
    );

    if (field.MachineType == VMT_OBJECT_REF)
        ObjectHandleOffsets.push_back(field.InstanceOffset);
}

u32 CInstanceLayout::GetInstanceSize() const // 98
{
    return InstanceSize;
}

u32 CInstanceLayout::GetNumFields() const // 107
{
    return FieldLayoutDetailsVec.size();
}

const SFieldLayoutDetails* CInstanceLayout::GetFieldByIndex(u32 idx) const // 116
{
    return &FieldLayoutDetailsVec[idx];
}

const SFieldLayoutDetails* CInstanceLayout::LookupFieldByHash(u32 field_name_hash) const // 125
{
    const SOrderedFieldName* it = std::lower_bound(FieldNameIndex.begin(), FieldNameIndex.end(), field_name_hash, SSortByFieldNameHash());
    if (it != FieldNameIndex.end() && it->FieldNameHash == field_name_hash)
        return &FieldLayoutDetailsVec[it->LayoutIdx];
    return NULL;
}

const SFieldLayoutDetails* CInstanceLayout::LookupField(const char* name) const // 141
{
    return LookupFieldByHash(MakeFieldNameHash(name));
}

u32 CInstanceLayout::MakeFieldNameHash(const char* field_name) // 152
{
    return JenkinsHash((u8*)field_name, StringLength(field_name), 0);
}

void CInstanceLayout::UpdateInternals()
{
    ObjectHandleOffsets.resize(0);
    FieldNameIndex.resize(0);
    FieldNameIndex.reserve(FieldLayoutDetailsVec.size());
    for (u32 i = 0; i < FieldLayoutDetailsVec.size(); ++i)
    {
        SFieldLayoutDetails& field = FieldLayoutDetailsVec[i];
        if (field.MachineType == VMT_OBJECT_REF)
            ObjectHandleOffsets.push_back(field.InstanceOffset);
        
        SOrderedFieldName entry;
        entry.FieldNameHash = MakeFieldNameHash(field.FieldName);
        entry.LayoutIdx = i;

        FieldNameIndex.insert(
            std::lower_bound(FieldNameIndex.begin(), FieldNameIndex.end(), entry, SSortByFieldNameHash()),
            entry
        );
    }
}

bool CInstanceLayout::NeedsGCScan() const // 187
{
    return ObjectHandleOffsets.size() != 0;
}

bool operator==(const CVector<SFieldLayoutDetails>& lhs, const CVector<SFieldLayoutDetails>& rhs) // 211
{
    if (lhs.size() != rhs.size()) return false;
    for (u32 i = 0; i < lhs.size(); ++i)
    {
        if (lhs[i] != rhs[i])
            return false;
    }

    return true;
}

bool operator!=(const CVector<SFieldLayoutDetails>& lhs, const CVector<SFieldLayoutDetails>& rhs) // 227
{
    return !(lhs == rhs);
}

bool CInstanceLayout::IsEquivalent(const CInstanceLayout& other) const // 252
{
    if (this == &other) return true;
    if (InstanceSize != other.InstanceSize) return false;
    return !(FieldLayoutDetailsVec != other.FieldLayoutDetailsVec);
}