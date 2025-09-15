#pragma once

#include "StringUtil.h"

template <typename T>
class StringTraits {
public:
    static T* Malloc(size_t c)
    {
        return new T[c];
    }

    static void Copy(T* d, const T* s, size_t n)
    {
        memcpy(d, s, n * sizeof(T));
    }

    static void Move(T* d, const T* s, size_t n)
    {
        memmove(d, s, n * sizeof(T));
    }
};

template <typename T>
class MMString {
    static const u32 LOCAL_STORE_BYTES = 16;
    static const u32 LOCAL_STORE_CHARS = (LOCAL_STORE_BYTES - sizeof(T)) / sizeof(T);
public:
    typedef StringTraits<T> Traits;
    typedef size_t size_type;
    typedef T* iterator;
    typedef const T* const_iterator;
public:
    MMString()
    {
        LocalData.LocalStoreFlag = LOCAL_STORE_CHARS;
        LocalBuffer[0] = 0;
    }

    MMString(const T* b, const T* e)
    {
        Construct(b, (size_t)(e - b));
    }

    MMString(const T* s, size_t l)
    {
        Construct(s, l);
    }

    MMString(const T* s)
    {
        size_t l = StringLength(s);
        Construct(s, l);
    }

    MMString(const MMString<T>& s)
    {
        Construct(s.c_str(), s.size());
    }

    ~MMString()
    {
        if (!IsUsingLocalData())
            delete HeapData.Buffer;
    }

    inline bool CanUseLocalData(size_t l)
    {
        return l <= LOCAL_STORE_CHARS;
    }

    inline bool IsUsingLocalData() const
    {
        return LocalData.LocalStoreFlag != (T)-1;
    }

    inline T MakeLocalStoreFlag(size_t l)
    {
        return (l > LOCAL_STORE_CHARS) ? (T)-1 : LOCAL_STORE_CHARS - l;
    }

    void Terminate(size_t l)
    {
        LocalData.LocalStoreFlag = MakeLocalStoreFlag(l);
        if (IsUsingLocalData()) LocalBuffer[l] = 0;
        else
        {
            HeapData.Buffer[l] = 0;
            HeapData.Length = l;
        }
    }

    MMString<T>& append(const T* s) { return append(s, StringLength(s)); }
    MMString<T>& append(T c)
    {
        return append(&c, 1);
    }
    
    MMString<T>& append(const T* s, size_t s_l)
    {
        int old_length = length();
        int new_length = old_length + s_l;

        if (!IsUsingLocalData())
        {
            if (new_length <= HeapData.Capacity)
            {
                Traits::Copy(HeapData.Buffer + old_length, s, s_l);
                HeapData.Buffer[new_length] = '\0';
                HeapData.Length = new_length;

                return *this;
            }

            T* data = Traits::Malloc(new_length + 1);
            Traits::Copy(data, HeapData.Buffer, old_length);
            Traits::Copy(data + old_length, s, s_l);

            if (HeapData.Buffer != NULL)
                delete[] HeapData.Buffer;
            
            HeapData.Buffer = data;
            HeapData.Capacity = new_length;
            HeapData.Length = new_length;
            HeapData.Buffer[new_length] = '\0';

            return *this;
        }

        LocalData.LocalStoreFlag = MakeLocalStoreFlag(new_length);
        if (CanUseLocalData(new_length))
        {
            Traits::Copy(LocalBuffer + old_length, s, s_l);
            LocalBuffer[new_length] = 0;
        }
        else
        {
            T* data = Traits::Malloc(new_length + 1);
            Traits::Copy(data, LocalBuffer, old_length);
            Traits::Copy(data + old_length, s, s_l);
            data[new_length] = 0;

            HeapData.Buffer = data;
            HeapData.Capacity = new_length;
            HeapData.Length = new_length;
        }

        return *this;
    }

    void reserve(size_t l) { Grow(l); }

    void resize(size_t l, char c)
    {
        u32 len = length();

        if (l < len)
        {
            Terminate(l);
            return;
        }

        if (len < l && Grow(l))
        {
            T* buf = begin() + len;
            for (int i = 0; i < l - len; ++i) buf[i] = c;
            Terminate(l);
        }
    }

    bool Grow(size_t l)
    {
        int len = length();
        if (IsUsingLocalData())
        {
            if (CanUseLocalData(l)) return true;

            T* data = Traits::Malloc(l + 1);
            Traits::Copy(data, LocalBuffer, len);
            data[len] = '\0';

            HeapData.Buffer = data;
            HeapData.Capacity = l;
            HeapData.Length = len;
            LocalData.LocalStoreFlag = MakeLocalStoreFlag(l);

            return true;
        }

        if (HeapData.Capacity >= l) return true;
        
        T* data = Traits::Malloc(l + 1);
        Traits::Copy(data, HeapData.Buffer, len);
        delete[] HeapData.Buffer;

        HeapData.Buffer = data;
        HeapData.Capacity = l;

        return true;
    }

