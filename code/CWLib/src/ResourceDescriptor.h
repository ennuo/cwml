#pragma once

#include <GuidHash.h>
#include <ReflectionVisitable.h>
#include <ResourceList.h>
#include <SerialiseEnums.h>

enum EResourceType
{
    RTYPE_INVALID,
#define RESOURCE_MACRO(type, class_name, headers) type,
#define RESOURCE_MACRO_UNIMPLEMENTED RESOURCE_MACRO
    #include <ResourceList.h>
#undef RESOURCE_MACRO_UNIMPLEMENTED
#undef RESOURCE_MACRO
    RTYPE_LAST
};

class CResourceDescriptorBase : public CDependencyWalkable {
    template <typename R>
    friend ReflectReturn ReflectDescriptor(R& r, CResourceDescriptorBase& d, bool cp, bool type);
public:
    inline CResourceDescriptorBase()
    {
        memset(this, 0, sizeof(CResourceDescriptorBase));
    }

    inline CResourceDescriptorBase(EResourceType type, CGUID guid) :
    CDependencyWalkable(), Hash()
    {
        Type = type;
        GUID = guid;
        Valid = Type != RTYPE_INVALID;
    }

    inline CResourceDescriptorBase(EResourceType type, const CHash& hash) :
    CDependencyWalkable(), GUID()
    {
        Type = type;
        Hash = hash;
        Valid = hash.IsSet() || Type != RTYPE_INVALID;
    }

    inline CResourceDescriptorBase(EResourceType type, CGUID guid, const CHash& hash) :
    CDependencyWalkable(), GUID(), Hash(), Type(type)
    {
        if (!guid)
        {
            Valid = hash.IsSet() || Type != RTYPE_INVALID;
            Hash = hash;
        }
        else
        {
            GUID = guid;
            Valid = Type != RTYPE_INVALID;
        }
    }
    
    inline CResourceDescriptorBase(const CResourceDescriptorBase& rhs) : CDependencyWalkable()
    {
        GUID = rhs.GUID;
        Hash = rhs.Hash;
        Type = rhs.Type;
        Valid = rhs.Valid;
    }
public:
    inline bool operator==(CResourceDescriptorBase const& r) const
    {
        return Type == r.Type && GUID == r.GUID && Hash == r.Hash;
    }

    inline bool operator!=(CResourceDescriptorBase const& r) const
    {
        return Type != r.Type || GUID != r.GUID || Hash != r.Hash;
    }

    inline bool operator<(CResourceDescriptorBase const& r) const
    {
        if (Type != r.Type) return Type < r.Type;

        if (GUID == r.GUID)
            return Hash < r.Hash;

        return GUID < r.GUID;
    }

    inline void UpdateValidity()
    {
        if (!GUID)
            Valid = Hash.IsSet() && Type != RTYPE_INVALID;
        else
            Valid = Type != RTYPE_INVALID;

    }

    CHash LatestHash() const;
public:
    inline bool IsValid() const { return Valid; }
    inline bool HasGUID() const { return (bool)GUID; }
    inline const CGUID& GetGUID() const { return GUID; }
    inline const CHash& GetHash() const { return Hash; }
    inline EResourceType GetType() const { return (EResourceType)Type; }
protected:
    CGUID GUID;
    CHash Hash;
#ifdef LBP1
    u32 Type;
    bool Valid;
#else
    struct
    {
        u32 Valid : 1;
        u32 Type: 31;
    };
#endif
};

class CPlanDescriptor {
enum { E_NONE, E_HASH, E_GUID };
public:
    inline bool IsValid() const { return Valid; }
    inline bool HasGUID() const { return Type == E_GUID; }
    inline CGUID& GetGUID() { return GUID; }
    inline CHash& GetHash()
    {
        return Hash != NULL ? *Hash : CHash::Zero;
    }

    inline EResourceType GetType() const { return RTYPE_PLAN; }
private:
    CHash* Hash;
    CGUID GUID;
    struct
    {
        u32 Valid : 1;
        u32 Type: 31;
    };
};

template<class T> inline EResourceType GetResourceType();
#define RESOURCE_MACRO(type, class_name, headers) template <> inline EResourceType GetResourceType<class_name>() { return type; }
#define RESOURCE_MACRO_UNIMPLEMENTED RESOURCE_MACRO
    #include <ResourceList.h>
#undef RESOURCE_MACRO_UNIMPLEMENTED
#undef RESOURCE_MACRO

template <class T>
class CResourceDescriptor : public CResourceDescriptorBase {
public:
    inline CResourceDescriptor() : CResourceDescriptorBase()
    {
        Type = GetResourceType<T>();
    }

    inline CResourceDescriptor(CGUID guid) : CResourceDescriptorBase(GetResourceType<T>(), guid) {}
    inline CResourceDescriptor(const CHash& hash) : CResourceDescriptorBase(GetResourceType<T>(), hash) {}
};
