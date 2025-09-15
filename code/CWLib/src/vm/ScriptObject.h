#pragma once

#include <map>

#include <ReflectionVisitable.h>
#include <MMString.h>
#include <vector.h>

namespace NVirtualMachine
{
    enum EScriptObjectType 
    {
        SO_NULL,
        SO_ARRAY_BOOL,
        SO_ARRAY_CHAR,
        SO_ARRAY_S32,
        SO_ARRAY_F32,
        SO_ARRAY_VECTOR4,
        SO_ARRAY_M44,
        SO_ARRAY_STRING,
        SO_ARRAY_RAW_PTR,
        SO_ARRAY_REF_PTR,
        SO_ARRAY_SAFE_PTR,
        SO_ARRAY_OBJECT_REF,
        SO_RESOURCE,
        SO_INSTANCE,
        SO_STRINGW,
        SO_AUDIOHANDLE,
        SO_STRINGA,
        SO_POPPET,
        SO_EXPOSED_COLLECTBUBBLE,
    #ifndef LBP1
        SO_ARRAY_S64,
        SO_ARRAY_F64,
    #endif
        NUM_SCRIPT_OBJECT_TYPES
    };

    class CScriptObject;
    class MMOTextStreamA;
    class CResource;

    struct ScriptObjectMeta {
        u32 ObjectID;
        u16 Flags;
        u16 GarbageCollectPassID;
        CScriptObject* Object;
    };

    class CScriptObjectManager {
        typedef CRawVector<ScriptObjectMeta> ObjectVec;
        typedef std::map<const char*, unsigned int> StringAMap;
        typedef std::map<const wchar_t*, unsigned int> StringWMap;
        typedef std::map<CResource*, unsigned int> ResourceMap;
    public:
        ObjectVec ScriptObjects;
        u32 NextScriptObjectID;
        StringAMap StringAs;
        StringWMap StringWs;
        ResourceMap Resources;
        CRawVector<unsigned int> RootObjects;
        u16 GarbageCollectPassID;
        u32 LastGraphicsFrameNum;
        u32 GarbageCollectCountdown;
        CRawVector<CScriptObject*> DeadObjects;
    };

    class CScriptObject : public CReflectionVisitable {
    public:
        virtual ~CScriptObject() {};
        virtual bool IsInstance() { return false; }
        virtual EScriptObjectType GetType() { return SO_NULL; }
        virtual void Stream(MMOTextStreamA& stream) { return; }
        virtual bool IsArray() { return false; }
        
        inline u32 GetUID() { return ObjectID; }
        inline void SetObjectID(u32 object_id) { ObjectID = object_id; }
    protected:
        u32 ObjectID;
    };

    class CScriptObjectStringA : public CScriptObject {
    public:
        inline bool IsInstance() { return false; }
        inline EScriptObjectType GetType() { return SO_STRINGA; }
        inline void Stream() {};
        inline bool IsArray() { return false; }

        inline const char* GetString() { return String.c_str(); }
        inline void SetCanonical(bool canonical) { Canonical = canonical; }
        inline bool IsCanonical() { return Canonical; }
    private:
        MMString<char> String;
        bool Canonical;
    };

    class CScriptObjectStringW : public CScriptObject {
    public:
        inline bool IsInstance() { return false; }
        inline EScriptObjectType GetType() { return SO_STRINGW; }
        inline void Stream() {};
        inline bool IsArray() { return false; }

        inline const wchar_t* GetString() { return String.c_str(); }
        inline void SetCanonical(bool canonical) { Canonical = canonical; }
        inline bool IsCanonical() { return Canonical; }
    private:
        MMString<wchar_t> String;
        bool Canonical;
    };

    extern CScriptObjectManager* gScriptObjectManager;

}