    void Construct(const T* s, size_t l)
    {
        T flag = MakeLocalStoreFlag(l);
        T* data = LocalBuffer;
        if (flag == (T)-1)
        {
            data = Traits::Malloc(l + 1);

            HeapData.Buffer = data;
            HeapData.Length = l;
            HeapData.Capacity = l; 
        }

        LocalData.LocalStoreFlag = flag;
        Traits::Move(data, s, l);
        data[l] = 0;
    }

    inline void clear()
    {
        if (!IsUsingLocalData())
            delete HeapData.Buffer;
        
        LocalData.LocalStoreFlag = LOCAL_STORE_CHARS;
        LocalBuffer[0] = 0;
    }

    inline bool empty() const
    {
        return size() == 0;
    }

    MMString<T>& assign(const MMString<T>& s)
    {
        if (this != s)
        {
            if (!IsUsingLocalData())
                delete HeapData.Buffer;
            Construct(s.c_str(), s.size());
        }

        return *this;
    }

    MMString<T>& assign(const T* s, size_t l)
    {
        // If the string already has data in it...
        if (!IsUsingLocalData())
        {
            // If there's still enough space in the heap data,
            // just copy the string in
            if (l <= HeapData.Capacity)
            {
                Traits::Move(HeapData.Buffer, s, l);
                HeapData.Buffer[l] = 0;
                HeapData.Length = l;
                return *this;
            }

            // Otherwise allocate a new buffer

            T* data = Traits::Malloc(l + 1);
            Traits::Move(data, s, l);
            data[l] = 0;

            if (HeapData.Buffer != NULL)
                delete HeapData.Buffer;

            HeapData.Buffer = data;
            HeapData.Capacity = l;
            HeapData.Length = l;
        }
        // String is empty or uses local data, need to construct
        else Construct(s, l);

        return *this;
    }
    
    MMString<T>& operator=(MMString<T> const& s)
    {
        if (&s == this) return *this;
        return assign(s.c_str(), s.size());
    }

    MMString<T>& operator=(T const* s)
    {
        return assign(s, StringLength(s));
    }

    MMString<T>& operator+=(T const* s)
    {
        return append(s);
    }

    MMString<T>& operator+=(T c)
    {
        return append(c);
    }

    bool operator==(const T* rhs) const
    {
        return compare(rhs) == 0;
    }

    bool operator!=(const T* rhs) const
    {
        return compare(rhs) != 0;
    }

    bool operator==(const MMString<T>& rhs) const
    {
        return compare(rhs.c_str()) == 0;
    }

    bool operator!=(const MMString<T>& rhs) const
    {
        return compare(rhs.c_str()) != 0;
    }
    
    friend MMString<T> operator+(const MMString<T>& lhs, const MMString<T>& rhs)
    {
        MMString<T> r = lhs;
        r += rhs;
        return r;
    }

    friend MMString<T> operator+(const MMString<T>& lhs, const T* rhs)
    {
        MMString<T> r = lhs;
        r += rhs;
        return r;
    }

    inline int compare(const T* s) const
    {
        return StringCompare(c_str(), s);
    }

    inline size_t size() const
    {
        if (IsUsingLocalData())
            return LOCAL_STORE_CHARS - LocalData.LocalStoreFlag;
        return HeapData.Length;
    }

    inline size_t length() const
    {
        if (IsUsingLocalData())
            return LOCAL_STORE_CHARS - LocalData.LocalStoreFlag;
        return HeapData.Length;
    }

    inline size_t capacity()
    {
        if (IsUsingLocalData())
            return LOCAL_STORE_CHARS;
        return HeapData.Capacity;
    }

    inline T* begin()
    {
        return IsUsingLocalData() ? (T*)&LocalBuffer : HeapData.Buffer;
    }

    inline T* c_str() const
    {
        return IsUsingLocalData() ? (T*)&LocalBuffer : HeapData.Buffer;
    }

private:
    union {
        T LocalBuffer[LOCAL_STORE_BYTES / sizeof(T)];
        struct {
            T _LocalBufferPad[LOCAL_STORE_CHARS];
            T LocalStoreFlag;
        } LocalData;
        struct {
            T* Buffer;
            size_t Length;
            size_t Capacity;
            u32 Dummy;
        } HeapData;
        u64 Bits[LOCAL_STORE_BYTES / sizeof(u64)];
    };
};

const tchar_t* MultiByteToTChar(MMString<tchar_t>& dst, const char* b, const char* e);