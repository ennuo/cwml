#pragma once

#include <StringUtil.h>
#include <MMString.h>
#include <Hash.h>

const u32 MAX_FIELD_NAME_LENGTH = 52;

enum EMachineType
{
    VMT_VOID,
    VMT_BOOL,
    VMT_CHAR,
    VMT_S32,
    VMT_F32,
    VMT_VECTOR4,
    VMT_M44,
    VMT_DEPRECATED,
    VMT_RAW_PTR,
    VMT_REF_PTR,
    VMT_SAFE_PTR,
    VMT_OBJECT_REF,
#ifndef LBP1
    VMT_S64,
    VMT_F64,
#endif
    NUM_MACHINE_TYPES
};

enum EBuiltInType
{
    BT_VOID,
    BT_BOOL,
    BT_CHAR,
    BT_S32,
    BT_F32,
    BT_V2,
    BT_V3,
    BT_V4,
    BT_M44,
    BT_GUID,
#ifndef LBP1
    BT_S64,
    BT_F64
#endif
};

enum EModifierType
{
    MT_STATIC,
    MT_NATIVE,
    MT_EPHEMERAL,
    MT_PINNED,
    MT_CONST,
    MT_PUBLIC,
    MT_PROTECTED,
    MT_PRIVATE,
    MT_PROPERTY,
    MT_ABSTRACT,
    MT_VIRTUAL,
    MT_OVERRIDE,
    MT_DIVERGENT,
#ifndef LBP1
    MT_EXPORT
#endif
};

class ModifierBits {
public:
    inline ModifierBits() : Bits() {}
    inline ModifierBits(u32 bits) : Bits(bits) {}
    inline ModifierBits(EModifierType type) : Bits(1 << type) {}
public:
    inline bool operator==(const ModifierBits& rhs) const { return Bits == rhs.Bits; }
    inline bool operator!=(const ModifierBits& rhs) const { return Bits != rhs.Bits; }
public:
    inline static u32 MakeBits(EModifierType type) { return 1 << type; }
    inline static u32 IsSet(u32 bits, EModifierType type) { return bits & (1 << type); }
public:
    inline void Set(EModifierType type) { Bits |= (1 << type); }
    inline void Clear(EModifierType type) { Bits &= ~(1 << type); }
    inline bool IsSet(EModifierType type) const { return Bits & (1 << type); }
    inline u32 GetBits() const { return Bits; }
    u32 CountAccessModifiers() const; // count leading zeroes
    bool ValidAccessModifiers() const;
    u32 CountVirtualModifiers() const;
    bool ValidVirtualModifiers() const;
    MMString<char> GetDescription(const char*) const;
private:
    u32 Bits;
};

struct ScriptObjectUID {
    inline ScriptObjectUID() : UID() {}
    inline ScriptObjectUID(u32 uid) { UID = uid; }
    inline ScriptObjectUID(const ScriptObjectUID& uid) { UID = uid.UID; }
    inline ScriptObjectUID& operator=(const ScriptObjectUID& rhs)
    {
        UID = rhs.UID;
        return *this;
    }
    inline bool operator==(const ScriptObjectUID& rhs) const { return UID == rhs.UID; }
    inline bool operator!=(const ScriptObjectUID& rhs) const { return UID != rhs.UID; }

    u32 UID;
};

class CSignature {
public:
    inline CSignature() : MangledName(), MangledNameHash() {}
public:
    inline bool IsEmpty() const { return MangledNameHash == 0; }
    inline u32 GetMangledNameHash() const { return MangledNameHash; }
public:
    CSignature(const char* mangled) : MangledName(), MangledNameHash()
    {
        SetMangledName(mangled);
    }
public:
    inline const char* GetMangledName() const
    {
        return MangledName.c_str();
    }

    void SetMangledName(const char* mangled)
    {
        MangledName = mangled;
        UpdateHash();
    }

    bool ExtractFunctionName(MMString<char>&);
    static bool ExtractFunctionName(MMString<char>&, const char*);
    bool Unmangle(MMString<char>&) const;
    bool Unmangle(MMString<char>&, const char*);
    
    inline int Compare(const CSignature& rhs) const
    {
        return rhs.MangledNameHash - MangledNameHash;
    }
    
    inline bool operator==(const CSignature& rhs) const
    {
        return MangledNameHash == rhs.MangledNameHash;
    }

    inline bool operator<(const CSignature& rhs) const
    {
        return MangledNameHash < rhs.MangledNameHash;
    }
public:
    inline static u32 MakeHash(const char* mangled)
    {
        return JenkinsHash((uint8_t*)mangled, StringLength(mangled), 0);
    }

    inline void UpdateHash()
    {
        MangledNameHash = MakeHash(MangledName.c_str());
    }
public:
    inline void MakeName(const char* name)
    {
        MangledName = name;
        MangledName += "__";
        UpdateHash();
    }

    void MakeName(const char* name, const char* a1)
    {
        MangledName = name;
        MangledName += "__";
        MangledName += a1;
        UpdateHash();
    }
    
    void MakeName(const char* name, const char* a1, const char* a2)
    {
        MangledName = name;
        MangledName += "__";
        MangledName += a1;
        MangledName += a2;
        UpdateHash();
    }

    void MakeName(const char*, const char*, const char*, const char*);
    void MakeName(const char*, const char*, const char*, const char*, const char*);
    void MakeName(const char*, const char*, const char*, const char*, const char*, const char*);
    void MakeName(const char*, const char*, const char*, const char*, const char*, const char*, const char*);
    void MakeName(const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*);
    void MakeName(const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*);
    void MakeName(const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*);
    void MakeName(const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*);
    void MakeName(const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*);
    void MakeName(const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*);
public:
    static bool ExtractAndUnmangle(MMString<char>&, const char*, bool);
private:
    MMString<char> MangledName;
    u32 MangledNameHash;
};

namespace NVirtualMachine
{
    u32 GetTypeSize(EMachineType machine_type);

    template <typename T> struct Arg; 
    
    #define MAKE_ARG_TEMPLATE(type, mangled) template <> struct Arg<type> { const char* Name; const char* GetName() const { return Name; } Arg() { Name = mangled; } };
    
    MAKE_ARG_TEMPLATE(s32, "i");
    MAKE_ARG_TEMPLATE(u32, "i");
    MAKE_ARG_TEMPLATE(float, "f");

    #undef MAKE_ARG_TEMPLATE
}