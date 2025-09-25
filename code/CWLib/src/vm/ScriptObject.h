#pragma once

#include <map>

#include <refcount.h>
#include <ReflectionVisitable.h>
#include <MMString.h>
#include <vector.h>
#include <TextStream.h>

#include <vm/ScriptVariant.h>
#include <vm/ScriptArguments.h>
#include <vm/ScriptInstance.h>

class RScript;
class PWorld;
class CResource;
class MMOTextStreamA;

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

    class CScriptObjectInstance;
    class CScriptObjectStringA;
    class CScriptObjectStringW;
    class CScriptObject;
    class CScriptObjectResource;

    class ScriptObjectMeta {
    public:
        ScriptObjectMeta(u32 object_id, CScriptObject* object);
    public:
        u32 ObjectID;
        u16 Flags;
        u16 GarbageCollectPassID;
        CScriptObject* Object;
    };

    bool NeedsScan(EScriptObjectType object_type);
    bool NeedsScan(CScriptObject* object);

    class CScriptObjectManager {
    public:
        struct SCompareStringPtrs
        {
            inline bool operator()(const char* lhs, const char* rhs) const
            {
                return StringCompare(lhs, rhs) < 0;
            }

            inline bool operator()(const wchar_t* lhs, const wchar_t* rhs) const
            {
                return StringCompare(lhs, rhs);
            }
        };
    public:
        typedef CRawVector<ScriptObjectMeta> ObjectVec;
        typedef std::map<const char*, unsigned int, SCompareStringPtrs> StringAMap;
        typedef std::map<const wchar_t*, unsigned int, SCompareStringPtrs> StringWMap;
        typedef std::map<CResource*, unsigned int> ResourceMap;
    public:
        CScriptObjectManager();
        ~CScriptObjectManager();
    public:
        ScriptObjectUID RegisterStringA(const char*);
        ScriptObjectUID RegisterStringW(const wchar_t*);
        ScriptObjectUID RegisterStringT(const tchar_t*);
        ScriptObjectUID RegisterResource(CResource*);
        ScriptObjectUID RegisterAndCanonicalise(CScriptObject* object);
        ScriptObjectUID RegisterObjectForSerialisation(CScriptObject*);
        void AddRoot(CScriptObject*);
        void RemoveRoot(CScriptObject*);
        bool HasRoot(CScriptObject*);
        CScriptObject* LookupObject(u32) const;
        CScriptObject** LookupObjectForSerialisation(u32);
        bool CheckObjectGraph();
        inline CScriptObject* LookupObject(ScriptObjectUID object_uid) const { return LookupObject(object_uid.UID); }
        CScriptObjectInstance* LookupInstance(ScriptObjectUID) const;
        CScriptObjectStringA* LookupStringA(ScriptObjectUID) const;
        CScriptObjectStringW* LookupStringW(ScriptObjectUID) const;
        CScriptObjectStringW* LookupStringT(ScriptObjectUID) const;
        void CollectGarbage(bool);
        void UpgradeScripts(PWorld*);
        void TriggerOnScriptReloaded(PWorld*);
        void FixupDivergentScriptVariables(PWorld*);
        bool CheckRegisteredObjects() const;
        void RegisterObject(CScriptObject*);
        ScriptObjectUID Canonicalise(CScriptObject*);
        ScriptObjectMeta* LookupObjectMeta(u32);
        void UnregisterStringA(const CScriptObjectStringA*);
        void UnregisterStringW(const CScriptObjectStringW*);
        void UnregisterResource(const CScriptObjectResource*);
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

    class CScriptObject : public CReflectionVisitable { // 131
    public:
        inline CScriptObject() : CReflectionVisitable(), ObjectID(~0ul) {}
        virtual ~CScriptObject();
    public:
        virtual bool IsInstance() const { return false; }
        virtual EScriptObjectType GetType() const { return SO_NULL; }
        virtual void Stream(MMOTextStreamA& stream) const;
        inline u32 GetUID() const { return ObjectID; }
        virtual bool IsArray() const { return false; }
    public:
        inline void SetObjectID(u32 object_id) { ObjectID = object_id; }
    protected:
        u32 ObjectID;
    };

    class CScriptObjectStringA : public CScriptObject { // 539
    public:
        CScriptObjectStringA();
        CScriptObjectStringA(const char*);
        ~CScriptObjectStringA();
    public:
        EScriptObjectType GetType() const { return SO_STRINGA; }
        void Stream(MMOTextStreamA&) const;
    public:
        inline const char* GetString() const { return String.c_str(); }
        inline void SetCanonical(bool canonical) { Canonical = canonical; }
        inline bool IsCanonical() const { return Canonical; }
    private:
        MMString<char> String;
        bool Canonical;
    };

    class CScriptObjectStringW : public CScriptObject {
    public:
        inline EScriptObjectType GetType() const { return SO_STRINGW; }
        void Stream(MMOTextStreamA&) const;
    public:
        inline const wchar_t* GetString() { return String.c_str(); }
        inline void SetCanonical(bool canonical) { Canonical = canonical; }
        inline bool IsCanonical() { return Canonical; }
    private:
        MMString<wchar_t> String;
        bool Canonical;
    };

    class CScriptObjectInstance : public CScriptObject { // 381
    public:
        CScriptObjectInstance();
        CScriptObjectInstance(const CP<RScript>& script);
        ~CScriptObjectInstance();
    public:
        inline bool IsInstance() const { return true; }
        inline EScriptObjectType GetType() const { return SO_INSTANCE; }
        void Stream(MMOTextStreamA&) const;
    public:
        inline CScriptInstance& GetInstance() { return ScriptInstance; }
        inline const CP<RScript>& GetScript() const { return ScriptInstance.GetScript(); }
    public:
        void InvokeAsync(PWorld*, const CSignature&, const CScriptArguments&);
        bool InvokeSync(PWorld*, const CSignature&, const CScriptArguments&);
        bool InvokeSync(PWorld*, const CSignature&, const CScriptArguments&, CScriptVariant*);
        void OnDestroy(PWorld*);
    public:
        static CScriptObjectInstance* Create(const CP<RScript>&, PWorld*, bool);
        static CScriptObjectInstance* CreateForSerialisation();
    public:
        void InitialiseMemberData(PWorld*, bool);

    protected:
        CScriptInstance ScriptInstance;
    };

    extern CScriptObjectManager* gScriptObjectManager;

}