#include <vm/ScriptObject.h>
#include <vm/VirtualMachine.h>
#include <ResourceScript.h>
#include <PartPhysicsWorld.h>

namespace NVirtualMachine
{
#ifdef WIN32
    CScriptObjectManager* gScriptObjectManager;
#endif

    bool gScriptObjectTypeNeedsScan[NUM_SCRIPT_OBJECT_TYPES] =
    {
        false, // SO_NULL
        false, // SO_ARRAY_BOOL
        false, // SO_ARRAY_CHAR
        false, // SO_ARRAY_S32
        false, // SO_ARRAY_F32
        false, // SO_ARRAY_VECTOR4
        false, // SO_ARRAY_M44
        false, // SO_ARRAY_STRING
        false, // SO_ARRAY_RAW_PTR
        false, // SO_ARRAY_REF_PTR
        false, // SO_ARRAY_SAFE_PTR
        true, // SO_ARRAY_OBJECT_REF
        false, // SO_RESOURCE
        true, // SO_INSTANCE
        false, // SO_STRINGW
        false, // SO_AUDIOHANDLE
        false, // SO_STRINGA
        true, // SO_POPPET
        true, // SO_EXPOSED_COLLECTBUBBLE
#ifndef LBP1
        false, // SO_ARRAY_S64
        false // SO_ARRAY_F64
#endif
    };

    bool NeedsScan(EScriptObjectType object_type) // 80
    {
        return gScriptObjectTypeNeedsScan[object_type];
    }

    bool NeedsScan(CScriptObject* object) // 86
    {
        return NeedsScan(object->GetType());
    }

    ScriptObjectMeta::ScriptObjectMeta(u32 object_id, CScriptObject* object) :
    ObjectID(object_id), Flags(0), GarbageCollectPassID(-1), Object(object)
    {
        if (NeedsScan(object))
            Flags |= 1;
    }


    CScriptObjectManager::CScriptObjectManager() : // 110
    ScriptObjects(), NextScriptObjectID(1), StringAs(), StringWs(),
    Resources(), RootObjects(), GarbageCollectPassID(), LastGraphicsFrameNum(),
    GarbageCollectCountdown(1), DeadObjects()
    {

    }

    CScriptObjectManager::~CScriptObjectManager() // 121
    {
        for (u32 i = 0; i < DeadObjects.size(); ++i)
            delete DeadObjects[i];
        
        for (u32 i = 0; i < ScriptObjects.size(); ++i)
            delete ScriptObjects[i].Object;
    }

    CScriptObjectInstance::CScriptObjectInstance() :
    CScriptObject(), ScriptInstance()
    {

    }

    ScriptObjectUID CScriptObjectManager::RegisterStringA(const char* s) // 288
    {
        if (s == NULL) return 0;
    
        StringAMap::iterator it = StringAs.find(s);
        if (it != StringAs.end())
            return it->second;

        CScriptObjectStringA* object = new CScriptObjectStringA(s);
        RegisterObject(object);
        object->SetCanonical(true);

        StringAs.insert(StringAMap::value_type(object->GetString(), object->GetUID()));
        return object->GetUID();
    }


    void CScriptObjectManager::RegisterObject(CScriptObject* object) // 495
    {
        object->SetObjectID(NextScriptObjectID++);
        ScriptObjects.push_back(ScriptObjectMeta(object->GetUID(), object));
    }

    ScriptObjectUID CScriptObjectManager::RegisterAndCanonicalise(CScriptObject* object) // 517
    {
        if (object == NULL) return 0;
        RegisterObject(object);
        return Canonicalise(object);
    }

    ScriptObjectUID CScriptObjectManager::Canonicalise(CScriptObject* object) // 547
    {
        if (object == NULL) return 0;

        // TODO: implement!
        switch (object->GetType())
        {
            case SO_STRINGA: break;
            case SO_STRINGW: break;
            case SO_RESOURCE: break;
        }

        return object->GetUID();
    }

    struct SCompareScriptUID // 670
    {
        inline bool operator()(const ScriptObjectMeta& lhs, u32 rhs) const
        {
            return lhs.ObjectID < rhs;
        }

