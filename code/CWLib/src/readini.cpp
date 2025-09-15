#include <ReadINI.h>
#include <vector.h>

const char* copyupto(char* dst, const char* src, bool allowspace)
{
    char c;
    while ((c = *src) != '\0' && IsWhiteSpace(c)) src++;

    bool quotes = false;
    while ((c = *src) != '\0')
    {
        if (c == '[' || c == ']' || c == '\n' || c == '\r') break;
        if (!allowspace && !quotes && IsWhiteSpace(c)) break;
        if (c == '#' || c == ';' || c == '=') break;

        src += 1;
        if (c == '"' && !allowspace)
        {
            quotes = !quotes;
            continue;
        }
        
        *dst++ = c;
    }

    *dst = '\0';

    while (*src != '\0' && IsWhiteSpace(*src)) src++;
    return src;
}

bool CIniSettings::GetBool(const char* v, bool d)
{
    typename SettingsMap::iterator it = Settings.find(v);
    if (it != Settings.end())
        return strcmp(it->second.c_str(), "false") ? true : false;
    return d;
}

const char* CIniSettings::GetString(const char* v, const char* d)
{
    typename SettingsMap::iterator it = Settings.find(v);
    if (it != Settings.end())
        return it->second.c_str();
    return d;
}

bool CIniSettings::ReadIniFile(const CFilePath& fpath)
{
    CVector<MMString<char> > lines;
    if (!FileLoad(fpath, lines, StripAndIgnoreFileHash))
    {
        Iniitalised = true;
        return false;
    }

    char key[256] = {0};
    char val[256] = {0};
    char valspaces[256] = {0};
    char path[256] = {0};

    CVector<MMString<char> >::iterator itr = lines.begin();
    CVector<MMString<char> >::iterator end = lines.end();
    for (; itr != end; ++itr)
    {
        const char* s = itr->c_str();

        if (*s == '[') s = copyupto(path, s + 1, false);

        s = copyupto(key, s, false);

        *valspaces = '\0';
        if (*s == '=')
            s = copyupto(valspaces, s + 1, true);
        
        copyupto(val, valspaces, false);

#ifdef PS3
        if (StringICompare(path, "pc") == 0) continue; 
#endif

        if (*key != '\0')
            Settings[key] = val;
    }

    Iniitalised = true;
    return true;
}


CIniSettings::CIniSettings() : Settings(), CategorisedSettings(), Iniitalised()
{
}

