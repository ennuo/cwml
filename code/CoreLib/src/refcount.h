#pragma once


typedef void (*RemoveRefFunc)(void*);

/* refcount.h: 13 */
class CBaseCounted {
public:
	int AddRef();
	int Release();
	int GetRefCount() const { return RefCount; }
	CBaseCounted() : RefCount() {}
protected:
	virtual ~CBaseCounted() = 0;
private:
	CBaseCounted(const CBaseCounted&);
	CBaseCounted& operator=(const CBaseCounted&);
private:
	volatile int RefCount;
};

template <class T>
class _NoAddRefRelease : public T {
public:
	int AddRef() const { return this->GetRefCount(); }
	int Release() const { return this->GetRefCount(); }
	int AddWeakRef() const { return this->GetRefCount(); }
	int ReleaseWeakRef() const { return this->GetRefCount(); }
};

/* refcount.h: 55 */
template <typename T>
class CP {
public:
	T& operator*() const { return *Ref; }
	_NoAddRefRelease<T>* operator->() const { return (_NoAddRefRelease<T>*)Ref; }
	operator T*() const { return Ref; }
	T* GetRef() const { return Ref; }
public:
	CP() : Ref(NULL) {}

	CP(T* ptr) : Ref(NULL) { this->CopyFrom(ptr); }

	CP(const CP<T>& rhs) : Ref(NULL) { this->CopyFrom(rhs.Ref); }

	~CP() 
	{
		if (Ref == NULL) return;
		if ((Ref->Release() - 1) == 0)
			delete Ref;
	}
public:
	CP<T>& operator=(T* rhs)
	{
		CopyFrom(rhs);
		return *this;
	}

	CP<T>& operator=(CP<T> const& rhs) 
	{
		CopyFrom(rhs.Ref);
		return *this;
	}
public:
	bool operator!() const { return !Ref; }
	bool operator==(const T* rhs) const { return Ref == rhs; }
	bool operator!=(const T* rhs) const { return Ref != rhs; }
	bool operator==(const CP<T>& rhs) const { return Ref == rhs.Ref; }
	bool operator!=(const CP<T>& rhs) const { return Ref != rhs.Ref; }
public:
	void CopyFrom(T* ptr)
	{
		if (Ref == ptr) return;
		if (ptr != NULL) ptr->AddRef();
		if (Ref != NULL)
		{
			if ((Ref->Release() - 1) == 0)
				delete Ref;
		}

		Ref = ptr;
	}
protected:
    T* Ref;
};

/* refcount.h: 111 */
class StaticCPForm {
public:
    void* Ref;
    RemoveRefFunc RemoveRefPtr;
    StaticCPForm* NextPtr;
};

extern StaticCPForm* gStaticCPHead;

/* refcount.h: 121 */
template <typename T>
class StaticCP : public CP<T> {
public:
	StaticCP()
	{
		this->Ref = NULL;
		RemoveRefPtr = &RemoveRef;
		NextPtr = gStaticCPHead;
		gStaticCPHead = (StaticCPForm*)this;
	}

	StaticCP(T* ptr) : CP<T>(ptr) {}
	StaticCP(const StaticCP<T>& rhs) : CP<T>(rhs.Ref) {}
	
	static void RemoveRef(void* ptr)
	{
		(*(StaticCP<T>*)ptr) = (T*)NULL;
	}
public:
	StaticCP<T>& operator=(T* rhs)
	{
		this->CopyFrom(rhs);
		return *this;
	}

	StaticCP<T>& operator=(StaticCP<T> const& rhs) 
	{
		this->CopyFrom(rhs.Ref);
		return *this;
	}
public:
    RemoveRefFunc RemoveRefPtr;
    StaticCPForm* NextPtr;
};