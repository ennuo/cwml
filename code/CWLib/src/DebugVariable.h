#pragma once

#ifndef LBP1
    #include <MMString.h>
    #include <vector.h>
#endif

class CThing;

enum EDebugVariable
{
    DEBUG_INT,
    DEBUG_FLOAT = 2
};

class CIniSettings {};

class CDebugVariable {
public:
    CDebugVariable(const char* inifile_name, const char* display_name);
public:
    virtual ~CDebugVariable();
protected:
#ifndef LBP1
    virtual int GetType() const;
#endif
    virtual void AddToDebugRegistry(const char*);
    virtual void RefreshFromInifile(CIniSettings&, const char*);
    virtual void OnInitComplete();
private:
    CDebugVariable* Next;
    const char* InifileName;
    const char* DisplayName;
};

class CDebugFloat : public CDebugVariable {
public:
    CDebugFloat(const char*, const char*, float, float, float, float);
public:
    inline float* operator&() { return &Value; }
    inline operator float() const { return Value; }
    CDebugFloat& operator=(float);
private:
    float Value;
    float MinValue;
    float MaxValue;
    float Step;
};

class CDebugInt : public CDebugVariable {
public:
    CDebugInt(const char*, const char*, int);
    CDebugInt(const char*, const char*, int, int, int, int);
public:
    inline int* operator&() { return &Value; }
    inline operator int() const { return Value; }
    CDebugInt& operator=(int value);
private:
    int Value;
    int MinValue;
    int MaxValue;
    int Step;
};

class CDebugBool : public CDebugVariable {
public:
    typedef void (*DebugFn)(void);
public:
    CDebugBool(const char*, const char*, bool, DebugFn);
public:
    inline bool* operator&() { return &Value; }
    inline operator bool() const { return Value; }
    CDebugBool& operator=(bool);
private:
    DebugFn Function;
    bool Value;
    bool Invoke;
};

class CDebugFunction : public CDebugVariable {
public:
    typedef void (*DebugFn)(void);
    typedef void (*DebugThingFn)(CThing*);
public:
    CDebugFunction(DebugFn, const char*, const char*);
    CDebugFunction(DebugThingFn, const char*, const char*);
private:
    DebugFn Function;
    DebugThingFn FunctionThing;
    bool Invoke;
};

#ifndef LBP1

struct CDebugItem {
    CDebugVariable* Variable;
    bool Hidden;
};

struct CDebugFolder {
    u32 Index;
    u32 _0;
    MMString<char> Name;
    MMString<char> _1;
    CRawVector<CDebugFolder*> Folders;
    CRawVector<CDebugItem*> Variables;
    CDebugFolder* Parent;
    void* _2;
};

extern CDebugFolder* gDebugFolders[100];

#endif // LBP1

extern CDebugVariable* gDebugVariableHead;