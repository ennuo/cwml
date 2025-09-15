#pragma once

#include <map>

#include <ResourceDescriptor.h>
#include <Resource.h>
#include <SerialiseEnums.h>
#include <ReflectionVisitable.h>
#include <MMString.h>
#include <vector.h>
#include <Serialise.h>
#include <GuidHashMap.h>

class PWorld;
class CThing;
class CResource;

enum EGatherType {
    GATHER_TYPE_GATHER,
    GATHER_TYPE_SAVE,
    GATHER_TYPE_LOAD
};

enum EVariableType {
    VARIABLE_TYPE_NUL,
    VARIABLE_TYPE_U8,
    VARIABLE_TYPE_U16,
    VARIABLE_TYPE_U32,
    VARIABLE_TYPE_U64,
    VARIABLE_TYPE_FLOAT,
    VARIABLE_TYPE_BOOL,
    VARIABLE_TYPE_FLOAT_IN_V2,
    VARIABLE_TYPE_V2,
    VARIABLE_TYPE_V4,
    VARIABLE_TYPE_M44,
    VARIABLE_TYPE_STRING,
    VARIABLE_TYPE_WSTRING,
    VARIABLE_TYPE_STRUCT,
    VARIABLE_TYPE_THING,
    VARIABLE_TYPE_THINGPTR,
    VARIABLE_TYPE_RESOURCEPTR,
    VARIABLE_TYPE_PTR,
    VARIABLE_TYPE_ARRAY,
    VARIABLE_TYPE_FCURVE,
    VARIABLE_TYPE_PLAN,
    VARIABLE_TYPE_PLANDESC,
    VARIABLE_TYPE_RESOURCE,
    VARIABLE_TYPE_DESCRIPTOR,
    VARIABLE_TYPE_DESCRIPTOR_TYPED_LAST=VARIABLE_TYPE_DESCRIPTOR + RTYPE_LAST,
    VARIABLE_TYPE_EGGLINK,
    VARIABLE_TYPE_KEYLINK,
    VARIABLE_TYPE_ARRAY_EGGLINK,
    VARIABLE_TYPE_ARRAY_KEYLINK
};

class CGatherVariables {
public:
    typedef ReflectReturn (*ReflectFunctionPtr)(CGatherVariables&, void*);
public:
    inline CGatherVariables() :
    Data(NULL), Type(VARIABLE_TYPE_NUL), Purpose(GATHER_TYPE_GATHER), ResourceType(RTYPE_INVALID),
    ReflectFunction(), Visited(), Children(), World(NULL), TempString(), LazyCPPriority(STREAM_PRIORITY_DEFAULT),
    Name(NULL), DynamicName(false)
    {
    }

    inline CGatherVariables(const CGatherVariables& rhs) :
    Data(NULL), Type(VARIABLE_TYPE_NUL), Purpose(GATHER_TYPE_GATHER), ResourceType(RTYPE_INVALID),
    ReflectFunction(), Visited(), Children(), World(NULL), TempString(), LazyCPPriority(STREAM_PRIORITY_DEFAULT),
    Name(NULL), DynamicName(false)
    {
        *this = rhs;
    }

    CGatherVariables& operator=(const CGatherVariables& rhs)
    {
        if (this == &rhs) return *this;
        
        Data = rhs.Data;
        Type = rhs.Type;
        Purpose = rhs.Purpose;
        ResourceType = rhs.ResourceType;
        ReflectFunction = rhs.ReflectFunction;
        Visited = rhs.Visited;
        Children = rhs.Children;
        LazyCPPriority = rhs.LazyCPPriority;

        if (rhs.DynamicName) CopyName(rhs.Name);
        else SetName(rhs.Name);

        return *this;
    }

    inline ~CGatherVariables()
    {
        ClearName();
    }
public:
    inline void Set(char const* name, void* data, EVariableType type, ReflectFunctionPtr fn, CGatherVariables& parent)
    {
        World = parent.World;
        LazyCPPriority = STREAM_PRIORITY_DEFAULT;
        Data = data;
        Type = type;
        Purpose = parent.Purpose;
        ReflectFunction = fn;
        Visited = parent.Visited;
        Name = name;
        DynamicName = false;
        ResourceType = RTYPE_INVALID;
    }

    inline void Set(const char* name, void* data, EVariableType type, ReflectFunctionPtr fn, CGatherVariables& parent, EResourceType resource_type, bool dynamic_name)
    {
        World = parent.World;
        LazyCPPriority = STREAM_PRIORITY_DEFAULT;
        Data = data;
        Type = type;
        Purpose = parent.Purpose;
        ReflectFunction = fn;
        Visited = parent.Visited;
        Name = name;
        DynamicName = dynamic_name;
        ResourceType = resource_type;
    }

    ReflectReturn Expand();
    void Collapse();
    CGatherVariables* GetChild(const char*);
    void TakeReflectionCS();
    bool GetLimitThingRecursion();
    bool GetThingPtrAsUID();
    u32 MakeUID();
    bool CanVisitThing(CThing*);
    void* GetVisited(void*);
    void SetVisited(void*, void*);