        inline bool operator()(u32 lhs, const ScriptObjectMeta& rhs) const
        {
            return lhs < rhs.ObjectID;
        }

        inline bool operator()(const ScriptObjectMeta& lhs, const ScriptObjectMeta& rhs) const
        {
            return lhs.ObjectID < rhs.ObjectID;
        }
    };

    CScriptObject* CScriptObjectManager::LookupObject(u32 object_id) const // 698
    {
        if (object_id == 0) return NULL;
        ScriptObjectMeta* it = std::lower_bound(ScriptObjects.begin(), ScriptObjects.end(), object_id, SCompareScriptUID());
        if (it != ScriptObjects.end() && it->ObjectID == object_id)
            return it->Object;
        return NULL;
    }

    CScriptObjectInstance* CScriptObjectManager::LookupInstance(ScriptObjectUID object_uid) const // 724
    {
        CScriptObject* object = LookupObject(object_uid);
        if (object != NULL && object->IsInstance())
            return (CScriptObjectInstance*)object;
        return NULL;
    }

    CScriptObjectStringA* CScriptObjectManager::LookupStringA(ScriptObjectUID object_uid) const
    {
        CScriptObject* object = LookupObject(object_uid);
        if (object != NULL && object->GetType() == SO_STRINGA)
            return (CScriptObjectStringA*)object;
        return NULL;
    }

    CScriptObject::~CScriptObject() // 1196
    {

    }

    void CScriptObject::Stream(MMOTextStreamA& stream) const // 1204
    {
        stream << '@' << ObjectID;
    }

    CScriptObjectInstance::CScriptObjectInstance(const CP<RScript>& script) : // 1280
    CScriptObject(), ScriptInstance(script)
    {
        
    }

    CScriptObjectInstance::~CScriptObjectInstance() // 1288
    {

    }

    CScriptObjectInstance* CScriptObjectInstance::Create(const CP<RScript>& script, PWorld* pworld, bool default_construct)
    {
        CScriptObjectInstance* instance = new CScriptObjectInstance(script);
        GetScriptObjectManager()->RegisterAndCanonicalise(instance);
        instance->InitialiseMemberData(pworld, default_construct);
        return instance;
    }

    void CScriptObjectInstance::InitialiseMemberData(PWorld* pworld, bool default_construct) // 1336
    {
        ScriptInstance.InitialiseMemberData(this, pworld, default_construct);
    }

    bool CScriptObjectInstance::InvokeSync(PWorld* pworld, const CSignature& signature, const CScriptArguments& arguments) // 1360
    {
        return InvokeSync(pworld, signature, arguments, NULL);
    }

    bool CScriptObjectInstance::InvokeSync(PWorld* pworld, const CSignature& signature, const CScriptArguments& arguments, CScriptVariant* return_value) // 1368
    {
        CScriptFunctionBinding binding;
        const CP<RScript>& script = GetScript();
        if (script == NULL || !script->LookupFunction(signature, &binding)) return false;
        return binding.InvokeSync(pworld, this, arguments, return_value);
    }

    void CScriptObjectInstance::Stream(MMOTextStreamA& stream) const // 1385
    {
        CScriptObject::Stream(stream);
        stream << '(';
        const CP<RScript>& script = GetScript();
        if (script)
            stream << script->GetClassName();
        else
            stream << '?';
        stream << ')';
    }

    CScriptObjectStringA::CScriptObjectStringA() : CScriptObject(), String(), Canonical() // 1398
    {

    }

    CScriptObjectStringA::CScriptObjectStringA(const char* s) : CScriptObject(), String(s), Canonical() // 1407
    {

    }

    CScriptObjectStringA::~CScriptObjectStringA() // 1417
    {

    }

    void CScriptObjectStringA::Stream(MMOTextStreamA& stream) const // 1425
    {
        stream << String.c_str();
    }

    void CScriptObjectStringW::Stream(MMOTextStreamA& stream) const // 1460
    {
        stream << String.c_str();
    }
}