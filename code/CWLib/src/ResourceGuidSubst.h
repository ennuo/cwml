#pragma once

#include <Resource.h>
#include <vector.h>
#include <GuidHash.h>

class CGuidSubst {
public:
    inline CGuidSubst() : From(), To() {}
    inline CGuidSubst(CGUID from, CGUID to) : From(from), To(to) {}
public:
    CGUID From;
    CGUID To;
};

class RGuidSubst : public CResource {
public:
    RGuidSubst(EResourceFlag flags);
    ReflectReturn LoadFinished(const SRevision& revision);
    virtual void Unload();
public:
    bool Get(CGUID from, CGUID& to) const;
    void Set(CGUID from, CGUID to);
    void Reset();
public:
    typedef CRawVector<CGuidSubst> V;
    V Substitutions;
};

namespace NGuidSubst
{
    extern StaticCP<RGuidSubst> gLanguageSubst;
    extern StaticCP<RGuidSubst> gRegionSubst;
    extern StaticCP<RGuidSubst> gButtonSubst;
    
    bool DoGUIDSubstitution(CGUID from, CGUID& to);
}

