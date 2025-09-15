#pragma once

#include <stdio.h>
#include <string.h>

#include <MMString.h>

#define GENERIC_COMPARISONS


namespace cwml
{
    class ConfigOptionBase;
    extern ConfigOptionBase* gConfigOptionHead;

    enum EConfigOption
    {
        eConfigOption_Integer,
        eConfigOption_Float,
        eConfigOption_Boolean,
        eConfigOption_String,
    };

    template <typename T>
    EConfigOption GetConfigOptionType();

    template <> inline EConfigOption GetConfigOptionType<s32>() { return eConfigOption_Integer; }
    template <> inline EConfigOption GetConfigOptionType<u32>() { return eConfigOption_Integer; }
    template <> inline EConfigOption GetConfigOptionType<float>() { return eConfigOption_Float; }
    template <> inline EConfigOption GetConfigOptionType<bool>() { return eConfigOption_Boolean; }

    class ConfigOptionBase {
    protected:
        inline ConfigOptionBase(EConfigOption type, const char* name, const char* ini) : Next(), Type(type), DisplayName(name), IniFileName(ini)
        {
            Next = gConfigOptionHead;
            gConfigOptionHead = this;
        }
    public:
        inline ConfigOptionBase* GetNext() const { return Next; }
        inline EConfigOption GetType() const { return Type; }
        inline const char* GetDisplayName() const { return DisplayName; }
        inline const char* GeIniFilename() const { return IniFileName; }
    public:
        virtual void GetString(MMString<char>& out) const {};
        virtual void SetString(const char* value) {};
    protected:
        ConfigOptionBase* Next;
        EConfigOption Type;
        const char* DisplayName;
        const char* IniFileName;
    };

    template <typename T>
    class ConfigOption : public ConfigOptionBase {
    public:
        static const unsigned int kMaxNameLength = 256;
    public:
        inline ConfigOption(const char* name, const char* ini, const T& default_value) :
        ConfigOptionBase(GetConfigOptionType<T>(), name, ini), Value(default_value)
        {

        }
    public:
        inline void GetString(MMString<char>& out) const
        {
            char s[256];
            switch (GetConfigOptionType<T>())
            {
                case eConfigOption_Integer:
                {
                    sprintf(s, "%i", Value);
                    break;
                }

                case eConfigOption_Float:
                {
                    sprintf(s, "%f", Value);
                    break;
                }

                case eConfigOption_Boolean:
                {
                    strcpy(s, Value ? "true" : "false");
                    break;
                }

                default:
                {
                    *s = '\0';
                    break;
                }
            }

            out = s;
        }

        inline void SetString(const char* value)
        {
            switch (GetConfigOptionType<T>())
            {
                case eConfigOption_Boolean:
                {
                    *(bool*)&Value = strcmp(value, "false") ? true : false;
                    break;
                }
            }
        }
    public:
        inline operator T() const { return Value; }    
        inline T* operator&() const { return Value; }
        inline T& operator=(const T& rhs) { Value = rhs; return *Value; }
    private:
        T Value;
    };

    template <>
    class ConfigOption<const char*> : public ConfigOptionBase {
    public:
        inline ConfigOption(const char* name, const char* ini, const char* default_value) :
        ConfigOptionBase(eConfigOption_String, name, ini), Value()
        {
            if (default_value != NULL)
                Value = default_value;
        }
    public:
        inline void GetString(MMString<char>& out) const { out = Value; }
        inline void SetString(const char* value) { Value = value; }
    public:
        inline bool IsSet() const { return Value.size() != 0; }
        inline u32 size() const { return Value.size(); }
        inline u32 aligned_size() const
        {
            int size = Value.size() + 1;
            if ((size % 4) != 0)
                size += (4 - (size % 4));
            return size;
        }
        inline const char* c_str() const { return Value.c_str(); }
    public:
        inline operator const MMString<char>*() const { return &Value; }
        inline operator const MMString<char>&() const { return Value; }
        inline operator const char*() const { return Value.c_str(); }
        inline const char** operator&() const;
        inline void operator=(const char* rhs) { Value = rhs; }
    private:
        MMString<char> Value;
    };
}

