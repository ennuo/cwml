#pragma once

#include <filepath.h>
#include <map>
#include <MMString.h>

struct SCompareIgnoreCase 
{
    inline bool operator()(const MMString<char>& lhs, const MMString<char>& rhs) const
    {
        return StringICompare(lhs.c_str(), rhs.c_str()) < 0;
    }
};

typedef std::map<MMString<char>, MMString<char>, SCompareIgnoreCase> SettingsMap;
typedef std::map<MMString<char>, SettingsMap, SCompareIgnoreCase> CategorySettingsMap; 

class CIniSettings {
public:
    CIniSettings();
public:
    bool ReadIniFile(const CFilePath& fpath);
    bool GetBool(const char* v, bool d);
    const char* GetString(const char* v, const char* d);
private:
    SettingsMap Settings;
    CategorySettingsMap CategorisedSettings;
    bool Iniitalised;
};