    inline CStreamPriority GetLazyCPPriority() const
    {
        return LazyCPPriority;
    }

    inline CStreamPriority* GetLazyCPPriorityPtr()
    {
        return &LazyCPPriority;
    }
    
    inline void SetLazyCPPriority(CStreamPriority prio)
    {
        LazyCPPriority = prio;
    }

    bool GetReflectFast();
    inline bool GetSaving() { return Purpose == GATHER_TYPE_SAVE; }
    inline bool GetLoading() { return Purpose == GATHER_TYPE_LOAD; }
    inline u32 GetRevision() { return gHeadRevision.Revision; }

    u32 GetBranchDescription();
    u16 GetBranchID();
    u16 GetBranchRevision();
    
    inline u8 GetCompressionFlags() { return 0; }
    inline void SetCompressionFlags(u8) {}
    inline bool IsGatherVariables() { return true; }
    inline ReflectReturn ReadWrite(void*, int) { return REFLECT_OK; }

    bool RequestToAllocate(u64) { return true; }
    bool AllowNullEntries();
    void RegisterResource(CResource*);
    
    bool GetString(char* str)
    {
        *str = '\0';
        switch (Type)
        {
            case VARIABLE_TYPE_NUL: return true;
            case VARIABLE_TYPE_U8: sprintf(str, "%i", *(u8*)Data); return true;
            case VARIABLE_TYPE_U16: sprintf(str, "%i", *(u16*)Data); return true;
            case VARIABLE_TYPE_U32: sprintf(str, "%i", *(u32*)Data); return true;
            case VARIABLE_TYPE_U64: sprintf(str, "%llu", *(u64*)Data); return true;
            case VARIABLE_TYPE_FLOAT: sprintf(str, "%f", *(float*)Data); return true;
            case VARIABLE_TYPE_BOOL: strcpy(str, *(bool*)Data ? "true" : "false"); return true;
            case VARIABLE_TYPE_STRING:
            {
                MMString<char>& s = *(MMString<char>*)Data;
                *str = '"';
                strcpy(str + 1, s.c_str());
                *(str + 1 + s.size()) = '"';
                *(str + 2 + s.size()) = '\0';
                return true;
            }
            case VARIABLE_TYPE_ARRAY: sprintf(str, "%i", ((CBaseVector<char>*)Data)->size()); return true;
            case VARIABLE_TYPE_RESOURCEPTR:
            {
                CP<CResource>& res = *(CP<CResource>*)Data;
                if (!res) return false;
                if (res->GetGUID())
                {
                    const CFileDBRow* row = FileDB::FindByGUID(res->GetGUID());
                    strcpy(str, row != NULL ? row->GetPath() : "");
                }
                else if (res->GetLoadedHash())
                {
                    strcpy(str, StringifyHash(res->GetLoadedHash()).c_str());
                }
                else strcpy(str, "?");

                return true;
            }
            default: break;
        }

        if (Type >= VARIABLE_TYPE_DESCRIPTOR)
            return false;

        return false;
    }
    
    inline void SetString(char* str)
    {
        switch (Type)
        {
            case VARIABLE_TYPE_U8: *(u8*)Data = strtol(str, NULL, 10); break;
            case VARIABLE_TYPE_U16: *(u16*)Data = strtol(str, NULL, 10); break;
            case VARIABLE_TYPE_U32: *(u32*)Data = strtol(str, NULL, 10); break;
            case VARIABLE_TYPE_U64: *(u64*)Data = strtoull(str, NULL, 10); break;
            case VARIABLE_TYPE_FLOAT: *(float*)Data = strtod(str, NULL); break;
            case VARIABLE_TYPE_BOOL: *(bool*)Data = strcmp(str, "false") ? true : false; break;
            case VARIABLE_TYPE_STRING:
            {
                // technically supposed to unescape the string, but womp
                *(MMString<char>*)Data = str;
                break;
            }
            case VARIABLE_TYPE_ARRAY:
            {
                int size;
                sscanf(str, "%i", &size);
                ((CBaseVector<char>*)Data)->GetSizeForSerialisation() = size;
                break;
            }
            default: break;
        }
    }

    void AddDependency(CDependencyWalkable*, int, const CHash&, const CGUID&);
    bool ToggleDependencies(bool);
    inline bool GetCompressInts() const { return false; }
    inline bool GetCompressVectors() const { return false; }
    inline bool GetCompressMatrices() const { return false; }
    
    inline const char* GetName() const
    {
        return Name;
    }
    
    inline bool HasName() const
    {
        return Name != NULL;
    }

    bool IsName(const char*) const;
    
    void SetName(const char* name)
    {
        ClearName();
        if (name != NULL)
        {
            Name = name;
            DynamicName = false;
        }
    }

