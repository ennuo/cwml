#pragma once

#include <Variable.h>
#include <Serialise.h>
#include <ResourceSystem.h>

const char GUID_HASH_INLINE_BITFIELD_CONTAINS_HASH = 1;
const char GUID_HASH_INLINE_BITFIELD_CONTAINS_GUID = 2;

#define FORWARD_DECLARE_SERIALISE_TYPE(type) \
    extern template ReflectReturn Reflect<CReflectionLoadVector>(CReflectionLoadVector& r, type& d); \
    extern template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, type& d);

#define INSTANTIATE_SERIALISE_TYPE(d) \
    template ReflectReturn Reflect<CReflectionLoadVector>(CReflectionLoadVector& r, d& d); \
    template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, d& d);

#define SERIALISE_TYPE(type) template <typename R> ReflectReturn Reflect(R& r, type& d) { ReflectReturn rv = REFLECT_OK;
#define SERIALISE_END return rv; }
#define ADD(revision, name) if (r.GetRevision() >= revision && (rv = Add(r, d.name, #name)) != REFLECT_OK) return rv;
#define ADD_CONDITIONAL(revision, condition, name) if (condition) ADD(revision, name);
#define REM(added, removed, type, field, default_value) \
    type field = (default_value); \
    if (r.GetRevision() >= (revision) && r.GetRevision() < (removed)) \
    { \
        if ((rv = Add(r, field, #field)) != REFLECT_OK) \
            return rv; \
    }

template <typename R>
ReflectReturn ReflectDescriptor(R& r, CResourceDescriptorBase& d, bool cp, bool type)
{
    ReflectReturn rv;
    char guid_hash_inline_bitfield = 0;
    if (r.GetSaving())
    {
        if (d.HasGUID()) guid_hash_inline_bitfield = GUID_HASH_INLINE_BITFIELD_CONTAINS_GUID;
        else if (d.GetHash()) guid_hash_inline_bitfield = GUID_HASH_INLINE_BITFIELD_CONTAINS_HASH;
    }

    if ((rv = Add(r, guid_hash_inline_bitfield, "guid_hash_inline_bitfield")) != REFLECT_OK)
        return rv;
    
    if (r.GetLoading() && guid_hash_inline_bitfield && r.GetRevision() < 0x191 && cp)
    {
        guid_hash_inline_bitfield = 
            ((guid_hash_inline_bitfield & ~3)) |
            ((guid_hash_inline_bitfield & 2) >> 1) |
            ((guid_hash_inline_bitfield & 1) << 1);

        // guid_hash_inline_bitfield = 
        //     ((guid_hash_inline_bitfield & ~0b11)) |
        //     ((guid_hash_inline_bitfield & 0b10) >> 1) |
        //     ((guid_hash_inline_bitfield & 0b01) << 1);
        
        if (guid_hash_inline_bitfield & 4)
            return REFLECT_FORMAT_TOO_OLD;
    }

    if (guid_hash_inline_bitfield & GUID_HASH_INLINE_BITFIELD_CONTAINS_GUID)
    {
        if ((rv = Add(r, d.GUID, "GUID")) != REFLECT_OK)
            return rv;
    }

    if (guid_hash_inline_bitfield & GUID_HASH_INLINE_BITFIELD_CONTAINS_HASH)
    {
        if ((rv = Add(r, d.Hash, "Hash")) != REFLECT_OK)
            return rv;
    }

#ifndef LBP2 // figure this out later
    if (type)
    {
        if ((rv = Add(r, d.Type, "Type")) != REFLECT_OK)
            return rv;
    }
#endif

    if (r.GetLoading())
        d.UpdateValidity();

    // r.AddDependency(d, d.GetType(), d.GetHash(), d.GetGUID());

    return REFLECT_OK;
}

template <typename R, typename D>
ReflectReturn Reflect(R& r, CP<D>& d);

ReflectReturn SerialiseResource(CResource& d, const CSerialiseControlParams& params, CHash* ret_hash);