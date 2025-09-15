#include <ResourceGuidSubst.h>
#include <algorithm>

struct CGuidSubstComparer
{
    inline bool operator()(const CGuidSubst& lhs, const CGuidSubst& rhs)
    {
        return lhs.From < rhs.From;
    }

    inline bool operator()(const CGuidSubst& lhs, const CGUID& rhs)
    {
        return lhs.From < rhs;
    }

    inline bool operator()(const CGUID& lhs, const CGuidSubst& rhs)
    {
        return lhs < rhs.From;
    }
};

RGuidSubst::RGuidSubst(EResourceFlag flags) : CResource(flags), Substitutions()
{

}

ReflectReturn RGuidSubst::LoadFinished(const SRevision& revision)
{
    std::sort(Substitutions.begin(), Substitutions.end(), CGuidSubstComparer());
    return REFLECT_OK;
}

bool RGuidSubst::Get(CGUID from, CGUID& to) const
{
    CGuidSubst* subst = std::lower_bound(Substitutions.begin(), Substitutions.end(), from, CGuidSubstComparer());
    if (subst != Substitutions.end() && subst->From == from)
    {
        to = subst->To;
        return true;
    }

    return false;
}

void RGuidSubst::Reset()
{
    Substitutions.clear();
}

void RGuidSubst::Unload()
{
    CResource::Unload();
    Reset();
}

namespace NGuidSubst
{
#ifdef RESOURCE_SYSTEM_REIMPLEMENTATION
    StaticCP<RGuidSubst> gLanguageSubst;
    StaticCP<RGuidSubst> gRegionSubst;
    StaticCP<RGuidSubst> gButtonSubst;
#endif

    bool DoGUIDSubstitution(CGUID from, CGUID& to)
    {
        to = from;
        RGuidSubst* substitutions[] =
        {
            gRegionSubst,
            gButtonSubst,
            gLanguageSubst
        };

        for (int i = 0; i < ARRAY_LENGTH(substitutions); ++i)
        {
            RGuidSubst* subst = substitutions[i];
            if (subst == NULL || !subst->IsLoaded()) continue;

            if (subst->Get(from, to))
                return true;
        }

        return false;
    }
}