    void CopyName(const char* name)
    {
        ClearName();
        if (name != NULL)
        {
            char* dyn = new char[strlen(name + 1)];
            strcpy(dyn, name);

            Name = dyn;
            DynamicName = true;
        }
    }
    
    inline void ClearName()
    {
        if (DynamicName)
        {
            if (Name != NULL)
                delete Name;
            Name = NULL;
        }

        DynamicName = false;
        Name = NULL;
    }

    inline CGatherVariables& AddChild()
    {
        Children.resize(Children.size() + 1);
        return Children.back();
    }
public:
    void* Data;
    EVariableType Type;
    EGatherType Purpose;
    EResourceType ResourceType;
    ReflectFunctionPtr ReflectFunction;
    std::map<void*, void*>* Visited;
    CVector<CGatherVariables> Children;
    PWorld* World;
    MMString<char> TempString;
private:
    CStreamPriority LazyCPPriority;
public:
    const char* Name;
    bool DynamicName;
};

template<typename R, typename D>
ReflectReturn ReflectWrapper(R& r, D& d) // 147
{
    return Reflect(r, d);
}

template <typename T, bool some_bool>
class GetReflectFunction {
public:
    static ReflectReturn (*Get())(CGatherVariables&, T&)
    {
        return ReflectWrapper<CGatherVariables, T>;
    }
};

template <typename T> inline EVariableType GetVariableType() { return VARIABLE_TYPE_STRUCT; }
template <> inline EVariableType GetVariableType<MMString<char> >() { return VARIABLE_TYPE_STRING; }
template <> inline EVariableType GetVariableType<MMString<wchar_t> >() { return VARIABLE_TYPE_WSTRING; }
template <> inline EVariableType GetVariableType<MMString<tchar_t> >() { return VARIABLE_TYPE_WSTRING; }
template <> inline EVariableType GetVariableType<s8>() { return VARIABLE_TYPE_U8; }
template <> inline EVariableType GetVariableType<u8>() { return VARIABLE_TYPE_U8; }
template <> inline EVariableType GetVariableType<s16>() { return VARIABLE_TYPE_U16; }
template <> inline EVariableType GetVariableType<u16>() { return VARIABLE_TYPE_U16; }
template <> inline EVariableType GetVariableType<s32>() { return VARIABLE_TYPE_U32; }
template <> inline EVariableType GetVariableType<u32>() { return VARIABLE_TYPE_U32; }
template <> inline EVariableType GetVariableType<f32>() { return VARIABLE_TYPE_FLOAT; }
template <> inline EVariableType GetVariableType<bool>() { return VARIABLE_TYPE_BOOL; }
template <> inline EVariableType GetVariableType<v4>() { return VARIABLE_TYPE_V4; }

template <class T>
struct is_vector { static const bool value = false; };
template <class T>
struct is_vector<CVector<T> > { static const bool value = true; };
template <class T>
struct is_vector<CRawVector<T> > { static const bool value = true; };

template <class T>
struct is_pointer { static const bool value = false; };
template <class T>
struct is_pointer<T*> { static const bool value = true; };

template <class T>
struct is_resource_ptr 
{ 
    static const bool value = false; 
    static const EResourceType type = RTYPE_INVALID; 
};

#define RESOURCE_MACRO(resource_type, class_name, headers) \
    template <> \
    struct is_resource_ptr<CP<class_name> > \
    { \
        static const bool value = true; \
        static const EResourceType type = resource_type; \
    };
    #include <ResourceList.h>
#undef RESOURCE_MACRO

template <typename D>
ReflectReturn Add(CGatherVariables& r, D& d, const char* c) // 288
{
    EVariableType type = is_resource_ptr<D>::value ? VARIABLE_TYPE_RESOURCEPTR :
        is_vector<D>::value ? VARIABLE_TYPE_ARRAY : GetVariableType<D>();

    CGatherVariables::ReflectFunctionPtr ptr = NULL;
    if (type >= VARIABLE_TYPE_STRUCT)
        ptr = (CGatherVariables::ReflectFunctionPtr)GetReflectFunction<D, false>::Get();
    
    if (is_resource_ptr<D>::value)
    {
        r.AddChild().Set(c, (void*)&d, type, ptr, r, is_resource_ptr<D>::type, false);
    }
    else
    {
        r.AddChild().Set(c, (void*)&d, type, ptr, r);
    }
    
    return REFLECT_OK;
}

template<typename T>
void Init(CGatherVariables& variables, T* resource)
{
    variables.SetName("<Root>");
    variables.ReflectFunction = (CGatherVariables::ReflectFunctionPtr)GetReflectFunction<T, false>::Get();
    variables.Data = resource;
    variables.Type = VARIABLE_TYPE_STRUCT;
}

ReflectReturn GatherVariablesLoad(ByteArray& v, CGatherVariables& variables, bool ignore_head, char* header_4bytes);
ReflectReturn GatherVariablesSave(ByteArray& v, CGatherVariables& variables, bool ignore_head, const char* header_4bytes);